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

// C++ includes.
#include <stdlib.h>

// Qt includes.
#include <qregexp.h>
#include <qpair.h>
#include <qvaluelist.h>
#include <qdom.h>

// KDE includes.
#include <kdebug.h>
#include <kglobal.h>
#include <kapplication.h>

// SpeechData includes.
#include "speechdata.h"
#include "speechdata.moc"

/**
* Constructor
* Sets text to be stopped and warnings and messages queues to be autodelete.
*/
SpeechData::SpeechData(){
    // kdDebug() << "Running: SpeechData::SpeechData()" << endl;
    // The text should be stoped at the beggining (thread safe)
    jobCounter = 0;
    config = 0;
    textJobs.setAutoDelete(true);

    // Warnings queue to be autodelete  (thread safe)
    warnings.setAutoDelete(true);

    // Messages queue to be autodelete (thread safe)
    messages.setAutoDelete(true);

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
    textPreSnd = config->readPathEntry("TextPreSnd");

    textPostMsgEnabled = config->readBoolEntry("TextPostMsgEnabled", false);
    textPostMsg = config->readEntry("TextPostMsg");

    textPostSndEnabled = config->readBoolEntry("TextPostSndEnabled", false);
    textPostSnd = config->readPathEntry("TextPostSnd");

    // Notification (KNotify).
    notify = config->readBoolEntry("Notify", false);
    notifyPassivePopupsOnly = config->readBoolEntry("NotifyPassivePopupsOnly", false);

    return true;
}
/**
* Destructor
*/
SpeechData::~SpeechData(){
    // kdDebug() << "Running: SpeechData::~SpeechData()" << endl;
    // Walk through jobs and emit a textRemoved signal for each job.
    for (mlJob* job = textJobs.first(); (job); job = textJobs.next())
    {
        emit textRemoved(job->appId, job->jobNum);
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
void SpeechData::setScreenReaderOutput(const QString &msg, const QString &talker, const QCString &appId)
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
void SpeechData::enqueueWarning( const QString &warning, const QString &talker, const QCString &appId){
    // kdDebug() << "Running: SpeechData::enqueueWarning( const QString &warning )" << endl;
    mlText *temp = new mlText();
    temp->text = warning;
    temp->talker = talker;
    temp->appId = appId;
    temp->seq = 1;
    warnings.enqueue( temp );
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
    mlText *temp = warnings.dequeue();
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
void SpeechData::enqueueMessage( const QString &message, const QString &talker, const QCString& appId){
    // kdDebug() << "Running: SpeechData::enqueueMessage" << endl;
    mlText *temp = new mlText();
    temp->text = message;
    temp->talker = talker;
    temp->appId = appId;
    temp->seq = 1;
    messages.enqueue( temp );
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
    mlText *temp = messages.dequeue();
    // uint count = warnings.count();
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

QStringList SpeechData::parseText(const QString &text, const QCString &appId /*=NULL*/)
{
    // There has to be a better way
    // kdDebug() << "I'm getting: " << endl << text << " from application " << appId << endl;
    if (isSsml(text))
    {
        QString tempList(text);
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
/*
    // This should be something better, like "[a-zA-Z]\. " (a regexp of course) The dot (.) is used for more than ending a sentence.
    temp.replace('.', '\n');
    QStringList tempList = QStringList::split('\n', temp, true);
*/
    
//    for ( QStringList::Iterator it = tempList.begin(); it != tempList.end(); ++it ) {
//        kdDebug() << "'" << *it << "'" << endl;
//    }
    return tempList;
}

/**
* Queues a text job.
*/
uint SpeechData::setText( const QString &text, const QString &talker, const QCString &appId)
{
    // kdDebug() << "Running: SpeechData::setText" << endl;
    QStringList tempList = parseText(text, appId);
    if (talker != NULL)
        textTalker = talker;
    else
        textTalker = QString::null;
    mlJob* job = new mlJob;
    uint jobNum = ++jobCounter;
    job->jobNum = jobNum;
    job->appId = appId;
    job->talker = textTalker;
    job->state = kspeech::jsQueued;
    job->seq = 0;
    job->sentences = tempList;
    job->partSeqNums.append(tempList.count());
    textJobs.append(job);
    emit textSet(appId, jobNum);
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
int SpeechData::appendText(const QString &text, const uint jobNum, const QCString& appId)
{
    // kdDebug() << "Running: SpeechData::appendText" << endl;
    QStringList tempList = parseText(text, appId);
    int newPartNum = 0;
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        int sentenceCount = job->sentences.count();
        job->sentences += tempList;
        job->partSeqNums.append(sentenceCount + tempList.count());
        newPartNum = job->partSeqNums.count();
        emit textAppended(job->appId, jobNum, newPartNum);
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
mlJob* SpeechData::findLastJobByAppId(const QCString& appId)
{
    if (appId == NULL)
        return textJobs.getLast();
    else
    {
        QPtrListIterator<mlJob> it(textJobs);
        for (it.toLast() ; it.current(); --it )
        {
            if (it.current()->appId == appId)
            {
                return it.current();
            }
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
mlJob* SpeechData::findAJobByAppId(const QCString& appId)
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
uint SpeechData::findAJobNumByAppId(const QCString& appId)
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
    QPtrListIterator<mlJob> it(textJobs);
    for ( ; it.current(); ++it )
    {
        if (it.current()->jobNum == jobNum)
        {
            return it.current();
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
QCString SpeechData::getAppIdByJobNum(const uint jobNum)
{
    QCString appId;
    mlJob* job = findJobByJobNum(jobNum);
    if (job) appId = job->appId;
    return appId;
}

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
    QCString removeAppId;    // The appId of the removed (and stopped) job.
    mlJob* removeJob = findJobByJobNum(jobNum);
    if (removeJob)
    {
        removeAppId = removeJob->appId;
        removeJobNum = removeJob->jobNum;
        // Delete the job.
        textJobs.removeRef(removeJob);
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
    uint partNum = 0;
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
* (thread safe)
* @param finishedJobNum Job number of a job that just finished.
* The just finished job is not deleted, but any other finished jobs are.
* Does not change the textJobs.current() pointer.
*/
void SpeechData::deleteExpiredJobs(const uint finishedJobNum)
{
    // Save current pointer.
    typedef QPair<QCString, uint> removedJob;
    typedef QValueList<removedJob> removedJobsList;
    removedJobsList removedJobs;
    // Walk through jobs and delete any other finished jobs.
    for (mlJob* job = textJobs.first(); (job); job = textJobs.next())
    {
        if (job->jobNum != finishedJobNum and job->state == kspeech::jsFinished)
        {
            removedJobs.append(removedJob(job->appId, job->jobNum));
            textJobs.removeRef(job);
        }
    }
    // Emit signals for removed jobs.
    removedJobsList::const_iterator it;
    removedJobsList::const_iterator endRemovedJobsList(removedJobs.constEnd());
    for (it = removedJobs.constBegin(); it != endRemovedJobsList ; ++it)
    {
        QCString appId = (*it).first;
        uint jobNum = (*it).second;
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
    for (mlJob* job = textJobs.first(); (job); job = textJobs.next())
        if (job->jobNum != prevJobNum)
            if (job->state == kspeech::jsSpeakable) return job;
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
    kdDebug() << "SpeechData::getNextSentenceText running with prevJobNum " << prevJobNum << " prevSeq " << prevSeq << endl;
    mlText* temp = 0;
    uint jobNum = prevJobNum;
    mlJob* job = 0;
    uint seq = prevSeq;
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
            if ((job->state != kspeech::jsSpeakable) and (job->state != kspeech::jsSpeaking))
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
        kdDebug() << "SpeechData::getNextSentenceText: return job number " << temp->jobNum << " seq " << temp->seq << endl;
    } else kdDebug() << "SpeechData::getNextSentenceText: no more sentences in queue" << endl;
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
void SpeechData::setSentenceDelimiter(const QString &delimiter, const QCString appId)
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
        temp = job->sentences.count();
    else
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
    QPtrListIterator<mlJob> it(textJobs);
    for ( ; it.current(); ++it )
    {
        if (!jobs.isEmpty()) jobs.append(",");
        jobs.append(QString::number(it.current()->jobNum));
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
void SpeechData::setTextJobState(const uint jobNum, const kspeech::kttsdJobState state)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        job->state = state;
        if (state == kspeech::jsFinished) deleteExpiredJobs(jobNum);
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
*   - QCString appId    DCOP senderId of the application that requested the speech job.
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
            QCString appId;
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
        QDataStream stream(temp, IO_WriteOnly);
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
void SpeechData::changeTextTalker(const uint jobNum, const QString &talker)
{
    mlJob* job = findJobByJobNum(jobNum);
    if (job)
    {
        if (!talker.isEmpty())
            job->talker = talker;
        else
            job->talker = QString::null;
    }
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
        uint index = textJobs.findRef(job);
        // Move job down one position in the queue.
        // kdDebug() << "In SpeechData::moveTextLater, moving jobNum " << movedJobNum << endl;
        if (textJobs.insert(index + 2, job)) textJobs.take(index);
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

