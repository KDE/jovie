/*************************************************** vim:set ts=4 sw=4 sts=4:
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

#ifndef _SPEECHDATA_H_
#define _SPEECHDATA_H_

// Qt includes.

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QMultiHash>
#include <QMap>

// KDE includes.
#include <kconfig.h>
#include <kspeech.h>

// KTTSD includes.
#include "speechjob.h"
#include "talkercode.h"
#include "filtermgr.h"
#include "appdata.h"
#include "configdata.h"

class TalkerMgr;

/**
 * Struct used to keep a pool of FilterMgr objects.
 */
struct PooledFilterMgr {
    FilterMgr* filterMgr;       /* The FilterMgr object. */
    bool busy;                  /* True if the FilterMgr is busy. */
    SpeechJob* job;             /* The job the FilterMgr is filtering. */
    TalkerCode* talkerCode;     /* TalkerCode object passed to FilterMgr. */
};

class SpeechDataPrivate;
class SpeechData : public QObject {
    Q_OBJECT

public:
    /**
    * Constructor
    */
    SpeechData();

    /**
    * Destructor
    */
    ~SpeechData();

    /**
    * Get application data.
    * If this is a new application, a new AppData object is created and initialized
    * with defaults.
    * Caller may set properties, but must not delete the returned AppData object.
    * Use releaseAppData instead.
    * @param appId          The DBUS senderId of the application.
    */
    AppData* getAppData(const QString& appId) const;

    /**
    * Destroys the application data.
    */
    void releaseAppData(const QString& appId);

    /**
    * Queue and start a speech job.
    * @param appId          The DBUS senderId of the application.
    * @param text           The text to be spoken.
    * @param options        Option flags.  @see SayOptions.  Defaults to KSpeech::soNone.
    *
    * Based on the options, the text may contain the text to be spoken, with or withou
    * markup, or it may contain characters to be spelled out, or it may contain
    * the symbolic name of a keyboard key, or it may contain the name of a sound
    * icon.
    *
    * The job is given the applications current defaultPriority.  @see defaultPriority.
    * The job is assigned the applications current defaultTalker.  @see defaultTalker.
    */
    int say(const QString& appId, const QString& text, int sayOptions);

    /**
    * Remove a job from the queue.
    * @param jobNum         Job number.
    *
    * The job is deleted from the queue and the textRemoved signal is emitted.
    */
    void removeJob(int jobNum);

    /**
    * Remove all jobs owned by the application.
    * @param appId          The DBUS senderId of the application.
    */
    void removeAllJobs(const QString& appId);

    /**
    * Get the number of sentences in a job.
    * @param jobNum         Job number.
    * @return               The number of sentences in the job.  -1 if no such job.
    *
    * The sentences of a job are given numbers from 1 to the number returned by this
    * method.
    *
    * If the job is being filtered and split into sentences, waits until that is finished
    * before returning.
    */
    int sentenceCount(int jobNum);

    /**
    * Get the number of jobs in the queue of the specified type owned by
    * the application.
    * @param appId          The DBUS senderId of the application.
    * @param priority       Type of job.  Text, Message, Warning, or ScreenReaderOutput.
    * @return               Number of jobs in the queue.  0 if none.
    *
    * A priority of KSpeech::jpAll will return all the job numbers 
    * owned by the application.
    */
    int jobCount(const QString& appId, KSpeech::JobPriority priority) const;

    /**
    * Get a list of job numbers in the queue of the
    * specified type owned by the application.
    * @param appId          The DBUS senderId of the application.
    * @param priority       Type of job.  Text, Message, Warning, or ScreenReaderOutput.
    * @return               List of job numbers in the queue.
    */
    QStringList jobNumbers(const QString& appId, KSpeech::JobPriority priority) const;

    /**
    * Get the state of a job.
    * @param jobNum         Job number.
    * @return               State of the job. -1 if invalid job number.
    *
    * @see setJobState
    */
    int jobState(int jobNum) const;

    /**
    * Get information about a job.
    * @param jobNum         Job number of the job.
    * @return               A QDataStream containing information about the job.
    *                       Blank if no such job.
    *
    * The stream contains the following elements:
    *   - int priority      Job Type.
    *   - int state         Job state.
    *   - QString appId     DBUS senderId of the application that requested the speech job.
    *   - QString talker    Talker code as requested by application.
    *   - int sentenceNum   Current sentence being spoken.  Sentences are numbered starting at 1.
    *   - int sentenceCount Total number of sentences in the job.
    *   - QString applicationName Application's friendly name (if provided by app)
    *
    * If the job is currently filtering, waits for that to finish before returning.
    *
    * The following sample code will decode the stream:
            @verbatim
                QByteArray jobInfo = getTextJobInfo(jobNum);
                QDataStream stream(jobInfo, QIODevice::ReadOnly);
                qint32 priority;
                qint32 state;
                QString appId;
                QString talker;
                qint32 sentenceNum;
                qint32 sentenceCount;
                QString applicationName;
                stream >> priority;
                stream >> state;
                stream >> appId;
                stream >> talker;
                stream >> sentenceNum;
                stream >> sentenceCount;
                stream >> applicationName;
            @endverbatim
    */
    QByteArray jobInfo(int jobNum);

    /**
    * Return a sentence of a job.
    * @param jobNum         Job number.
    * @param sentenceNum    Sentence number of the sentence.
    * @return               The specified sentence in the specified job.  If no such
    *                       job or sentence, returns "".
    */
    QString jobSentence(int jobNum, int sentenceNum=1);

    /**
    * Return the talker code for a job.
    * @param jobNum         Job number of the job.
    *
    * @see setTalker
    */
    QString talker(int jobNum);

    /**
    * Change the talker for a job.
    * @param jobNum         Job number of the job.
    * @param talker         New code for the talker to do speaking.  Example "en".
    *                       If NULL, defaults to the user's default talker.
    *                       If no plugin has been configured for the specified Talker code,
    *                       defaults to the closest matching talker.
    *
    * @see talker
    */
    void setTalker(int jobNum, const QString &talker);

    /**
    * Move a job down in the queue so that it is spoken later.
    * @param jobNum         Job number.
    *
    * Since there is only one ScreenReaderOutput, this method is meaningless
    * for ScreenReaderOutput jobs.
    */
    void moveJobLater(int jobNum);

    /**
    * Advance or rewind N sentences in a job.
    * @param jobNum         Job number of the job.
    * @param n              Number of sentences to advance (positive) or rewind (negative)
    *                       in the job.
    * @return               Sentence number of the sentence actually moved to.  Sentence numbers
    *                       are numbered starting at 1.
    *
    * If no such job, does nothing and returns 0.
    * If n is zero, returns the current sentence number of the job.
    * Does not affect the current speaking/not-speaking state of the job.
    *
    * Since ScreenReaderOutput jobs are not split into sentences, this method
    * is meaningless for ScreenReaderOutput jobs.
    */
    int moveRelSentence(int jobNum, int n);

    /**
    * Given an appId, returns the last (most recently queued) Job Number with that appId,
    * or if no such job, the Job Number of the last (most recent) job in the queue.
    * @param appId          The DBUS senderId of the application.
    * @return               Job Number.
    * If no such job, returns 0.
    * If appId is NULL, returns the Job Number of the last job in the queue.
    */
    int findJobNumByAppId(const QString& appId) const;

    /**
    * Given a jobNum, returns the first job with that jobNum.
    * @return               Pointer to the job.
    * If no such job, returns 0.
    */
    SpeechJob* findJobByJobNum(int jobNum) const;

    /**
    * Given an appId, returns the last (most recently queued) job with that appId.
    * @param appId          The DBUS senderId of the application.
    * @return               Pointer to the job.
    * If no such job, returns 0.
    * If appId is NULL, returns the last job in the queue.
    */
    SpeechJob* findLastJobByAppId(const QString& appId) const;

    /**
    * Given a Job Type and Job Number, returns the next speakable job on the queue.
    * @param priority       Type of job.  Text, Message, Warning, or ScreenReaderOutput.
    * @return               Pointer to the speakable job.
    *
    * Caller must not delete the job.
    */
    SpeechJob* getNextSpeakableJob(KSpeech::JobPriority priority);

    /**
    * Given a Job Number, returns the current sentence number begin spoken.
    * @param jobNum         Job Number.
    * @return               Sentence number of the job.  If no such job, returns 0.
    */
    int jobSentenceNum(int jobNum) const;

    /**
    * Given a Job Number, sets the current sentence number begin spoken.
    * @param jobNum          Job Number.
    * @param sentenceNum     Sentence number.
    * If for some reason, the job does not exist, nothing happens.
    */
    void setJobSentenceNum(int jobNum, int sentenceNum);

    /**
    * Given a jobNum, returns the appId of the application that owns the job.
    * @param jobNum         Job number.
    * @return               appId of the job.
    * If no such job, returns "".
    */
    QString getAppIdByJobNum(int jobNum) const;

    /**
    * Delete expired jobs.  At most, one finished job per application 
    * is kept on the queue.
    */
    void deleteExpiredJobs();
    
    /**
    * Return true if the application is paused.
    */
    bool isApplicationPaused(const QString& appId);
    
    /**
    * Pauses the application.
    */
    void pause(const QString& appId);
    
    /**
    * Resumes the application.
    */
    void resume(const QString& appId);

    /**
    * Sets pointer to the TalkerMgr object.
    */
    void setTalkerMgr(TalkerMgr* talkerMgr);

    /**
    * Sets pointer to the Configuration data object.
    */
    void setConfigData(ConfigData* configData);

Q_SIGNALS:
    /**
    * Emitted when the state of a job changes.
    */
    void jobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state);
    
    /**
    * Emitted when job filtering completes.
    */
    void filteringFinished();

private slots:
    void slotFilterMgrFinished();
    void slotFilterMgrStopped();
    
    void slotServiceUnregistered(const QString& serviceName);

private:
    /**
    * Determines whether the given text is SSML markup.
    */
    bool isSsml(const QString &text);

    /**
    * Parses a block of text into sentences using the application-specified regular expression
    * or (if not specified), the default regular expression.
    * @param text           The message to be spoken.
    * @param appId          The DBUS senderId of the application.
    * @return               List of parsed sentences.
    */

    QStringList parseText(const QString &text, const QString &appId);

    /**
    * Deletes job, removing it from all queues.
    */
    void deleteJob(int removeJobNum);

    /**
    * Assigns a FilterMgr to a job and starts filtering on it.
    */
    void startJobFiltering(SpeechJob* job, const QString& text, bool noSBD);

    /**
    * Waits for filtering to be completed on a job.
    * This is typically called because an app has requested job info that requires
    * filtering to be completed, such as getJobInfo.
    */
    void waitJobFiltering(const SpeechJob* job);

    /**
    * Processes filters by looping across the pool of FilterMgrs.
    * As each FilterMgr finishes, emits appropriate signals and flags it as no longer busy.
    */
    void doFiltering();

    /**
    * Loads notify events from a file.  Clearing data if clear is True.
    */
    void loadNotifyEventsFromFile( const QString& filename, bool clear);

    /**
    * Checks to see if an application has active jobs, and if not and
    * the application has exited, deletes the app and all its jobs.
    * @param appId          DBUS sender id of the application.
    */
    void deleteExpiredApp(const QString appId);

private:
    SpeechDataPrivate* d;
};

#endif // _SPEECHDATA_H_
