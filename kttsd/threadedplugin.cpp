/***************************************************** vim:set ts=4 sw=4 sts=4:
  Converts a synchronous plugin into an asynchronous one.
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include <qevent.h>
#include <qapplication.h>

#include <kdebug.h>

#include "speaker.h"
#include "threadedplugin.h"

/**
* Constructor.
*/
ThreadedPlugIn::ThreadedPlugIn(PlugInProc* plugin, 
    QObject *parent /*= 0*/, const char *name /*= 0*/):
    PlugInProc(parent, name),
    QThread(),
    m_plugin(plugin),
    m_filename(QString::null),
    m_requestExit(false),
    m_supportsSynth(false)
{
    m_waitingStop = false;
    m_state = psIdle;
}

/**
* Destructor.
*/
ThreadedPlugIn::~ThreadedPlugIn()
{
    if (running())
    {
        // If thread is busy, try to stop it.
        m_requestExit = true;
        if (m_threadRunningMutex.locked()) m_plugin->stopText();
        m_action = paNone;
        m_waitCondition.wakeOne();
        wait(5000);
        // If thread still active, stopText didn't succeed.  Terminate the thread.
        if (running())
        {
            terminate();
            wait();
        }
    }
    delete m_plugin;
}

/**
* Initialize the speech plugin.
*/
bool ThreadedPlugIn::init(KConfig *config, const QString &configGroup)
{
    bool stat = m_plugin->init(config, configGroup);
    m_supportsSynth = m_plugin->supportsSynth();
    // Start thread running, which will immediately go to sleep.
    start();
    return stat;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately
* and emit sayFinished signal when synthesis and audibilizing is finished.
*/
void ThreadedPlugIn::sayText(const QString &text)
{
    kdDebug() << "ThreadedPlugin::sayText running with text " << text << endl;
    waitThreadNotBusy();
    m_action = paSayText;
    m_text = text;
    m_state = psSaying;
    m_waitCondition.wakeOne();
}

/**
* Synthesize text into an audio file, but do not send to the audio device.
* @param text                    The text to be synthesized.
* @param suggestedFilename       Full pathname of file to create.  The plugin
*                                may ignore this parameter and choose its own
*                                filename.  KTTSD will query the generated
*                                filename using getFilename().
*
* If the plugin supports asynchronous operation, it should return immediately
* and emit synthFinished signal when synthesis is completed.
*/
void ThreadedPlugIn::synthText(const QString &text, const QString &suggestedFilename)
{
    waitThreadNotBusy();
    m_action = paSynthText;
    m_text = text;
    m_filename = suggestedFilename;
    m_state = psSynthing;
    m_waitCondition.wakeOne();
}

/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString ThreadedPlugIn::getFilename()
{
    return m_filename;
}

/**
* Stop current operation (saying or synthesizing text).
* This function only makes sense in asynchronus modes.
* The plugin should return to the psIdle state.
*/
void ThreadedPlugIn::stopText()
{
    // If thread is busy, call stopText and wait for thread to stop being busy.
    if (m_threadRunningMutex.locked())
    {
        kdDebug() << "ThreadedPlugIn::stopText:: calling m_plugin->stopText" << endl;
        m_plugin->stopText();
        // Set flag that will force state to idle once the plugin finishes.
        m_waitingStop = true;
//        waitThreadNotBusy();
    } else m_state = psIdle;
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @ref pluginState
*/
pluginState ThreadedPlugIn::getState()
{
    m_stateMutex.unlock();
    bool emitStopped = false;
    // If stopText was called, plugin may not have truly stopped, in which
    // case, if has finally completed the operation, return idle state.
    if (m_waitingStop)
    {
        if (m_state == psFinished) m_state = psIdle;
        if (m_state == psIdle)
        {
            m_waitingStop = false;
            emitStopped = true;
        }
    }
    pluginState plugState = m_state;
    m_stateMutex.unlock();
    if (emitStopped) emit stopped();
    return plugState;
}

/**
* Acknowleges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void ThreadedPlugIn::ackFinished()
{
    // Since plugin should not be running, don't bother with Mutex here.
    if (m_state == psFinished) m_state = psIdle;
    m_filename = QString::null;
}

/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*
* Since this is a threaded wrapper, return True.
*/
bool ThreadedPlugIn::supportsAsync() { return true; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool ThreadedPlugIn::supportsSynth() { return m_supportsSynth; }

/**
* Waits for the thread to go to sleep.
*/
void ThreadedPlugIn::waitThreadNotBusy()
{
    m_threadRunningMutex.lock();
    m_threadRunningMutex.unlock();
}

/**
* Base function, where the thread will start.
*/
void ThreadedPlugIn::run()
{
    while (!m_requestExit)
    {
    
        if (!m_threadRunningMutex.locked()) m_threadRunningMutex.lock();
        // Go to sleep until asked to do something.
        // Mutex unlocks as we go to sleep and locks as we wake up.
        kdDebug() << "ThreadedPlugIn::run going to sleep." << endl;
        m_waitCondition.wait(&m_threadRunningMutex);
        kdDebug() << "ThreadedPlugIn::run waking up." << endl;
        // Woken up.
        // See if we've been told to exit.
        if (m_requestExit) 
        {
            m_threadRunningMutex.unlock();
            return;
        }
        
        // Branch on requested action.
        switch( m_action )
        {
            case paNone: break;
            
            case paSayText:
                {
                    m_stateMutex.lock();
                    m_state = psSaying;
                    m_stateMutex.unlock();
                    kdDebug() << "ThreadedPlugIn::run calling sayText" << endl;
                    m_plugin->sayText(m_text);
                    kdDebug() << "ThreadedPlugIn::run back from sayText" << endl;
                    m_stateMutex.lock();
                    if (m_state == psSaying) m_state = psFinished;
                    m_stateMutex.unlock();
                    emit sayFinished();
                    break;
                }
                
            case paSynthText:
                {
                    m_stateMutex.lock();
                    m_state = psSynthing;
                    m_stateMutex.unlock();
                    QString filename = m_filename;
                    m_filename = QString::null;
                    kdDebug() << "ThreadedPlugIn::run calling synthText" << endl;
                    m_plugin->synthText(m_text, filename);
                    kdDebug() << "ThreadedPlugIn::run back from synthText" << endl;
                    m_filename = m_plugin->getFilename();
                    m_stateMutex.lock();
                    if (m_state == psSynthing) m_state = psFinished;
                    m_stateMutex.unlock();
                    emit synthFinished();
                    break;
                }
        }
    }
    if (m_threadRunningMutex.locked()) m_threadRunningMutex.unlock();
}
