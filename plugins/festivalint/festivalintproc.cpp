/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalintproc.cpp
  Main speaking functions for the Festival (Interactive) Plug in
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$

#include <qstring.h>
#include <qstringlist.h>
#include <qthread.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "festivalintproc.h"
#include "festivalintproc.moc"
 
/** Constructor */
FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList& ) : 
    PlugInProc( parent, name ){
    // kdDebug() << "FestivalIntProc::FestivalIntProc: Running" << endl;
    m_ready = true;
    m_writingStdin = false;
    m_festProc = 0;
    m_state = psIdle;
}

/** Destructor */
FestivalIntProc::~FestivalIntProc(){
    // kdDebug() << "FestivalIntProc::~FestivalIntProc: Running" << endl;
    if (m_festProc)
    {
        if (m_festProc->isRunning())
        {
            if (m_ready)
            {
                m_state = psIdle;
                // kdDebug() << "FestivalIntProc::~FestivalIntProc: telling Festival to quit." << endl;
                m_ready = false;
                m_waitingStop = true;
                m_festProc->writeStdin(QString("(quit)"), true);
            }
            else
            {
                // kdDebug() << "FestivalIntProc::~FestivalIntProc: killing Festival." << endl;
                m_waitingStop = true;
                m_festProc->kill();
            }
        }
        delete m_festProc;
    }
}

/** Initialize the speech */
bool FestivalIntProc::init(const QString &lang, KConfig *config){
    // kdDebug() << "FestivalIntProc::init: Initializing plug in: Festival" << endl;

    // To save resources, this function should get a KConfig too
    // This KConfig will be passed to this function (already opened) from speaker.cpp
    // KConfig *config = new KConfig("kttsdrc");
    // KConfig *config = KGlobal::config();
    config->setGroup(QString("Lang_")+lang);

    // Get the code for the selected voice
    KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festivalint/voices", true, false);

    voices.setGroup(config->readEntry("Voice"));
    m_voiceCode = "("+voices.readEntry("Code")+")";
    // kdDebug() << "---- The code for the selected voice " << config->readEntry("Voice") << " is " << voiceCode << endl;
    
    return true;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void FestivalIntProc::sayText(const QString &text)
{
    synth(text, QString::null, m_voiceCode);
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
void FestivalIntProc::synthText(const QString& text, const QString& suggestedFilename)
{
    synth(text, suggestedFilename, m_voiceCode);
};

/**
* Say or Synthesize text.
* @param text                    The text to be synthesized.
* @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
*                                synthesize and audibilize the text.
* @param voiceCode               Voice code in which to speak text.
*/
void FestivalIntProc::synth(
    const QString &text,
    const QString &synthFilename,
    const QString& voiceCode)
{
    // kdDebug() << "FestivalIntProc::synth: Running" << endl;

    // Initialize Festival only if it's not initialized
    if (m_festProc)
        // If festProc exists but is not running, it is because it was stopped.
        // Recreate festProc object.
        if (!m_festProc->isRunning())
        {
            delete m_festProc;
            m_festProc = 0;
        }
    if(!m_festProc)
    {
        // kdDebug()<< "FestivalIntProc::synth: Creating Festival object" << endl;
        m_festProc = new KProcess;
        *m_festProc << "festival";
        *m_festProc << "--interactive";
        connect(m_festProc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*)));
        connect(m_festProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
        connect(m_festProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
        connect(m_festProc, SIGNAL(wroteStdin(KProcess*)),
            this, SLOT(slotWroteStdin(KProcess*)));
    }
    m_outputQueue.clear();
    if (!m_festProc->isRunning())
    {
        // kdDebug() << "FestivalIntProc::synth: Starting Festival process" << endl;
        m_runningVoiceCode = QString::null;
        m_ready = false;
        if (m_festProc->start(KProcess::NotifyOnExit, KProcess::All))
        {
            // kdDebug()<< "FestivalIntProc:synth: Festival initialized" << endl;
        }
        else
        {
            kdDebug() << "FestivalIntProc::synth: Error starting Festival process.  Is festival in the PATH?" << endl;
            m_ready = true;
            m_state = psIdle;
            return;
        }
    }
    // If we just started Festival, or voiceCode has changed, send code to Festival.
    if (m_runningVoiceCode != voiceCode) sendToFestival(voiceCode);

    // Encode quotation characters.
    QString saidText = text;
    saidText.replace("\\\"", "#!#!");
    saidText.replace("\"", "\\\"");
    saidText.replace("#!#!", "\\\"");
    // Remove certain comment characters.
    saidText.replace("--", "");

    // Ok, let's rock.
    // If no longer running, it has been stopped.  Bail out.
    if (synthFilename.isNull())
    {
        m_state = psSaying;
        m_synthFilename = QString::null;
        // kdDebug() << "FestivalIntProc::synth: Saying text: '" << saidText << "' using Festival plug in with voice "
        //    << voiceCode << endl;
        saidText = "(SayText \"" + saidText + "\")";
        sendToFestival(saidText);
    } else {
        m_state = psSynthing;
        m_synthFilename = synthFilename;
        // kdDebug() << "FestivalIntProc::synth: Synthing text: '" << saidText << "' using Festival plug in with voice "
        //    << voiceCode << endl;
        saidText = "(set! utt1 (Utterance Text \"" + 
            saidText + 
            "\"))(utt.synth utt1)(utt.save.wave utt1 \"" + synthFilename + "\")";
        sendToFestival(saidText);
    }
}

/**
* If ready for more output, sends the given text to Festival process, otherwise,
* puts it in the queue.
* @param text                    Text to send or queue.
*/
void FestivalIntProc::sendToFestival(const QString& text)
{
    if (text.isNull()) return;
    m_outputQueue.append(text);
    sendIfReady();
}

/**
* If Festival is ready for more input and there is more output to send, send it.
* To be ready for more input, the Stdin buffer must be empty and the "festival>"
* prompt must have been received (m_ready = true).
* @return                        False when Festival is ready for more input
*                                but there is nothing to be sent, or if Festival
*                                has exited.
*/
bool FestivalIntProc::sendIfReady()
{
    if (!m_ready) return true;
    if (m_writingStdin) return true;
    if (m_outputQueue.isEmpty()) return false;
    if (!m_festProc->isRunning()) return false;
    QString text = m_outputQueue[0];
    text += "\n";
    m_outputQueue.pop_front();
    m_ready = false;
    // kdDebug() << "FestivalIntProc::sendIfReady: sending to Festival: " << text << endl;
    m_writingStdin = true;
    m_festProc->writeStdin(text.latin1(), text.length());
    return true;
}
        
/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString FestivalIntProc::getFilename() { return m_synthFilename; }

/**
 * Stop text
 */
void FestivalIntProc::stopText(){
    // kdDebug() << "FestivalIntProc::stopText: Running" << endl;
    if (m_festProc)
    {
        if (m_festProc->isRunning())
        {
            if (m_ready)
                m_state = psIdle;
            else
            {
                // kdDebug() << "FestivalIntProc::stopText: killing Festival." << endl;
                m_waitingStop = true;
                m_festProc->kill();
            }
        } else m_state = psIdle;
    } else m_state = psIdle;
}

void FestivalIntProc::slotProcessExited(KProcess*)
{
    // kdDebug() << "FestivalIntProc:slotProcessExited: Festival process has exited." << endl;
    m_ready = true;
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
    delete m_festProc;
    m_festProc = 0;
    m_outputQueue.clear();
}

void FestivalIntProc::slotReceivedStdout(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    // kdDebug() << "FestivalIntProc::slotReceivedStdout: Received from Festival: " << buf << endl;
    if (buf.contains("festival>") > 0) 
    {
        m_ready = true;
        if (!sendIfReady())
        {
            pluginState prevState = m_state;
            m_state = psFinished;
            if (prevState == psSaying)
                emit sayFinished();
            else
                if (prevState == psSynthing)
                    emit synthFinished();
        }
    }
}

void FestivalIntProc::slotReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "FestivalIntProc::slotReceivedStderr: Received error from Festival: " << buf << endl;
}

void FestivalIntProc::slotWroteStdin(KProcess* /*proc*/)
{
    // kdDebug() << "FestivalIntProc::slotWroteStdin" << endl;
    m_writingStdin = false;
    if (!sendIfReady())
    {
        pluginState prevState = m_state;
        m_state = psFinished;
        if (prevState == psSaying)
            emit sayFinished();
        else
            if (prevState == psSynthing)
                emit synthFinished();
    }
}


bool FestivalIntProc::isReady() { return m_ready; }

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @see pluginState
*/
pluginState FestivalIntProc::getState() { return m_state; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void FestivalIntProc::ackFinished()
{
    if (m_state == psFinished)
    {
        m_state = psIdle;
        m_synthFilename = QString::null;
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
bool FestivalIntProc::supportsAsync() { return true; }
        
/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool FestivalIntProc::supportsSynth() { return true; }
    
