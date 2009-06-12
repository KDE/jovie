/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  
  This class is in charge of getting the messages, warnings and text from
  the queue and calling speech-dispatcher to actually speak the texts.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jeremy@scitools.com>
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

// Speaker includes.
#include "speaker.h"
#include "speaker.moc"

// System includes.

// Qt includes. 
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtDBus/QtDBus>

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
#include "utils.h"
#include "talkercode.h"

// KTTSD includes.
#include "talkermgr.h"
#include "ssmlconvert.h"


/**
* The Speaker class manages all speech requests coming in from DBus, does any
* filtering, and passes the resulting text on to speech-dispatcher through its
* C api.
*
* The message queues are maintained in speech-dispatcher itself.
*
* Text jobs in the text queue each have a state (queued, speakable,
* speaking, paused, finished).
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
*/


class SpeakerPrivate
{
    SpeakerPrivate() :
        configData(NULL),
        exitRequested(false),
        again(false),
        currentJobNum(0),
        connection(NULL),
        lastJobNum(0),
        filterMgr(NULL),
        supportsHTML(false)
    {
        if (!ConnectToSpeechd())
            kError() << "could not get a connection to speech-dispatcher"<< endl;

        filterMgr = new FilterMgr();
        filterMgr->init();
    }
    
    ~SpeakerPrivate()
    {
        spd_close(connection);
        connection = NULL;
        
        // from speechdata class
        // kDebug() << "Running: SpeechDataPrivate::~SpeechDataPrivate";
        // Walk through jobs and emit jobStateChanged signal for each job.
        foreach (SpeechJob* job, allJobs)
            delete job;
        allJobs.clear();

        delete filterMgr;

        foreach (AppData* applicationData, appData)
            delete applicationData;
        appData.clear();
    }
    
    friend class Speaker;
    
protected:

    bool ConnectToSpeechd()
    {
        bool retval = false;
        connection = spd_open("kttsd", "main", NULL, SPD_MODE_THREADED);
        if (connection != NULL)
        {
            kDebug() << "successfully opened connection to speech dispatcher";
            connection->callback_begin = connection->callback_end = 
                connection->callback_cancel = connection->callback_pause = 
                connection->callback_resume = Speaker::speechdCallback;

            spd_set_notification_on(connection, SPD_BEGIN);
            spd_set_notification_on(connection, SPD_END);
            spd_set_notification_on(connection, SPD_CANCEL);
            spd_set_notification_on(connection, SPD_PAUSE);
            spd_set_notification_on(connection, SPD_RESUME);
            char ** modulenames = spd_list_modules(connection);
            while (modulenames != NULL && modulenames[0] != NULL)
            {
                outputModules << modulenames[0];
                modulenames++;
                kDebug() << "added module " << outputModules.last();
            }
            retval = true;
        }
        return retval;
    }

    // try to reconnect to speech-dispatcher, return true on success
    bool reconnect()
    {
        spd_close(connection);
        return ConnectToSpeechd();
    }
    
    /**
    * Configuration Data object.
    */
    ConfigData* configData;

    /**
    * True if the speaker was requested to exit.
    */
    volatile bool exitRequested;

    /**
    * list of output modules speech-dispatcher has
    */
    QStringList outputModules;

    /**
    * Used to prevent doUtterances from prematurely exiting.
    */
    bool again;

    /**
    * Current Text job being played.
    */
    int currentJobNum;

    SPDConnection * connection;
    
    // from speechdata class
    /**
    * All jobs.
    */
    QHash<int, SpeechJob*> allJobs;

    /**
    * Application data.
    */
    mutable QMap<QString, AppData*> appData;

    /**
    * The last job queued by any App.
    */
    int lastJobNum;

    /**
    * the filter manager
    */
    FilterMgr * filterMgr;

    /**
    * Job counter.  Each new job increments this counter.
    */
    int jobCounter;

    /**
    * True if at least one XML Transformer plugin for html is enabled.
    */
    bool supportsHTML;
};

/* Public Methods ==========================================================*/

Speaker * Speaker::m_instance = NULL;

Speaker * Speaker::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new Speaker();
    }
    return m_instance;
}

void Speaker::speechdCallback(size_t msg_id, size_t /*client_id*/, SPDNotificationType type)
{
    kDebug() << "speechdCallback called with messageid: " << msg_id << " and type: " << type;
    SpeechJob * job = Speaker::Instance()->d->allJobs[msg_id];
    if (job) {
        switch (type) {
            case SPD_EVENT_BEGIN:
                job->setState(KSpeech::jsSpeaking);
                break;
            case SPD_EVENT_END:
                job->setState(KSpeech::jsFinished);
                break;
            case SPD_EVENT_INDEX_MARK:
                break;
            case SPD_EVENT_CANCEL:
                job->setState(KSpeech::jsDeleted);
                break;
            case SPD_EVENT_PAUSE:
                job->setState(KSpeech::jsPaused);
                break;
            case SPD_EVENT_RESUME:
                job->setState(KSpeech::jsSpeaking);
                break;
        }
    }
    else
    {
        kDebug() << "job number " << msg_id << " not in allJobs yet ";
    }
}

Speaker::Speaker() :
    d(new SpeakerPrivate())
{
    // kDebug() << "Running: Speaker::Speaker()";
    // Connect ServiceUnregistered signal from DBUS so we know when apps have exited.
    connect (QDBusConnection::sessionBus().interface(), SIGNAL(serviceUnregistered(const QString&)),
        this, SLOT(slotServiceUnregistered(const QString&)));
}

Speaker::~Speaker(){
    kDebug() << "Running: Speaker::~Speaker()";
    delete d;
}

void Speaker::setConfigData(ConfigData* configData)
{
    d->configData = configData;

    // from speechdata

    // Create an initial FilterMgr for the pool to save time later.
    delete d->filterMgr;
    d->filterMgr = new FilterMgr();
    d->filterMgr->init();
    d->supportsHTML = d->filterMgr->supportsHTML();
}

AppData* Speaker::getAppData(const QString& appId) const
{
    if (!d->appData.contains(appId))
        d->appData.insert(appId, new AppData(appId));
    return d->appData[appId];
}

void Speaker::releaseAppData(const QString& appId)
{
    if (d->appData.contains(appId))
        delete d->appData.take(appId);
}

bool Speaker::isSsml(const QString &text)
{
    /// This checks to see if the root tag of the text is a <speak> tag.
    QDomDocument ssml;
    ssml.setContent(text, false);  // No namespace processing.
    /// Check to see if this is SSML
    QDomElement root = ssml.documentElement();
    return (root.tagName() == "speak");
}

QStringList Speaker::moduleNames()
{
    return d->outputModules;
}

QStringList Speaker::parseText(const QString &text, const QString &appId /*=NULL*/)
{
    // There has to be a better way
    // kDebug() << "I'm getting: "<< text << " from application " << appId;
    if (isSsml(text)) {
        QStringList tempList(text);
        return tempList;
    }
    // See if app has specified a custom sentence delimiter and use it, otherwise use default.
    QRegExp sentenceDelimiter(getAppData(appId)->sentenceDelimiter());
    QString temp = text;
    // Replace spaces, tabs, and formfeeds with a single space.
    temp.replace(QRegExp("[ \\t\\f]+"), " ");
    // Replace sentence delimiters with tab.
    temp.replace(sentenceDelimiter, "\\1\t");
    // Replace remaining newlines with spaces.
    temp.replace('\n',' ');
    temp.replace('\r',' ');
    // Remove leading spaces.
    temp.replace(QRegExp("\\t +"), "\t");
    // Remove trailing spaces.
    temp.replace(QRegExp(" +\\t"), "\t");
    // Remove blank lines.
    temp.replace(QRegExp("\t\t+"),"\t");
    // Split into sentences.
    QStringList tempList = temp.split( '\t', QString::SkipEmptyParts);

//    for ( QStringList::Iterator it = tempList.begin(); it != tempList.end(); ++it ) {
//        kDebug() << "'" << *it << "'";
//    }
    return tempList;
}

int Speaker::say(const QString& appId, const QString& text, int sayOptions)
{
    int jobNum = -1;
    QString filteredText = text;

    AppData* appData = getAppData(appId);
    KSpeech::JobPriority priority = appData->defaultPriority();
    //kDebug() << "Speaker::say priority = " << priority;
    //kDebug() << "Running: Speaker::say appId = " << appId << " text = " << text;
    //QString talker = appData->defaultTalker();

    SPDPriority spdpriority = SPD_PROGRESS; // default to least priority
    switch (priority)
    {
        case KSpeech::jpScreenReaderOutput: /**< Screen Reader job. SPD_IMPORTANT */
            spdpriority = SPD_IMPORTANT;
            break;
        case KSpeech::jpWarning: /**< Warning job. SPD_NOTIFICATION */
            spdpriority = SPD_NOTIFICATION;
            break;
        case KSpeech::jpMessage: /**< Message job.SPD_MESSAGE */
            spdpriority = SPD_MESSAGE;
            break;
        case KSpeech::jpText: /**< Text job. SPD_TEXT */
            spdpriority = SPD_TEXT;
            break;
        case KSpeech::jpProgress: /**< Progress report. SPD_PROGRESS added KDE 4.4 */
            spdpriority = SPD_PROGRESS;
            break;
    }

    SpeechJob* job = new SpeechJob(priority);

    if (appData->filteringOn()) {
        filteredText = d->filterMgr->convert(text, NULL, appId);
        job->setSentences(QStringList(filteredText));
    }
    else
    {
        job->setSentences(QStringList(filteredText));
    }

    bool failedconnect = false;
    while (jobNum == -1 && !failedconnect)
    {
        switch (sayOptions)
        {
            case KSpeech::soNone: /**< No options specified.  Autodetected. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soPlainText: /**< The text contains plain text. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soHtml: /**< The text contains HTML markup. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soSsml: /**< The text contains SSML markup. */
                spd_set_data_mode(d->connection, SPD_DATA_SSML);
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                spd_set_data_mode(d->connection, SPD_DATA_TEXT);
                break;
            case KSpeech::soChar: /**< The text should be spoken as individual characters. */
                spd_set_spelling(d->connection, SPD_SPELL_ON);
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                spd_set_spelling(d->connection, SPD_SPELL_OFF);
                break;
            case KSpeech::soKey: /**< The text contains a keyboard symbolic key name. */
                jobNum = spd_key(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soSoundIcon: /**< The text is the name of a sound icon. */
                jobNum = spd_sound_icon(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
        }
        if (jobNum == -1 && !failedconnect)
        {
            // job failure
            // try to reconnect once
            kDebug() << "trying to reconnect to speech dispatcher";
            if (!d->reconnect())
            {
                failedconnect = true;
            }
        }
    }

    if (jobNum != -1)
    {
        kDebug() << "saying post filtered text: " << filteredText;
    }

    job->setJobNum(jobNum);
    job->setAppId(appId);
    //job->setTalker(talker);
    //// Note: Set state last so job is fully populated when jobStateChanged signal is emitted.
    d->allJobs.insert(jobNum, job);
    appData->jobList()->append(jobNum);
    d->lastJobNum = jobNum;

    return jobNum;
}

SpeechJob* Speaker::findLastJobByAppId(const QString& appId) const
{
    int jobNum = findJobNumByAppId(appId);
    if (jobNum)
        return d->allJobs[jobNum];
    else
        return NULL;
}

int Speaker::findJobNumByAppId(const QString& appId) const
{
    if (appId.isEmpty())
        return d->lastJobNum;
    else
        return getAppData(appId)->lastJobNum();
}

/**
* Given a jobNum, returns the first job with that jobNum.
* @return               Pointer to the job.
* If no such job, returns 0.
*/
SpeechJob* Speaker::findJobByJobNum(int jobNum) const
{
    if (d->allJobs.contains(jobNum))
        return d->allJobs[jobNum];
    else
        return NULL;
}

QString Speaker::getAppIdByJobNum(int jobNum) const
{
    QString appId;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        appId = job->appId();
    return appId;
}

void Speaker::requestExit(){
    // kDebug() << "Speaker::requestExit: Running";
    d->exitRequested = true;
}

//void Speaker::doUtterances()
//{
    //// kDebug() << "Running: Speaker::doUtterances()";

    //// Used to prevent exiting prematurely.
    //d->again = true;

    //while(d->again && !d->exitRequested)
    //{
    //    d->again = false;

    //    if (d->exitRequested)
    //    {
    //        // kDebug() << "Speaker::run: exiting due to request 1.";
    //        return;
    //    }

    //    uttIterator it;
    //    uttIterator itBegin;
    //    uttIterator itEnd = 0;  // Init to zero to avoid compiler warning.

    //    // If Screen Reader Output is waiting, we need to process it ASAP.
    //    d->again = getNextUtterance(KSpeech::jpScreenReaderOutput);

    //    kDebug() << "Speaker::doUtterances: queue dump:";
    //    for (it = d->uttQueue.begin(); it != d->uttQueue.end(); ++it)
    //    {
    //        QString jobState = "no job";
    //        if (it->job())
    //            jobState = SpeechJob::jobStateToStr(it->job()->state());
    //        kDebug() << 
    //            "  State: " << Utt::uttStateToStr(it->state()) << 
    //            "," << jobState <<
    //            " Type: " << Utt::uttTypeToStr(it->utType()) << 
    //            " Text: " << it->sentence() << endl;
    //     }

    //    if (!d->uttQueue.isEmpty())
    //    {
    //        // Delete utterances that are finished.
    //        it = d->uttQueue.begin();
    //        while (it != d->uttQueue.end())
    //        {
    //            if (Utt::usFinished == it->state())
    //                it = deleteUtterance(it);
    //            else
    //                ++it;
    //        }
    //        // Loop through utterance queue.
    //        int waitingCnt = 0;
    //        int waitingMsgCnt = 0;
    //        int transformingCnt = 0;
    //        bool playing = false;
    //        int synthingCnt = 0;
    //        itEnd = d->uttQueue.end();
    //        itBegin = d->uttQueue.begin();
    //        for (it = itBegin; it != itEnd; ++it)
    //        {
    //            // Skip the utterance if application is paused.
    //            if (!Speaker::Instance()->isApplicationPaused(it->appId()))
    //            {
    //                Utt::uttState utState = it->state();
    //                Utt::uttType utType = it->utType();
    //                switch (utState)
    //                {
    //                    case Utt::usNone:
    //                    {
    //                        it->setInitialState();
    //                        d->again = true;
    //                        break;
    //                    }
    //                    case Utt::usWaitingTransform:
    //                    {
    //                        // Create an XSLT transformer and transform the text.
    //                        SSMLConvert* transformer = new SSMLConvert();
    //                        it->setTransformer(transformer);
    //                        connect(transformer, SIGNAL(transformFinished()),
    //                            this, SLOT(slotTransformFinished()));
    //                        if (transformer->transform(it->sentence(),
    //                            it->plugin()->getSsmlXsltFilename()))
    //                        {
    //                            it->setState(Utt::usTransforming);
    //                            ++transformingCnt;
    //                        }
    //                        else
    //                        {
    //                            // If an error occurs transforming, skip it.
    //                            it->setState(Utt::usTransforming);
    //                            it->setInitialState();
    //                        }
    //                        d->again = true;
    //                        break;
    //                    }
    //                    case Utt::usTransforming:
    //                    {
    //                        // See if transformer is finished.
    //                        if (it->transformer()->getState() == SSMLConvert::tsFinished)
    //                        {
    //                            // Get the transformed text.
    //                            it->setSentence(it->transformer()->getOutput());
    //                            // Set next state (usWaitingSynth or usWaitingSay)
    //                            it->setInitialState();
    //                            d->again = true;
    //                            --transformingCnt;
    //                        }
    //                        break;
    //                    }
    //                    case Utt::usPlaying:
    //                    {
    //                        kDebug() << "Speaker::doUtterances: state usPlaying";
    //                        playing = true;
    //                        break;
    //                    }
    //                    case Utt::usPaused: 
    //                    case Utt::usPreempted:
    //                    {
    //                        if (!playing) 
    //                        {
    //                            if (startPlayingUtterance(it))
    //                            {
    //                                playing = true;
    //                                d->again = true;
    //                            } else {
    //                                ++waitingCnt;
    //                                if (Utt::utWarning == utType || Utt::utMessage == utType)
    //                                    ++waitingMsgCnt;
    //                            }
    //                        } else {
    //                            ++waitingCnt;
    //                            if (Utt::utWarning == utType || Utt::utMessage == utType)
    //                                ++waitingMsgCnt;
    //                        }
    //                        break;
    //                    }
    //                    case Utt::usWaitingSay:
    //                    {
    //                        // If first in queue, start it.
    //                        if (it == itBegin)
    //                        {
    //                                if (it->plugin()->getState() == psIdle)
    //                                {
    //                                    // Set job to speaking state and set sentence number.
    //                                    d->currentJobNum = it->job()->jobNum();
    //                                    it->setState(Utt::usSaying);
    //                                    prePlaySignals(it);
    //                                    // kDebug() << "Async synthesis and audibilizing.";
    //                                    playing = true;
    //                                    it->plugin()->sayText(it->sentence());
    //                                    d->again = true;
    //                                } else {
    //                                    ++waitingCnt;
    //                                    if (Utt::utWarning == utType || Utt::utMessage == utType)
    //                                        ++waitingMsgCnt;
    //                                }
    //                        } else {
    //                            ++waitingCnt;
    //                            if (Utt::utWarning == utType || Utt::utMessage == utType)
    //                                ++waitingMsgCnt;
    //                        }
    //                        break;
    //                    }
    //                    case Utt::usSaying:
    //                    {
    //                        kDebug() << "Speaker::doUtterances: state usSaying";
    //                        // See if synthesis and audibilizing is finished.
    //                        if (it->plugin()->getState() == psFinished)
    //                        {
    //                            it->plugin()->ackFinished();
    //                            it->setState(Utt::usFinished);
    //                            d->again = true;
    //                        } else {
    //                            playing = true;
    //                            ++waitingCnt;
    //                            if (Utt::utWarning == utType || Utt::utMessage == utType)
    //                                ++waitingMsgCnt;
    //                        }
    //                        break;
    //                    }
    //                    case Utt::usFinished: break;
    //                }
    //            }
    //        }
    //        // See if there are any messages or warnings to process.
    //        // We keep up to 2 such utterances in the queue.
    //        if ((waitingMsgCnt < 2) && (transformingCnt < 3))
    //        {
    //            if (getNextUtterance(KSpeech::jpWarning))
    //                d->again = true;
    //            else
    //                if (getNextUtterance(KSpeech::jpMessage))
    //                    d->again = true;
    //        }
    //        // Try to keep at least two utterances in the queue waiting to be played,
    //        // and no more than 3 transforming at one time.
    //        if ((waitingCnt < 2) && (transformingCnt < 3))
    //            if (getNextUtterance(KSpeech::jpAll))
    //                d->again = true;
    //    } else {
    //        // See if another utterance is ready to be worked on.
    //        // If so, loop again since we've got work to do.
    //        d->again = getNextUtterance(KSpeech::jpAll);
    //    }
    //}
    //if (!d->exitRequested)
    //    Speaker::Instance()->deleteExpiredJobs();
    //// kDebug() << "Speaker::doUtterances: exiting.";
//}

bool Speaker::isSpeaking()
{
    return (KSpeech::jsSpeaking == jobState(d->currentJobNum));
}

int Speaker::getCurrentJobNum()
{ 
    return d->currentJobNum;
}

//void Speaker::moveJobLater(int jobNum)
//{
//    SpeechJob* job = SpeechData::Instance()->3(jobNum);
//    if (job)
//        pause(job->appId());
//    deleteUtteranceByJobNum(jobNum);
//    SpeechData::Instance()->moveJobLater(jobNum);
//    doUtterances();
//}

//int Speaker::moveRelSentence(int jobNum, int n)
//{
//    if (0 == n)
//        return SpeechData::Instance()->jobSentenceNum(jobNum);
//    else {
//        deleteUtteranceByJobNum(jobNum);
//        // TODO: More efficient way to advance one or two sentences, since there is a
//        // good chance those utterances are already in the queue and synthesized.
//        int sentenceNum = SpeechData::Instance()->moveRelSentence(jobNum, n);
//        kDebug() << "Speaker::moveRelTextSentence: job num: " << jobNum << " moved to: " << sentenceNum;
//        doUtterances();
//        return sentenceNum;
//    }
//}

/* Private Methods ==========================================================*/

//QString Speaker::pluginStateToStr(pluginState state)
//{
//    switch( state )
//    {
//        case psIdle:         return "psIdle";
//        case psSaying:       return "psSaying";
//        case psSynthing:     return "psSynthing";
//        case psFinished:     return "psFinished";
//    }
//    return QString();
//}

//void Speaker::deleteUtteranceByJobNum(int jobNum)
//{
//    uttIterator it = d->uttQueue.begin();
//    while (it != d->uttQueue.end())
//    {
//        if (it->job() && it->job()->jobNum() == jobNum)
//            it = deleteUtterance(it);
//        else
//            ++it;
//    }
//    if (d->currentJobNum == jobNum) d->currentJobNum = 0;
//}

//bool Speaker::getNextUtterance(KSpeech::JobPriority requestedPriority)
//{
//    //Utt* utt = NULL;
//    //QString appId;
//    //QString sentence;
//    //Utt::uttType utType;
//    //KSpeech::JobPriority priority;
//    //if (KSpeech::jpAll == requestedPriority) {
//	////As the variabele priority is used further on, we can't make it a reference type
//    //    foreach (priority, d->currentJobs.keys()){ //krazy:exclude=foreach
//    //        d->currentJobs[priority] = SpeechData::Instance()->getNextSpeakableJob(priority);
//    //        if (d->currentJobs[priority])
//    //            sentence = d->currentJobs[priority]->getNextSentence();
//    //        if (!sentence.isEmpty())
//    //            break;
//	//}
//    //}
//    //else {
//    //    priority = requestedPriority;
//    //    d->currentJobs[priority] = SpeechData::Instance()->getNextSpeakableJob(priority);
//    //    if (d->currentJobs[priority])
//    //        sentence = d->currentJobs[priority]->getNextSentence();
//    //}
//    //if (!sentence.isEmpty()) {
//    //    switch (priority) {
//    //        case KSpeech::jpAll:    // should not happen.
//    //            Q_ASSERT(0);
//    //            break;
//    //        case KSpeech::jpScreenReaderOutput:
//    //            utType = Utt::utScreenReader;
//    //            break;
//    //        case KSpeech::jpWarning:
//    //            utType = Utt::utWarning;
//    //            break;
//    //        case KSpeech::jpMessage:
//    //            utType = Utt::utMessage;
//    //            break;
//    //        case KSpeech::jpText:
//    //            utType = Utt::utText;
//    //            break;
//    //    }
//    //    appId = d->currentJobs[priority]->appId();
//    //    SpeechJob* job = d->currentJobs[priority];
//    //    utt = new Utt(utType, appId, job, sentence);
//    //    utt->setSeq(job->seq());
//    //}

//    //bool r = (utt != NULL);

//    //if (utt)
//    //{
//    //    // Screen Reader Outputs need to be processed ASAP.
//    //    if (Utt::utScreenReader == utType)
//    //    {
//    //        d->uttQueue.insert(d->uttQueue.begin(), *utt);
//    //        // Delete any other Screen Reader Outputs in the queue.
//    //        // Only one Screen Reader Output at a time.
//    //        uttIterator it = d->uttQueue.begin();
//    //        ++it;
//    //        while (it != d->uttQueue.end())
//    //        {
//    //            if (Utt::utScreenReader == it->utType())
//    //                it = deleteUtterance(it);
//    //            else
//    //                ++it;
//    //        }
//    //    }
//    //    // If the new utterance is a Warning or Message...
//    //    if ((Utt::utWarning == utType) || (Utt::utMessage == utType))
//    //    {
//    //        uttIterator itEnd = d->uttQueue.end();
//    //        uttIterator it = d->uttQueue.begin();
//    //        bool interrupting = false;
//    //        if (it != itEnd)
//    //        {
//    //            // New Warnings go after Screen Reader Output, other Warnings,
//    //            // Interruptions, and in-process text,
//    //            // but before Resumes, waiting text or signals.
//    //            if (Utt::utWarning == utType)
//    //                while ( it != itEnd && 
//    //                        ((Utt::utScreenReader == it->utType()) || 
//    //                        (Utt::utWarning == it->utType()) ||
//    //                        (Utt::utInterruptMsg == it->utType()) ||
//    //                        (Utt::utInterruptSnd == it->utType()))) ++it;
//    //            // New Messages go after Screen Reader Output, Warnings, other Messages,
//    //            // Interruptions, and in-process text,
//    //            // but before Resumes, waiting text or signals.
//    //            if (Utt::utMessage == utType)
//    //                while ( it != itEnd && 
//    //                        ((Utt::utScreenReader == it->utType()) ||
//    //                        (Utt::utWarning == it->utType()) ||
//    //                        (Utt::utMessage == it->utType()) ||
//    //                        (Utt::utInterruptMsg == it->utType()) ||
//    //                        (Utt::utInterruptSnd == it->utType()))) ++it;
//    //            if (it != itEnd)
//    //                if (Utt::utText == it->utType() &&
//    //                    ((Utt::usPlaying == it->state()) ||
//    //                    (Utt::usSaying == it->state()))) ++it;
//    //            // If now pointing at a text message, we are interrupting.
//    //            // Insert optional Interruption message and sound.
//    //            if (it != itEnd) interrupting = (Utt::utText == it->utType() && Utt::usPaused != it->state());
//    //            if (interrupting)
//    //            {
//    //                if (d->configData->textPreSndEnabled)
//    //                {
//    //                    Utt intrUtt(Utt::utInterruptSnd, appId, d->configData->textPreSnd);
//    //                    it = d->uttQueue.insert(it, intrUtt);
//    //                    ++it;
//    //                }
//    //                if (d->configData->textPreMsgEnabled)
//    //                {
//    //                    Utt intrUtt(Utt::utInterruptMsg, appId, NULL, d->configData->textPreMsg, TalkerMgr::Instance()->talkerToPlugin(""));;
//    //                    it = d->uttQueue.insert(it, intrUtt);
//    //                    ++it;
//    //                }
//    //            }
//    //        }
//    //        // Insert the new message or warning.
//    //        it = d->uttQueue.insert(it, *utt);
//    //        ++it;
//    //        // Resumption message and sound.
//    //        if (interrupting)
//    //        {
//    //            if (d->configData->textPostSndEnabled)
//    //            {
//    //                Utt resUtt(Utt::utResumeSnd, appId, d->configData->textPostSnd);
//    //                it = d->uttQueue.insert(it, resUtt);
//    //                ++it;
//    //            }
//    //            if (d->configData->textPostMsgEnabled)
//    //            {
//    //                Utt resUtt(Utt::utResumeMsg, appId, NULL, d->configData->textPostMsg, TalkerMgr::Instance()->talkerToPlugin(""));
//    //                it = d->uttQueue.insert(it, resUtt);
//    //            }
//    //        }
//    //    }
//    //    // If a text message...
//    //    if (Utt::utText == utt->utType())
//    //        d->uttQueue.append(*utt);
//    //}

//    //return r;
//}

//uttIterator Speaker::deleteUtterance(uttIterator it)
//{
//    //switch (it->state())
//    //{
//    //    case Utt::usNone:
//    //    case Utt::usWaitingTransform:
//    //    case Utt::usWaitingSay:
//    //    case Utt::usFinished:
//    //        break;

//    //    case Utt::usTransforming:
//    //        delete it->transformer();
//    //        break;
//    //    case Utt::usSaying:
//    //    case Utt::usPaused:
//    //    case Utt::usPreempted:
//    //        // Note: Must call stop(), even if player not currently playing.  Why?
//    //        it->audioPlayer()->stop();
//    //        delete it->audioPlayer();
//    //        break;
//    //}
//    //// Delete the utterance from queue.
//    //return d->uttQueue.erase(it);
//}

//bool Speaker::startPlayingUtterance(uttIterator it)
//{
//    //// kDebug() << "Speaker::startPlayingUtterance running";
//    //if (Utt::usPlaying == it->state()) return false;
//    //bool started = false;
//    //// Pause (preempt) any other utterance currently being spoken.
//    //// If any plugins are audibilizing, must wait for them to finish.
//    //uttIterator itEnd = d->uttQueue.end();
//    //for (uttIterator it2 = d->uttQueue.begin(); it2 != itEnd; ++it2)
//    //    if (it2 != it)
//    //    {
//    //        if (Utt::usPlaying == it2->state())
//    //        {
//    //            it2->audioPlayer()->pause();
//    //            it2->setState(Utt::usPreempted);
//    //        }
//    //        if (Utt::usSaying == it2->state()) return false;
//    //    }
//    //Utt::uttState utState = it->state();
//    //switch (utState)
//    //{
//    //    case Utt::usNone:
//    //    case Utt::usWaitingTransform:
//    //    case Utt::usTransforming:
//    //    case Utt::usWaitingSay:
//    //    case Utt::usWaitingSynth:
//    //    case Utt::usSaying:
//    //    case Utt::usSynthing:
//    //    case Utt::usSynthed:
//    //    case Utt::usStretching:
//    //    case Utt::usPlaying:
//    //    case Utt::usFinished:
//    //        break;

//    //    case Utt::usPaused:
//    //        // kDebug() << "Speaker::startPlayingUtterance: resuming play";
//    //        it->audioPlayer()->startPlay(QString());  // resume
//    //        it->setState(Utt::usPlaying);
//    //        started = true;
//    //        break;

//    //    case Utt::usPreempted:
//    //        // Preempted playback automatically resumes.
//    //        it->audioPlayer()->startPlay(QString());  // resume
//    //        it->setState(Utt::usPlaying);
//    //        started = true;
//    //        break;
//    //}
//    //return started;
//}

//void Speaker::prePlaySignals(uttIterator it)
//{
//    //if (it->job())
//    //    emit marker(it->job()->appId(), it->job()->jobNum(), KSpeech::mtSentenceBegin, QString::number(it->seq()));
//}

//void Speaker::postPlaySignals(uttIterator it)
//{
//    //if (it->job())
//    //    emit marker(it->job()->appId(), it->job()->jobNum(), KSpeech::mtSentenceEnd, QString::number(it->seq()));
//}

//QString Speaker::makeSuggestedFilename()
//{
//    KTemporaryFile *tempFile = new KTemporaryFile();
//    tempFile->setPrefix("kttsd-");
//    tempFile->setSuffix(".wav");
//    tempFile->open();
//    QString waveFile = tempFile->fileName();
//    delete tempFile;
//    kDebug() << "Speaker::makeSuggestedFilename: Suggesting filename: " << waveFile;
//    return KStandardDirs::realFilePath(waveFile);
//}

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
        //doUtterances();
        return true;
    }
    else return false;
}

void Speaker::deleteJob(int removeJobNum)
{
    if (d->allJobs.contains(removeJobNum)) {
        SpeechJob* job = d->allJobs.take(removeJobNum);
        //KSpeech::JobPriority priority = job->jobPriority();
        QString appId = job->appId();
        if (job->refCount() != 0)
            kWarning() << "Speaker::deleteJob: deleting job " << removeJobNum << " with non-zero refCount." ;
        delete job;
        getAppData(appId)->jobList()->removeAll(removeJobNum);
    }
}

void Speaker::removeJob(int jobNum)
{
    // kDebug() << "Running: Speaker::removeJob";
    SpeechJob* removeJob = findJobByJobNum(jobNum);
    if (removeJob) {
        QString removeAppId = removeJob->appId();
        int removeJobNum = removeJob->jobNum();
        // If filtering on the job, cancel it.
        deleteJob(removeJobNum);
    }
}

void Speaker::removeAllJobs(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager())
        foreach (SpeechJob* job, d->allJobs)
            removeJob(job->jobNum());
    else
        foreach (int jobNum, *appData->jobList())
            removeJob(jobNum);
}

void Speaker::deleteExpiredJobs()
{
    foreach (AppData* appData, d->appData) {
        if (appData->unregistered())
            deleteExpiredApp(appData->appId());
        else {
            int finishedCount = 0;
            // Copy JobList.
            TJobList jobList = *(appData->jobList());
            for (int ndx = jobList.size() - 1; ndx >= 0; --ndx) {
                int jobNum = jobList[ndx];
                SpeechJob* job = d->allJobs[jobNum];
                if (KSpeech::jsFinished == job->state()) {
                    ++finishedCount;
                    if (finishedCount > 1 && 0 == job->refCount())
                        deleteJob(jobNum);
                }
            }
        }
    }
}

void Speaker::deleteExpiredApp(const QString appId)
{
    if (d->appData.contains(appId)) {
        AppData* appData = getAppData(appId);
        // Scan the app's job list.  If there are no utterances on any jobs,
        // delete the app altogether.
        bool speaking = false;
        foreach (int jobNum, *appData->jobList())
            if (0 != d->allJobs[jobNum]->refCount()) {
                speaking = true;
                break;
            }
        if (!speaking) {
            foreach (int jobNum, *appData->jobList())
                deleteJob(jobNum);
            releaseAppData(appId);
            kDebug() << "Speaker::deleteExpiredApp: application " << appId << " deleted.";
        }
    }
}

//SpeechJob* Speaker::getNextSpeakableJob(KSpeech::JobPriority priority)
//{
//    foreach (int jobNum, *d->jobLists[priority]) {
//        SpeechJob* job = d->allJobs[jobNum];
//        if (!d->appData[job->appId()]->isApplicationPaused()) {
//            switch (job->state()) {
//                case KSpeech::jsQueued:
//                    break;
//                case KSpeech::jsFiltering:
//                    waitJobFiltering(job);
//                    return job;
//                case KSpeech::jsSpeakable:
//                    return job;
//                case KSpeech::jsSpeaking:
//                    if (job->seq() < job->sentenceCount())
//                        return job;
//                    break;
//                case KSpeech::jsPaused:
//                case KSpeech::jsInterrupted:
//                case KSpeech::jsFinished:
//                case KSpeech::jsDeleted:
//                    break;
//            }
//        }
//    }
//    return NULL;
//}

void Speaker::setJobSentenceNum(int jobNum, int sentenceNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        job->setSentenceNum(sentenceNum);
}

int Speaker::jobSentenceNum(int jobNum) const
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        return job->sentenceNum();
    else
        return 0;
}

int Speaker::sentenceCount(int jobNum)
{
    int retval = -1;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        retval = job->sentenceCount();
    return retval;
}

int Speaker::jobCount(const QString& appId, KSpeech::JobPriority priority) const
{
    AppData* appData = getAppData(appId);
    // If System Manager app, return count of all jobs for all apps.
    if (appData->isSystemManager())
        return d->allJobs.count();
    else {
        if (KSpeech::jpAll == priority)
            // Return count of all jobs for this app.
            return appData->jobList()->count();
        else {
            // Return count of jobs for this app of the specified priority.
            int cnt = 0;
            foreach (int jobNum, d->allJobs.keys()) {
                SpeechJob* job = d->allJobs[jobNum];
                if (job->appId() == appId && job->jobPriority() == priority)
                    ++cnt;
            }
            return cnt;
        }
    }
}

QStringList Speaker::jobNumbers(const QString& appId, KSpeech::JobPriority priority) const
{
    QStringList jobs;
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager()) {
        if (KSpeech::jpAll == priority) {
            // Return job numbers for all jobs.
            foreach (int jobNum, d->allJobs.keys())
                jobs.append(QString::number(jobNum));
        } else {
            // Return job numbers for all jobs of specified priority.
            foreach (int jobNum, d->allJobs.keys())
                if (d->allJobs[jobNum]->jobPriority() == priority)
                    jobs.append(QString::number(jobNum));
        }
    } else {
        if (KSpeech::jpAll == priority) {
            // Return job numbers for all jobs for this app.
            foreach (int jobNum, *appData->jobList())
                jobs.append(QString::number(jobNum));
        } else {
            // Return job numbers for this app's jobs of the specified priority.
            foreach (int jobNum, d->allJobs.keys()) {
                SpeechJob* job = d->allJobs[jobNum];
                if (job->appId() == appId && job->jobPriority() == priority)
                    jobs.append(QString::number(jobNum));
            }
        }
    }
    // kDebug() << "Speaker::jobNumbers: appId = " << appId << " priority = " << priority
    //     << " jobs = " << jobs << endl;
    return jobs;
}

int Speaker::jobState(int jobNum) const
{
    int temp = -1;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        temp = (int)job->state();
    return temp;
}

QByteArray Speaker::jobInfo(int jobNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        //waitJobFiltering(job);
        QByteArray temp = job->serialize();
        QDataStream stream(&temp, QIODevice::Append);
        stream << getAppData(job->appId())->applicationName();
        return temp;
    } else {
        kDebug() << "Speaker::jobInfo: request for job info on non-existent jobNum = " << jobNum;
        return QByteArray();
    }
}

QString Speaker::jobSentence(int jobNum, int sentenceNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        //waitJobFiltering(job);
        if (sentenceNum <= job->sentenceCount())
            return job->sentences()[sentenceNum - 1];
        else
            return QString();
    } else
        return QString();
}

void Speaker::setTalker(int jobNum, const QString &talker)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        job->setTalker(talker);
}

void Speaker::moveJobLater(int jobNum)
{
    // kDebug() << "Running: Speaker::moveTextLater";
    if (d->allJobs.contains(jobNum)) {
        //KSpeech::JobPriority priority = d->allJobs[jobNum]->jobPriority();
        //TJobListPtr jobList = d->jobLists[priority];
        // Get index of the job.
        //uint index = jobList->indexOf(jobNum);
        // Move job down one position in the queue.
        // kDebug() << "In Speaker::moveTextLater, moving jobNum " << movedJobNum;
        //jobList->insert(index + 2, jobNum);
        //jobList->takeAt(index);
    }
}

int Speaker::moveRelSentence(int jobNum, int n)
{
    // kDebug() << "Running: Speaker::moveRelTextSentence";
    int newSentenceNum = 0;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        //waitJobFiltering(job);
        int oldSentenceNum = job->sentenceNum();
        newSentenceNum = oldSentenceNum + n;
        if (0 != n) {
            // Position one before the desired sentence.
            int seq = newSentenceNum - 1;
            if (seq < 0) seq = 0;
            int sentenceCount = job->sentenceCount();
            if (seq > sentenceCount) seq = sentenceCount;
            job->setSentenceNum(seq);
            job->setSeq(seq);
            // If job was previously finished, but is now rewound, set state to speakable.
            // If job was not finished, but now is past end, set state to finished.
            if (KSpeech::jsFinished == job->state()) {
                if (seq < sentenceCount)
                    job->setState(KSpeech::jsSpeakable);
            } else {
                if (seq == sentenceCount)
                    job->setState(KSpeech::jsFinished);
            }
        }
    }
    return newSentenceNum;
}

//void Speaker::startJobFiltering(SpeechJob* job, const QString& text, bool noSBD)
//{
//    job->setState(KSpeech::jsFiltering);
//    int jobNum = job->jobNum();
//    // kDebug() << "Speaker::startJobFiltering: jobNum = " << jobNum << " text.left(500) = " << text.left(500);
//    PooledFilterMgr* pooledFilterMgr = 0;
//    if (d->pooledFilterMgrs.contains(jobNum)) return;
//    // Find an idle FilterMgr, if any.
//    // If filtering is already in progress for this job, do nothing.
//    QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.begin();
//    while (it != d->pooledFilterMgrs.end()) {
//        if (!it.value()->busy) {
//            if (it.value()->job && it.value()->job->jobNum() == jobNum)
//                return;
//        } else {
//            if (!it.value()->job && !pooledFilterMgr)
//                pooledFilterMgr = it.value();
//        }
//        ++it;
//    }
//    // Create a new FilterMgr if needed and add to pool.
//    if (!pooledFilterMgr) {
//         // kDebug() << "Speaker::startJobFiltering: adding new pooledFilterMgr for job " << jobNum;
//        pooledFilterMgr = new PooledFilterMgr();
//        FilterMgr* filterMgr = new FilterMgr();
//        filterMgr->init();
//        pooledFilterMgr->filterMgr = filterMgr;
//        // Connect signals from FilterMgr.
//        connect (filterMgr, SIGNAL(filteringFinished()), this, SLOT(slotFilterMgrFinished()));
//        connect (filterMgr, SIGNAL(filteringStopped()),  this, SLOT(slotFilterMgrStopped()));
//        d->pooledFilterMgrs.insert(jobNum, pooledFilterMgr);
//    }
//    // else kDebug() << "Speaker::startJobFiltering: re-using idle pooledFilterMgr for job " << jobNum;
//    // Flag the FilterMgr as busy and set it going.
//    pooledFilterMgr->busy = true;
//    pooledFilterMgr->job = job;
//    //pooledFilterMgr->filterMgr->setNoSBD( noSBD );
//    // Get TalkerCode structure of closest matching Talker.
//    pooledFilterMgr->talkerCode = TalkerMgr::Instance()->talkerToTalkerCode(job->talker());
//    // Pass Sentence Boundary regular expression.
//    AppData* appData = getAppData(job->appId());
//    pooledFilterMgr->filterMgr->setSbRegExp(appData->sentenceDelimiter());
//    kDebug() << "Speaker::startJobFiltering: job = " << job->jobNum() << " NoSBD = "
//        << noSBD << " sentenceDelimiter = " << appData->sentenceDelimiter() << endl;
//    pooledFilterMgr->filterMgr->asyncConvert(text, pooledFilterMgr->talkerCode, appData->applicationName());
//}

//void Speaker::waitJobFiltering(const SpeechJob* job)
//{
//    int jobNum = job->jobNum();
//    bool waited = false;
//    bool notOptimum = false;
//    if (!d->pooledFilterMgrs.contains(jobNum)) return;
//    QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.find(jobNum);
//    while (it != d->pooledFilterMgrs.end() && it.key() == jobNum) {
//        PooledFilterMgr* pooledFilterMgr = it.value();
//        if (pooledFilterMgr->busy) {
//            if (!pooledFilterMgr->filterMgr->noSBD())
//                notOptimum = true;
//            pooledFilterMgr->filterMgr->waitForFinished();
//            waited = true;
//        }
//        ++it;
//    }
//    if (waited) {
//        if (notOptimum)
//            kDebug() << "Speaker::waitJobFiltering: Waited for filtering to finish on job "
//                << jobNum << ".  Not optimium.  "
//                << "Try waiting for jobStateChanged signal with jsSpeakable before querying for job information." << endl;
//        doFiltering();
//    }
//}

//void Speaker::doFiltering()
//{
//    // kDebug() << "Speaker::doFiltering: Running.";
//    kDebug() << "Speaker::doFiltering: Scanning " << d->pooledFilterMgrs.count() << " pooled filter managers.";
//    bool again = true;
//    bool filterFinished = false;
//    while (again) {
//        again = false;
//        QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.begin();
//        QMultiHash<int, PooledFilterMgr*>::iterator nextIt;
//        while (it != d->pooledFilterMgrs.end()) {
//            nextIt = it;
//            ++nextIt;
//            PooledFilterMgr* pooledFilterMgr = it.value();
//            // If FilterMgr is busy, see if it is now finished.
//            Q_ASSERT(pooledFilterMgr);
//            if (pooledFilterMgr->busy) {
//                FilterMgr* filterMgr = pooledFilterMgr->filterMgr;
//                if (FilterMgr::fsFinished == filterMgr->getState()) {
//                    filterFinished = true;
//                    SpeechJob* job = pooledFilterMgr->job;
//                    kDebug() << "Speaker::doFiltering: filter finished, jobNum = " << job->jobNum();
//                    pooledFilterMgr->busy = false;
//                    // Retrieve text from FilterMgr.
//                    QString text = filterMgr->getOutput();
//                    kDebug() << "Speaker::doFiltering: text.left(500) = " << text.left(500);
//                    // kDebug() << "Speaker::doFiltering: filtered text: " << text;
//                    filterMgr->ackFinished();
//                    // Convert the TalkerCode back into string.
//                    job->setTalker(pooledFilterMgr->talkerCode->getTalkerCode());
//                    // TalkerCode object no longer needed.
//                    delete pooledFilterMgr->talkerCode;
//                    pooledFilterMgr->talkerCode = 0;
//                    if (filterMgr->noSBD()) {
//                        job->setSentences(QStringList(text));
//                    } else {
//                        // Split the text into sentences and store in the job.
//                        // The SBD plugin does all the real sentence parsing, inserting tabs at each
//                        // sentence boundary.
//                        QStringList sentences = text.split( '\t', QString::SkipEmptyParts);
//                        job->setSentences(sentences);
//                    }
//                    // Clean up.
//                    pooledFilterMgr->job = 0;
//                    // Re-index pool of FilterMgrs;
//                    d->pooledFilterMgrs.erase(it);
//                    d->pooledFilterMgrs.insert(0, pooledFilterMgr);
//                    // Emit signal.
//                    job->setState(KSpeech::jsSpeakable);
//                }
//                else kDebug() << "Speaker::doFiltering: filter for job " << pooledFilterMgr->job->jobNum() << " is busy.";
//            }
//            else kDebug() << "Speaker::doFiltering: filter is idle";
//            it = nextIt;
//        }
//    }
//    if (filterFinished)
//        emit filteringFinished();
//}

void Speaker::pause(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager()) {
        foreach (AppData* app, d->appData)
            app->setIsApplicationPaused(true);
    } else
        appData->setIsApplicationPaused(true);
}

void Speaker::resume(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager()) {
        foreach (AppData* app, d->appData)
            app->setIsApplicationPaused(false);
    } else
        appData->setIsApplicationPaused(false);
}

bool Speaker::isApplicationPaused(const QString& appId)
{
    return getAppData(appId)->isApplicationPaused();
}

//void Speaker::slotFilterMgrFinished()
//{
    // kDebug() << "Speaker::slotFilterMgrFinished: received signal FilterMgr finished signal.";
    //doFiltering();
//}

//void Speaker::slotFilterMgrStopped()
//{
    //doFiltering();
//}

void Speaker::slotServiceUnregistered(const QString& serviceName)
{
    if (d->appData.contains(serviceName))
        d->appData[serviceName]->setUnregistered(true);
}
