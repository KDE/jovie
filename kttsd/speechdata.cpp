/***************************************************** vim:set ts=4 sw=4 sts=4:
  This contains the SpeechData class which is in charge of maintaining
  all the speech data.
  We could say that this is the common repository between the KTTSD class
  (dbus service) and the Speaker class (speaker, loads plug ins, call plug in
  functions)
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

// C++ includes.
#include <stdlib.h>

// Qt includes.
#include <QtCore/QRegExp>
#include <QtCore/QPair>
#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtDBus/QtDBus>

// KDE includes.
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "talkermgr.h"
#include "configdata.h"

// SpeechData includes.
#include "speechdata.h"
#include "speechdata.moc"

/* -------------------------------------------------------------------------- */

class SpeechDataPrivate
{
public:
    SpeechDataPrivate() :
        configData(NULL),
        lastJobNum(0),
        talkerMgr(NULL),
        jobCounter(0),
        supportsHTML(false)
    {
        jobLists.insert(KSpeech::jpScreenReaderOutput, new TJobList());
        jobLists.insert(KSpeech::jpWarning, new TJobList());
        jobLists.insert(KSpeech::jpMessage, new TJobList());
        jobLists.insert(KSpeech::jpText, new TJobList());
    }

    ~SpeechDataPrivate()
    {
        // kDebug() << "Running: SpeechDataPrivate::~SpeechDataPrivate";
        // Walk through jobs and emit jobStateChanged signal for each job.
        foreach (SpeechJob* job, allJobs)
            delete job;
        allJobs.clear();

        foreach (TJobListPtr jobList, jobLists)
            jobList->clear();
        jobLists.clear();

        foreach (PooledFilterMgr* pooledFilterMgr, pooledFilterMgrs) {
            delete pooledFilterMgr->filterMgr;
            delete pooledFilterMgr->talkerCode;
            delete pooledFilterMgr;
        }
        pooledFilterMgrs.clear();

        foreach (AppData* applicationData, appData)
            delete applicationData;
        appData.clear();
    }

    friend class SpeechData;

protected:
    /**
    * Configuration data.
    */
    ConfigData* configData;

    /**
    * All jobs.
    */
    QHash<int, SpeechJob*> allJobs;

    /**
    * List of jobs for each job priority type.
    */
    QMap<KSpeech::JobPriority, TJobListPtr> jobLists;

    /**
    * Application data.
    */
    mutable QMap<QString, AppData*> appData;

    /**
    * The last job queued by any App.
    */
    int lastJobNum;

    /**
    * TalkerMgr object local pointer.
    */
    TalkerMgr* talkerMgr;

    /**
    * Pool of FilterMgrs.
    */
    QMultiHash<int, PooledFilterMgr*> pooledFilterMgrs;

    /**
    * Job counter.  Each new job increments this counter.
    */
    int jobCounter;

    /**
    * True if at least one XML Transformer plugin for html is enabled.
    */
    bool supportsHTML;
};

/* -------------------------------------------------------------------------- */

/**
* Constructor
*/
SpeechData::SpeechData()
{
    d = new SpeechDataPrivate();

    // Connect ServiceUnregistered signal from DBUS so we know when apps have exited.
    connect (QDBusConnection::sessionBus().interface(), SIGNAL(serviceUnregistered(const QString&)),
        this, SLOT(slotServiceUnregistered(const QString&)));
}

/**
* Destructor
*/
SpeechData::~SpeechData()
{
    delete d;
}

AppData* SpeechData::getAppData(const QString& appId) const
{
    if (!d->appData.contains(appId))
        d->appData.insert(appId, new AppData(appId));
    return d->appData[appId];
}

void SpeechData::releaseAppData(const QString& appId)
{
    if (d->appData.contains(appId))
        delete d->appData.take(appId);
}

bool SpeechData::isSsml(const QString &text)
{
    /// This checks to see if the root tag of the text is a <speak> tag.
    QDomDocument ssml;
    ssml.setContent(text, false);  // No namespace processing.
    /// Check to see if this is SSML
    QDomElement root = ssml.documentElement();
    return (root.tagName() == "speak");
}

QStringList SpeechData::parseText(const QString &text, const QString &appId /*=NULL*/)
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

int SpeechData::say(const QString& appId, const QString& text, int sayOptions)
{
    // TODO: sayOptions
    Q_UNUSED(sayOptions);

    // kDebug() << "Running: SpeechData::say appId = " << appId << " text = " << text;
    AppData* appData = getAppData(appId);
    KSpeech::JobPriority priority = appData->defaultPriority();
    QString talker = appData->defaultTalker();
    // kDebug() << "SpeechData::say priority = " << priority;

    // Screen Reader Outputs replace other Screen Reader Outputs not yet speaking.
    if (KSpeech::jpScreenReaderOutput == priority)
        foreach(int jobNum, *d->jobLists[priority]) {
            SpeechJob* job = d->allJobs[jobNum];
            // TODO: OK to delete jobs while iterating inside foreach?
            switch (job->state()) {
                case KSpeech::jsQueued:
                    removeJob(jobNum);
                    break;
                case KSpeech::jsFiltering:
                    waitJobFiltering(job);
                    removeJob(jobNum);
                    break;
                case KSpeech::jsSpeakable:
                    removeJob(jobNum);
                    break;
                case KSpeech::jsSpeaking:
                    break;
                case KSpeech::jsPaused:
                case KSpeech::jsInterrupted:
                    break;
                case KSpeech::jsFinished:
                    removeJob(jobNum);
                    break;
                case KSpeech::jsDeleted:
                    break;
            }
        };
    SpeechJob* job = new SpeechJob(priority);
    connect(job, SIGNAL(jobStateChanged(const QString&, int, KSpeech::JobState)),
        this, SIGNAL(jobStateChanged(const QString&, int, KSpeech::JobState)));
    ++d->jobCounter;
    if (d->jobCounter <= 0) d->jobCounter = 1;  // Overflow is OK, but don't want any 0 jobNums.
    int jobNum = d->jobCounter;
    job->setJobNum(jobNum);
    job->setAppId(appId);
    job->setTalker(talker);
    // Note: Set state last so job is fully populated when jobStateChanged signal is emitted.
    d->allJobs.insert(jobNum, job);
    d->jobLists[priority]->append(jobNum);
    appData->jobList()->append(jobNum);
    d->lastJobNum = jobNum;
    if (!appData->filteringOn()) {
        QStringList tempList = parseText(text, appId);
        job->setSentences(tempList);
        job->setState(KSpeech::jsSpeakable);
    } else {
        job->setSentences(QStringList());
        startJobFiltering(job, text, (KSpeech::jpScreenReaderOutput == priority));
        emit jobStateChanged(appId, jobNum, KSpeech::jsQueued);
    }
    return jobNum;
}

SpeechJob* SpeechData::findLastJobByAppId(const QString& appId) const
{
    int jobNum = findJobNumByAppId(appId);
    if (jobNum)
        return d->allJobs[jobNum];
    else
        return NULL;
}

int SpeechData::findJobNumByAppId(const QString& appId) const
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
SpeechJob* SpeechData::findJobByJobNum(int jobNum) const
{
    if (d->allJobs.contains(jobNum))
        return d->allJobs[jobNum];
    else
        return NULL;
}

QString SpeechData::getAppIdByJobNum(int jobNum) const
{
    QString appId;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) appId = job->appId();
    return appId;
}

void SpeechData::setTalkerMgr(TalkerMgr* talkerMgr)
{
    d->talkerMgr = talkerMgr;
}

void SpeechData::setConfigData(ConfigData* configData)
{
    d->configData = configData;

    // Clear the pool of filter managers so that filters re-init themselves.
    QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.begin();
    while (it != d->pooledFilterMgrs.end()) {
        PooledFilterMgr* pooledFilterMgr = it.value();
        delete pooledFilterMgr->filterMgr;
        delete pooledFilterMgr->talkerCode;
        delete pooledFilterMgr;
        ++it;
    }
    d->pooledFilterMgrs.clear();

    // Create an initial FilterMgr for the pool to save time later.
    PooledFilterMgr* pooledFilterMgr = new PooledFilterMgr();
    FilterMgr* filterMgr = new FilterMgr();
    filterMgr->init();
    d->supportsHTML = filterMgr->supportsHTML();
    pooledFilterMgr->filterMgr = filterMgr;
    pooledFilterMgr->busy = false;
    pooledFilterMgr->job = 0;
    pooledFilterMgr->talkerCode = 0;
    // Connect signals from FilterMgr.
    connect (filterMgr, SIGNAL(filteringFinished()), this, SLOT(slotFilterMgrFinished()));
    connect (filterMgr, SIGNAL(filteringStopped()),  this, SLOT(slotFilterMgrStopped()));
    d->pooledFilterMgrs.insert(0, pooledFilterMgr);
}

void SpeechData::deleteJob(int removeJobNum)
{
    if (d->allJobs.contains(removeJobNum)) {
        SpeechJob* job = d->allJobs.take(removeJobNum);
        KSpeech::JobPriority priority = job->jobPriority();
        QString appId = job->appId();
        if (0 != job->refCount())
            kWarning() << "SpeechData::deleteJob: deleting job " << removeJobNum << " with non-zero refCount." ;
        delete job;
        d->jobLists[priority]->removeAll(removeJobNum);
        getAppData(appId)->jobList()->removeAll(removeJobNum);
    }
}

void SpeechData::removeJob(int jobNum)
{
    // kDebug() << "Running: SpeechData::removeJob";
    SpeechJob* removeJob = findJobByJobNum(jobNum);
    if (removeJob) {
        QString removeAppId = removeJob->appId();
        int removeJobNum = removeJob->jobNum();
        // If filtering on the job, cancel it.
        if (d->pooledFilterMgrs.contains(removeJobNum)) {
            while (PooledFilterMgr* pooledFilterMgr = d->pooledFilterMgrs.take(removeJobNum)) {
                pooledFilterMgr->busy = false;
                pooledFilterMgr->job = 0;
                delete pooledFilterMgr->talkerCode;
                pooledFilterMgr->talkerCode = 0;
                pooledFilterMgr->filterMgr->stopFiltering();
                d->pooledFilterMgrs.insert(0, pooledFilterMgr);
            }
        }
        deleteJob(removeJobNum);
    }
}

void SpeechData::removeAllJobs(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager())
        foreach (SpeechJob* job, d->allJobs)
            removeJob(job->jobNum());
    else
        foreach (int jobNum, *appData->jobList())
            removeJob(jobNum);
}

void SpeechData::deleteExpiredJobs()
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

void SpeechData::deleteExpiredApp(const QString appId)
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
            kDebug() << "SpeechData::deleteExpiredApp: application " << appId << " deleted.";
        }
    }
}

SpeechJob* SpeechData::getNextSpeakableJob(KSpeech::JobPriority priority)
{
    foreach (int jobNum, *d->jobLists[priority]) {
        SpeechJob* job = d->allJobs[jobNum];
        if (!d->appData[job->appId()]->isApplicationPaused()) {
            switch (job->state()) {
                case KSpeech::jsQueued:
                    break;
                case KSpeech::jsFiltering:
                    waitJobFiltering(job);
                    return job;
                case KSpeech::jsSpeakable:
                    return job;
                case KSpeech::jsSpeaking:
                    if (job->seq() < job->sentenceCount())
                        return job;
                    break;
                case KSpeech::jsPaused:
                case KSpeech::jsInterrupted:
                case KSpeech::jsFinished:
                case KSpeech::jsDeleted:
                    break;
            }
        }
    }
    return NULL;
}

void SpeechData::setJobSentenceNum(int jobNum, int sentenceNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) job->setSentenceNum(sentenceNum);
}

int SpeechData::jobSentenceNum(int jobNum) const
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job)
        return job->sentenceNum();
    else
        return 0;
}

int SpeechData::sentenceCount(int jobNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    int temp;
    if (job) {
        waitJobFiltering(job);
        temp = job->sentenceCount();
    } else
        temp = -1;
    return temp;
}

int SpeechData::jobCount(const QString& appId, KSpeech::JobPriority priority) const
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
            foreach (int jobNum, *d->jobLists[priority]) {
                SpeechJob* job = d->allJobs[jobNum];
                if (job->appId() == appId)
                    ++cnt;
            }
            return cnt;
        }
    }
}

QStringList SpeechData::jobNumbers(const QString& appId, KSpeech::JobPriority priority) const
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
            foreach (int jobNum, *d->jobLists[priority])
                jobs.append(QString::number(jobNum));
        }
    } else {
        if (KSpeech::jpAll == priority) {
            // Return job numbers for all jobs for this app.
            foreach (int jobNum, *appData->jobList())
                jobs.append(QString::number(jobNum));
        } else {
            // Return job numbers for this app's jobs of the specified priority.
            foreach (int jobNum, *d->jobLists[priority]) {
                SpeechJob* job = d->allJobs[jobNum];
                if (job->appId() == appId)
                    jobs.append(QString::number(jobNum));
            }
        }
    }
    // kDebug() << "SpeechData::jobNumbers: appId = " << appId << " priority = " << priority
    //     << " jobs = " << jobs << endl;
    return jobs;
}

int SpeechData::jobState(int jobNum) const
{
    SpeechJob* job = findJobByJobNum(jobNum);
    int temp;
    if (job)
        temp = (int)job->state();
    else
        temp = -1;
    return temp;
}

QByteArray SpeechData::jobInfo(int jobNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        waitJobFiltering(job);
        QByteArray temp = job->serialize();
        QDataStream stream(&temp, QIODevice::Append);
        stream << getAppData(job->appId())->applicationName();
        return temp;
    } else {
        kDebug() << "SpeechData::jobInfo: request for job info on non-existent jobNum = " << jobNum;
        return QByteArray();
    }
}

QString SpeechData::jobSentence(int jobNum, int sentenceNum)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        waitJobFiltering(job);
        if (sentenceNum <= job->sentenceCount())
            return job->sentences()[sentenceNum - 1];
        else
            return QString();
    } else
        return QString();
}

void SpeechData::setTalker(int jobNum, const QString &talker)
{
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) job->setTalker(talker);
}

void SpeechData::moveJobLater(int jobNum)
{
    // kDebug() << "Running: SpeechData::moveTextLater";
    if (d->allJobs.contains(jobNum)) {
        KSpeech::JobPriority priority = d->allJobs[jobNum]->jobPriority();
        TJobListPtr jobList = d->jobLists[priority];
        // Get index of the job.
        uint index = jobList->indexOf(jobNum);
        // Move job down one position in the queue.
        // kDebug() << "In SpeechData::moveTextLater, moving jobNum " << movedJobNum;
        jobList->insert(index + 2, jobNum);
        jobList->takeAt(index);
    }
}

int SpeechData::moveRelSentence(int jobNum, int n)
{
    // kDebug() << "Running: SpeechData::moveRelTextSentence";
    int newSentenceNum = 0;
    SpeechJob* job = findJobByJobNum(jobNum);
    if (job) {
        waitJobFiltering(job);
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

void SpeechData::startJobFiltering(SpeechJob* job, const QString& text, bool noSBD)
{
    job->setState(KSpeech::jsFiltering);
    int jobNum = job->jobNum();
    // kDebug() << "SpeechData::startJobFiltering: jobNum = " << jobNum << " text.left(500) = " << text.left(500);
    PooledFilterMgr* pooledFilterMgr = 0;
    if (d->pooledFilterMgrs.contains(jobNum)) return;
    // Find an idle FilterMgr, if any.
    // If filtering is already in progress for this job, do nothing.
    QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.begin();
    while (it != d->pooledFilterMgrs.end()) {
        if (!it.value()->busy) {
            if (it.value()->job && it.value()->job->jobNum() == jobNum)
                return;
        } else {
            if (!it.value()->job && !pooledFilterMgr)
                pooledFilterMgr = it.value();
        }
        ++it;
    }
    // Create a new FilterMgr if needed and add to pool.
    if (!pooledFilterMgr) {
         // kDebug() << "SpeechData::startJobFiltering: adding new pooledFilterMgr for job " << jobNum;
        pooledFilterMgr = new PooledFilterMgr();
        FilterMgr* filterMgr = new FilterMgr();
        filterMgr->init();
        pooledFilterMgr->filterMgr = filterMgr;
        // Connect signals from FilterMgr.
        connect (filterMgr, SIGNAL(filteringFinished()), this, SLOT(slotFilterMgrFinished()));
        connect (filterMgr, SIGNAL(filteringStopped()),  this, SLOT(slotFilterMgrStopped()));
        d->pooledFilterMgrs.insert(jobNum, pooledFilterMgr);
    }
    // else kDebug() << "SpeechData::startJobFiltering: re-using idle pooledFilterMgr for job " << jobNum;
    // Flag the FilterMgr as busy and set it going.
    pooledFilterMgr->busy = true;
    pooledFilterMgr->job = job;
    pooledFilterMgr->filterMgr->setNoSBD( noSBD );
    // Get TalkerCode structure of closest matching Talker.
    pooledFilterMgr->talkerCode = d->talkerMgr->talkerToTalkerCode(job->talker());
    // Pass Sentence Boundary regular expression.
    AppData* appData = getAppData(job->appId());
    pooledFilterMgr->filterMgr->setSbRegExp(appData->sentenceDelimiter());
    kDebug() << "SpeechData::startJobFiltering: job = " << job->jobNum() << " NoSBD = "
        << noSBD << " sentenceDelimiter = " << appData->sentenceDelimiter() << endl;
    pooledFilterMgr->filterMgr->asyncConvert(text, pooledFilterMgr->talkerCode, appData->applicationName());
}

void SpeechData::waitJobFiltering(const SpeechJob* job)
{
    int jobNum = job->jobNum();
    bool waited = false;
    bool notOptimum = false;
    if (!d->pooledFilterMgrs.contains(jobNum)) return;
    QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.find(jobNum);
    while (it != d->pooledFilterMgrs.end() && it.key() == jobNum) {
        PooledFilterMgr* pooledFilterMgr = it.value();
        if (pooledFilterMgr->busy) {
            if (!pooledFilterMgr->filterMgr->noSBD())
                notOptimum = true;
            pooledFilterMgr->filterMgr->waitForFinished();
            waited = true;
        }
        ++it;
    }
    if (waited) {
        if (notOptimum)
            kDebug() << "SpeechData::waitJobFiltering: Waited for filtering to finish on job "
                << jobNum << ".  Not optimium.  "
                << "Try waiting for jobStateChanged signal with jsSpeakable before querying for job information." << endl;
        doFiltering();
    }
}

void SpeechData::doFiltering()
{
    // kDebug() << "SpeechData::doFiltering: Running.";
    kDebug() << "SpeechData::doFiltering: Scanning " << d->pooledFilterMgrs.count() << " pooled filter managers.";
    bool again = true;
    bool filterFinished = false;
    while (again) {
        again = false;
        QMultiHash<int, PooledFilterMgr*>::iterator it = d->pooledFilterMgrs.begin();
        QMultiHash<int, PooledFilterMgr*>::iterator nextIt;
        while (it != d->pooledFilterMgrs.end()) {
            nextIt = it;
            ++nextIt;
            PooledFilterMgr* pooledFilterMgr = it.value();
            // If FilterMgr is busy, see if it is now finished.
            Q_ASSERT(pooledFilterMgr);
            if (pooledFilterMgr->busy) {
                FilterMgr* filterMgr = pooledFilterMgr->filterMgr;
                if (FilterMgr::fsFinished == filterMgr->getState()) {
                    filterFinished = true;
                    SpeechJob* job = pooledFilterMgr->job;
                    kDebug() << "SpeechData::doFiltering: filter finished, jobNum = " << job->jobNum();
                    pooledFilterMgr->busy = false;
                    // Retrieve text from FilterMgr.
                    QString text = filterMgr->getOutput();
                    kDebug() << "SpeechData::doFiltering: text.left(500) = " << text.left(500);
                    // kDebug() << "SpeechData::doFiltering: filtered text: " << text;
                    filterMgr->ackFinished();
                    // Convert the TalkerCode back into string.
                    job->setTalker(pooledFilterMgr->talkerCode->getTalkerCode());
                    // TalkerCode object no longer needed.
                    delete pooledFilterMgr->talkerCode;
                    pooledFilterMgr->talkerCode = 0;
                    if (filterMgr->noSBD()) {
                        job->setSentences(QStringList(text));
                    } else {
                        // Split the text into sentences and store in the job.
                        // The SBD plugin does all the real sentence parsing, inserting tabs at each
                        // sentence boundary.
                        QStringList sentences = text.split( '\t', QString::SkipEmptyParts);
                        job->setSentences(sentences);
                    }
                    // Clean up.
                    pooledFilterMgr->job = 0;
                    // Re-index pool of FilterMgrs;
                    d->pooledFilterMgrs.erase(it);
                    d->pooledFilterMgrs.insert(0, pooledFilterMgr);
                    // Emit signal.
                    job->setState(KSpeech::jsSpeakable);
                }
                else kDebug() << "SpeechData::doFiltering: filter for job " << pooledFilterMgr->job->jobNum() << " is busy.";
            }
            else kDebug() << "SpeechData::doFiltering: filter is idle";
            it = nextIt;
        }
    }
    if (filterFinished)
        emit filteringFinished();
}

void SpeechData::pause(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager()) {
        foreach (AppData* app, d->appData)
            app->setIsApplicationPaused(true);
    } else
        appData->setIsApplicationPaused(true);
}

void SpeechData::resume(const QString& appId)
{
    AppData* appData = getAppData(appId);
    if (appData->isSystemManager()) {
        foreach (AppData* app, d->appData)
            app->setIsApplicationPaused(false);
    } else
        appData->setIsApplicationPaused(false);
}

bool SpeechData::isApplicationPaused(const QString& appId)
{
    return getAppData(appId)->isApplicationPaused();
}

void SpeechData::slotFilterMgrFinished()
{
    // kDebug() << "SpeechData::slotFilterMgrFinished: received signal FilterMgr finished signal.";
    doFiltering();
}

void SpeechData::slotFilterMgrStopped()
{
    doFiltering();
}

void SpeechData::slotServiceUnregistered(const QString& serviceName)
{
    if (d->appData.contains(serviceName))
        d->appData[serviceName]->setUnregistered(true);
}
