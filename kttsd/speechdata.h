/*************************************************** vim:set ts=4 sw=4 sts=4:
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

#ifndef _SPEECHDATA_H_
#define _SPEECHDATA_H_

#include <qptrqueue.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kconfig.h>
#include <kspeech.h>

// Change to 1 to enable SSML support.
#define SUPPORT_SSML 1

/**
* Struct containing a text cell, for messages, warnings, and texts.
* Contains the text itself, the associated talker, 
* the ID of the application that requested it be spoken, and a sequence number.
*/
struct mlText{
    QString talker;              /* Requested Talker code for the sentence. */
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
    QString talker;              /* Requested Talker code in which to speak the text. */
    int seq;                     /* Current sentence being spoken. */
    QValueList<int> partSeqNums; /* List containing last sequence number for each part of a job. */
    QStringList sentences;       /* List of sentences in the job. */
};

/**
 * SpeechData class which is in charge of maintaining all the data on the memory.
 * It maintains queues and has methods to enque
 * messages and warnings and manage the text queues.
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
        * Say a message as soon as possible, interrupting any other speech in progress.
        * IMPORTANT: This method is reserved for use by Screen Readers and should not be used
        * by any other applications.
        * @param msg            The message to be spoken.
        * @param talker         Code for the talker to speak the message.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        * @param appId          The DCOP senderId of the application.
        *
        * If an existing Screen Reader output is in progress, it is stopped and discarded and
        * replaced with this new message.
        */
        void setScreenReaderOutput(const QString &msg, const QString &talker,
            const QCString& appId);

        /**
        * Given an appId, returns the last (most recently queued) Job Number with that appId,
        * or if no such job, the Job Number of the last (most recent) job in the queue.
        * @param appId          The DCOP senderId of the application.
        * @return               Job Number of the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the Job Number of the last job in the queue.
        * Does not change textJobs.current().
        */
        uint findAJobNumByAppId(const QCString& appId);

        /**
        * Retrieves the Screen Reader Output.
        */
        mlText* getScreenReaderOutput();

        /**
        * Returns true if Screen Reader Output is ready to be spoken.
        */
        bool screenReaderOutputReady();

       /**
        * Add a new warning to the queue.
        */
        void enqueueWarning( const QString &, const QString &talker,
            const QCString& appId);

        /**
        * Pop (get and erase) a warning from the queue.
        * @return                Pointer to mlText structure containing the warning.
        *
        * Caller is responsible for deleting the structure.
        */
        mlText* dequeueWarning();

        /**
        * Are there any Warnings?
        */
        bool warningInQueue();

        /**
        * Add a new message to the queue.
        */
        void enqueueMessage( const QString &, const QString &talker,
            const QCString&);

        /**
        * Pop (get and erase) a message from the queue.
        * @return                Pointer to mlText structure containing the message.
        *
        * Caller is responsible for deleting the structure.
        */
        mlText* dequeueMessage();

        /**
        * Are there any Messages?
        */
        bool messageInQueue();

        /**
        * Sets the GREP pattern that will be used as the sentence delimiter.
        * @param delimiter      A valid GREP pattern.
        * @param appId          The DCOP senderId of the application.
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
        void setSentenceDelimiter(const QString &delimiter, const QCString appId);

        /* The following methods correspond to the methods in kspeech interface. */

        /**
        * Queue a text job.  Does not start speaking the text.
        * (thread safe)
        * @param text           The message to be spoken.
        * @param talker         Code for the talker to speak the text.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        * @param appId          The DCOP senderId of the application.
        * @return               Job number.
        *
        * The text is parsed into individual sentences.  Call getTextCount to retrieve
        * the sentence count.  Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see startText.
        */
        uint setText(const QString &text, const QString &talker, const QCString& appId);

        /**
        * Adds another part to a text job.  Does not start speaking the text.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        * @param text           The message to be spoken.
        * @param appId          The DCOP senderId of the application.
        * @return               Part number for the added part.  Parts are numbered starting at 1.
        *
        * The text is parsed into individual sentences.  Call getTextCount to retrieve
        * the sentence count.  Call startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see setText.
        * @see startText.
        */
        int appendText(const QString &text, const uint jobNum, const QCString& appId);

        /**
        * Get the number of sentences in a text job.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        * @return               The number of sentences in the job.  -1 if no such job.
        *
        * The sentences of a job are given sequence numbers from 1 to the number returned by this
        * method.  The sequence numbers are emitted in the sentenceStarted and sentenceFinished signals.
        */
        int getTextCount(const uint jobNum);

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
        * @return               State of the job. -1 if invalid job number.
        */
        int getTextJobState(const uint jobNum);

        /**
        * Set the state of a text job.
        * @param jobNum         Job Number of the job.
        * @param state          New state for the job.
        *
        **/
        void setTextJobState(const uint jobNum, const kspeech::kttsdJobState state);

        /**
        * Get information about a text job.
        * @param jobNum         Job number of the text job.
        * @return               A QDataStream containing information about the job.
        *                       Blank if no such job.
        *
        * The stream contains the following elements:
        *   - int state         Job state.
        *   - QCString appId    DCOP senderId of the application that requested the speech job.
        *   - QString talker    Talker code as requested by application.
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
        QByteArray getTextJobInfo(const uint jobNum);

        /**
        * Return a sentence of a job.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the sentence.
        * @return               The specified sentence in the specified job.  If no such
        *                       job or sentence, returns "".
        */
        QString getTextJobSentence(const uint jobNum, const uint seq=1);

        /**
        * Remove a text job from the queue.
        * (thread safe)
        * @param jobNum         Job number of the text job.
        *
        * The job is deleted from the queue and the textRemoved signal is emitted.
        */
        void removeText(const uint jobNum);

        /**
        * Change the talker for a text job.
        * @param jobNum         Job number of the text job.
        * @param talker         New code for the talker to do speaking.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified Talker code,
        *                       defaults to the closest matching talker.
        */
        void changeTextTalker(const uint jobNum, const QString &talker);

        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        */
        void moveTextLater(const uint jobNum);

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
        int jumpToTextPart(const int partNum, const uint jobNum);

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
        uint moveRelTextSentence(const int n, const uint jobNum);

        /**
        * Given a jobNum, returns the first job with that jobNum.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * Does not change textJobs.current().
        */
        mlJob* findJobByJobNum(const uint jobNum);

        /**
        * Given a Job Number, returns the next speakable text job on the queue.
        * @param prevJobNum       Current job number (which should not be returned).
        * @return                 Pointer to mlJob structure of the first speakable job
        *                         not equal prevJobNum.  If no such job, returns null.
        *
        * Caller must not delete the job.
        */
        mlJob* getNextSpeakableJob(const uint prevJobNum);

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
        mlText* getNextSentenceText(const uint prevJobNum, const uint prevSeq);

        /**
        * Given a Job Number, sets the current sequence number of the job.
        * @param jobNum          Job Number.
        * @param seq             Sequence number.
        * If for some reason, the job does not exist, nothing happens.
        */
        void setJobSequenceNum(const uint jobNum, const uint seq);

        /**
        * Given a Job Number, returns the current sequence number of the job.
        * @param jobNum         Job Number.
        * @return               Sequence number of the job.  If no such job, returns 0.
        */
        uint getJobSequenceNum(const uint jobNum);

        /**
        * Given a jobNum, returns the appId of the application that owns the job.
        * @param jobNum         Job number of the text job.
        * @return               appId of the job.
        * If no such job, returns "".
        * Does not change textJobs.current().
        */
        QCString getAppIdByJobNum(const uint jobNum);

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
        * Configuration
        */
        KConfig *config;

    signals:
        /**
        * This signal is emitted whenever a new text job is added to the queue.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textSet(const QCString& appId, const uint jobNum);

        /**
        * This signal is emitted whenever a new part is appended to a text job.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        * @param partNum        Part number of the new part.  Parts are numbered starting
        *                       at 1.
        */
        void textAppended(const QCString& appId, const uint jobNum, const int partNum);

        /**
        * This signal is emitted whenever a text job is deleted from the queue.
        * The job is no longer in the queue when this signal is emitted.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textRemoved(const QCString& appId, const uint jobNum);

    private:
        /**
        * Screen Reader Output.
        */
        mlText screenReaderOutput;

        /**
        * Queue of warnings
        */
        QPtrQueue<mlText> warnings;

        /**
        * Queue of messages
        */
        QPtrQueue<mlText> messages;

        /**
        * Queue of text jobs.
        */
        QPtrList<mlJob> textJobs;

        /**
        * Job counter.  Each new job increments this counter.
        */
        uint jobCounter;

        /**
        * Talker of the text
        */
        QString textTalker;

        /**
        * Map of sentence delimiters.  One per app.  If none specified for an app, uses default.
        */
        QMap<QCString, QString> sentenceDelimiters;

#if SUPPORT_SSML
        /**
        * Determines whether the given text is SSML markup.
        */
        bool isSsml(const QString &text);
#endif

        /**
        * Given an appId, returns the last (most recently queued) job with that appId.
        * @param appId          The DCOP senderId of the application.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the last job in the queue.
        * Does not change textJobs.current().
        */
        mlJob* findLastJobByAppId(const QCString& appId);

        /**
        * Given an appId, returns the last (most recently queued) job with that appId,
        * or if no such job, the last (most recent) job in the queue.
        * @param appId          The DCOP senderId of the application.
        * @return               Pointer to the text job.
        * If no such job, returns 0.
        * If appId is NULL, returns the last job in the queue.
        * Does not change textJobs.current().
        */
        mlJob* SpeechData::findAJobByAppId(const QCString& appId);

        /**
        * Given a job and a sequence number, returns the part that sentence is in.
        * If no such job or sequence number, returns 0.
        * @param job            The text job.
        * @param seq            Sequence number of the sentence.  Sequence numbers begin with 1.
        * @return               Part number of the part the sentence is in.  Parts are numbered
        *                       beginning with 1.  If no such job or sentence, returns 0.
        */
        int getJobPartNumFromSeq(const mlJob& job, const int seq);

        /**
        * Parses a block of text into sentences using the application-specified regular expression
        * or (if not specified), the default regular expression.
        * @param text           The message to be spoken.
        * @param appId          The DCOP senderId of the application.
        * @return               List of parsed sentences.
        */

        QStringList SpeechData::parseText(const QString &text, const QCString &appId);

        /**
        * Delete expired jobs.  At most, one finished job is kept on the queue.
        * @param finishedJobNum Job number of a job that just finished
        * The just finished job is not deleted, but any other finished jobs are.
        * Does not change the textJobs.current() pointer.
        */
        void deleteExpiredJobs(const uint finishedJobNum);
};

#endif // _SPEECHDATA_H_
