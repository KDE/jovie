/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  
  This class is in charge of getting the messages, warnings and text from
  the queue and calling the plugins to actually speak the texts.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
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

// System includes.

// Qt includes. 
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtGui/QApplication>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kservicetypetrader.h>
#include <kspeech.h>
//#include <kio/job.h>

// KTTS includes.
#include "pluginproc.h"
#include "player.h"
#include "utils.h"
#include "talkercode.h"
#include "stretcher.h"

// KTTSD includes.
#include "speechdata.h"
#include "talkermgr.h"
#include "ssmlconvert.h"

// Speaker includes.
#include "speaker.h"
#include "speaker.moc"


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


class SpeakerPrivate
{
    SpeakerPrivate(SpeechData* speechData, TalkerMgr* talkerMgr) :
        speechData(speechData),
        talkerMgr(talkerMgr),
        configData(NULL),
        exitRequested(false),
        again(false),
        timer(NULL),
        currentJobNum(0)
    {
    }
    
    ~SpeakerPrivate() { }
    
    friend class Speaker;
    
protected:
    /**
    * SpeechData local pointer
    */
    SpeechData* speechData;

    /**
    * TalkerMgr local pointer.
    */
    TalkerMgr* talkerMgr;
    
    /**
    * Configuration Data object.
    */
    ConfigData* configData;

    /**
    * True if the speaker was requested to exit.
    */
    volatile bool exitRequested;

    /**
    * Queue of utterances we are currently processing.
    */
    QList<Utt> uttQueue;

    /**
    * Used to prevent doUtterances from prematurely exiting.
    */
    bool again;

    /**
    * Timer for monitoring audio player.
    */
    QTimer* timer;

    /**
    * Current Text job being played.
    */
    int currentJobNum;

    QMap<KSpeech::JobPriority, SpeechJob*> currentJobs;
};

/* Public Methods ==========================================================*/

Speaker::Speaker(
    SpeechData*speechData,
    TalkerMgr* talkerMgr,
    QObject *parent) :
    
    QObject(parent), 
    d(new SpeakerPrivate(speechData, talkerMgr))
{
    // kDebug() << "Running: Speaker::Speaker()";
    d->timer = new QTimer(this);
    // Connect timer timeout signal.
    connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

    // Connect plugins to slots.
    PlugInList plugins = d->talkerMgr->getLoadedPlugIns();
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
    
    d->currentJobs.insert(KSpeech::jpScreenReaderOutput, NULL);
    d->currentJobs.insert(KSpeech::jpWarning, NULL);
    d->currentJobs.insert(KSpeech::jpMessage, NULL);
    d->currentJobs.insert(KSpeech::jpText, NULL);
}

Speaker::~Speaker(){
    // kDebug() << "Running: Speaker::~Speaker()";
    d->timer->stop();
    delete d->timer;
    if (!d->uttQueue.isEmpty())
    {
        uttIterator it;
        for (it = d->uttQueue.begin(); it != d->uttQueue.end(); )
            it = deleteUtterance(it);
    }
    delete d;
}

void Speaker::setConfigData(ConfigData* configData)
{
    d->configData = configData;
}

void Speaker::requestExit(){
    // kDebug() << "Speaker::requestExit: Running";
    d->exitRequested = true;
}

void Speaker::doUtterances()
{
    // kDebug() << "Running: Speaker::doUtterances()";

    // Used to prevent exiting prematurely.
    d->again = true;

    while(d->again && !d->exitRequested)
    {
        d->again = false;

        if (d->exitRequested)
        {
            // kDebug() << "Speaker::run: exiting due to request 1.";
            return;
        }

        uttIterator it;
        uttIterator itBegin;
        uttIterator itEnd = 0;  // Init to zero to avoid compiler warning.

        // If Screen Reader Output is waiting, we need to process it ASAP.
        d->again = getNextUtterance(KSpeech::jpScreenReaderOutput);

        kDebug() << "Speaker::doUtterances: queue dump:";
        for (it = d->uttQueue.begin(); it != d->uttQueue.end(); ++it)
        {
            QString pluginState = "no plugin";
            if (it->plugin()) pluginState = pluginStateToStr(it->plugin()->getState());
            QString jobState = "no job";
            if (it->job())
                jobState = SpeechJob::jobStateToStr(it->job()->state());
            kDebug() << 
                "  State: " << Utt::uttStateToStr(it->state()) << 
                "," << pluginState <<
                "," << jobState <<
                " Type: " << Utt::uttTypeToStr(it->utType()) << 
                " Text: " << it->sentence() << endl;
         }

        if (!d->uttQueue.isEmpty())
        {
            // Delete utterances that are finished.
            it = d->uttQueue.begin();
            while (it != d->uttQueue.end())
            {
                if (Utt::usFinished == it->state())
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
            itEnd = d->uttQueue.end();
            itBegin = d->uttQueue.begin();
            for (it = itBegin; it != itEnd; ++it)
            {
                // Skip the utterance if application is paused.
                if (!d->speechData->isApplicationPaused(it->appId()))
                {
                    Utt::uttState utState = it->state();
                    Utt::uttType utType = it->utType();
                    switch (utState)
                    {
                        case Utt::usNone:
                        {
                            it->setInitialState();
                            d->again = true;
                            break;
                        }
                        case Utt::usWaitingTransform:
                        {
                            // Create an XSLT transformer and transform the text.
                            SSMLConvert* transformer = new SSMLConvert();
                            it->setTransformer(transformer);
                            connect(transformer, SIGNAL(transformFinished()),
                                this, SLOT(slotTransformFinished()));
                            if (transformer->transform(it->sentence(),
                                it->plugin()->getSsmlXsltFilename()))
                            {
                                it->setState(Utt::usTransforming);
                                ++transformingCnt;
                            }
                            else
                            {
                                // If an error occurs transforming, skip it.
                                it->setState(Utt::usTransforming);
                                it->setInitialState();
                            }
                            d->again = true;
                            break;
                        }
                        case Utt::usTransforming:
                        {
                            // See if transformer is finished.
                            if (it->transformer()->getState() == SSMLConvert::tsFinished)
                            {
                                // Get the transformed text.
                                it->setSentence(it->transformer()->getOutput());
                                // Set next state (usWaitingSynth or usWaitingSay)
                                it->setInitialState();
                                d->again = true;
                                --transformingCnt;
                            }
                            break;
                        }
                        case Utt::usSynthed:
                        {
                            // Don't bother stretching if factor is 1.0.
                            // Don't bother stretching if SSML.
                            // TODO: This is because sox mangles SSML pitch settings.  Would be nice
                            // to figure out how to avoid this.
                            kDebug() << "Speaker::doUtterances: state usSynthed";
                            if (d->configData->audioStretchFactor == 1.0 || it->isSsml())
                            {
                                it->setState(Utt::usStretched);
                                d->again = true;
                            }
                            else
                            {
                                Stretcher* stretcher = new Stretcher();
                                it->setAudioStretcher(stretcher);
                                connect(stretcher, SIGNAL(stretchFinished()),
                                    this, SLOT(slotStretchFinished()));
                                if (stretcher->stretch(it->audioUrl(), makeSuggestedFilename(),
                                    d->configData->audioStretchFactor))
                                {
                                    it->setState(Utt::usStretching);
                                    d->again = true;  // Is this needed?
                                }
                                else
                                {
                                    // If stretch failed, it is most likely caused by sox not being
                                    // installed.  Just skip it.
                                    it->setState(Utt::usStretched);
                                    d->again = true;
                                    delete stretcher;
                                    it->setAudioStretcher(NULL);
                                }
                            }
                            break;
                        }
                        case Utt::usStretching:
                        {
                            // See if Stretcher is finished.
                            Stretcher* stretcher = it->audioStretcher();
                            if (stretcher->getState() == Stretcher::ssFinished)
                            {
                                QFile::remove(it->audioUrl());
                                it->setAudioUrl(stretcher->getOutFilename());
                                it->setState(Utt::usStretched);
                                delete stretcher;
                                it->setAudioStretcher(NULL);
                                d->again = true;
                            }
                            break;
                        }
                        case Utt::usStretched:
                        {
                            kDebug() << "Speaker::doUtterances: state usStretched";
                            // If first in queue, start playback.
                            if (it == itBegin)
                            {
                                if (startPlayingUtterance(it))
                                {
                                    playing = true;
                                    d->again = true;
                                } else {
                                    ++waitingCnt;
                                    if (Utt::utWarning == utType || Utt::utMessage == utType)
                                        ++waitingMsgCnt;
                                }
                            } else {
                                ++waitingCnt;
                                if (Utt::utWarning == utType || Utt::utMessage == utType)
                                    ++waitingMsgCnt;
                            }
                            break;
                        }
                        case Utt::usPlaying:
                        {
                            kDebug() << "Speaker::doUtterances: state usPlaying";
                            playing = true;
                            break;
                        }
                        case Utt::usPaused: 
                        case Utt::usPreempted:
                        {
                            if (!playing) 
                            {
                                if (startPlayingUtterance(it))
                                {
                                    playing = true;
                                    d->again = true;
                                } else {
                                    ++waitingCnt;
                                    if (Utt::utWarning == utType || Utt::utMessage == utType)
                                        ++waitingMsgCnt;
                                }
                            } else {
                                ++waitingCnt;
                                if (Utt::utWarning == utType || Utt::utMessage == utType)
                                    ++waitingMsgCnt;
                            }
                            break;
                        }
                        case Utt::usWaitingSay:
                        {
                            // If first in queue, start it.
                            if (it == itBegin)
                            {
                                    if (it->plugin()->getState() == psIdle)
                                    {
                                        // Set job to speaking state and set sentence number.
                                        d->currentJobNum = it->job()->jobNum();
                                        it->setState(Utt::usSaying);
                                        prePlaySignals(it);
                                        // kDebug() << "Async synthesis and audibilizing.";
                                        playing = true;
                                        it->plugin()->sayText(it->sentence());
                                        d->again = true;
                                    } else {
                                        ++waitingCnt;
                                        if (Utt::utWarning == utType || Utt::utMessage == utType)
                                            ++waitingMsgCnt;
                                    }
                            } else {
                                ++waitingCnt;
                                if (Utt::utWarning == utType || Utt::utMessage == utType)
                                    ++waitingMsgCnt;
                            }
                            break;
                        }
                        case Utt::usWaitingSynth:
                        {
                            // TODO: If the synth is busy and the waiting text is screen
                            // reader output, it would be nice to call the synth's
                            // stopText() method.  However, some of the current plugins
                            // have horrible startup times, so we won't do that for now.
                            kDebug() << "Speaker::doUtterances: state usWaitingSynth";
                            if (it->plugin()->getState() == psIdle)
                            {
                                // kDebug() << "Async synthesis.";
                                it->setState(Utt::usSynthing);
                                ++synthingCnt;
                                it->plugin()->synthText(it->sentence(),
                                    makeSuggestedFilename());
                                d->again = true;
                            }
                            ++waitingCnt;
                            if (Utt::utMessage == utType || Utt::utMessage == utType)
                                ++waitingMsgCnt;
                            break;
                        }
                        case Utt::usSaying:
                        {
                            kDebug() << "Speaker::doUtterances: state usSaying";
                            // See if synthesis and audibilizing is finished.
                            if (it->plugin()->getState() == psFinished)
                            {
                                it->plugin()->ackFinished();
                                it->setState(Utt::usFinished);
                                d->again = true;
                            } else {
                                playing = true;
                                ++waitingCnt;
                                if (Utt::utWarning == utType || Utt::utMessage == utType)
                                    ++waitingMsgCnt;
                            }
                            break;
                        }
                        case Utt::usSynthing:
                        {
                            kDebug() << "Speaker::doUtterances: state usSynthing";
                            // See if synthesis is completed.
                            if (it->plugin()->getState() == psFinished)
                            {
                                it->setAudioUrl(KStandardDirs::realFilePath(it->plugin()->getFilename()));
                                kDebug() << "Speaker::doUtterances: synthesized filename: " << it->audioUrl();
                                it->plugin()->ackFinished();
                                it->setState(Utt::usSynthed);
                                d->again = true;
                            } else ++synthingCnt;
                            ++waitingCnt;
                            if (Utt::utWarning == utType || Utt::utMessage == utType)
                                ++waitingMsgCnt;
                            break;
                        }
                        case Utt::usFinished: break;
                    }
                }
            }
            // See if there are any messages or warnings to process.
            // We keep up to 2 such utterances in the queue.
            if ((waitingMsgCnt < 2) && (transformingCnt < 3))
            {
                if (getNextUtterance(KSpeech::jpWarning))
                    d->again = true;
                else
                    if (getNextUtterance(KSpeech::jpMessage))
                        d->again = true;
            }
            // Try to keep at least two utterances in the queue waiting to be played,
            // and no more than 3 transforming at one time.
            if ((waitingCnt < 2) && (transformingCnt < 3))
                if (getNextUtterance(KSpeech::jpAll))
                    d->again = true;
        } else {
            // See if another utterance is ready to be worked on.
            // If so, loop again since we've got work to do.
            d->again = getNextUtterance(KSpeech::jpAll);
        }
    }
    if (!d->exitRequested)
        d->speechData->deleteExpiredJobs();
    // kDebug() << "Speaker::doUtterances: exiting.";
}

bool Speaker::isSpeaking()
{
    return (KSpeech::jsSpeaking == d->speechData->jobState(d->currentJobNum));
}

int Speaker::getCurrentJobNum() { return d->currentJobNum; }

void Speaker::removeJob(int jobNum)
{
    deleteUtteranceByJobNum(jobNum);
    d->speechData->removeJob(jobNum);
    doUtterances();
}

void Speaker::removeAllJobs(const QString& appId)
{
    AppData* appData = d->speechData->getAppData(appId);
    if (appData->isSystemManager()) {
        uttIterator it = d->uttQueue.begin();
        while (it != d->uttQueue.end())
            it = deleteUtterance(it);
    } else {
        uttIterator it = d->uttQueue.begin();
        while (it != d->uttQueue.end()) {
            if (it->appId() == appId)
                it = deleteUtterance(it);
            else
                ++it;
        }
    }
    d->speechData->removeAllJobs(appId);
    doUtterances();
}

void Speaker::pause(const QString& appId)
{
    Utt* pausedUtt = NULL;
    AppData* appData = d->speechData->getAppData(appId);
    if (appData->isSystemManager()) {
        uttIterator it = d->uttQueue.begin();
        while (it != d->uttQueue.end())
            if (Utt::usPlaying == it->state()) {
                pausedUtt = &(*it);
                break;
            } else
                ++it;
    } else {
        uttIterator it = d->uttQueue.begin();
        while (it != d->uttQueue.end())
            if (it->appId() == appId && Utt::usPlaying == it->state()) {
                pausedUtt = &(*it);
                break;
            } else
                ++it;
    }
              
    if (pausedUtt) {
        Q_ASSERT(d->speechData->isApplicationPaused(pausedUtt->appId()));
        if (pausedUtt->audioPlayer() && pausedUtt->audioPlayer()->playing()) {
            d->timer->stop();
            kDebug() << "Speaker::pause: pausing audio player";
            pausedUtt->audioPlayer()->pause();
            pausedUtt->setState(Utt::usPaused);
            kDebug() << "Speaker::pause: Setting utterance state to usPaused";
            return;
        }
        // Audio player has finished, but timeout hasn't had a chance
        // to clean up.  So do nothing, and let timeout do the cleanup.
        doUtterances();
    }
}

void Speaker::moveJobLater(int jobNum)
{
    SpeechJob* job = d->speechData->findJobByJobNum(jobNum);
    if (job)
        pause(job->appId());
    deleteUtteranceByJobNum(jobNum);
    d->speechData->moveJobLater(jobNum);
    doUtterances();
}

int Speaker::moveRelSentence(int jobNum, int n)
{
    if (0 == n)
        return d->speechData->jobSentenceNum(jobNum);
    else {
        deleteUtteranceByJobNum(jobNum);
        // TODO: More efficient way to advance one or two sentences, since there is a
        // good chance those utterances are already in the queue and synthesized.
        int sentenceNum = d->speechData->moveRelSentence(jobNum, n);
        kDebug() << "Speaker::moveRelTextSentence: job num: " << jobNum << " moved to: " << sentenceNum;
        doUtterances();
        return sentenceNum;
    }
}

/* Private Methods ==========================================================*/

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

void Speaker::deleteUtteranceByJobNum(int jobNum)
{
    uttIterator it = d->uttQueue.begin();
    while (it != d->uttQueue.end())
    {
        if (it->job() && it->job()->jobNum() == jobNum)
            it = deleteUtterance(it);
        else
            ++it;
    }
    if (d->currentJobNum == jobNum) d->currentJobNum = 0;
}

bool Speaker::getNextUtterance(KSpeech::JobPriority requestedPriority)
{
    Utt* utt = NULL;
    QString appId;
    QString sentence;
    Utt::uttType utType;
    KSpeech::JobPriority priority;
    if (KSpeech::jpAll == requestedPriority)
        foreach (priority, d->currentJobs.keys()) {
            d->currentJobs[priority] = d->speechData->getNextSpeakableJob(priority);
            if (d->currentJobs[priority])
                sentence = d->currentJobs[priority]->getNextSentence();
            if (!sentence.isEmpty())
                break;
        }
    else {
        priority = requestedPriority;
        d->currentJobs[priority] = d->speechData->getNextSpeakableJob(priority);
        if (d->currentJobs[priority])
            sentence = d->currentJobs[priority]->getNextSentence();
    }
    if (!sentence.isEmpty()) {
        switch (priority) {
            case KSpeech::jpAll:    // should not happen.
                Q_ASSERT(0);
                break;
            case KSpeech::jpScreenReaderOutput:
                utType = Utt::utScreenReader;
                break;
            case KSpeech::jpWarning:
                utType = Utt::utWarning;
                break;
            case KSpeech::jpMessage:
                utType = Utt::utMessage;
                break;
            case KSpeech::jpText:
                utType = Utt::utText;
                break;
        }
        appId = d->currentJobs[priority]->appId();
        SpeechJob* job = d->currentJobs[priority];
        utt = new Utt(utType, appId, job, sentence, d->talkerMgr->talkerToPlugin(job->talker()));
        utt->setSeq(job->seq());
    }

    bool r = (utt != NULL);

    if (utt)
    {
        // Screen Reader Outputs need to be processed ASAP.
        if (Utt::utScreenReader == utType)
        {
            d->uttQueue.insert(d->uttQueue.begin(), *utt);
            // Delete any other Screen Reader Outputs in the queue.
            // Only one Screen Reader Output at a time.
            uttIterator it = d->uttQueue.begin();
            ++it;
            while (it != d->uttQueue.end())
            {
                if (Utt::utScreenReader == it->utType())
                    it = deleteUtterance(it);
                else
                    ++it;
            }
        }
        // If the new utterance is a Warning or Message...
        if ((Utt::utWarning == utType) || (Utt::utMessage == utType))
        {
            uttIterator itEnd = d->uttQueue.end();
            uttIterator it = d->uttQueue.begin();
            bool interrupting = false;
            if (it != itEnd)
            {
                // New Warnings go after Screen Reader Output, other Warnings,
                // Interruptions, and in-process text,
                // but before Resumes, waiting text or signals.
                if (Utt::utWarning == utType)
                    while ( it != itEnd && 
                            ((Utt::utScreenReader == it->utType()) || 
                            (Utt::utWarning == it->utType()) ||
                            (Utt::utInterruptMsg == it->utType()) ||
                            (Utt::utInterruptSnd == it->utType()))) ++it;
                // New Messages go after Screen Reader Output, Warnings, other Messages,
                // Interruptions, and in-process text,
                // but before Resumes, waiting text or signals.
                if (Utt::utMessage == utType)
                    while ( it != itEnd && 
                            ((Utt::utScreenReader == it->utType()) ||
                            (Utt::utWarning == it->utType()) ||
                            (Utt::utMessage == it->utType()) ||
                            (Utt::utInterruptMsg == it->utType()) ||
                            (Utt::utInterruptSnd == it->utType()))) ++it;
                if (it != itEnd)
                    if (Utt::utText == it->utType() &&
                        ((Utt::usPlaying == it->state()) ||
                        (Utt::usSaying == it->state()))) ++it;
                // If now pointing at a text message, we are interrupting.
                // Insert optional Interruption message and sound.
                if (it != itEnd) interrupting = (Utt::utText == it->utType() && Utt::usPaused != it->state());
                if (interrupting)
                {
                    if (d->configData->textPreSndEnabled)
                    {
                        Utt intrUtt(Utt::utInterruptSnd, appId, d->configData->textPreSnd);
                        it = d->uttQueue.insert(it, intrUtt);
                        ++it;
                    }
                    if (d->configData->textPreMsgEnabled)
                    {
                        Utt intrUtt(Utt::utInterruptMsg, appId, NULL, d->configData->textPreMsg, d->talkerMgr->talkerToPlugin(""));;
                        it = d->uttQueue.insert(it, intrUtt);
                        ++it;
                    }
                }
            }
            // Insert the new message or warning.
            it = d->uttQueue.insert(it, *utt);
            ++it;
            // Resumption message and sound.
            if (interrupting)
            {
                if (d->configData->textPostSndEnabled)
                {
                    Utt resUtt(Utt::utResumeSnd, appId, d->configData->textPostSnd);
                    it = d->uttQueue.insert(it, resUtt);
                    ++it;
                }
                if (d->configData->textPostMsgEnabled)
                {
                    Utt resUtt(Utt::utResumeMsg, appId, NULL, d->configData->textPostMsg, d->talkerMgr->talkerToPlugin(""));
                    it = d->uttQueue.insert(it, resUtt);
                }
            }
        }
        // If a text message...
        if (Utt::utText == utt->utType())
            d->uttQueue.append(*utt);
    }

    return r;
}

uttIterator Speaker::deleteUtterance(uttIterator it)
{
    switch (it->state())
    {
        case Utt::usNone:
        case Utt::usWaitingTransform:
        case Utt::usWaitingSay:
        case Utt::usWaitingSynth:
        case Utt::usSynthed:
        case Utt::usFinished:
        case Utt::usStretched:
            break;

        case Utt::usTransforming:
            delete it->transformer();
            break;
        case Utt::usSaying:
        case Utt::usSynthing:
        {
            // If plugin supports asynchronous mode, and it is busy, halt it.
            PlugInProc* plugin = it->plugin();
            if (plugin->supportsAsync())
                if ((plugin->getState() == psSaying) || (plugin->getState() == psSynthing))
                {
                    kDebug() << "Speaker::deleteUtterance calling stopText";
                    plugin->stopText();
                }
            break;
        }
        case Utt::usStretching:
            delete it->audioStretcher();
            break;
        case Utt::usPlaying:
            d->timer->stop();
            it->audioPlayer()->stop();
            delete it->audioPlayer();
            break;
        case Utt::usPaused:
        case Utt::usPreempted:
            // Note: Must call stop(), even if player not currently playing.  Why?
            it->audioPlayer()->stop();
            delete it->audioPlayer();
            break;
    }
    if (!it->audioUrl().isNull())
    {
        // If the audio file was generated by a plugin, delete it.
        if (it->plugin())
        {
            if (d->configData->keepAudio)
            {
                QString seqStr;
                seqStr.sprintf("%08i", it->seq());    // Zero-fill to 8 chars.
                QString jobStr;
                jobStr.sprintf("%08i", it->job()->jobNum());
                QString dest = d->configData->keepAudioPath + "/kttsd-" +
                    QString("%1-%2").arg(jobStr).arg(seqStr) + ".wav";
                QFile::remove(dest);
                QDir d;
                d.rename(it->audioUrl(), dest);
                // TODO: This is always producing the following.  Why and how to fix?
                // It moves the files just fine.
                //  kio (KIOJob): stat file:///home/kde-devel/.kde/share/apps/kttsd/audio/kttsd-5-1.wav
                //  kio (KIOJob): error 11 /home/kde-devel/.kde/share/apps/kttsd/audio/kttsd-5-1.wav
                //  kio (KIOJob): This seems to be a suitable case for trying to rename before stat+[list+]copy+del
                // KIO::move(it->audioUrl, dest, false);
            }
            else
                QFile::remove(it->audioUrl());
        }
    }
    // Delete the utterance from queue.
    return d->uttQueue.erase(it);
}

bool Speaker::startPlayingUtterance(uttIterator it)
{
    // kDebug() << "Speaker::startPlayingUtterance running";
    if (Utt::usPlaying == it->state()) return false;
    if (it->audioUrl().isEmpty()) return false;
    bool started = false;
    // Pause (preempt) any other utterance currently being spoken.
    // If any plugins are audibilizing, must wait for them to finish.
    uttIterator itEnd = d->uttQueue.end();
    for (uttIterator it2 = d->uttQueue.begin(); it2 != itEnd; ++it2)
        if (it2 != it)
        {
            if (Utt::usPlaying == it2->state())
            {
                d->timer->stop();
                it2->audioPlayer()->pause();
                it2->setState(Utt::usPreempted);
            }
            if (Utt::usSaying == it2->state()) return false;
        }
    Utt::uttState utState = it->state();
    switch (utState)
    {
        case Utt::usNone:
        case Utt::usWaitingTransform:
        case Utt::usTransforming:
        case Utt::usWaitingSay:
        case Utt::usWaitingSynth:
        case Utt::usSaying:
        case Utt::usSynthing:
        case Utt::usSynthed:
        case Utt::usStretching:
        case Utt::usPlaying:
        case Utt::usFinished:
            break;

        case Utt::usStretched:
            it->setAudioPlayer(createPlayerObject());
            if (it->audioPlayer()) {
                it->audioPlayer()->startPlay(it->audioUrl());
                // Set job to speaking state and set sequence number.
                d->currentJobNum = it->job()->jobNum();
                it->setState(Utt::usPlaying);
                prePlaySignals(it);
                d->timer->start(timerInterval);
                started = true;
            } else {
                // If could not create audio player object, best we can do is silence.
                it->setState(Utt::usFinished);
            }
            break;

        case Utt::usPaused:
            // kDebug() << "Speaker::startPlayingUtterance: resuming play";
            it->audioPlayer()->startPlay(QString());  // resume
            it->setState(Utt::usPlaying);
            d->timer->start(timerInterval);
            started = true;
            break;

        case Utt::usPreempted:
            // Preempted playback automatically resumes.
            it->audioPlayer()->startPlay(QString());  // resume
            it->setState(Utt::usPlaying);
            d->timer->start(timerInterval);
            started = true;
            break;
    }
    return started;
}

void Speaker::prePlaySignals(uttIterator it)
{
    if (it->job())
        emit marker(it->job()->appId(), it->job()->jobNum(), KSpeech::mtSentenceBegin, QString::number(it->seq()));
}

void Speaker::postPlaySignals(uttIterator it)
{
    if (it->job())
        emit marker(it->job()->appId(), it->job()->jobNum(), KSpeech::mtSentenceEnd, QString::number(it->seq()));
}

QString Speaker::makeSuggestedFilename()
{
    KTemporaryFile *tempFile = new KTemporaryFile();
    tempFile->setPrefix("kttsd-");
    tempFile->setSuffix(".wav");
    tempFile->open();
    QString waveFile = tempFile->fileName();
    delete tempFile;
    kDebug() << "Speaker::makeSuggestedFilename: Suggesting filename: " << waveFile;
    return KStandardDirs::realFilePath(waveFile);
}

Player* Speaker::createPlayerObject()
{
    Player* player = 0;
    QString plugInName;
    switch(d->configData->playerOption)
    {
        case 0 :
            {
                plugInName = "kttsd_phononplugin";
                break;
            }
        case 2 :
            {
                plugInName = "kttsd_alsaplugin";
                break;
            }
        default:
            {
                // TODO: Default to Phonon.
                plugInName = "kttsd_phononplugin";
                break;
            }
    }
	KService::List offers = KServiceTypeTrader::self()->query(
            "KTTSD/AudioPlugin", QString("DesktopEntryName == '%1'").arg(plugInName));

    if(offers.count() == 1)
    {
        kDebug() << "Speaker::createPlayerObject: Loading " << offers[0]->library();
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().toLatin1());
        if (factory)
            player = 
                KLibLoader::createInstance<Player>(
                    offers[0]->library().toLatin1(), this, QStringList(offers[0]->library().toLatin1()));
    }
    if (player == 0)
    {
        // If we failed, fall back to default plugin.
        // Default to Phonon.
        if (d->configData->playerOption != 0)
        {
            kDebug() << "Speaker::createPlayerObject: Could not load " + plugInName + 
                " plugin.  Falling back to KDE (Phonon)." << endl;
            d->configData->playerOption = 0;
            return createPlayerObject();
        }
        else
            kDebug() << "Speaker::createPlayerObject: Could not load KDE (Phonon) plugin.  Is KDEDIRS set  correctly?";
    }
    if (player) {
        player->setSinkName(d->configData->sinkName);
        player->setPeriodSize(d->configData->periodSize);
        player->setPeriods(d->configData->periods);
        player->setDebugLevel(d->configData->playerDebugLevel);
    }
    return player;
}


/* Slots ==========================================================*/

/**
* Received from PlugIn objects when they finish asynchronous synthesis
* and audibilizing.
* TODO: In Qt4, custom events are no longer necessary as events may pass
*       through thread boundaries now.
*/
void Speaker::slotSayFinished()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QEvent* ev = new QEvent(QEvent::Type(QEvent::User + 101));
    QApplication::postEvent(this, ev);
}

/**
* Received from PlugIn objects when they finish asynchronous synthesis.
*/
void Speaker::slotSynthFinished()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QEvent* ev = new QEvent(QEvent::Type(QEvent::User + 102));
    QApplication::postEvent(this, ev);
}

/**
* Received from PlugIn objects when they asynchronously stopText.
*/
void Speaker::slotStopped()
{
    // Since this signal handler may be running from a plugin's thread,
    // convert to postEvent and return immediately.
    QEvent* ev = new QEvent(QEvent::Type(QEvent::User + 103));
    QApplication::postEvent(this, ev);
}

/**
* Received from audio stretcher when stretching (speed adjustment) is finished.
*/
void Speaker::slotStretchFinished()
{
    // Convert to postEvent and return immediately.
    QEvent* ev = new QEvent(QEvent::Type(QEvent::User + 104));
    QApplication::postEvent(this, ev);
}

/**
* Received from transformer (SSMLConvert) when transforming is finished.
*/
void Speaker::slotTransformFinished()
{
    // Convert to postEvent and return immediately.
    QEvent* ev = new QEvent(QEvent::Type(QEvent::User + 105));
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
    uttIterator itEnd = d->uttQueue.end();
    for (uttIterator it = d->uttQueue.begin(); it != itEnd; ++it)
    {
        if (Utt::usPlaying == it->state())
        {
            if (it->audioPlayer()->playing())
                return;  // Still playing.
            else {
                d->timer->stop();
                postPlaySignals(it);
                it->setState(Utt::usFinished);
                doUtterances();
                return;
            }
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
        // kDebug() << "Speaker::event: received event.";
        doUtterances();
        return true;
    }
    else return false;
}
