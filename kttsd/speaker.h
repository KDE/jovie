/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  
  This class is in charge of getting the messages, warnings and text from
  the queue and calling speech-dispatcher to actually speak the texts.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

#ifndef SPEAKER_H
#define SPEAKER_H

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QEvent>

#include <kspeech.h>

// KTTSD includes.
#include <libspeechd.h>
#include "filtermgr.h"
#include "appdata.h"
#include "speechjob.h"

/**
 * Struct used to keep a pool of FilterMgr objects.
 */
//struct PooledFilterMgr {
//    FilterMgr* filterMgr;       /* The FilterMgr object. */
//    bool busy;                  /* True if the FilterMgr is busy. */
//    SpeechJob* job;             /* The job the FilterMgr is filtering. */
//    TalkerCode* talkerCode;     /* TalkerCode object passed to FilterMgr. */
//};

/**
 * This class is in charge of getting the messages, warnings and text from
 * the queue and call the plug ins function to actually speak the texts.
 */
class SpeakerPrivate;
class Speaker : public QObject
{
Q_OBJECT

public:
    /**
    * singleton accessor
    */
    static Speaker * Instance();

    static void speechdCallback(size_t msg_id, size_t client_id, SPDNotificationType type);

    /**
    * Destructor.
    */
    ~Speaker();
    
    /**
    * (re)initializes the filtermgr
    */
    void init();

    /**
    * Tells the thread to exit.
    * TODO: Is this used anymore?
    */
    void requestExit();

    /**
    * Main processing loop.  Dequeues utterances and sends them to the
    * plugins and/or Audio Player.
    */
    //void doUtterances();

    /**
    * Determine if kttsd is currently speaking any jobs.
    * @return               True if currently speaking any jobs.
    */
    bool isSpeaking();

    /**
    * Get the job number of the current job speaking.
    * @return               Job number of the current job. 0 if no jobs.
    *
    * @see isSpeakingText
    */
    int getCurrentJobNum();

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
    //SpeechJob* getNextSpeakableJob(KSpeech::JobPriority priority);

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

signals:
    /**
    * Emitted when a marker is processed.
    * Currently only emits mtSentenceBegin and mtSentenceEnd.
    * @param appId         The DBUS sender ID of the application that queued the job.
    * @param jobNum        Job Number of the job emitting the marker.
    * @param markerType    The type of marker.
    *                      Currently either mtSentenceBegin or mtSentenceEnd.
    * @param markerData    Data for the marker.
    *                      Currently, this is the sentence number of the sentence
    *                      begun or ended.  Sentence numbers begin at 1.
    */
    void marker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData);

    /**
    * Emitted when the state of a job changes.
    */
    void jobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state);

    /**
     * This signal is emitted when a new job coming in is filtered (or not filtered if no filters
     * are on).
     * @param prefilterText     The text of the speech job
     * @param postfilterText    The text of the speech job after any filters have been applied
     */
    void newJobFiltered(const QString &prefilterText, const QString &postfilterText);

protected:
    /**
    * Processes events posted by ThreadedPlugIns.
    */
    virtual bool event ( QEvent * e );

private slots:
    /**
    * Received from PlugIn objects when they finish asynchronous synthesis.
    */
    void slotSynthFinished();
    /**
    * Received from PlugIn objects when they finish asynchronous synthesis
    * and audibilizing.
    */
    void slotSayFinished();
    /**
    * Received from PlugIn objects when they asynchronously stopText.
    */
    void slotStopped();
    /**
    * Received from transformer (SSMLConvert) when transforming is finished.
    */
    void slotTransformFinished();
    /** Received from PlugIn object when they encounter an error.
    * @param keepGoing               True if the plugin can continue processing.
    *                                False if the plugin cannot continue, for example,
    *                                the speech engine could not be started.
    * @param msg                     Error message.
    */
    void slotError(bool keepGoing, const QString &msg);
    /**
    * Received from Timer when it fires.
    * Check audio player to see if it is finished.
    */
    //void slotTimeout();

    //void slotFilterMgrFinished();
    //void slotFilterMgrStopped();
    
    void slotServiceUnregistered(const QString& serviceName);

private:
    /**
    * Constructor.
    */
    Speaker();

    /**
    * Converts a job state enumerator to a displayable string.
    * @param state           Job state.
    * @return                Displayable string for job state.
    */
    QString jobStateToStr(int state);

    /**
    * get the list of available modules that are configured in speech-dispatcher
    */
    QStringList moduleNames();

    /**
    * Gets the next utterance of the specified priority to be spoken from
    * speechdata and adds it to the queue.
    * @param requestedPriority     Job priority to check for.
    * @return                      True if one or more utterances were added to the queue.
    *
    * If priority is KSpeech::jpAll, checks for waiting ScreenReaderOutput,
    * Warnings, Messages, or Text, in that order.
    * If Warning or Message and interruption messages have been configured,
    * adds those to the queue as well.
    * Determines which plugin should be used for the utterance.
    */
    //bool getNextUtterance(KSpeech::JobPriority requestedPriority);

    /**
    * Given an iterator pointing to the m_uttQueue, deletes the utterance
    * from the queue.  If the utterance is currently being processed by a
    * plugin or the Audio Player, halts that operation and deletes Audio Player.
    * Also takes care of deleting temporary audio file.
    * @param it                      Iterator pointer to m_uttQueue.
    * @return                        Iterator pointing to the next utterance in the
    *                                queue, or m_uttQueue.end().
    */
    //uttIterator deleteUtterance(uttIterator it);

    /**
    * Given an iterator pointing to the m_uttQueue, starts playing audio if
    *   1) An audio file is ready to be played, and
    *   2) It is not already playing.
    * If another audio player is already playing, pauses it before starting
    * the new audio player.
    * @param it                      Iterator pointer to m_uttQueue.
    * @return                        True if an utterance began playing or resumed.
    */
    //bool startPlayingUtterance(uttIterator it);

    /**
    * Delete any utterances in the queue with this jobNum.
    * @param jobNum          The Job Number of the utterance(s) to delete.
    * If currently processing any deleted utterances, stop them.
    */
    //void deleteUtteranceByJobNum(int jobNum);

    /**
    * Takes care of emitting reading interrupted/resumed and sentence started signals.
    * Should be called just before audibilizing an utterance.
    * @param it                      Iterator pointer to m_uttQueue.
    */
    //void prePlaySignals(uttIterator it);

    /**
    * Takes care of emitting sentenceFinished signal.
    * Should be called immediately after an utterance has completed playback.
    * @param it                      Iterator pointer to m_uttQueue.
    */
    //void postPlaySignals(uttIterator it);

    /**
    * Constructs a temporary filename for plugins to use as a suggested filename
    * for synthesis to write to.
    * @return                        Full pathname of suggested file.
    */
    //QString makeSuggestedFilename();

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
    //void startJobFiltering(SpeechJob* job, const QString& text, bool noSBD);

    /**
    * Waits for filtering to be completed on a job.
    * This is typically called because an app has requested job info that requires
    * filtering to be completed, such as getJobInfo.
    */
    //void waitJobFiltering(const SpeechJob* job);

    /**
    * Processes filters by looping across the pool of FilterMgrs.
    * As each FilterMgr finishes, emits appropriate signals and flags it as no longer busy.
    */
    //void doFiltering();

    /**
    * Checks to see if an application has active jobs, and if not and
    * the application has exited, deletes the app and all its jobs.
    * @param appId          DBUS sender id of the application.
    */
    void deleteExpiredApp(const QString appId);

private:
    SpeakerPrivate* d;
    static Speaker * m_instance;
};

#endif // SPEAKER_H
