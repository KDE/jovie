/***************************************************** vim:set ts=4 sw=4 sts=4:
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

// C++ includes.
#include <math.h>

// Qt includes.
#include <qstring.h>
#include <qstringlist.h>
#include <qthread.h>
#include <qtextcodec.h>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

// FestivalInt includes.
#include "festivalintproc.h"
#include "festivalintproc.moc"

/** Constructor */
FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList& ) : 
    PlugInProc( parent, name ){
    // kdDebug() << "FestivalIntProc::FestivalIntProc: Running" << endl;
    m_ready = true;
    m_writingStdin = false;
    m_waitingQueryVoices = false;
    m_waitingStop = false;
    m_festProc = 0;
    m_state = psIdle;
    m_languageCode = "en";
    m_codec = QTextCodec::codecForName("ISO8859-1");
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
                m_festProc->writeStdin("(quit)", true);
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
    // TODO: Should m_codec be deleted?
    // delete m_codec;
}

/** Initialize the speech */
bool FestivalIntProc::init(KConfig *config, const QString &configGroup)
{
    // kdDebug() << "FestivalIntProc::init: Initializing plug in: Festival" << endl;

    config->setGroup(configGroup);
    m_voiceCode = config->readEntry("Voice");
    m_festivalExePath = config->readEntry("FestivalExecutablePath", "festival");
    // kdDebug() << "---- The code for the selected voice " << config->readEntry("Voice") << " is " << voiceCode << endl;
    m_time = config->readNumEntry("time", 100);
    m_pitch = config->readNumEntry("pitch", 100);
    m_volume = config->readNumEntry("volume", 100);
    // If voice should be pre-loaded, start Festival and load the voice.
    m_preload = config->readBoolEntry("Preload", false);
    m_languageCode = config->readEntry("LanguageCode", "en");
    QString codecName = config->readEntry("Codec", "Latin1");
    if (codecName == "Local")
        m_codec = QTextCodec::codecForLocale();
    else if (codecName == "Latin1")
        m_codec = QTextCodec::codecForName("ISO8859-1");
    else if (codecName == "Unicode")
        m_codec = QTextCodec::codecForName("utf16");
    else
        m_codec = QTextCodec::codecForName(codecName.latin1());
    if (!m_codec)
    {
        kdDebug() << "FestivalIntProc::init: Invalid codec name " << codecName << endl;
        kdDebug() << "FestivalIntProc::init: Defaulting to ISO 8859-1" << endl;
        m_codec = QTextCodec::codecForName("ISO8859-1");
    }
    if (m_preload) startEngine(m_festivalExePath, m_voiceCode, m_languageCode, m_codec);
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
    synth(m_festivalExePath, text, QString::null, m_voiceCode, m_time, m_pitch, m_volume,
        m_languageCode, m_codec);
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
    synth(m_festivalExePath, text, suggestedFilename, m_voiceCode, m_time, m_pitch, m_volume,
        m_languageCode, m_codec);
};

/**
* Sends command to Festival to query for a list of supported voice codes.
* Fires queryVoicesFinished when completed.
* @return                       False if busy doing something else and therefore cannot
*                               do the query.
*/
bool FestivalIntProc::queryVoices(const QString &festivalExePath)
{
    // kdDebug() << "FestivalIntProc::queryVoices: Running" << endl;
    if (m_state != psIdle && m_waitingQueryVoices && m_waitingStop) return false;
    // Start Festival if not already running.
    startEngine(festivalExePath, QString::null, m_languageCode, m_codec);
    // Set state, waiting for voice codes list from Festival.
    m_waitingQueryVoices = true;
    // Send command to query the voice codes.
    sendToFestival("(print (mapcar (lambda (pair) (car pair)) voice-locations))");
    return true;
}

/**
* Start Festival engine.
* @param festivalExePath         Path to the Festival executable, or just "festival".
* @param voiceCode               Voice code in which to speak text.
* @param languageCode            Language code, for example, "en".
*/
void FestivalIntProc::startEngine(const QString &festivalExePath, const QString &voiceCode,
    const QString &languageCode, QTextCodec* codec)
{
    // Initialize Festival only if it's not initialized.
    if (m_festProc)
    {
        // Stop Festival if a different EXE is requested or different language code.
        // If festProc exists but is not running, it is because it was stopped.
        if ((festivalExePath != m_festivalExePath) || !m_festProc->isRunning() ||
             (m_languageCode != languageCode) || (codec->name() != m_codec->name()))
        {
            delete m_festProc;
            m_festProc = 0;
        }
    }
    if(!m_festProc)
    {
        // kdDebug()<< "FestivalIntProc::startEngine: Creating Festival object" << endl;
        m_festProc = new KProcess;
        *m_festProc << festivalExePath;
        *m_festProc << "--interactive";
        m_festProc->setEnvironment("LANG", languageCode + "." + codec->mimeName());
        m_festProc->setEnvironment("LC_CTYPE", languageCode + "." + codec->mimeName());
        kdDebug() << "FestivalIntProc::startEngine: setting LANG = LC_CTYPE = " << languageCode << "." << codec->mimeName() << endl;
        connect(m_festProc, SIGNAL(processExited(KProcess*)),
                this, SLOT(slotProcessExited(KProcess*)));
        connect(m_festProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
                this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
        connect(m_festProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
                this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
        connect(m_festProc, SIGNAL(wroteStdin(KProcess*)),
                this, SLOT(slotWroteStdin(KProcess*)));
    }
    if (!m_festProc->isRunning())
    {
        // kdDebug() << "FestivalIntProc::startEngine: Starting Festival process" << endl;
        m_runningVoiceCode = QString::null;
        m_runningTime = 100;
        m_runningPitch = 100;
        m_ready = false;
        m_outputQueue.clear();
        if (m_festProc->start(KProcess::NotifyOnExit, KProcess::All))
        {
            // kdDebug()<< "FestivalIntProc:startEngine: Festival initialized" << endl;
            m_festivalExePath = festivalExePath;
            m_languageCode = languageCode;
            m_codec = codec;
            // Load the SABLE to Wave module.
            sendToFestival("(load \"" +
                KGlobal::dirs()->resourceDirs("data").last() + "kttsd/festivalint/sabletowave.scm\")");
        }
        else
        {
            kdDebug() << "FestivalIntProc::startEngine: Error starting Festival process.  Is festival in the PATH?" << endl;
            m_ready = true;
            m_state = psIdle;
            return;
        }
    }
    // If we just started Festival, or voiceCode has changed, send code to Festival.
    if (m_runningVoiceCode != voiceCode and !voiceCode.isEmpty()) {
        sendToFestival("(voice_" + voiceCode + ")");
        m_runningVoiceCode = voiceCode;
    }
}

/**
* Say or Synthesize text.
* @param festivalExePath         Path to the Festival executable, or just "festival".
* @param text                    The text to be synthesized.
* @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
*                                synthesize and audibilize the text.
* @param voiceCode               Voice code in which to speak text.
* @param time                    Speed percentage. 50 to 200. 200% = 2x normal.
* @param pitch                   Pitch persentage.  50 to 200.
* @param volume                  Volume percentage.  50 to 200.
* @param languageCode            Language code, for example, "en".
*/
void FestivalIntProc::synth(
    const QString &festivalExePath,
    const QString &text,
    const QString &synthFilename,
    const QString &voiceCode,
    const int time,
    const int pitch,
    const int volume,
    const QString &languageCode,
    QTextCodec* codec)
{
    // kdDebug() << "FestivalIntProc::synth: festivalExePath = " << festivalExePath
    //         << " voiceCode = " << voiceCode << endl;

    // Initialize Festival only if it's not initialized
    startEngine(festivalExePath, voiceCode, languageCode, codec);
    // If we just started Festival, or rate changed, tell festival.
    if (m_runningTime != time) {
        QString timeMsg;
        if (voiceCode.contains("_hts") > 0)
        {
            // Map 50% to 200% onto 0 to 1000.
            // slider = alpha * (log(percent)-log(50))
            // with alpha = 1000/(log(200)-log(50))
            double alpha = 1000 / (log(200) - log(50));
            int slider = (int)floor (0.5 + alpha * (log(time)-log(50)));
            // Center at 0.
            slider = slider - 500;
            // Map -500 to 500 onto 0.15 to -0.15.
            float stretchValue = -float(slider) * 0.15 / 500.0;
            timeMsg = QString("(set! hts_duration_stretch %1)").arg(
                    stretchValue, 0, 'f', 3);
        }
        else
            timeMsg = QString("(Parameter.set 'Duration_Stretch %1)").arg(
                1.0/(float(time)/100.0), 0, 'f', 2);
        sendToFestival(timeMsg);
        m_runningTime = time;
    }
    // If we just started Festival, or pitch changed, tell festival.
    if (m_runningPitch != pitch) {
        // Pitch values range from 50 to 200 %, with 100% as the midpoint,
        // while frequency values range from 41 to 500 with 105 as the "midpoint".
        int pitchValue;
        if (pitch <= 100)
        {
            pitchValue = (((pitch - 50) * 64) / 50) + 41;
        }
        else
        {
            pitchValue = (((pitch - 100) * 395) / 100) + 105;
        }
        QString pitchMsg = QString(
            "(set! int_lr_params '((target_f0_mean %1) (target_f0_std 14)"
            "(model_f0_mean 170) (model_f0_std 34)))").arg(pitchValue, 0, 10);
        sendToFestival(pitchMsg);
        m_runningPitch = pitch;
    }

    // Encode quotation characters.
    QString saidText = text;
    saidText.replace("\\\"", "#!#!");
    saidText.replace("\"", "\\\"");
    saidText.replace("#!#!", "\\\"");
    // Remove certain comment characters.
    saidText.replace("--", "");

    // Ok, let's rock.
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
        // Volume must be given for each utterance.
        // Volume values range from 50 to 200%, with 100% = normal.
        // Map onto rescale range of .5 to 2.
        float volumeValue = float(volume) / 100;
        // Expand to range .25 to 4.
        // float volumeValue = exp(log(volumeValue) * 2);
        // kdDebug() << "FestivalIntProc::synth: Synthing text: '" << saidText << "' using Festival plug in with voice "
        //    << voiceCode << endl;
        if (isSable(saidText))
        {
            // Synth the text and adjust volume.
            saidText =
                "(ktts_sabletowave \"" + saidText + "\" \"" +
                synthFilename + "\" " +
                QString::number(volumeValue) + ")";
        }
        else
        {
            saidText =
                // Suppress pause at the beginning of each utterance.
                "(define (insert_initial_pause utt) "
                "(item.set_feat (utt.relation.first utt 'Segment) 'end 0.0))"
                // Synth the text and adjust volume.
                "(set! utt1 (Utterance Text \"" +  saidText + 
                "\"))(utt.synth utt1)" +
                "(utt.wave.rescale utt1 " + QString::number(volumeValue) + " t)" +
                "(utt.save.wave utt1 \"" + synthFilename + "\")";
        }
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
    QCString encodedText;
    if (m_codec)
        encodedText = m_codec->fromUnicode(text);
    else
        encodedText = text.latin1();
    m_outputQueue.pop_front();
    m_ready = false;
    // kdDebug() << "FestivalIntProc::sendIfReady: sending to Festival: " << text << endl;
    m_writingStdin = true;
    m_festProc->writeStdin(encodedText, encodedText.length());
    return true;
}

/**
* Determine if the text has SABLE tags.  If so, we will have to use a different
* synthesis method.
*/
bool FestivalIntProc::isSable(const QString &text)
{
    return (text.contains("<SABLE"));
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
                // If using a preloaded voice, killing Festival is a bad idea because of
                // huge startup times.  So if synthing (not saying), let Festival continue
                // synthing.  When it completes, we will emit the stopped signal.
                if (m_preload && (m_state == psSynthing))
                {
                    m_waitingStop = true;
                    // kdDebug() << "FestivalIntProc::stopText: Optimizing stopText() for preloaded voice." << endl;
                }
                else
                {
                    // kdDebug() << "FestivalIntProc::stopText: killing Festival." << endl;
                    m_waitingStop = true;
                    m_festProc->kill();
                }
            }
        } else m_state = psIdle;
    } else m_state = psIdle;
}

void FestivalIntProc::slotProcessExited(KProcess*)
{
    // kdDebug() << "FestivalIntProc:slotProcessExited: Festival process has exited." << endl;
    m_ready = true;
    pluginState prevState = m_state;
    if (m_waitingStop || m_waitingQueryVoices)
    {
        if (m_waitingStop)
        {
            m_waitingStop = false;
            m_state = psIdle;
            // kdDebug() << "FestivalIntProc::slotProcessExited: emitting stopped signal" << endl;
            emit stopped();
        }
        if (m_waitingQueryVoices)
        {
            // kdDebug() << "FestivalIntProc::slotProcessExited: canceling queryVoices operation" << endl;
            m_waitingQueryVoices = false;
            m_state = psIdle;
        }
    } else {
        if (m_state != psIdle) m_state = psFinished;
        if (prevState == psSaying)
        {
            // kdDebug() << "FestivalIntProc::slotProcessExited: emitting sayFinished signal" << endl;
            emit sayFinished();
        } else
            if (prevState == psSynthing)
            {
                // kdDebug() << "FestivalIntProc::slotProcessExited: emitting synthFinished signal" << endl;
                emit synthFinished();
            }
    }
    delete m_festProc;
    m_festProc = 0;
    m_outputQueue.clear();
}

void FestivalIntProc::slotReceivedStdout(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    // kdDebug() << "FestivalIntProc::slotReceivedStdout: Received from Festival: " << buf << endl;
    bool promptSeen = (buf.contains("festival>") > 0);
    bool emitQueryVoicesFinished = false;
    QStringList voiceCodesList;
    if (m_waitingQueryVoices)
    {
        // Look for opening ( and closing ).
        buf.simplifyWhiteSpace();
        if (buf.left(1) == "(")
        {
            int rightParen = buf.find(')');
            if (rightParen > 0)
            {
                m_waitingQueryVoices = false;
                // Extract contents between parens.
                buf = buf.mid(1, rightParen - 1);
                // Space separated list.
                voiceCodesList = QStringList::split(" ", buf, false);
                emitQueryVoicesFinished = true;
            }
        }
    }
    if (promptSeen)
    {
        // kdDebug() << "FestivalIntProc::slotReceivedStdout: Prompt seen" << endl;
        m_ready = true;
        if (!sendIfReady())
        {
            // kdDebug() << "FestivalIntProc::slotReceivedStdout: All output sent. " << endl;
            pluginState prevState = m_state;
            if (m_state != psIdle) m_state = psFinished;
            if (prevState == psSaying)
            {
                // kdDebug() << "FestivalIntProc::slotReceivedStdout: emitting sayFinished signal" << endl;
                emit sayFinished();
            } else
                if (prevState == psSynthing)
                {
                    if (m_waitingStop)
                    {
                        m_waitingStop = false;
                        m_state = psIdle;
                        // kdDebug() << "FestivalIntProc::slotReceivedStdout: emitting optimized stopped signal" << endl;
                        emit stopped();
                    }
                    else
                    {
                        // kdDebug() << "FestivalIntProc::slotReceivedStdout: emitting synthFinished signal" << endl;
                        emit synthFinished();
                    }
                }
        }
    }
    if (emitQueryVoicesFinished)
    {
        // kdDebug() << "FestivalIntProc::slotReceivedStdout: emitting queryVoicesFinished" << endl;
        emit queryVoicesFinished(voiceCodesList);
    }
}

void FestivalIntProc::slotReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "FestivalIntProc::slotReceivedStderr: Received error from Festival: " << buf << endl;
}

void FestivalIntProc::slotWroteStdin(KProcess* /*proc*/)
{
    // kdDebug() << "FestivalIntProc::slotWroteStdin: Running" << endl;
    m_writingStdin = false;
    if (!sendIfReady())
    {
        // kdDebug() << "FestivalIntProc::slotWroteStdin: all output sent" << endl;
        pluginState prevState = m_state;
        if (m_state != psIdle) m_state = psFinished;
        if (prevState == psSaying)
        {
            // kdDebug() << "FestivalIntProc::slotWroteStdin: emitting sayFinished signal" << endl;
            emit sayFinished();
        } else
            if (prevState == psSynthing)
            {
                // kdDebug() << "FestivalIntProc::slotWroteStdin: emitting synthFinished signal" << endl;
                emit synthFinished();
            }
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

/**
* Returns the name of an XSLT stylesheet that will convert a valid SSML file
* into a format that can be processed by the synth.  For example,
* The Festival plugin returns a stylesheet that will convert SSML into
* SABLE.  Any tags the synth cannot handle should be stripped (leaving
* their text contents though).  The default stylesheet strips all
* tags and converts the file to plain text.
* @return            Name of the XSLT file.
*/
QString FestivalIntProc::getSsmlXsltFilename()
{
    return KGlobal::dirs()->resourceDirs("data").last() + "kttsd/festivalint/xslt/SSMLtoSable.xsl";
}

