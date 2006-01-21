/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  This class is in charge of getting the messages, warnings and text from
  the queue and call the plug ins function to actually speak the texts.
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
 ******************************************************************************/

#include <config.h>

// Qt includes. 
#include <qfile.h>
#include <qtimer.h>
#include <qdir.h>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kparts/componentfactory.h>
#include <ktrader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
//#include <kio/job.h>

// KTTSD includes.
#include "player.h"
#include "speaker.h"
#include "speaker.moc"
#include "talkermgr.h"
#include "utils.h"

/**
* The Speaker class takes sentences from the text queue, messages from the
* messages queue, warnings from the warnings queue, and Screen Reader
* output and places them into an internal "utterance queue".  It then
* loops through this queue, farming the work off to the plugins.
* It tries to optimize processing so as to keep the plugins busy as
* much as possible, while ensuring that only one stream of audio is
* heard at any one time.
*
* The message queues are maintained in the SpeechData class.
*
* Text jobs in the text queue each have a state (queued, speakable,
* speaking, paused, finished).  Each plugin has a state (idle, saying, synthing,
* or finished).  And finally, each utterance has a state (waiting, saying,
* synthing, playing, finished).  It can be confusing if you are not aware
* of all these states.
*
* Speaker takes some pains to ensure speech is spoken in the correct order,
* namely Screen Reader Output has the highest priority, Warnings are next,
* Messages are next, and finally regular text jobs.  Since Screen Reader
* Output, Warnings, and Messages can be queued in the middle of a text
* job, Speaker must be prepared to reorder utterances in its queue.
*
* At the same time, it must issue the signals to inform programs
* what is happening.
*
* Finally, users can pause, restart, delete, advance, or rewind text jobs
* and Speaker must respond to these commands.  In some cases, utterances that
* have already been synthesized and are ready for audio output must be
* discarded in response to these commands.
*
* Some general guidelines for programmers modifying this code:
* - Avoid blocking at all cost.  If a plugin won't stopText, keep going.
*   You might have to wait for the plugin to complete before asking it
*   to perform the next operation, but in the meantime, there might be
*   other useful work that can be performed.
* - In no case allow the main thread Qt event loop to block.
* - Plugins that do not have asynchronous support are wrapped in the
*   ThreadedPlugin class, which attempts to make them as asynchronous as
*   it can, but there are limits.
* - doUtterances is the main worker method.  If in doubt, call doUtterances.
* - Because Speaker controls the ordering of utterances, most sequence-related
*   signals must be emitted by Speaker; not SpeechData.  Only the add
*   and delete job-related signals eminate from SpeechData.
* - The states of the 3 types of objects mentioned above (jobs, utterances,
*   and plugins) can interact in subtle ways.  Test fully.  For example, while
*   a text job might be in a paused state, the plugin could be synthesizing
*   in anticipation of resume, or sythesizing a Warning, Message, or
*   Screen Reader Output.  Meanwhile, while one of the utterances might
*   have a paused state, others from the same job could be synthing, waiting,
*   or finished.
* - There can be more than one Audio Player object in existence at one time, although
*   there must never be more than one actually playing at one time.  For
*   example, an Audio Player playing an utterance from a text job can be
*   in a paused state, while another Audio Player is playing a Screen Reader
*   Output utterance.  Finally, since some plugins do their own audio, it
*   might be that none of the Audio Player objects are playing.
*/

/* Public Methods ==========================================================*/

/**
* Constructor.
* Loads plugins.
*/
Speaker::Speaker( SpeechData*speechData, TalkerMgr* talkerMgr,
    QObject *parent, const char *name) :
    QObject(parent, name), 
    m_speechData(speechData),
    m_talkerMgr(talkerMgr)
{
    // kdDebug() << "Running: Speaker::Speaker()" << endl;
    m_exitRequested = false;
    m_textInterrupted = false;
    m_currentJobNum = 0;
    m_lastAppId = 0;
    m_lastJobNum = 0;
    m_lastSeq = 0;
    m_timer = new QTimer(this, "kttsdAudioTimer");
    m_speechData->config->setGroup("General");
    m_playerOption = m_speechData->config->readEntry("AudioOutputMethod", 0);  // default to aRts.
    // Map 50% to 100% onto 2.0 to 0.5.
    m_audioStretchFactor = 1.0/(float(m_speechData->config->readEntry("AudioStretchFactor", 100))/100.0);
    switch (m_playerOption)
    {
        case 0: break;
        case 1:
            m_speechData->config->setGroup("GStreamerPlayer");
            m_sinkName = m_speechData->config->readEntry("SinkName", "osssink");
            break;
        case 2:
            m_speechData->config->setGroup("ALSAPlayer");
            m_sinkName = m_speechData->config->readEntry("PcmName", "default");
            break;
        case 3:
            m_speechData->config->setGroup("aKodePlayer");
            m_sinkName = m_speechData->config->readEntry("SinkName", "auto");
            break;
    }
    // Connect timer timeout signal.
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

    // Connect plugins to slots.
    PlugInList plugins = m_talkerMgr->getLoadedPlugIns();
    const int pluginsCount = plugins.count();
    for (int ndx = 0; ndx < pluginsCount; ++ndx)
    {
        PlugInProc* speech = plugins.at(ndx);
        connect(speech, SIGNAL(synthFinished()),
            this, SLOT(slotSynthFinished()));
        connect(speech, SIGNAL(sayFinished()),
            this, SLOT(slotSayFinished()));
        connect(speech, SIGNAL(stopped()),
            this, SLOT(slotStopped()));
        connect(speech, SIGNAL(error(bool, const QString&)),
            this, SLOT(slotError(bool, const QString&)));
    }
}

/**
* Destructor.
*/
Speaker::~Speaker(){
    // kdDebug() << "Running: Speaker::~Speaker()" << endl;
    m_timer->stop();
    delete m_timer;
    if (!m_uttQueue.isEmpty())
    {
        uttIterator it;
        for (it = m_uttQueue.begin(); it != m_uttQueue.end(); )
            it = deleteUtterance(it);
    }
}

/**
 * Tells the speaker it is requested to exit.
 * TODO: I don't think this actually accomplishes anything.
 */
void Speaker::requestExit(){
    // kdDebug() << "Speaker::requestExit: Running" << endl;
    m_exitRequested = true;
}

/**
 * Main processing loop.  Dequeues utterances and sends them to the
 * plugins and/or Audio Player.
 */
void Speaker::doUtterances()
{
    // kdDebug() << "Running: Speaker::doUtterances()" << endl;

    // Used to prevent exiting prematurely.
    m_again = true;

    while(m_again && !m_exitRequested)
    {
        m_again = false;

        if (m_exitRequested)
        {
            // kdDebug() << "Speaker::run: exiting due to request 1." << endl;
            return;
        }

        uttIterator it;
        uttIterator itBegin;
        uttIterator itEnd = 0;  // Init to zero to avoid compiler warning.

        // If Screen Reader Output is waiting, we need to process it ASAP.
        if (m_speechData->screenReaderOutputReady())
        {
            m_again = getNextUtterance();
        }
//         kdDebug() << "Speaker::doUtterances: queue dump:" << endl;
//         for (it = m_uttQueue.begin(); it != m_uttQueue.end(); ++it)
//         {
//             QString pluginState = "no plugin";
//             if (it->plugin) pluginState = pluginStateToStr(it->plugin->getState());
//             QString jobState =
//                     jobStateToStr(m_speechData->getTextJobState(it->sentence->jobNum));
//             kdDebug() << 
//                     "  State: " << uttStateToStr(it->state) << 
//                     "," << pluginState <<
//                     "," << jobState <<
//                     " Type: " << uttTypeToStr(it->utType) << 
//                     " Text: " << it->sentence->text << endl;
//         }

        if (!m_uttQueue.isEmpty())
        {
            // Delete utterances that are finished.
            it = m_uttQueue.begin();
            while (it != m_uttQueue.end())
            {
                if (it->state == usFinished)
                    it = deleteUtterance(it);
                else
                    ++it;
            }
            // Loop through utterance queue.
            int waitingCnt = 0;
            int waitingMsgCnt = 0;
            int transformingCnt = 0;
            bool playing = false;
            int synthingCnt = 0;
            itEnd = m_uttQueue.end();
            itBegin = m_uttQueue.begin();
            for (it = itBegin; it != itEnd; ++it)
            {
                uttState utState = it->state;
                uttType utType = it->utType;
                switch (utState)
                {
                    case usNone:
                    {
                        setInitialUtteranceState(*it);
                        m_again = true;
                        break;
                    }
                    case usWaitingTransform:
                    {
                        // Create an XSLT transformer and transform the text.
                        it->transformer = new SSMLConvert();
                        connect(it->transformer, SIGNAL(transformFinished()),
                            this, SLOT(slotTransformFinished()));
                        if (it->transformer->transform(it->sentence->text,
                            it->plugin->getSsmlXsltFilename()))
                        {
                            it->state = usTransforming;
                            ++transformingCnt;
                        }
                        else
                        {
                            // If an error occurs transforming, skip it.
                            it->state = usTransforming;
                            setInitialUtteranceState(*it);
                        }
                        m_again = true;
                        break;
                    }
                    case usTransforming:
                    {
                        // See if transformer is finished.
                        if (it->transformer->getState() == SSMLConvert::tsFinished)
                        {
                            // Get the transformed text.
                            it->sentence->text = it->transformer->getOutput();
                            // Set next state (usWaitingSynth or usWaitingSay)
                            setInitialUtteranceState(*it);
                            m_again = true;
                            --transformingCnt;
                        }
                        break;
                    }
                    case usWaitingSignal:
                    {
                        // If first in queue, emit signal.
                        if (it == itBegin)
                        {
                            if (utType == utStartOfJob)
                            {
                                m_speechData->setTextJobState(
                                    it->sentence->jobNum, KSpeech::jsSpeaking);
                                if (it->sentence->seq == 0)
                                    emit textStarted(it->sentence->appId,
                                        it->sentence->jobNum);
                                else
                                    emit textResumed(it->sentence->appId,
                                        it->sentence->jobNum);
                            } else {
                                m_speechData->setTextJobState(
                                    it->sentence->jobNum, KSpeech::jsFinished);
                                emit textFinished(it->sentence->appId, it->sentence->jobNum);
                            }
                            it->state = usFinished;
                            m_again = true;
                        }
                        break;
                    }
                    case usSynthed:
                    {
                        // Don't bother stretching if factor is 1.0.
                        // Don't bother stretching if SSML.
                        // TODO: This is because sox mangles SSML pitch settings.  Would be nice
                        // to figure out how to avoid this.
                        if (m_audioStretchFactor == 1.0 || it->isSSML)
                        {
                            it->state = usStretched;
                            m_again = true;
                        }
                        else
                        {
                            it->audioStretcher = new Stretcher();
                            connect(it->audioStretcher, SIGNAL(stretchFinished()),
                                this, SLOT(slotStretchFinished()));
                            if (it->audioStretcher->stretch(it->audioUrl, makeSuggestedFilename(),
                                m_audioStretchFactor))
                            {
                                it->state = usStretching;
                                m_again = true;  // Is this needed?
                            }
                            else
                            {
                                // If stretch failed, it is most likely caused by sox not being
                                // installed.  Just skip it.
                                it->state = usStretched;
                                m_again = true;
                                delete it->audioStretcher;
                                it->audioStretcher= 0;
                            }
                        }
                        break;
                    }
                    case usStretching:
                    {
                        // See if Stretcher is finished.
                        if (it->audioStretcher->getState() == Stretcher::ssFinished)
                        {
                            QFile::remove(it->audioUrl);
                            it->audioUrl = it->audioStretcher->getOutFilename();
                            it->state = usStretched;
                            delete it->audioStretcher;
                            it->audioStretcher = 0;
                            m_again = true;
                        }
                        break;
                    }
                    case usStretched:
                    {
                        // If first in queue, start playback.
                        if (it == itBegin)
                        {
                            if (startPlayingUtterance(it))
                            {
                                playing = true;
                                m_again = true;
                            } else {
                                ++waitingCnt;
                                if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                            }
                        } else {
                            ++waitingCnt;
                            if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        }
                        break;
                    }
                    case usPlaying:
                    {
                        playing = true;
                        break;
                    }
                    case usPaused: 
                    case usPreempted:
                    {
                        if (!playing) 
                        {
                            if (startPlayingUtterance(it))
                            {
                                playing = true;
                                m_again = true;
                            } else {
                                ++waitingCnt;
                                if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                            }
                        } else {
                            ++waitingCnt;
                            if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        }
                        break;
                    }
                    case usWaitingSay:
                    {
                        // If first in queue, start it.
                        if (it == itBegin)
                        {
                            int jobState =
                                m_speechData->getTextJobState(it->sentence->jobNum);
                            if ((jobState == KSpeech::jsSpeaking) ||
                                (jobState == KSpeech::jsSpeakable))
                            {
                                if (it->plugin->getState() == psIdle)
                                {
                                    // Set job to speaking state and set sequence number.
                                    mlText* sentence = it->sentence;
                                    m_currentJobNum = sentence->jobNum;
                                    m_speechData->setTextJobState(m_currentJobNum, KSpeech::jsSpeaking);
                                    m_speechData->setJobSequenceNum(m_currentJobNum, sentence->seq);
                                    prePlaySignals(it);
                                    // kdDebug() << "Async synthesis and audibilizing." << endl;
                                    it->state = usSaying;
                                    playing = true;
                                    it->plugin->sayText(it->sentence->text);
                                    m_again = true;
                                } else {
                                    ++waitingCnt;
                                    if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                                }
                            } else {
                                ++waitingCnt;
                                if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                            }
                        } else {
                            ++waitingCnt;
                            if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        }
                        break;
                    }
                    case usWaitingSynth:
                    {
                        // TODO: If the synth is busy and the waiting text is screen
                        // reader output, it would be nice to call the synth's
                        // stopText() method.  However, some of the current plugins
                        // have horrible startup times, so we won't do that for now.
                        if (it->plugin->getState() == psIdle)
                        {
                            // kdDebug() << "Async synthesis." << endl;
                            it->state = usSynthing;
                            ++synthingCnt;
                            it->plugin->synthText(it->sentence->text,
                                makeSuggestedFilename());
                            m_again = true;
                        }
                        ++waitingCnt;
                        if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        break;
                    }
                    case usSaying:
                    {
                        // See if synthesis and audibilizing is finished.
                        if (it->plugin->getState() == psFinished)
                        {
                            it->plugin->ackFinished();
                            it->state = usFinished;
                            m_again = true;
                        } else {
                            playing = true;
                            ++waitingCnt;
                            if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        }
                        break;
                    }
                    case usSynthing:
                    {
                        // See if synthesis is completed.
                        if (it->plugin->getState() == psFinished)
                        {
                            it->audioUrl = KStandardDirs::realFilePath(it->plugin->getFilename());
                            // kdDebug() << "Speaker::doUtterances: synthesized filename: " << it->audioUrl << endl;
                            it->plugin->ackFinished();
                            it->state = usSynthed;
                            m_again = true;
                        } else ++synthingCnt;
                        ++waitingCnt;
                        if (utType == utWarning || utType == utMessage) ++waitingMsgCnt;
                        break;
                    }
                    case usFinished: break;
                }
            }
            // See if there are any messages or warnings to process.
            // We keep up to 2 such utterances in the queue.
            if ((waitingMsgCnt < 2) && (transformingCnt < 3))
            {
                if (m_speechData->warningInQueue() || m_speechData->messageInQueue())
                {
                    getNextUtterance();
                    m_again = true;
                }
            }
            // Try to keep at least two utterances in the queue waiting to be played,
            // and no more than 3 transforming at one time.
            if ((waitingCnt < 2) && (transformingCnt < 3))
                if (getNextUtterance()) m_again = true;
        } else {
            // See if another utterance is ready to be worked on.
            // If so, loop again since we've got work to do.
            m_again = getNextUtterance();
        }
    }
    // kdDebug() << "Speaker::doUtterances: exiting." << endl;
}

/**
 * Determine if kttsd is currently speaking any text jobs.
 * @return               True if currently speaking any text jobs.
 */
bool Speaker::isSpeakingText()
{
    return (m_speechData->getTextJobState(m_currentJobNum) == KSpeech::jsSpeaking);
}

/**
 * Get the job number of the current text job.
 * @return               Job number of the current text job. 0 if no jobs.
 *
 * Note that the current job may not be speaking. See @ref isSpeakingText.
 */
uint Speaker::getCurrentTextJob() { return m_currentJobNum; }

/**
 * Remove a text job from the queue.
 * @param jobNum         Job number of the text job.
 *
 * The job is deleted from the queue and the @ref textRemoved signal is emitted.
 *
 * If there is another job in the text queue, and it is marked speakable,
 * that job begins speaking.
 */
void Speaker::removeText(const uint jobNum)
{
    deleteUtteranceByJobNum(jobNum);
    m_speechData->removeText(jobNum);
    doUtterances();
}

/**
 * Start a text job at the beginning.
 * @param jobNum         Job number of the text job.
 *
 * Rewinds the job to the beginning.
 *
 * The job is marked speakable.
 * If there are other speakable jobs preceeding this one in the queue,
 * those jobs continue speaking and when finished, this job will begin speaking.
 * If there are no other speakable jobs preceeding this one, it begins speaking.
 *
 * The @ref textStarted signal is emitted when the text job begins speaking.
 * When all the sentences of the job have been spoken, the job is marked for deletion from
 * the text queue and the @ref textFinished signal is emitted.
 */
void Speaker::startText(const uint jobNum)
{
    deleteUtteranceByJobNum(jobNum);
    m_speechData->setJobSequenceNum(jobNum, 1);
    m_speechData->setTextJobState(jobNum, KSpeech::jsSpeakable);
    if (m_lastJobNum == jobNum)
    {
        // kdDebug() << "Speaker::startText: startText called on speaking job " << jobNum << endl;
        m_lastJobNum = 0;
        m_lastAppId = 0;
        m_lastSeq = 0;
    }
    doUtterances();
}

/**
 * Stop a text job and rewind to the beginning.
 * @param jobNum         Job number of the text job.
 *
 * The job is marked not speakable and will not be speakable until @ref startText or @ref resumeText
 * is called.
 *
 * If there are speaking jobs preceeding this one in the queue, they continue speaking.
 * If the job is currently speaking, the @ref textStopped signal is emitted and the job stops speaking.
 * Depending upon the speech engine and plugin used, speeking may not stop immediately
 * (it might finish the current sentence).
 */
void Speaker::stopText(const uint jobNum)
{
    bool emitSignal = (m_speechData->getTextJobState(jobNum) == KSpeech::jsSpeaking);
    deleteUtteranceByJobNum(jobNum);
    m_speechData->setJobSequenceNum(jobNum, 1);
    m_speechData->setTextJobState(jobNum, KSpeech::jsQueued);
    if (emitSignal) textStopped(m_speechData->getAppIdByJobNum(jobNum), jobNum);
    // Call doUtterances to process other jobs.
    doUtterances();
}

/**
 * Pause a text job.
 * @param jobNum         Job number of the text job.
 *
 * The job is marked as paused and will not be speakable until @ref resumeText or
 * @ref startText is called.
 *
 * If there are speaking jobs preceeding this one in the queue, they continue speaking.
 * If the job is currently speaking, the @ref textPaused signal is emitted and the job stops speaking.
 * Depending upon the speech engine and plugin used, speeking may not stop immediately
 * (it might finish the current sentence).
 * @see resumeText
 */
void Speaker::pauseText(const uint jobNum)
{
    bool emitSignal = (m_speechData->getTextJobState(jobNum) == KSpeech::jsSpeaking);
    pauseUtteranceByJobNum(jobNum);
    kdDebug() << "Speaker::pauseText: setting Job State of job " << jobNum << " to jsPaused" << endl;
    m_speechData->setTextJobState(jobNum, KSpeech::jsPaused);
    if (emitSignal) textPaused(m_speechData->getAppIdByJobNum(jobNum),jobNum);
}

/**
 * Start or resume a text job where it was paused.
 * @param jobNum         Job number of the text job.
 *
 * The job is marked speakable.
 *
 * If the job is currently speaking, or is waiting to be spoken (speakable 
 * state), the resumeText() call is ignored.
 *
 * If the job is currently queued, or is finished, it is the same as calling
 * @ref startText .
 *
 * If there are speaking jobs preceeding this one in the queue, those jobs continue speaking and,
 * when finished this job will begin speaking where it left off.
 *
 * The @ref textResumed signal is emitted when the job resumes.
 * @see pauseText
 */
void Speaker::resumeText(const uint jobNum)
{
    int state = m_speechData->getTextJobState(jobNum);
    switch (state)
    {
        case KSpeech::jsQueued:
        case KSpeech::jsFinished:
            startText(jobNum);
            break;
        case KSpeech::jsSpeakable:
        case KSpeech::jsSpeaking:
            doUtterances();
            break;
        case KSpeech::jsPaused:
            if (jobNum == m_currentJobNum)
                m_speechData->setTextJobState(jobNum, KSpeech::jsSpeaking);
            else
                m_speechData->setTextJobState(jobNum, KSpeech::jsSpeakable);
            doUtterances();
            break;
    }
}

/**
 * Move a text job down in the queue so that it is spoken later.
 * @param jobNum         Job number of the text job.
 *
 * If the job is currently speaking, it is paused.
 * If the next job in the queue is speakable, it begins speaking.
 */
void Speaker::moveTextLater(const uint jobNum)
{
    if (m_speechData->getTextJobState(jobNum) == KSpeech::jsSpeaking)
        m_speechData->setTextJobState(jobNum, KSpeech::jsPaused);
    deleteUtteranceByJobNum(jobNum);
    m_speechData->moveTextLater(jobNum);
    doUtterances();
}

/**
 * Jump to the first sentence of a specified part of a text job.
 * @param partNum        Part number of the part to jump to.  Parts are numbered starting at 1.
 * @param jobNum         Job number of the text job.
 * @return               Part number of the part actually jumped to.
 *
 * If partNum is greater than the number of parts in the job, jumps to last part.
 * If partNum is 0, does nothing and returns the current part number.
 * If no such job, does nothing and returns 0.
 * Does not affect the current speaking/not-speaking state of the job.
 */
int Speaker::jumpToTextPart(const int partNum, const uint jobNum)
{
    if (partNum == 0) return m_speechData->jumpToTextPart(partNum, jobNum);
    deleteUtteranceByJobNum(jobNum);
    int pNum = m_speechData->jumpToTextPart(partNum, jobNum);
    if (pNum)
    {
        uint seq = m_speechData->getJobSequenceNum(jobNum);
        if (jobNum == m_lastJobNum)
        {
            if (seq == 0)
                m_lastSeq = seq;
            else
                m_lastSeq = seq - 1;
        }
        if (jobNum == m_currentJobNum)
        {
            m_lastJobNum = jobNum;
            if (seq == 0)
                m_lastSeq = 0;
            else
                m_lastSeq = seq - 1;
            doUtterances();
        }
    }
    return pNum;
}
     
/**
 * Advance or rewind N sentences in a text job.
 * @param n              Number of sentences to advance (positive) or rewind (negative)
 *                       in the job.
 * @param jobNum         Job number of the text job.
 * @return               Sequence number of the sentence actually moved to.
 *                       Sequence numbers are numbered starting at 1.
 *
 * If no such job, does nothing and returns 0.
 * If n is zero, returns the current sequence number of the job.
 * Does not affect the current speaking/not-speaking state of the job.
 */
uint Speaker::moveRelTextSentence(const int n, const uint jobNum)
{
    deleteUtteranceByJobNum(jobNum);
    // TODO: More efficient way to advance one or two sentences, since there is a
    // good chance those utterances are already in the queue and synthesized.
    uint seq = m_speechData->moveRelTextSentence(n, jobNum);
    kdDebug() << "Speaker::moveRelTextSentence: job num: " << jobNum << " moved to seq: " << seq << endl;
    if (jobNum == m_lastJobNum)
    {
        if (seq == 0)
            m_lastSeq = seq;
        else
            m_lastSeq = seq - 1;
    }
    if (jobNum == m_currentJobNum)
    {
        m_lastJobNum = jobNum;
        if (seq == 0)
            m_lastSeq = 0;
        else
            m_lastSeq = seq - 1;
        doUtterances();
    }
    return seq;
}

/* Private Methods ==========================================================*/

/**
 * Converts an utterance state enumerator to a displayable string.
 * @param state           Utterance state.
 */
QString Speaker::uttStateToStr(uttState state)
{
    switch (state)
    {
        case usNone:                return "usNone";
        case usWaitingTransform:    return "usWaitingTransform";
        case usTransforming:        return "usTransforming";
        case usWaitingSay:          return "usWaitingSay";
        case usWaitingSynth:        return "usWaitingSynth";
        case usWaitingSignal:       return "usWaitingSignal";
        case usSaying:              return "usSaying";
        case usSynthing:            return "usSynthing";
        case usSynthed:             return "usSynthed";
        case usStretching:          return "usStretching";
        case usStretched:           return "usStretched";
        case usPlaying:             return "usPlaying";
        case usPaused:              return "usPaused";
        case usPreempted:           return "usPreempted";
        case usFinished:            return "usFinished";
    }
    return QString();
}

/**
 * Converts an utterance type enumerator to a displayable string.
 * @param utType          Utterance type.
 * @return                Displayable string for utterance type.
 */
QString Speaker::uttTypeToStr(uttType utType)
{
    switch (utType)
    {
        case utText:         return "utText";
        case utInterruptMsg: return "utInterruptMsg";
        case utInterruptSnd: return "utInterruptSnd";
        case utResumeMsg:    return "utResumeMsg";
        case utResumeSnd:    return "utResumeSnd";
        case utMessage:      return "utMessage";
        case utWarning:      return "utWarning";
        case utScreenReader: return "utScreenReader";
        case utStartOfJob:   return "utStartOfJob";
        case utEndOfJob:     return "utEndOfJob";
    }
    return QString();
}

/**
 * Converts a plugin state enumerator to a displayable string.
 * @param state           Plugin state.
 * @return                Displayable string for plugin state.
 */
QString Speaker::pluginStateToStr(pluginState state)
{
    switch( state )
    {
        case psIdle:         return "psIdle";
        case psSaying:       return "psSaying";
        case psSynthing:     return "psSynthing";
        case psFinished:     return "psFinished";
    }
    return QString();
}

/**
 * Converts a job state enumerator to a displayable string.
 * @param state           Job state.
 * @return                Displayable string for job state.
 */
QString Speaker::jobStateToStr(int state)
{
    switch ( state )
    {
        case KSpeech::jsQueued:      return "jsQueued";
        case KSpeech::jsSpeakable:   return "jsSpeakable";
        case KSpeech::jsSpeaking:    return "jsSpeaking";
        case KSpeech::jsPaused:      return "jsPaused";
        case KSpeech::jsFinished:    return "jsFinished";
    }
    return QString();
}

/**
 * Delete any utterances in the queue with this jobNum.
 * @param jobNum                  Job Number of the utterances to delete.
 * If currently processing any deleted utterances, stop them.
 */
void Speaker::deleteUtteranceByJobNum(const uint jobNum)
{
    uttIterator it = m_uttQueue.begin();
    while (it != m_uttQueue.end())
    {
        if (it->sentence)
        {
            if (it->sentence->jobNum == jobNum)
                it = deleteUtterance(it);
            else
                ++it;
        } else
            ++it;
    }
}

/**
 * Pause the utterance with this jobNum if it is playing on the Audio Player.
 * @param jobNum          The Job Number of the utterance to pause.
 */
void Speaker::pauseUtteranceByJobNum(const uint jobNum)
{
    uttIterator itEnd = m_uttQueue.end();
    for (uttIterator it = m_uttQueue.begin(); it != itEnd; ++it)
    {
        if (it->sentence)        // TODO: Why is this necessary?
            if (it->sentence->jobNum == jobNum)
                if (it->state == usPlaying)
        {
            if (it->audioPlayer)
                if (it->audioPlayer->playing())
            {
                m_timer->stop();
                kdDebug() << "Speaker::pauseUtteranceByJobNum: pausing audio player" << endl;
                it->audioPlayer->pause();
                kdDebug() << "Speaker::pauseUtteranceByJobNum: Setting utterance state to usPaused" << endl;
                it->state = usPaused;
                return;
            }
                        // Audio player has finished, but timeout hasn't had a chance
                        // to clean up.  So do nothing, and let timeout do the cleanup.
        }
    }
}

/**
 * Determines whether the given text is SSML markup.
 */
bool Speaker::isSsml(const QString &text)
{
    return KttsUtils::hasRootElement( text, "speak" );
}

/**
 * Determines the initial state of an utterance.  If the utterance contains
 * SSML, the state is set to usWaitingTransform.  Otherwise, if the plugin
 * supports async synthesis, sets to usWaitingSynth, otherwise usWaitingSay.
 * If an utterance has already been transformed, usWaitingTransform is
 * skipped to either usWaitingSynth or usWaitingSay.
 * @param utt             The utterance.
 */
void Speaker::setInitialUtteranceState(Utt &utt)
{
    if ((utt.state != usTransforming) && utt.isSSML)
{
    utt.state = usWaitingTransform;
    return;
}
    if (utt.plugin->supportsSynth())
        utt.state = usWaitingSynth;
    else
        utt.state = usWaitingSay;
}

/**
 * Returns true if the given job and sequence number are already in the utterance queue.
 */
bool Speaker::isInUtteranceQueue(uint jobNum, uint seqNum)
{
    uttIterator itEnd = m_uttQueue.end();
    for (uttIterator it = m_uttQueue.begin(); it != itEnd; ++it)
    {
        if (it->sentence)
        {
            if (it->sentence->jobNum == jobNum && it->sentence->seq == seqNum) return true;
        }
    }
    return false;
}

/**
 * Gets the next utterance to be spoken from speechdata and adds it to the queue.
 * @return                True if one or more utterances were added to the queue.
 *
 * Checks for waiting ScreenReaderOutput, Warnings, Messages, or Text,
 * in that order.
 * If Warning or Message and interruption messages have been configured,
 * adds those to the queue as well.
 * Determines which plugin should be used for the utterance.
 */
bool Speaker::getNextUtterance()
{
    bool gotOne = false;
    Utt* utt = 0;
    if (m_speechData->screenReaderOutputReady()) {
        utt = new Utt;
        utt->utType = utScreenReader;
        utt->sentence = m_speechData->getScreenReaderOutput();
    } else {
        if (m_speechData->warningInQueue()) {
            utt = new Utt;
            utt->utType = utWarning;
            utt->sentence = m_speechData->dequeueWarning();
        } else {
            if (m_speechData->messageInQueue()) {
                utt = new Utt;
                utt->utType = utMessage;
                utt->sentence = m_speechData->dequeueMessage();
            } else {
                uint jobNum = m_lastJobNum;
                uint seq = m_lastSeq;
                mlText* sentence = m_speechData->getNextSentenceText(jobNum, seq);
                // Skip over blank lines.
                while (sentence && sentence->text.isEmpty())
                {
                    jobNum = sentence->jobNum;
                    seq = sentence->seq;
                    sentence = m_speechData->getNextSentenceText(jobNum, seq);
                }
                // If this utterance is already in the queue, it means we have run out of
                // stuff to say and are trying to requeue already queued (and waiting stuff).
                if (sentence && !isInUtteranceQueue(sentence->jobNum, sentence->seq))
                {
                    utt = new Utt;
                    utt->utType = utText;
                    utt->sentence = sentence;
                }
            }
        }
    }
    if (utt)
    {
        gotOne = true;
        utt->isSSML = isSsml(utt->sentence->text);
        utt->state = usNone;
        utt->audioPlayer = 0;
        utt->audioStretcher = 0;
        utt->audioUrl.clear();
        utt->plugin = m_talkerMgr->talkerToPlugin(utt->sentence->talker);
        // Save some time by setting initial state now.
        setInitialUtteranceState(*utt);
        // Screen Reader Outputs need to be processed ASAP.
        if (utt->utType == utScreenReader)
        {
            m_uttQueue.insert(m_uttQueue.begin(), *utt);
            // Delete any other Screen Reader Outputs in the queue.
            // Only one Screen Reader Output at a time.
            uttIterator it = m_uttQueue.begin();
            ++it;
            while (it != m_uttQueue.end())
            {
                if (it->utType == utScreenReader)
                    it = deleteUtterance(it);
                else
                    ++it;
            }
        }
        // If the new utterance is a Warning or Message...
        if ((utt->utType == utWarning) || (utt->utType == utMessage))
        {
            uttIterator itEnd = m_uttQueue.end();
            uttIterator it = m_uttQueue.begin();
            bool interrupting = false;
            if (it != itEnd)
            {
                // New Warnings go after Screen Reader Output, other Warnings,
                // Interruptions, and in-process text,
                // but before Resumes, waiting text or signals.
                if (utt->utType == utWarning)
                    while ( it != itEnd && 
                            ((it->utType == utScreenReader) || 
                            (it->utType == utWarning) ||
                            (it->utType == utInterruptMsg) ||
                            (it->utType == utInterruptSnd))) ++it;
                // New Messages go after Screen Reader Output, Warnings, other Messages,
                // Interruptions, and in-process text,
                // but before Resumes, waiting text or signals.
                if (utt->utType == utMessage)
                    while ( it != itEnd && 
                            ((it->utType == utScreenReader) ||
                            (it->utType == utWarning) ||
                            (it->utType == utMessage) ||
                            (it->utType == utInterruptMsg) ||
                            (it->utType == utInterruptSnd))) ++it;
                if (it != itEnd)
                    if (it->utType == utText &&
                        ((it->state == usPlaying) ||
                        (it->state == usSaying))) ++it;
                // If now pointing at a text message, we are interrupting.
                // Insert optional Interruption message and sound.
                if (it != itEnd) interrupting = (it->utType == utText && it->state != usPaused);
                if (interrupting)
                {
                    if (m_speechData->textPreSndEnabled)
                    {
                        Utt intrUtt;
                        intrUtt.sentence = new mlText;
                        intrUtt.sentence->text.clear();
                        intrUtt.sentence->talker = utt->sentence->talker;
                        intrUtt.sentence->appId = utt->sentence->appId;
                        intrUtt.sentence->jobNum = utt->sentence->jobNum;
                        intrUtt.sentence->seq = 0;
                        intrUtt.audioUrl = m_speechData->textPreSnd;
                        intrUtt.audioPlayer = 0;
                        intrUtt.utType = utInterruptSnd;
                        intrUtt.isSSML = false;
                        intrUtt.state = usStretched;
                        intrUtt.plugin = 0;
                        it = m_uttQueue.insert(it, intrUtt);
                        ++it;
                    }
                    if (m_speechData->textPreMsgEnabled)
                    {
                        Utt intrUtt;
                        intrUtt.sentence = new mlText;
                        intrUtt.sentence->text = m_speechData->textPreMsg;
                        // Interruptions are spoken using default Talker.
                        intrUtt.sentence->talker.clear();
                        intrUtt.sentence->appId = utt->sentence->appId;
                        intrUtt.sentence->jobNum = utt->sentence->jobNum;
                        intrUtt.sentence->seq = 0;
                        intrUtt.audioUrl.clear();
                        intrUtt.audioPlayer = 0;
                        intrUtt.utType = utInterruptMsg;
                        intrUtt.isSSML = isSsml(intrUtt.sentence->text);
                        intrUtt.plugin = m_talkerMgr->talkerToPlugin(intrUtt.sentence->talker);
                        intrUtt.state = usNone;
                        setInitialUtteranceState(intrUtt);
                        it = m_uttQueue.insert(it, intrUtt);
                        ++it;
                    }
                }
            }
            // Insert the new message or warning.
            it = m_uttQueue.insert(it, *utt);
            ++it;
            // Resumption message and sound.
            if (interrupting)
            {
                if (m_speechData->textPostSndEnabled)
                {
                    Utt resUtt;
                    resUtt.sentence = new mlText;
                    resUtt.sentence->text.clear();
                    resUtt.sentence->talker = utt->sentence->talker;
                    resUtt.sentence->appId = utt->sentence->appId;
                    resUtt.sentence->jobNum = utt->sentence->jobNum;
                    resUtt.sentence->seq = 0;
                    resUtt.audioUrl = m_speechData->textPostSnd;
                    resUtt.audioPlayer = 0;
                    resUtt.utType = utResumeSnd;
                    resUtt.isSSML = false;
                    resUtt.state = usStretched;
                    resUtt.plugin = 0;
                    it = m_uttQueue.insert(it, resUtt);
                    ++it;
                }
                if (m_speechData->textPostMsgEnabled)
                {
                    Utt resUtt;
                    resUtt.sentence = new mlText;
                    resUtt.sentence->text = m_speechData->textPostMsg;
                    resUtt.sentence->talker.clear();
                    resUtt.sentence->appId = utt->sentence->appId;
                    resUtt.sentence->jobNum = utt->sentence->jobNum;
                    resUtt.sentence->seq = 0;
                    resUtt.audioUrl.clear();
                    resUtt.audioPlayer = 0;
                    resUtt.utType = utResumeMsg;
                    resUtt.isSSML = isSsml(resUtt.sentence->text);
                    resUtt.plugin = m_talkerMgr->talkerToPlugin(resUtt.sentence->talker);
                    resUtt.state = usNone;
                    setInitialUtteranceState(resUtt);
                    it = m_uttQueue.insert(it, resUtt);
                }
            }
        }
        // If a text message...
        if (utt->utType == utText)
        {
            // If job number has changed...
            if (utt->sentence->jobNum != m_lastJobNum)
            {
                // If we finished the last job, append End-of-job to the queue,
                // which will become a textFinished signal when it is processed.
                if (m_lastJobNum)
                {
                    if (m_lastSeq == static_cast<uint>(m_speechData->getTextCount(m_lastJobNum)))
                    {
                        Utt jobUtt;
                        jobUtt.sentence = new mlText;
                        jobUtt.sentence->text.clear();
                        jobUtt.sentence->talker.clear();
                        jobUtt.sentence->appId = m_lastAppId;
                        jobUtt.sentence->jobNum = m_lastJobNum;
                        jobUtt.sentence->seq = 0;
                        jobUtt.audioUrl.clear();
                        jobUtt.utType = utEndOfJob;
                        jobUtt.isSSML = false;
                        jobUtt.plugin = 0;
                        jobUtt.state = usWaitingSignal;
                        m_uttQueue.append(jobUtt);
                    }
                }
                m_lastJobNum = utt->sentence->jobNum;
                m_lastAppId = utt->sentence->appId;
                // If we are at beginning of new job, append Start-of-job to queue,
                // which will become a textStarted signal when it is processed.
                if (utt->sentence->seq == 1)
                {
                    Utt jobUtt;
                    jobUtt.sentence = new mlText;
                    jobUtt.sentence->text.clear();
                    jobUtt.sentence->talker.clear();
                    jobUtt.sentence->appId = m_lastAppId;
                    jobUtt.sentence->jobNum = m_lastJobNum;
                    jobUtt.sentence->seq = utt->sentence->seq;
                    jobUtt.audioUrl.clear();
                    jobUtt.utType = utStartOfJob;
                    jobUtt.isSSML = false;
                    jobUtt.plugin = 0;
                    jobUtt.state = usWaitingSignal;
                    m_uttQueue.append(jobUtt);
                }
            }
            m_lastSeq = utt->sentence->seq;
            // Add the new utterance to the queue.
            m_uttQueue.append(*utt);
        }
        delete utt;
    } else {
        // If no more text to speak, and we finished the last job, issue textFinished signal.
        if (m_lastJobNum)
        {
            if (m_lastSeq == static_cast<uint>(m_speechData->getTextCount(m_lastJobNum)))
            {
                Utt jobUtt;
                jobUtt.sentence = new mlText;
                jobUtt.sentence->text.clear();
                jobUtt.sentence->talker.clear();
                jobUtt.sentence->appId = m_lastAppId;
                jobUtt.sentence->jobNum = m_lastJobNum;
                jobUtt.sentence->seq = 0;
                jobUtt.audioUrl.clear();
                jobUtt.utType = utEndOfJob;
                jobUtt.isSSML = false;
                jobUtt.plugin = 0;
                jobUtt.state = usWaitingSignal;
                m_uttQueue.append(jobUtt);
                gotOne = true;
                ++m_lastSeq; // Don't append another End-of-job
            }
        }
    }

    return gotOne;
}

/**
 * Given an iterator pointing to the m_uttQueue, deletes the utterance
 * from the queue.  If the utterance is currently being processed by a
 * plugin or the Audio Player, halts that operation and deletes Audio Player.
 * Also takes care of deleting temporary audio file.
 * @param it                      Iterator pointer to m_uttQueue.
 * @return                        Iterator pointing to the next utterance in the
 *                                queue, or m_uttQueue.end().
 */
uttIterator Speaker::deleteUtterance(uttIterator it)
{
    switch (it->state)
    {
        case usNone:
        case usWaitingTransform:
        case usWaitingSay:
        case usWaitingSynth:
        case usWaitingSignal:
        case usSynthed:
        case usFinished:
        case usStretched:
            break;

        case usTransforming:
        {
            delete it->transformer;
            it->transformer = 0;
            break;
        }
        case usSaying:
        case usSynthing:
        {
                        // If plugin supports asynchronous mode, and it is busy, halt it.
            PlugInProc* plugin = it->plugin;
            if (it->plugin->supportsAsync())
                if ((plugin->getState() == psSaying) || (plugin->getState() == psSynthing))
            {
                kdDebug() << "Speaker::deleteUtterance calling stopText" << endl;
                plugin->stopText();
            }
            break;
        }
        case usStretching:
        {
            delete it->audioStretcher;
            it->audioStretcher = 0;
            break;
        }
        case usPlaying:
        {
            m_timer->stop();
            it->audioPlayer->stop();
            delete it->audioPlayer;
            break;
        }
        case usPaused:
        case usPreempted:
        {
            // Note: Must call stop(), even if player not currently playing.  Why?
            it->audioPlayer->stop();
            delete it->audioPlayer;
            break;
        }
    }
    if (!it->audioUrl.isNull())
    {
        // If the audio file was generated by a plugin, delete it.
        if (it->plugin)
        {
            if (m_speechData->keepAudio)
            {
                QString seqStr;
                seqStr.sprintf("%08i", it->sentence->seq);    // Zero-fill to 8 chars.
                QString jobStr;
                jobStr.sprintf("%08i", it->sentence->jobNum);
                QString dest = m_speechData->keepAudioPath + "/kttsd-" +
                    QString("%1-%2").arg(jobStr).arg(seqStr) + ".wav";
                QFile::remove(dest);
                QDir d;
                d.rename(it->audioUrl, dest);
                // TODO: This is always producing the following.  Why and how to fix?
                // It moves the files just fine.
                //  kio (KIOJob): stat file:///home/kde-devel/.kde/share/apps/kttsd/audio/kttsd-5-1.wav
                //  kio (KIOJob): error 11 /home/kde-devel/.kde/share/apps/kttsd/audio/kttsd-5-1.wav
                //  kio (KIOJob): This seems to be a suitable case for trying to rename before stat+[list+]copy+del
                // KIO::move(it->audioUrl, dest, false);
            }
            else
                QFile::remove(it->audioUrl);
        }
    }
    // Delete the utterance from queue.
    delete it->sentence;
    return m_uttQueue.erase(it);
}

/**
 * Given an iterator pointing to the m_uttQueue, starts playing audio if
 *   1) An audio file is ready to be played, and
 *   2) It is not already playing.
 * If another audio player is already playing, pauses it before starting
 * the new audio player.
 * @param it                      Iterator pointer to m_uttQueue.
 * @return                        True if an utterance began playing or resumed.
 */
bool Speaker::startPlayingUtterance(uttIterator it)
{
    // kdDebug() << "Speaker::startPlayingUtterance running" << endl;
    if (it->state == usPlaying) return false;
    if (it->audioUrl.isNull()) return false;
    bool started = false;
    // Pause (preempt) any other utterance currently being spoken.
    // If any plugins are audibilizing, must wait for them to finish.
    uttIterator itEnd = m_uttQueue.end();
    for (uttIterator it2 = m_uttQueue.begin(); it2 != itEnd; ++it2)
        if (it2 != it)
    {
        if (it2->state == usPlaying)
        {
            m_timer->stop();
            it2->audioPlayer->pause();
            it2->state = usPreempted;
        }
        if (it2->state == usSaying) return false;
    }
    uttState utState = it->state;
    switch (utState)
    {
        case usNone:
        case usWaitingTransform:
        case usTransforming:
        case usWaitingSay:
        case usWaitingSynth:
        case usWaitingSignal:
        case usSaying:
        case usSynthing:
        case usSynthed:
        case usStretching:
        case usPlaying:
        case usFinished:
            break;

        case usStretched:
        {
                // Don't start playback yet if text job is paused.
            if ((it->utType != utText) ||
                 (m_speechData->getTextJobState(it->sentence->jobNum) != KSpeech::jsPaused))
            {

                it->audioPlayer = createPlayerObject();
                if (it->audioPlayer)
                {
                    it->audioPlayer->startPlay(it->audioUrl);
                        // Set job to speaking state and set sequence number.
                    mlText* sentence = it->sentence;
                    m_currentJobNum = sentence->jobNum;
                    m_speechData->setTextJobState(m_currentJobNum, KSpeech::jsSpeaking);
                    m_speechData->setJobSequenceNum(m_currentJobNum, sentence->seq);
                    prePlaySignals(it);
                    it->state = usPlaying;
                    if (!m_timer->start(timerInterval, FALSE))
                        kdDebug() << "Speaker::startPlayingUtterance: timer.start failed" << endl;
                    started = true;
                } else {
                    // If could not create audio player object, best we can do is silence.
                    it->state = usFinished;
                }
            }
            break;
        }
        case usPaused:
        {
            // Unpause playback only if user has resumed.
            // kdDebug() << "Speaker::startPlayingUtterance: checking whether to resume play" << endl;
            if ((it->utType != utText) ||
                 (m_speechData->getTextJobState(it->sentence->jobNum) != KSpeech::jsPaused))
            {
                // kdDebug() << "Speaker::startPlayingUtterance: resuming play" << endl;
                it->audioPlayer->startPlay(QString());  // resume
                it->state = usPlaying;
                if (!m_timer->start(timerInterval, FALSE))
                    kdDebug() << "Speaker::startPlayingUtterance: timer.start failed" << endl;
                started = true;
            }
            break;
        }

        case usPreempted:
        {
                // Preempted playback automatically resumes.
                // Note: Must call stop(), even if player not currently playing.  Why?
            it->audioPlayer->startPlay(QString());  // resume
            it->state = usPlaying;
            if (!m_timer->start(timerInterval, FALSE))
                kdDebug() << "Speaker::startPlayingUtterance: timer.start failed" << endl;
            started = true;
            break;
        }
    }
    return started;
}

/**
 * Takes care of emitting reading interrupted/resumed and sentence started signals.
 * Should be called just before audibilizing an utterance.
 * @param it                      Iterator pointer to m_uttQueue.
 * It also makes sure the job state is set to jsSpeaking.
 */
void Speaker::prePlaySignals(uttIterator it)
{
    uttType utType = it->utType;
    if (utType == utText)
    {
        // If this utterance is for a regular text message,
        // and it was interrupted, emit reading resumed signal.
        if (m_textInterrupted)
        {
            m_textInterrupted = false;
            emit readingResumed();
        }
        // Set job to speaking state and set sequence number.
        mlText* sentence = it->sentence;
        // Emit sentence started signal.
        emit sentenceStarted(sentence->text, 
                             sentence->talker, sentence->appId,
                             m_currentJobNum, sentence->seq);
    } else {
        // If this utterance is not a regular text message,
        // and we are doing a text job, emit reading interrupted signal.
        if (isSpeakingText())
        {
            m_textInterrupted = true;
            emit readingInterrupted();
        }
    }
}

/**
 * Takes care of emitting sentenceFinished signal.
 * Should be called immediately after an utterance has completed playback.
 * @param it                      Iterator pointer to m_uttQueue.
 */
void Speaker::postPlaySignals(uttIterator it)
{
    uttType utType = it->utType;
    if (utType == utText)
    {
        // If this utterance is for a regular text message,
        // emit sentence finished signal.
        mlText* sentence = it->sentence;
        emit sentenceFinished(sentence->appId,
                              sentence->jobNum, sentence->seq);
    }
}

/**
 * Constructs a temporary filename for plugins to use as a suggested filename
 * for synthesis to write to.
 * @return                        Full pathname of suggested file.
 */
QString Speaker::makeSuggestedFilename()
{
    QString tmpDir = locateLocal("tmp", "kttsd-");
    kdDebug() << "Speaker::makeSuggestedFilename: tmpDir = " << tmpDir << endl;
    KTempFile tempFile (tmpDir, ".wav");
    // TODO: This not working.  Why?
    // QString waveFile = tempFile.file()->name();
    // QString waveFile = mFile.name();
    // tempFile.close();
    QString waveFile = tempFile.name();
    QFile::remove(waveFile);
    kdDebug() << "Speaker::makeSuggestedFilename: Suggesting filename: " << waveFile << endl;
    return KStandardDirs::realFilePath(waveFile);
}

/**
 * Creates and returns a player object based on user option.
 */
Player* Speaker::createPlayerObject()
{
    Player* player = 0;
    QString plugInName;
    switch(m_playerOption)
    {
        case 1 :
            {
                plugInName = "kttsd_gstplugin";
                break;
            }
        case 2 :
            {
                plugInName = "kttsd_alsaplugin";
                break;
            }
        case 3 :
            {
                plugInName = "kttsd_akodeplugin";
                break;
            }
        default:
            {
                plugInName = "kttsd_artsplugin";
                break;
            }
    }
    KTrader::OfferList offers = KTrader::self()->query(
            "KTTSD/AudioPlugin", QString("DesktopEntryName == '%1'").arg(plugInName));

    if(offers.count() == 1)
    {
        kdDebug() << "Speaker::createPlayerObject: Loading " << offers[0]->library() << endl;
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if (factory)
            player = 
                KLibLoader::createInstance<Player>(
                    offers[0]->library().latin1(), this, offers[0]->library().latin1());
    }
    if (player == 0)
    {
        // If we tried for GStreamer or ALSA plugin and failed, fall back to aRts plugin.
        if (m_playerOption != 0)
        {
            kdDebug() << "Speaker::createPlayerObject: Could not load " + plugInName + 
                " plugin.  Falling back to aRts." << endl;
            m_playerOption = 0;
            return createPlayerObject();
        }
        else
            kdDebug() << "Speaker::createPlayerObject: Could not load aRts plugin.  Is KDEDIRS set  correctly?" << endl;
    } else
        // Must have GStreamer >= 0.8.7.  If not, use aRts.
        if (m_playerOption == 1)
        {
            if (!player->requireVersion(0, 8, 7))
            {
                delete player;
                m_playerOption = 0;
                return createPlayerObject();
            }
        }
    if (player) player->setSinkName(m_sinkName);
    return player;
}


/* Slots ==========================================================*/

/**
* Received from PlugIn objects when they finish asynchronous synthesis
* and audibilizing.
*/
void Speaker::slotSayFinished()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 101);
    QApplication::postEvent(this, ev);
}

/**
* Received from PlugIn objects when they finish asynchronous synthesis.
*/
void Speaker::slotSynthFinished()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 102);
    QApplication::postEvent(this, ev);
}

/**
* Received from PlugIn objects when they asynchronously stopText.
*/
void Speaker::slotStopped()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 103);
    QApplication::postEvent(this, ev);
}

/**
* Received from audio stretcher when stretching (speed adjustment) is finished.
*/
void Speaker::slotStretchFinished()
{
    // Convert to postEvent and return immediately.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 104);
    QApplication::postEvent(this, ev);
}

/**
* Received from transformer (SSMLConvert) when transforming is finished.
*/
void Speaker::slotTransformFinished()
{
    // Convert to postEvent and return immediately.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 105);
    QApplication::postEvent(this, ev);
}

/** Received from PlugIn object when they encounter an error.
* @param keepGoing               True if the plugin can continue processing.
*                                False if the plugin cannot continue, for example,
*                                the speech engine could not be started.
* @param msg                     Error message.
*/
void Speaker::slotError(bool /*keepGoing*/, const QString& /*msg*/)
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    // TODO: Do something with error messages.
    /*
    if (keepGoing)
        QCustomEvent* ev = new QCustomEvent(QEvent::User + 106);
    else
        QCustomEvent* ev = new QCustomEvent(QEvent::User + 107);
    QApplication::postEvent(this, ev);
    */
}

/**
* Received from Timer when it fires.
* Check audio player to see if it is finished.
*/
void Speaker::slotTimeout()
{
    uttIterator itEnd = m_uttQueue.end();
    for (uttIterator it = m_uttQueue.begin(); it != itEnd; ++it)
    {
        if (it->state == usPlaying)
        {
            if (it->audioPlayer->playing()) return;  // Still playing.
            m_timer->stop();
            postPlaySignals(it);
            deleteUtterance(it);
            doUtterances();
            return;
        }
    }
}

/**
* Processes events posted by plugins.  When asynchronous plugins emit signals
* they are converted into these events.
*/
bool Speaker::event ( QEvent * e )
{
    // TODO: Do something with event numbers 106 (error; keepGoing=True)
    // and 107 (error; keepGoing=False).
    if ((e->type() >= (QEvent::User + 101)) && (e->type() <= (QEvent::User + 105)))
    {
        // kdDebug() << "Speaker::event: received event." << endl;
        doUtterances();
        return TRUE;
    }
    else return FALSE;
}

