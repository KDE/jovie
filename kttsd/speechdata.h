/*************************************************** vim:set ts=4 sw=4 sts=4:
  speechdata.h
  This contains the SpeechData class which is in charge of maintaining
  all the data on the memory.
  It maintains queues, mutex, a wait condition and has methods to enque
  messages and warnings and manage the text that is thread safe.
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

#ifndef _SPEECHDATA_H_
#define _SPEECHDATA_H_

#include <qptrqueue.h>
#include <qptrlist.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kconfig.h>
#include <kspeech.h>

/**
 * Struct containing a text cell, for messages, warnings, and texts.
 * Contains the text itself, the associated talker, 
 * the ID of the application that requested it be spoken, and a sequence number.
 */
struct mlText{
    QString talker;              /* Language code for the sentence. */
    QString text;                /* Text of sentence. */
    QCString appId;              /* DCOP senderId of the application that requested the speech. */
    uint jobNum;                 /* Text jobNum.  Only applies to text messages; not warning and messages. */
    uint seq;                    /* Sequence number. */
};

/**
 * Struct containing a text job.
 */
struct mlJob {
    uint jobNum;                 /* Job number. */
    kspeech::kttsdJobState state; /* Job state. */
    QCString appId;              /* DCOP senderId of the application that requested the speech job. */
    QString talker;              /* Language code in which to speak the text. */
    int seq;                     /* Current sentence being spoken. */
    QStringList sentences;       /* List of sentences in the job. */
};

/**
 * SpeechData class which is in charge of maintaining all the data on the memory.
 * It maintains queues, mutex, a wait condition and has methods to enque
 * messages and warnings and manage the text queues that is thread safe.
 * We could say that this is the common repository between the KTTSD class
 * (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
 * functions)
 */
class SpeechData : public QObject {
    Q_OBJECT

    public:
        /**
        * Constructor
        * Sets text to be stopped and warnings and messages queues to be autodelete (thread safe)
        */
        SpeechData();

        /**
        * Destructor
        */
        ~SpeechData();

        /**
        * Read the configuration
        */
        bool readConfig();

        /**
        * Add a new warning to the queue (thread safe)
        */
        void enqueueWarning( const QString &, const QString &talker=NULL, const QCString& appId=NULL );

        /**
        * Pop (get and erase) a warning from the queue (thread safe)
        */
        mlText dequeueWarning();

        /**
        * Are there any Warnings (thread safe)
        */
        bool warningInQueue();

        /**
        * Add a new message to the queue (thread safe)
        */
        void enqueueMessage( const QString &, const QString &talker=NULL, const QCString& appId=NULL );

        /**
        * Pop (get and erase) a message from the queue (thread safe)
        */
        mlText dequeueMessage();

        /**
        * Are there any Message (thread safe)
        */
        bool messageInQueue();

        /**
        * Get a text sentence to speak.
        */
        mlText getSentenceText();

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
        void setSentenceDelimiter(const QString &delimiter, const QCString appId=NULL);
        
        /**
        * Returns true if curently speaking (thread safe)
        */
        bool currentlyReading();

        /* The following methods correspond to the methods in kspeech interface. */
        
        /**
        * Queue a text job.  Does not start speaking the text.
        * (thread safe)
        * @param text           The message to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default plugin.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default plugin.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               Job number.
        *
        * The text is parsed into individual sentences.  Call getTextCount to retrieve
        * the sentence count.  Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see startText.
        */
        uint setText(const QString &text, const QString &talker=NULL, const QCString& appId=NULL);
        
        /**
        * Get the number of sentences in a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               The number of sentences in the job.  -1 if no such job.
        *
        * The sentences of a job are given sequence numbers from 1 to the number returned by this
        * method.  The sequence numbers are emitted in the sentenceStarted and sentenceFinished signals.
        */
        int getTextCount(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Get the job number of the current text job.
        * (thread safe)
        * @return               Job number of the current text job. 0 if no jobs.
        *
        * Note that the current job may not be speaking.  @see isSpeaking.  @see getTextJobState.
        */
        uint getCurrentTextJob();
        
        /**
        * Get the number of jobs in the text job queue.
        * (thread safe)
        * @return               Number of text jobs in the queue.  0 if none.
        */
        uint getTextJobCount();
        
        /**
        * Get a comma-separated list of text job numbers in the queue.
        * @return               Comma-separated list of text job numbers in the queue.
        */
        QString getTextJobNumbers();
        
        /**
        * Get the state of a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               State of the job. -1 if invalid job number.
        */
        int getTextJobState(const uint jobNum=0, const QCString& appId=NULL);
        
        /**
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               A QDataStream containing information about the job.
        *                       Blank if no such job.
        *
        * The stream contains the following elements:
        *   - int state         Job state.
        *   - QCString appId    DCOP senderId of the application that requested the speech job.
        *   - QString talker    Language code in which to speak the text.
        *   - int seq           Current sentence being spoken.  Sentences are numbered starting at 1.
        *   - int sentenceCount Total number of sentences in the job.
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
            stream >> state;
            stream >> appId;
            stream >> talker;
            stream >> seq;
            stream >> sentenceCount;
          @endverbatim
        */
        QByteArray getTextJobInfo(const uint jobNum=0, const QCString& appId=NULL);
       
        /**
        * Return a sentence of a job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param seq            Sequence number of the sentence.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               The specified sentence in the specified job.  If not such
        *                       job or sentence, returns "".
        */
        virtual QString getTextJobSentence(const uint jobNum=0, const uint seq=1, const QCString& appId=NULL);
       
        /**
        * Determine if kttsd is currently speaking any text jobs.
        * (thread safe)
        * @return               True if currently speaking any text jobs.
        */
        bool isSpeakingText();
        
        /**
        * Remove a text job from the queue.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * The job is deleted from the queue and the textRemoved signal is emitted.
        *
        * If there is another job in the text queue, and it is marked speakable,
        * that job begins speaking.
        */
        void removeText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Start a text job at the beginning.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * Rewinds the job to the beginning.
        *
        * The job is marked speakable.
        * If there are other speakable jobs preceeding this one in the queue,
        * those jobs continue speaking and when finished, this job will begin speaking.
        * If there are no other speakable jobs preceeding this one, it begins speaking.
        *
        * The textStarted signal is emitted when the text job begins speaking.
        * When all the sentences of the job have been spoken, the job is marked for deletion from
        * the text queue and the textFinished signal is emitted.
        */
        void startText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Stop a text job and rewind to the beginning.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * The job is marked not speakable and will not be speakable until startText or resumeText
        * is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the textStopped signal is emitted and the job stops speaking.
        * If there are speakable jobs following this one, they are started.
        */
        void stopText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Pause a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * The job is marked as paused and will not be speakable until resumeText or
        * startText is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the textPaused signal is emitted and the job stops speaking.
        * If there are speakable jobs following this one, they are started.
        */
        void pauseText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Start or resume a text job where it was paused.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * The job is marked speakable.
        *
        * If the job was not paused, it is the same as calling startText.  @see startText
        *
        * If there are speaking jobs preceeding this one in the queue, those jobs continue speaking and,
        * when finished this job will begin speaking where it left off.
        *
        * The textResumed signal is emitted when the job resumes.
        */
        void resumeText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Change the talker for a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param talker         New code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        */
        void changeTextTalker(const uint jobNum=0, const QString &talker=NULL, const QCString& appId=NULL);
        
        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        *
        * If the job is currently speaking, it is paused.
        * If the next job in the queue is speakable, it begins speaking.
        */
        void moveTextLater(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Go to the previous paragraph in a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        */
        void prevParText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Go to the previous sentence in the queue.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        */
        void prevSenText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Go to next sentence in a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        */
        void nextSenText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Go to next paragraph in a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application,
        *                       but if no such job, applies to the last job queued by any application.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        */
        void nextParText(const uint jobNum=0, const QCString& appId=NULL);

        /**
        * Wait condition for new text, messages or warnings.
        * When there's no text, messages or warnings this wait condition
        * will prevent KTTSD from doing useless and CPU consuming loops.
        */
        QWaitCondition newTMW;

        /* The following properties come from the configuration. */
        
        /**
        * Text pre message
        */
        QString textPreMsg;

        /**
        * Text pre message enabled ?
        */
        bool textPreMsgEnabled;

        /**
        * Text pre sound
        */
        QString textPreSnd;

        /**
        * Text pre sound enabled ?
        */
        bool textPreSndEnabled;

        /**
        * Text post message
        */
        QString textPostMsg;

        /**
        * Text post message enabled ?
        */
        bool textPostMsgEnabled;

        /**
        * Text post sound
        */
        QString textPostSnd;

        /**
        * Text post sound enabled ?
        */
        bool textPostSndEnabled;

        /**
        * Paragraph pre message
        */
        QString parPreMsg;

        /**
        * Paragraph pre message enabled ?
        */
        bool parPreMsgEnabled;

        /**
        * Paragraph pre sound
        */
        QString parPreSnd;

        /**
        * Paragraph pre sound enabled ?
        */
        bool parPreSndEnabled;

        /**
        * Paragraph post message
        */
        QString parPostMsg;

        /**
        * Paragraph post message enabled ?
        */
        bool parPostMsgEnabled;

        /**
        * Paragraph post sound
        */
        QString parPostSnd;

        /**
        * Paragraph post sound enabled ?
        */
        bool parPostSndEnabled;
        
        /**
        * Notify.
        */
        bool notify;
        
        /**
        * Notify passive popups only.
        */
        bool notifyPassivePopupsOnly;

        /**
        * Default talker.
        */
        QString defaultTalker;

        /**
        * Configuration
        */
        KConfig *config;

    signals:
        /**
        * This signal is emitted whenever a new text job is added to the queue.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textSet(const QCString& appId, const uint jobNum);

        /* The following signals correspond to the signals in the kspeech interface. */
        
        /**
        * This signal is emitted whenever speaking of a text job begins.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textStarted(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a text job is finished.  The job has
        * been marked for deletion from the queue and will be deleted 10 seconds later
        * unless startText is called.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textFinished(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job stops speaking.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textStopped(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job is paused.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textPaused(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted when a text job, that was previously paused, resumes speaking.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textResumed(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a text job is deleted from the queue.
        * The job is no longer in the queue when this signal is emitted.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textRemoved(const QCString& appId, const uint jobNum);
        
    private:
        /**
        * Queue of warnings
        */
        QPtrQueue<mlText> warnings;

        /**
        * Mutex for reading/writing warnings
        */
        QMutex warningsMutex;

        /**
        * Queue of messages
        */
        QPtrQueue<mlText> messages;

        /**
        * Mutex for reading/writing messages
        */
        QMutex messagesMutex;
      
        /**
        * Queue of text jobs.
        */
        QPtrList<mlJob> textJobs;
      
        /**
        * Job counter.  Each new job increments this counter.
        */
        uint jobCounter;

        /**
        * Mutex for reading/writing jobs.
        */
        QMutex textMutex;
      
        /**
        * Talker of the text
        */
        QString textTalker;
      
        /**
        * True when currrently speaking a job.  The job is textJobs.current();
        */
        bool reading;
      
        /**
        * Iterator for the text of a job.  0 if no current job.
        * Note that textIterator can be nonzero even when reading is false.
        */
        QStringList::Iterator* textIterator;
        
        /**
        * Map of sentence delimiters.  One per app.  If none specified for an app, uses default.
        */
        QMap<QCString, QString> sentenceDelimiters;
        
        /**
        * Mutex for reading/writing sentence delimiters.
        */
        QMutex delimiterMutex;

        /**
        * Given an appId, returns the first job with that appId.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the first job in the queue.
        * Does not change textJobs.current().
        */
        mlJob* findFirstJobByAppId(const QCString& appId);
      
        /**
        * Given an appId, returns the last (most recently queued) job with that appId.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the last job in the queue.
        * Does not change textJobs.current().
        */
        mlJob* findLastJobByAppId(const QCString& appId);

        /**
        * Given an appId, returns the last (most recently queued) job with that appId,
        * or if no such job, the last (most recent) job in the queue.
        * @param appId          The DCOP senderId of the application.  NULL if kttsd.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the last job in the queue.
        * Does not change textJobs.current().
        */
        mlJob* SpeechData::findAJobByAppId(const QCString& appId);
                
        /**
        * Given a jobNum, returns the first job with that jobNum.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * Does not change textJobs.current().
        */
        mlJob* findJobByJobNum(const uint jobNum);

        /**
        * Given a jobNum, returns the appId of the application that owns the job.
        * @param jobNum         Job number of the text job.
        * @return               appId of the job.
        * If no such job, returns "".
        * Does not change textJobs.current().
        */
         QCString getAppIdByJobNum(const uint jobNum);

        /**
        * Start the next speakable job on the queue.
        * @return               Job number of the started text job.
        * Should not be called when another job is active.
        * Leaves textJobs.current() pointing to the started job.
        * Sets up the text iterator for the job.
        * Returns 0 if no speakable jobs.
        */
        uint startNextJob();
        
        /**
        * Delete expired jobs.  At most, one finished job is kept on the queue.
        * @param finishedJobNum Job number of a job that just finished
        * The just finished job is not deleted, but any other finished jobs are.
        * Does not change the textJobs.current() pointer.
        */
        void deleteExpiredJobs(const uint finishedJobNum);
};

#endif // _SPEECHDATA_H_
