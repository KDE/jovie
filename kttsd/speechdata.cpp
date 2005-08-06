/***************************************************** vim:set ts=4 sw=4 sts=4:
  This contains the SpeechData class which is in charge of maintaining
  all the data on the memory.
  It maintains queues manages the text.
  We could say that this is the common repository between the KTTSD class
  (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
  functions)
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

// C++ includes.
#include <stdlib.h>

// Qt includes.
#include <qregexp.h>
#include <qpair.h>
#include <qdom.h>
#include <qfile.h>

// KDE includes.
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// KTTS includes.
#include "talkermgr.h"
#include "notify.h"

// SpeechData includes.
#include "speechdata.h"
#include "speechdata.moc"

// Set this to 1 to turn off filter support, including SBD as a plugin.
#define NO_FILTERS 0

/**
* Constructor
* Sets text to be stopped and warnings and messages queues to be autodelete.
*/
SpeechData::SpeechData(){
    // kdDebug() << "Running: SpeechData::SpeechData()" << endl;
    // The text should be stoped at the beggining (thread safe)
    jobCounter = 0;
    config = 0;
    supportsHTML = false;

    screenReaderOutput.jobNum = 0;
    screenReaderOutput.text = "";
}

bool SpeechData::readConfig(){
    // Load configuration
    delete config;
    //config = KGlobal::config();
    config = new KConfig("kttsdrc");

    // Set the group general for the configuration of KTTSD itself (no plug ins)
    config->setGroup("General");

    // Load the configuration of the text interruption messages and sound
    textPreMsgEnabled = config->readBoolEntry("TextPreMsgEnabled", false);
    textPreMsg = config->readEntry("TextPreMsg");

    textPreSndEnabled = config->readBoolEntry("TextPreSndEnabled", false);
    textPreSnd = config->readEntry("TextPreSnd");

    textPostMsgEnabled = config->readBoolEntry("TextPostMsgEnabled", false);
    textPostMsg = config->readEntry("TextPostMsg");

    textPostSndEnabled = config->readBoolEntry("TextPostSndEnabled", false);
    textPostSnd = config->readEntry("TextPostSnd");
    keepAudio = config->readBoolEntry("KeepAudio", false);
    keepAudioPath = config->readEntry("KeepAudioPath", locateLocal("data", "kttsd/audio/"));

    // Notification (KNotify).
    notify = config->readBoolEntry("Notify", false);
    notifyExcludeEventsWithSound = config->readBoolEntry("ExcludeEventsWithSound", true);
    loadNotifyEventsFromFile( locateLocal("config", "kttsd_notifyevents.xml"), true );

    // KTTSMgr auto start and auto exit.
    autoStartManager = config->readBoolEntry("AutoStartManager", false);
    autoExitManager = config->readBoolEntry("AutoExitManager", false);

    // Clear the pool of filter managers so that filters re-init themselves.
    QHash<int, PooledFilterMgr*>::iterator it = m_pooledFilterMgrs.begin();
    while (it != m_pooledFilterMgrs.end()) {
        PooledFilterMgr* pooledFilterMgr = it.value();
        delete pooledFilterMgr->filterMgr;
        delete pooledFilterMgr;
        ++it;
    }
    m_pooledFilterMgrs.clear();

    // Create an initial FilterMgr for the pool to save time later.
    PooledFilterMgr* pooledFilterMgr = new PooledFilterMgr();
    FilterMgr* filterMgr = new FilterMgr();
    filterMgr->init(config, "General");
    supportsHTML = filterMgr->supportsHTML();
    pooledFilterMgr->filterMgr = filterMgr;
    pooledFilterMgr->busy = false;
    pooledFilterMgr->job = 0;
    // Connect signals from FilterMgr.
    connect (filterMgr, SIGNAL(filteringFinished()), this, SLOT(slotFilterMgrFinished()));
    connect (filterMgr, SIGNAL(filteringStopped()),  this, SLOT(slotFilterMgrStopped()));
    m_pooledFilterMgrs.insert(0, pooledFilterMgr);

    return true;
}

/**
 * Loads notify events from a file.  Clearing data if clear is True.
 */
void SpeechData::loadNotifyEventsFromFile( const QString& filename, bool clear)
{
    // Open existing event list.
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdDebug() << "SpeechData::loadNotifyEventsFromFile: Unable to open file " << filename << endl;
    }
    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        kdDebug() << "SpeechData::loadNotifyEventsFromFile: File not in proper XML format. " << filename << endl;
    }
    // kdDebug() << "StringReplacerConf::load: document successfully parsed." << endl;
    file.close();

    if ( clear )
    {
        notifyDefaultPresent = NotifyPresent::Passive;
        notifyDefaultOptions.action = NotifyAction::SpeakMsg;
        notifyDefaultOptions.talker = QString::null;
        notifyDefaultOptions.customMsg = QString::null;
        notifyAppMap.clear();
    }

    // Event list.
    QDomNodeList eventList = doc.elementsByTagName("notifyEvent");
    const int eventListCount = eventList.count();
    for (int eventIndex = 0; eventIndex < eventListCount; ++eventIndex)
    {
        QDomNode eventNode = eventList.item(eventIndex);
        QDomNodeList propList = eventNode.childNodes();
        QString eventSrc;
        QString event;
        QString actionName;
        QString message;
        TalkerCode talkerCode;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex)
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "eventSrc") eventSrc = prop.text();
            if (prop.tagName() == "event") event = prop.text();
            if (prop.tagName() == "action") actionName = prop.text();
            if (prop.tagName() == "message") message = prop.text();
            if (prop.tagName() == "talker") talkerCode = TalkerCode(prop.text(), false);
        }
        NotifyOptions notifyOptions;
        notifyOptions.action = NotifyAction::action( actionName );
        notifyOptions.talker = talkerCode.getTalkerCode();
        notifyOptions.customMsg = message;
        if ( eventSrc != "default" )
        {
            notifyOptions.eventName = NotifyEvent::getEventName( eventSrc, event );
            NotifyEventMap notifyEventMap = notifyAppMap[ eventSrc ];
            notifyEventMap[ event ] = notifyOptions;
            notifyAppMap[ eventSrc ] = notifyEventMap;
        } else {
            notifyOptions.eventName = QString::null;
            notifyDefaultPresent = NotifyPresent::present( event );
            notifyDefaultOptions = notifyOptions;
        }
    }
}

/**
* Destructor
*/
SpeechData::~SpeechData(){
    // kdDebug() << "Running: SpeechData::~SpeechData()" << endl;
    // Walk through jobs and emit a textRemoved signal for each job.
    while (!messages.isEmpty())
        delete messages.dequeue();
    while (!warnings.isEmpty())
        delete warnings.dequeue();

    while (!textJobs.isEmpty()) {
        mlJob* job = textJobs.takeFirst();
        emit textRemoved(job->appId, job->jobNum);
        delete job;
    }

    while (!m_pooledFilterMgrs.isEmpty()) {
        PooledFilterMgr* pooledFilterMgr = *m_pooledFilterMgrs.begin();
        m_pooledFilterMgrs.erase(m_pooledFilterMgrs.begin());
        delete pooledFilterMgr->filterMgr;
        delete pooledFilterMgr;
    }

    delete config;
}

/**
* Say a message as soon as possible, interrupting any other speech in progress.
* IMPORTANT: This method is reserved for use by Screen Readers and should not be used
* by any other applications.
* @param msg            The message to be spoken.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
*
* If an existing Screen Reader output is in progress, it is stopped and discarded and
* replaced with this new message.
*/
void SpeechData::setScreenReaderOutput(const QString &msg, const QString &talker, const Q3CString &appId)
{
    screenReaderOutput.text = msg;
    screenReaderOutput.talker = talker;
    screenReaderOutput.appId = appId;
    screenReaderOutput.seq = 1;
}

/**
* Retrieves the Screen Reader Output.
*/
mlText* SpeechData::getScreenReaderOutput()
{
    mlText* temp = new mlText();
    temp->text = screenReaderOutput.text;
    temp->talker = screenReaderOutput.talker;
    temp->appId = screenReaderOutput.appId;
    temp->seq = screenReaderOutput.seq;
    // Blank the Screen Reader to text to "empty" it.
    screenReaderOutput.text = "";
    return temp;
}

/**
* Returns true if Screen Reader Output is ready to be spoken.
*/
bool SpeechData::screenReaderOutputReady()
{
    return !screenReaderOutput.text.isEmpty();
}

/**
* Add a new warning to the queue.
*/
void SpeechData::enqueueWarning( const QString &warning, const QString &talker, const Q3CString &appId){
    // kdDebug() << "Running: SpeechData::enqueueWarning( const QString &warning )" << endl;
    mlJob* job = new mlJob();
    ++jobCounter;
    if (jobCounter == 0) ++jobCounter;  // Overflow is OK, but don't want any 0 jobNums.
    uint jobNum = jobCounter;
    job->jobNum = jobNum;
    job->talker = talker;
    job->appId = appId;
    job->seq = 1;
    warnings.enqueue( job );
    job->sentences = QStringList();
    // Do not apply Sentence Boundary Detection filters to warnings.
    startJobFiltering( job, warning, true );
    // uint count = warnings.count();
    // kdDebug() << "Adding '" << temp->text << "' with talker '" << temp->talker << "' from application " << appId << " to the warnings queue leaving a total of " << count << " items." << endl;
}

/**
* Pop (get and erase) a warning from the queue.
* @return                Pointer to mlText structure containing the message.
*
* Caller is responsible for deleting the structure.
*/
mlText* SpeechData::dequeueWarning(){
    // kdDebug() << "Running: SpeechData::dequeueWarning()" << endl;
    mlJob* job = warnings.dequeue();
    waitJobFiltering(job);
    mlText* temp = new mlText();
    temp->jobNum = job->jobNum;
    temp->text = job->sentences.join("");
    temp->talker = job->talker;
    temp->appId = job->appId;
    temp->seq = 1;
    delete job;
    // uint count = warnings.count();
    // kdDebug() << "Removing '" << temp->text << "' with talker '" << temp->talker << "' from the warnings queue leaving a total of " << count << " items." << endl;
    return temp;
}

/**
* Are there any Warnings?
*/
bool SpeechData::warningInQueue(){
    // kdDebug() << "Running: SpeechData::warningInQueue() const" << endl;
    bool temp = !warnings.isEmpty();
    // if(temp){
        // kdDebug() << "The warnings queue is NOT empty" << endl;
    // } else {
        // kdDebug() << "The warnings queue is empty" << endl;
    // }
    return temp;
}

/**
* Add a new message to the queue.
*/
void SpeechData::enqueueMessage( const QString &message, const QString &talker, const Q3CString& appId){
    // kdDebug() << "Running: SpeechData::enqueueMessage" << endl;
    mlJob* job = new mlJob();
    ++jobCounter;
    if (jobCounter == 0) ++jobCounter;  // Overflow is OK, but don't want any 0 jobNums.
    uint jobNum = jobCounter;
    job->jobNum = jobNum;
    job->talker = talker;
    job->appId = appId;
    job->seq = 1;
    messages.enqueue( job );
    job->sentences = QStringList();
    // Do not apply Sentence Boundary Detection filters to messages.
    startJobFiltering( job, message, true );
    // uint count = messages.count();
    // kdDebug() << "Adding '" << temp->text << "' with talker '" << temp->talker << "' from application " << appId << " to the messages queue leaving a total of " << count << " items." << endl;
}

/**
* Pop (get and erase) a message from the queue.
* @return                Pointer to mlText structure containing the message.
*
* Caller is responsible for deleting the structure.
*/
mlText* SpeechData::dequeueMessage(){
    // kdDebug() << "Running: SpeechData::dequeueMessage()" << endl;
    mlJob* job = messages.dequeue();
    waitJobFiltering(job);
    mlText* temp = new mlText();
    temp->jobNum = job->jobNum;
    temp->text = job->sentences.join("");
    temp->talker = job->talker;
    temp->appId = job->appId;
    temp->seq = 1;
    delete job;
    /* mlText *temp = messages.dequeue(); */
    // uint count = messages.count();
    // kdDebug() << "Removing '" << temp->text << "' with talker '" << temp->talker << "' from the messages queue leaving a total of " << count << " items." << endl;
    return temp;
}

/**
* Are there any Messages?
*/
bool SpeechData::messageInQueue(){
    // kdDebug() << "Running: SpeechData::messageInQueue() const" << endl;
    bool temp = !messages.isEmpty();
    // if(temp){
    //     kdDebug() << "The messages queue is NOT empty" << endl;
    //  } else {
    //     kdDebug() << "The messages queue is empty" << endl;
    // }
    return temp;
}

/**
* Determines whether the given text is SSML markup.
*/
bool SpeechData::isSsml(const QString &text)
{
    /// This checks to see if the root tag of the text is a <speak> tag. 
    QDomDocument ssml;
    ssml.setContent(text, false);  // No namespace processing.
    /// Check to see if this is SSML
    QDomElement root = ssml.documentElement();
    return (root.tagName() == "speak");
}

/**
* Parses a block of text into sentences using the application-specified regular expression
* or (if not specified), the default regular expression.
* @param text           The message to be spoken.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
* @return               List of parsed sentences.
*
* If the text contains SSML, it is not parsed into sentences at all.
* TODO: Need a way to preserve SSML but still parse into sentences.
* We will walk before we run for now and not sentence parse.
*/

QStringList SpeechData::parseText(const QString &text, const Q3CString &appId /*=NULL*/)
{
    // There has to be a better way
    // kdDebug() << "I'm getting: " << endl << text << " from application " << appId << endl;
    if (isSsml(text))
    {
        QStringList tempList(text);
        return tempList;
    }
    // See if app has specified a custom sentence delimiter and use it, otherwise use default.
    QRegExp sentenceDelimiter;
    if (sentenceDelimiters.find(appId) != sentenceDelimiters.end())
        sentenceDelimiter = QRegExp(sentenceDelimiters[appId]);
    else
        sentenceDelimiter = QRegExp("([\\.\\?\\!\\:\\;]\\s)|(\\n *\\n)");
    QString temp = text;
    // Replace spaces, tabs, and formfeeds with a single space.
    temp.replace(QRegExp("[ \\t\\f]+"), " ");
    // Replace sentence delimiters with tab.
    temp.replace(sentenceDelimiter, "\\1\t");
    // Replace remaining newlines with spaces.
    temp.replace("\n"," ");
    temp.replace("\r"," ");
    // Remove leading spaces.
    temp.replace(QRegExp("\\t +"), "\t");
    // Remove trailing spaces.
    temp.replace(QRegExp(" +\\t"), "\t");
    // Remove blank lines.
    temp.replace(QRegExp("\t\t+"),"\t");
    // Split into sentences.
    QStringList tempList = QStringList::split("\t", temp, false);

//    for ( QStringList::Iterator it = tempList.begin(); it != tempList.end(); ++it ) {
//        kdDebug() << "'" << *it << "'" << endl;
//    }
    return tempList;
}

/**
* Queues a text job.
*/
uint SpeechData::setText( const QString &text, const QString &talker, const Q3CString &appId)
{
    // kdDebug() << "Running: SpeechData::setText" << endl;
    mlJob* job = new mlJob;
    ++jobCounter;
    if (jobCounter == 0) ++jobCounter;  // Overflow is OK, but don't want any 0 jobNums.
    uint jobNum = jobCounter;
    job->jobNum = jobNum;
    job->appId = appId;
    job->talker = talker;
    job->state = KSpeech::jsQueued;
    job->seq = 0;
#if NO_FILTERS
    QStringList tempList = parseText(text, appId);
    job->sentences = tempList;
    job->partSeqNums.append(tempList.count());
    textJobs.append(job);
    emit textSet(appId, jobNum);
#else
    job->sentences = QStringList();
    job->partSeqNums = QList<int>();
    textJobs.append(job);
    startJobFiltering(job, text, false);
#endif
    return jobNum;
}

/**
* Adds another part to a text job.  Does not start speaking the text.
* (thread safe)
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application,
*                       but if no such job, applies to the last job queued by any application.
* @param text           The message to be spoken.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
* @return               Part number for the added part.  Parts are numbered starting at 1.
*
* The text is parsed into individual sentences.  Call getTextCount to retrieve
* the sentence count.  Call startText to mark the job as speakable and if the
* job is the first speakable job in the queue, speaking will begin.
* @see setText.
* @see startText.
*/
int SpeechData::appendText(const QString &text, const uint jobNum, const Q3CString& /*appId*/)
{
    // kdDebug() << "Running: SpeechData::appendText" << endl;
    int newPartNum = 0;
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
#if NO_FILTERS
        QStringList tempList = parseText(text, appId);
        int sentenceCount = job->sentences.count();
        job->sentences += tempList;
        job->partSeqNums.append(sentenceCount + tempList.count());
        newPartNum = job->partSeqNums.count() + 1;
        emit textAppended(job->appId, jobNum, newPartNum);
#else
        newPartNum = job->partSeqNums.count() + 1;
        startJobFiltering(job, text, false);
#endif
    }
    return newPartNum;
}

/**
* Given an appId, returns the last (most recently queued) job with that appId.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
* @return               Pointer to the text job.
* If no such job, returns 0.
* If appId is NULL, returns the last job in the queue.
* Does not change textJobs.current().
*/
mlJob* SpeechData::findLastJobByAppId(const Q3CString& appId)
{
    if (textJobs.isEmpty()) return 0;
    if (appId.isEmpty())
        return textJobs.last();
    else {
        QList<mlJob*>::const_iterator it = textJobs.constEnd();
        QList<mlJob*>::const_iterator itBegin = textJobs.constBegin();
        while (it != itBegin) {
            --it;
            mlJob* job = *it;
            if (job->appId == appId) return job;
        }
        return 0;
    }
}

/**
* Given an appId, returns the last (most recently queued) job with that appId,
* or if no such job, the last (most recent) job in the queue.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
* @return               Pointer to the text job.
* If no such job, returns 0.
* If appId is NULL, returns the last job in the queue.
* Does not change textJobs.current().
*/
mlJob* SpeechData::findAJobByAppId(const Q3CString& appId)
{
    mlJob* job = findLastJobByAppId(appId);
    // if (!job) job = textJobs.getLast();
    return job;
}

/**
* Given an appId, returns the last (most recently queued) Job Number with that appId,
* or if no such job, the Job Number of the last (most recent) job in the queue.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
* @return               Job Number of the text job.
* If no such job, returns 0.
* If appId is NULL, returns the Job Number of the last job in the queue.
* Does not change textJobs.current().
*/
uint SpeechData::findAJobNumByAppId(const Q3CString& appId)
{
    mlJob* job = findAJobByAppId(appId);
    if (job)
        return job->jobNum;
    else
        return 0;
}

/**
* Given a jobNum, returns the first job with that jobNum.
* @return               Pointer to the text job.
* If no such job, returns 0.
* Does not change textJobs.current().
*/
mlJob* SpeechData::findJobByJobNum(const uint jobNum)
{
    QList<mlJob*>::const_iterator it = textJobs.constBegin();
    QList<mlJob*>::const_iterator itEnd = textJobs.constEnd();
    for ( ; it != itEnd; ++it)
    {
        mlJob* job = *it;
        if (job->jobNum == jobNum)
        {
            return job;
        }
    }
    return 0;
}

/**
* Given a jobNum, returns the appId of the application that owns the job.
* @param jobNum         Job number of the text job.
* @return               appId of the job.
* If no such job, returns "".
* Does not change textJobs.current().
*/
Q3CString SpeechData::getAppIdByJobNum(const uint jobNum)
{
    Q3CString appId;
    mlJob* job = findJobByJobNum(jobNum);
    if (job) appId = job->appId;
    return appId;
}

/**
* Sets pointer to the TalkerMgr object.
*/
void SpeechData::setTalkerMgr(TalkerMgr* talkerMgr) { m_talkerMgr = talkerMgr; }

/**
* Remove a text job from the queue.
* (thread safe)
* @param jobNum         Job number of the text job.
*
* The job is deleted from the queue and the textRemoved signal is emitted.
*/
void SpeechData::removeText(const uint jobNum)
{
    // kdDebug() << "Running: SpeechData::removeText" << endl;
    uint removeJobNum = 0;
    Q3CString removeAppId;    // The appId of the removed (and stopped) job.
    mlJob* removeJob = findJobByJobNum(jobNum);
    if (removeJob)
    {
        removeAppId = removeJob->appId;
        removeJobNum = removeJob->jobNum;
        // If filtering on the job, cancel it.
        if (m_pooledFilterMgrs[removeJobNum])
        {
            PooledFilterMgr* pooledFilterMgr = m_pooledFilterMgrs[removeJobNum];
            pooledFilterMgr->busy = false;
            pooledFilterMgr->job = 0;
            pooledFilterMgr->filterMgr->stopFiltering();
        }
        // Delete the job.
        textJobs.removeAll(removeJob);
    }
    if (removeJobNum) emit textRemoved(removeAppId, removeJobNum);
}

/**
* Given a job and a sequence number, returns the part that sentence is in.
* If no such job or sequence number, returns 0.
* @param job            The text job.
* @param seq            Sequence number of the sentence.  Sequence numbers begin with 1.
* @return               Part number of the part the sentence is in.  Parts are numbered
*                       beginning with 1.  If no such job or sentence, returns 0.
* 
*/
int SpeechData::getJobPartNumFromSeq(const mlJob& job, const int seq)
{
    int foundPartNum = 0;
    int desiredSeq = seq;
    int partNum = 0;
    // Wait until all filtering has stopped for the job.
    waitJobFiltering(&job);
    while (partNum < job.partSeqNums.count())
    {
        if (desiredSeq <= job.partSeqNums[partNum])
        {
            foundPartNum = partNum + 1;
            break;
        }
        desiredSeq = desiredSeq - job.partSeqNums[partNum];
        ++partNum;
    }
    return foundPartNum;
}


/**
* Delete expired jobs.  At most, one finished job is kept on the queue.
* @param finishedJobNum Job number of a job that just finished.
* The just finished job is not deleted, but any other finished jobs are.
* Does not change the textJobs.current() pointer.
*/
void SpeechData::deleteExpiredJobs(const uint finishedJobNum)
{
    // Save current pointer.
    typedef QPair<Q3CString, uint> removedJob;
    typedef QList<removedJob> removedJobsList;
    removedJobsList removedJobs;
    // Walk through jobs and delete any other finished jobs.
    QList<mlJob*>::iterator it = textJobs.begin();
    while (it != textJobs.end()) {
        mlJob* job = *it;
        if (job->jobNum != finishedJobNum && job->state == KSpeech::jsFinished)
        {
            removedJobs.append(removedJob(job->appId, job->jobNum));
            it = textJobs.remove(it);
            delete job;
        } else
            ++it;
    }
    // Emit signals for removed jobs.
    for (int i = 0; i < removedJobs.size(); ++i)
    {
        Q3CString appId = removedJobs.at(i).first;
        uint jobNum = removedJobs.at(i).second;
        textRemoved(appId, jobNum);
    }
}

/**
* Given a Job Number, returns the next speakable text job on the queue.
* @param prevJobNum       Current job number (which should not be returned).
* @return                 Pointer to mlJob structure of the first speakable job
*                         not equal prevJobNum.  If no such job, returns null.
*
* Caller must not delete the job.
*/
mlJob* SpeechData::getNextSpeakableJob(const uint prevJobNum)
{
    QList<mlJob*>::const_iterator it = textJobs.constBegin();
    QList<mlJob*>::const_iterator itEnd = textJobs.constEnd();
    for ( ; it != itEnd; ++it)
    {
        mlJob* job = *it;
        if (job->jobNum != prevJobNum)
            if (job->state == KSpeech::jsSpeakable)
            {
                waitJobFiltering(job);
                return job;
            }
    }
    return 0;
}

/**
* Given previous job number and sequence number, returns the next sentence from the
* text queue.  If no such sentence is available, either because we've run out of
* jobs, or because all jobs are paused, returns null.
* @param prevJobNum       Previous Job Number.
* @param prevSeq          Previous sequency number.
* @return                 Pointer to n mlText structure containing the next sentence.  If no
*                         sentence, returns null.
*
* Caller is responsible for deleting the returned mlText structure (if not null).
*/
mlText* SpeechData::getNextSentenceText(const uint prevJobNum, const uint prevSeq)
{
    // kdDebug() << "SpeechData::getNextSentenceText running with prevJobNum " << prevJobNum << " prevSeq " << prevSeq << endl;
    mlText* temp = 0;
    uint jobNum = prevJobNum;
    mlJob* job = 0;
    int seq = prevSeq;
    ++seq;
    if (!jobNum)
    {
        job = getNextSpeakableJob(jobNum);
        if (job) seq =+ job->seq;
    } else
        job = findJobByJobNum(prevJobNum);
        if (!job)
        {
            job = getNextSpeakableJob(jobNum);
            if (job) seq =+ job->seq;
        }
        else
        {
            if ((job->state != KSpeech::jsSpeakable) && (job->state != KSpeech::jsSpeaking))
            {
                job = getNextSpeakableJob(job->jobNum);
                if (job) seq =+ job->seq;
            }
        }
    if (job)
    {
        // If we run out of sentences in the job, move on to next job.
        jobNum = job->jobNum;
        if (seq > job->sentences.count()) 
        {
            job = getNextSpeakableJob(jobNum);
            if (job) seq =+ job->seq;
        }
    }
    if (job)
    {
        if (seq == 0) seq = 1;
        temp = new mlText;
        temp->text = job->sentences[seq - 1];
        temp->appId = job->appId;
        temp->talker = job->talker;
        temp->jobNum = job->jobNum;
        temp->seq = seq;
        // kdDebug() << "SpeechData::getNextSentenceText: return job number " << temp->jobNum << " seq " << temp->seq << endl;
    } // else kdDebug() << "SpeechData::getNextSentenceText: no more sentences in queue" << endl;
    return temp;
}

/**
* Given a Job Number, sets the current sequence number of the job.
* @param jobNum          Job Number.
* @param seq             Sequence number.
* If for some reason, the job does not exist, nothing happens.
*/
void SpeechData::setJobSequenceNum(const uint jobNum, const uint seq)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job) job->seq = seq;    
}

/**
* Given a Job Number, returns the current sequence number of the job.
* @param jobNum         Job Number.
* @return               Sequence number of the job.  If no such job, returns 0.
*/
uint SpeechData::getJobSequenceNum(const uint jobNum)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
        return job->seq;
    else
        return 0;
}

/**
* Sets the GREP pattern that will be used as the sentence delimiter.
* @param delimiter      A valid GREP pattern.
* @param appId          The DCOP senderId of the application.  NULL if kttsd.
*
* The default delimiter is
  @verbatim
     ([\\.\\?\\!\\:\\;])\\s
  @endverbatim
*
* Note that backward slashes must be escaped.
*
* Changing the sentence delimiter does not affect other applications.
* @see sentenceparsing
*/
void SpeechData::setSentenceDelimiter(const QString &delimiter, const Q3CString appId)
{
    sentenceDelimiters[appId] = delimiter;
}

/**
* Get the number of sentences in a text job.
* (thread safe)
* @param jobNum         Job number of the text job.
* @return               The number of sentences in the job.  -1 if no such job.
*
* The sentences of a job are given sequence numbers from 1 to the number returned by this
* method.  The sequence numbers are emitted in the sentenceStarted and sentenceFinished signals.
*/
int SpeechData::getTextCount(const uint jobNum)
{
    mlJob* job = findJobByJobNum(jobNum);
    int temp;
    if (job)
    {
        waitJobFiltering(job);
        temp = job->sentences.count();
    } else
        temp = -1;
    return temp;
}

/**
* Get the number of jobs in the text job queue.
* (thread safe)
* @return               Number of text jobs in the queue.  0 if none.
*/
uint SpeechData::getTextJobCount()
{
    return textJobs.count();
}

/**
* Get a comma-separated list of text job numbers in the queue.
* @return               Comma-separated list of text job numbers in the queue.
*/
QString SpeechData::getTextJobNumbers()
{
    QString jobs;
    QList<mlJob*>::const_iterator it = textJobs.constBegin();
    QList<mlJob*>::const_iterator itEnd = textJobs.constEnd();
    for (; it != itEnd; ++it)
    {
        mlJob* job = *it;
        if (!jobs.isEmpty()) jobs.append(",");
        jobs.append(QString::number(job->jobNum));
    }
    return jobs;
}

/**
* Get the state of a text job.
* (thread safe)
* @param jobNum         Job number of the text job.
* @return               State of the job. -1 if invalid job number.
*/
int SpeechData::getTextJobState(const uint jobNum)
{
    mlJob* job = findJobByJobNum(jobNum);
    int temp;
    if (job)
        temp = job->state;
    else
        temp = -1;
    return temp;
}

/**
* Set the state of a text job.
* @param jobNum            Job Number of the job.
* @param state             New state for the job.
*
* If the new state is Finished, deletes other expired jobs.
*
**/
void SpeechData::setTextJobState(const uint jobNum, const KSpeech::kttsdJobState state)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        job->state = state;
        if (state == KSpeech::jsFinished) deleteExpiredJobs(jobNum);
    }
}

/**
* Get information about a text job.
* @param jobNum         Job number of the text job.
* @return               A QDataStream containing information about the job.
*                       Blank if no such job.
*
* The stream contains the following elements:
*   - int state         Job state.
*   - Q3CString appId   DCOP senderId of the application that requested the speech job.
*   - QString talker    Language code in which to speak the text.
*   - int seq           Current sentence being spoken.  Sentences are numbered starting at 1.
*   - int sentenceCount Total number of sentences in the job.
*   - int partNum       Current part of the job begin spoken.  Parts are numbered starting at 1.
*   - int partCount     Total number of parts in the job.
*
* Note that sequence numbers apply to the entire job.
* They do not start from 1 at the beginning of each part.
*
* The following sample code will decode the stream:
          @verbatim
            QByteArray jobInfo = getTextJobInfo(jobNum);
            QDataStream stream(jobInfo, IO_ReadOnly);
            int state;
            Q3CString appId;
            QString talker;
            int seq;
            int sentenceCount;
            int partNum;
            int partCount;
            stream >> state;
            stream >> appId;
            stream >> talker;
            stream >> seq;
            stream >> sentenceCount;
            stream >> partNum;
            stream >> partCount;
          @endverbatim
*/
QByteArray SpeechData::getTextJobInfo(const uint jobNum)
{
    mlJob* job = findJobByJobNum(jobNum);
    QByteArray temp;
    if (job)
    {
        waitJobFiltering(job);
        QDataStream stream(&temp, IO_WriteOnly);
        stream << job->state;
        stream << job->appId;
        stream << job->talker;
        stream << job->seq;
        stream << job->sentences.count();
        stream << getJobPartNumFromSeq(*job, job->seq);
        stream << job->partSeqNums.count();
    }
    return temp;
}

/**
* Return a sentence of a job.
* @param jobNum         Job number of the text job.
* @param seq            Sequence number of the sentence.
* @return               The specified sentence in the specified job.  If no such
*                       job or sentence, returns "".
*/
QString SpeechData::getTextJobSentence(const uint jobNum, const uint seq /*=1*/)
{
    mlJob* job = findJobByJobNum(jobNum);
    QString temp;
    if (job)
    {
        waitJobFiltering(job);
        temp = job->sentences[seq - 1];
    }
    return temp;
}

/**
* Change the talker for a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application,
*                       but if no such job, applies to the last job queued by any application.
* @param talker         New code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
*/
void SpeechData::changeTextTalker(const QString &talker, uint jobNum)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job) job->talker = talker;
}

/**
* Move a text job down in the queue so that it is spoken later.
* @param jobNum         Job number of the text job.
*/
void SpeechData::moveTextLater(const uint jobNum)
{
    // kdDebug() << "Running: SpeechData::moveTextLater" << endl;
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        // Get index of the job.
        uint index = textJobs.indexOf(job);
        // Move job down one position in the queue.
        // kdDebug() << "In SpeechData::moveTextLater, moving jobNum " << movedJobNum << endl;
        textJobs.insert(index + 2, job);
        textJobs.takeAt(index);
    }
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
int SpeechData::jumpToTextPart(const int partNum, const uint jobNum)
{
    // kdDebug() << "Running: SpeechData::jumpToTextPart" << endl;
    int newPartNum = 0;
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        waitJobFiltering(job);
        if (partNum > 0)
        {
            newPartNum = partNum;
            int partCount = job->partSeqNums.count();
            if (newPartNum > partCount) newPartNum = partCount;
            if (newPartNum > 1)
                job->seq = job->partSeqNums[newPartNum - 1];
            else
                job->seq = 0;
        }
        else
            newPartNum = getJobPartNumFromSeq(*job, job->seq);
    }
    return newPartNum;
}

/**
* Advance or rewind N sentences in a text job.
* @param n              Number of sentences to advance (positive) or rewind (negative)
*                       in the job.
* @param jobNum         Job number of the text job.
* @return               Sequence number of the sentence actually moved to.  Sequence numbers
*                       are numbered starting at 1.
*
* If no such job, does nothing and returns 0.
* If n is zero, returns the current sequence number of the job.
* Does not affect the current speaking/not-speaking state of the job.
*/
uint SpeechData::moveRelTextSentence(const int n, const uint jobNum /*=0*/)
{
    // kdDebug() << "Running: SpeechData::moveRelTextSentence" << endl;
    int newSeqNum = 0;
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        waitJobFiltering(job);
        int oldSeqNum = job->seq;
        newSeqNum = oldSeqNum + n;
        if (n != 0)
        {
            if (newSeqNum < 0) newSeqNum = 0;
            int sentenceCount = job->sentences.count();
            if (newSeqNum > sentenceCount) newSeqNum = sentenceCount;
            job->seq = newSeqNum;
        }
    }
    return newSeqNum;
}

/**
* Assigns a FilterMgr to a job and starts filtering on it.
*/
void SpeechData::startJobFiltering(mlJob* job, const QString& text, bool noSBD)
{
    // If filtering is already in progress for this job, do nothing.
    uint jobNum = job->jobNum;
    PooledFilterMgr* pooledFilterMgr = 0;
    if (m_pooledFilterMgrs.contains(jobNum)) return;
    // Find an idle FilterMgr, if any.
    QHash<int, PooledFilterMgr*>::iterator it = m_pooledFilterMgrs.begin();
    while (it != m_pooledFilterMgrs.end()) {
        if (!it.value()->busy)
        {
            // Reindex the pooled FilterMgr on the new job number.
            int oldJobNum = it.key();
            pooledFilterMgr = m_pooledFilterMgrs.take(oldJobNum);
            m_pooledFilterMgrs.insert(jobNum, pooledFilterMgr);
            break;
        }
        ++it;
    }
    // Create a new FilterMgr if needed and add to pool.
    if (!pooledFilterMgr)
    {
        pooledFilterMgr = new PooledFilterMgr();
        FilterMgr* filterMgr = new FilterMgr();
        filterMgr->init(config, "General");
        pooledFilterMgr->filterMgr = filterMgr;
        // Connect signals from FilterMgr.
        connect (filterMgr, SIGNAL(filteringFinished()), this, SLOT(slotFilterMgrFinished()));
        connect (filterMgr, SIGNAL(filteringStopped()),  this, SLOT(slotFilterMgrStopped()));
        m_pooledFilterMgrs.insert(jobNum, pooledFilterMgr);
    }
    // Flag the FilterMgr as busy and set it going.
    pooledFilterMgr->busy = true;
    pooledFilterMgr->job = job;
    pooledFilterMgr->filterMgr->setNoSBD( noSBD );
    // Get TalkerCode structure of closest matching Talker.
    pooledFilterMgr->talkerCode = m_talkerMgr->talkerToTalkerCode(job->talker);
    // Pass Sentence Boundary regular expression (if app overrode default);
    if (sentenceDelimiters.find(job->appId) != sentenceDelimiters.end())
        pooledFilterMgr->filterMgr->setSbRegExp(sentenceDelimiters[job->appId]);
    pooledFilterMgr->filterMgr->asyncConvert(text, pooledFilterMgr->talkerCode, job->appId);
}

/**
* Waits for filtering to be completed on a job.
* This is typically called because an app has requested job info that requires
* filtering to be completed, such as getJobInfo.
*/
void SpeechData::waitJobFiltering(const mlJob* job)
{
#if NO_FILTERS
    return;
#endif
    PooledFilterMgr* pooledFilterMgr = m_pooledFilterMgrs[job->jobNum];
    if (!pooledFilterMgr) return;
    if (pooledFilterMgr->busy)
    {
        if (!pooledFilterMgr->filterMgr->noSBD())
            kdDebug() << "SpeechData::waitJobFiltering: Waiting for filter to finish.  Not optimium.  " <<
                "Try waiting for textSet signal before querying for job information." << endl;
        pooledFilterMgr->filterMgr->waitForFinished();
        doFiltering();
    }
}

/**
* Processes filters by looping across the pool of FilterMgrs.
* As each FilterMgr finishes, emits appropriate signals and flags it as no longer busy.
*/
void SpeechData::doFiltering()
{
    kdDebug() << "SpeechData::doFiltering: Running." << endl;
    kdDebug() << "SpeechData::doFiltering: Scanning " << m_pooledFilterMgrs.count() << " pooled filter managers." << endl;
    bool again = true;
    while (again)
    {
        again = false;
        QHash<int, PooledFilterMgr*>::iterator it = m_pooledFilterMgrs.begin();
        while (it != m_pooledFilterMgrs.end()) {
            PooledFilterMgr* pooledFilterMgr = it.value();
            // If FilterMgr is busy, see if it is now finished.
            if (pooledFilterMgr->busy)
            {
                FilterMgr* filterMgr = pooledFilterMgr->filterMgr;
                if (filterMgr->getState() == FilterMgr::fsFinished)
                {
                    pooledFilterMgr->busy = false;
                    mlJob* job = pooledFilterMgr->job;
                    // Retrieve text from FilterMgr.
                    QString text = filterMgr->getOutput();
                    // kdDebug() << "SpeechData::doFiltering: text.left(500) = " << text.left(500) << endl;
                    // kdDebug() << "SpeechData::doFiltering: filtered text: " << text << endl;
                    filterMgr->ackFinished();
                    // Convert the TalkerCode back into string.
                    job->talker = pooledFilterMgr->talkerCode->getTalkerCode();
                    // TalkerCode object no longer needed.
                    delete pooledFilterMgr->talkerCode;
                    pooledFilterMgr->talkerCode = 0;
                    if (filterMgr->noSBD()) {
                        job->sentences.clear();
                        job->sentences.append(text);
                    } else {
                        // Split the text into sentences and store in the job.
                        // The SBD plugin does all the real sentence parsing, inserting tabs at each
                        // sentence boundary.
                        QStringList sentences = QStringList::split("\t", text, false);
                        int sentenceCount = job->sentences.count();
                        job->sentences += sentences;
                        job->partSeqNums.append(sentenceCount + sentences.count());
                    }
                    int partNum = job->partSeqNums.count();
                    // Clean up.
                    pooledFilterMgr->job = 0;
                    // Emit signal.
                    if (!filterMgr->noSBD())
                    {
                        if (partNum == 1)
                            emit textSet(job->appId, job->jobNum);
                        else
                            emit textAppended(job->appId, job->jobNum, partNum);
                    }
                }
            }
            ++it;
        }
    }
}

void SpeechData::slotFilterMgrFinished()
{
    // kdDebug() << "SpeechData::slotFilterMgrFinished: received signal FilterMgr finished signal." << endl;
    doFiltering();
}

void SpeechData::slotFilterMgrStopped()
{
    doFiltering();
}

