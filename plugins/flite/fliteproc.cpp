/***************************************************** vim:set ts=4 sw=4 sts=4:
  Main speaking functions for the Festival Lite (Flite) Plug in
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

// Qt includes.
#include <QString>
#include <QStringList>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <k3process.h>

// Flite Plugin includes.
#include "fliteproc.h"
#include "fliteproc.moc"

/** Constructor */
FliteProc::FliteProc( QObject* parent, const QStringList& ) :
    PlugInProc( parent, "fliteproc" ){
    kDebug() << "FliteProc::FliteProc: Running" << endl;
    m_state = psIdle;
    m_waitingStop = false;
    m_fliteProc = 0;
}

/** Destructor */
FliteProc::~FliteProc(){
    kDebug() << "FliteProc::~FliteProc:: Running" << endl;
    if (m_fliteProc)
    {
        stopText();
        delete m_fliteProc;
    }
}

/** Initialize the speech */
bool FliteProc::init(KConfig* c, const QString& configGroup){
    // kDebug() << "Running: FliteProc::init(const QString &lang)" << endl;
    // kDebug() << "Initializing plug in: Flite" << endl;
    // Retrieve path to flite executable.
    KConfigGroup config(c, configGroup);
    m_fliteExePath = config.readEntry("FliteExePath", "flite");
    kDebug() << "FliteProc::init: path to flite: " << m_fliteExePath << endl;
    return true;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void FliteProc::sayText(const QString &text)
{
    synth(text, QString(), m_fliteExePath);
}

/**
* Synthesize text into an audio file, but do not send to the audio device.
* @param text                    The text to be synthesized.
* @param suggestedFilename       Full pathname of file to create.  The plugin
*                                may ignore this parameter and choose its own
*                                filename.  KTTSD will query the generated
*                                filename using getFilename().
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void FliteProc::synthText(const QString& text, const QString& suggestedFilename)
{
    synth(text, suggestedFilename, m_fliteExePath);
}

/**
* Say or Synthesize text.
* @param text                    The text to be synthesized.
* @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
*                                synthesize and audibilize the text.
*/
void FliteProc::synth(
    const QString &text,
    const QString &synthFilename,
    const QString& fliteExePath)
{
    // kDebug() << "Running: FliteProc::synth(const QString &text)" << endl;

    if (m_fliteProc)
    {
        if (m_fliteProc->isRunning()) m_fliteProc->kill();
        delete m_fliteProc;
        m_fliteProc = 0;
    }
    // kDebug()<< "FliteProc::synth: Creating Flite object" << endl;
    m_fliteProc = new K3Process;
    connect(m_fliteProc, SIGNAL(processExited(K3Process*)),
        this, SLOT(slotProcessExited(K3Process*)));
    connect(m_fliteProc, SIGNAL(receivedStdout(K3Process*, char*, int)),
        this, SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(m_fliteProc, SIGNAL(receivedStderr(K3Process*, char*, int)),
        this, SLOT(slotReceivedStderr(K3Process*, char*, int)));
    connect(m_fliteProc, SIGNAL(wroteStdin(K3Process*)),
        this, SLOT(slotWroteStdin(K3Process* )));
    if (synthFilename.isNull())
        m_state = psSaying;
    else
        m_state = psSynthing;

    // Encode quotation characters.
    QString saidText = text;
/*
    saidText.replace("\\\"", "#!#!");
    saidText.replace("\"", "\\\"");
    saidText.replace("#!#!", "\\\"");
    // Remove certain comment characters.
    saidText.replace("--", "");
    saidText = "\"" + saidText + "\"";
*/
    saidText += '\n';

    *m_fliteProc << fliteExePath;
//    *m_fliteProc << "-t" << saidText;
    if (!synthFilename.isNull()) *m_fliteProc << "-o" << synthFilename;

    // Ok, let's rock.
    m_synthFilename = synthFilename;
    kDebug() << "FliteProc::synth: Synthing text: '" << saidText << "' using Flite plug in" << endl;
    if (!m_fliteProc->start(K3Process::NotifyOnExit, K3Process::All))
    {
        kDebug() << "FliteProc::synth: Error starting Flite process.  Is flite in the PATH?" << endl;
        m_state = psIdle;
        return;
    }
    kDebug()<< "FliteProc:synth: Flite initialized" << endl;
    m_fliteProc->writeStdin(saidText.toLatin1(), saidText.length());
}

/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString FliteProc::getFilename()
{
    kDebug() << "FliteProc::getFilename: returning " << m_synthFilename << endl;
    return m_synthFilename;
}

/**
* Stop current operation (saying or synthesizing text).
* Important: This function may be called from a thread different from the
* one that called sayText or synthText.
* If the plugin cannot stop an in-progress @ref sayText or
* @ref synthText operation, it must not block waiting for it to complete.
* Instead, return immediately.
*
* If a plugin returns before the operation has actually been stopped,
* the plugin must emit the @ref stopped signal when the operation has
* actually stopped.
*
* The plugin should change to the psIdle state after stopping the
* operation.
*/
void FliteProc::stopText(){
    kDebug() << "FliteProc::stopText:: Running" << endl;
    if (m_fliteProc)
    {
        if (m_fliteProc->isRunning())
        {
            kDebug() << "FliteProc::stopText: killing Flite." << endl;
            m_waitingStop = true;
            m_fliteProc->kill();
        } else m_state = psIdle;
    }else m_state = psIdle;
    kDebug() << "FliteProc::stopText: Flite stopped." << endl;
}

void FliteProc::slotProcessExited(K3Process*)
{
    kDebug() << "FliteProc:slotProcessExited: Flite process has exited." << endl;
    pluginState prevState = m_state;
    if (m_waitingStop)
    {
        m_waitingStop = false;
        m_state = psIdle;
        emit stopped();
    } else {
        m_state = psFinished;
        if (prevState == psSaying)
            emit sayFinished();
        else
            if (prevState == psSynthing)
                emit synthFinished();
    }
}

void FliteProc::slotReceivedStdout(K3Process*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kDebug() << "FliteProc::slotReceivedStdout: Received output from Flite: " << buf << endl;
}

void FliteProc::slotReceivedStderr(K3Process*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kDebug() << "FliteProc::slotReceivedStderr: Received error from Flite: " << buf << endl;
}

void FliteProc::slotWroteStdin(K3Process*)
{
    kDebug() << "FliteProc::slotWroteStdin: closing Stdin" << endl;
    m_fliteProc->closeStdin();
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @see pluginState
*/
pluginState FliteProc::getState() { return m_state; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void FliteProc::ackFinished()
{
    if (m_state == psFinished)
    {
        m_state = psIdle;
        m_synthFilename.clear();
    }
}

/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*
* If the plugin returns True, it must also implement @ref getState .
* It must also emit @ref sayFinished or @ref synthFinished signals when
* saying or synthesis is completed.
*/
bool FliteProc::supportsAsync() { return true; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool FliteProc::supportsSynth() { return true; }

