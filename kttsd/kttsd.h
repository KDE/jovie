/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.h
  KTTSD main class
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _KTTSD_H_
#define _KTTSD_H_

#include "speechdata.h"
#include "speaker.h"
#include "kspeech.h"

/**
 * KTTSD - the KDE Text-to-speech Deamon.
 *
 * Provides the capability for applications to speak text.
 * Applications may speak text by sending DCOM messages to application "kttsd" object "kspeech".
 *
 * @author José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 * @author Olaf Schmidt <ojschmidt@kde.org>
 * @author Gary Cramblitt <garycramblitt@comcast.net>
 */

class KTTSD : public QObject, virtual public kspeech
{
    Q_OBJECT

    public:
        /**
        * Constructor.
        *
        * Create objects, speechData and speaker.
        * Start thread
        */
        KTTSD(QObject *parent=0, const char *name=0);

        /**
        * Destructor.
        *
        * Terminate speaker thread.
        */
        ~KTTSD();

        /**
        * Holds if we are ok to go or not.
        */
        bool ok;
    
        /** DCOP exported functions for kspeech interface **/
        
        /**
        * Determine whether the currently-configured speech plugin supports a speech markup language.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @param markupType     The kttsd code for the desired speech markup language.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports the indicated speech markup language.
        * @see kttsdMarkupType
        */
        virtual bool supportsMarkup(const QString &talker=NULL, const uint markupType = 0);
        
        /**
        * Determine whether the currently-configured speech plugin supports markers in speech markup.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        * @return               True if the plugin currently configured for the indicated
        *                       talker supports markers.
        */
        virtual bool supportsMarkers(const QString &talker=NULL);
        
        /**
        * Say a warning.  The warning will be spoken when the current sentence
        * stops speaking and takes precedence over Messages and regular text.  Warnings should only
        * be used for high-priority messages requiring immediate user attention, such as
        * "WARNING. CPU is overheating."
        * @param warning        The warning to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC sayWarning(const QString &warning, const QString &talker=NULL);

        /**
        * Say a message.  The message will be spoken when the current text paragraph
        * stops speaking.  Messages should be used for one-shot messages that can't wait for
        * normal text messages to stop speaking, such as "You have mail.".
        * @param message        The message to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no talker has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC sayMessage(const QString &message, const QString &talker=NULL);

        /**
        * Sets the GREP pattern that will be used as the sentence delimiter.
        * @param delimiter      A valid GREP pattern.
        *
        * The default sentence delimiter is
          @verbatim
              ([\\.\\?\\!\\:\\;])\\s
          @endverbatim
        *
        * Note that backward slashes must be escaped.
        *
        * Changing the sentence delimiter does not affect other applications.
        * @see sentenceparsing
        */
        virtual ASYNC setSentenceDelimiter(const QString &delimiter);
        
        /**
        * Queue a text job.  Does not start speaking the text.
        * @param text           The message to be spoken.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default plugin.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default plugin.
        * @return               Job number.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
        * Call @ref getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SMML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call @ref startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see getTextCount
        * @see startText
        */
        virtual uint setText(const QString &text, const QString &talker=NULL);
        
        /**
        * Queue a text job from the contents of a file.  Does not start speaking the text.
        * @param filename       Full path to the file to be spoken.  May be a URL.
        * @param talker         Code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        * @return               Job number.  0 if an error occurs.
        *
        * Plain text is parsed into individual sentences using the current sentence delimiter.
        * Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
        * Call @ref getTextCount to retrieve the sentence count after calling setText.
        *
        * The text may contain speech mark language, such as Sable, JSML, or SMML,
        * provided that the speech plugin/engine support it.  In this case,
        * sentence parsing follows the semantics of the markup language.
        *
        * Call @ref startText to mark the job as speakable and if the
        * job is the first speakable job in the queue, speaking will begin.
        * @see getTextCount
        * @see startText
        */
        virtual uint setFile(const QString &filename, const QString &talker=NULL);
        
        /**
        * Get the number of sentences in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        * @return               The number of sentences in the job.  -1 if no such job.
        *
        * The sentences of a job are given sequence numbers from 1 to the number returned by this
        * method.  The sequence numbers are emitted in the @ref sentenceStarted and
        * @ref sentenceFinished signals.
        */
        virtual int getTextCount(const uint jobNum=0);

        /**
        * Get the job number of the current text job.
        * @return               Job number of the current text job. 0 if no jobs.
        *
        * Note that the current job may not be speaking. See @ref isSpeakingText.
        * @see getTextJobState.
        * @see isSpeakingText
        */
        virtual uint getCurrentTextJob();
        
        /**
        * Get the number of jobs in the text job queue.
        * @return               Number of text jobs in the queue.  0 if none.
        */
        virtual uint getTextJobCount();
        
        /**
        * Get a comma-separated list of text job numbers in the queue.
        * @return               Comma-separated list of text job numbers in the queue.
        */
        virtual QString getTextJobNumbers();
        
        /**
        * Get the state of a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        * @return               State of the job. -1 if invalid job number.
        *
        * @see kttsdJobState
        */
        virtual int getTextJobState(const uint jobNum=0);
        
        /**
        * Get information about a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
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
        virtual QByteArray getTextJobInfo(const uint jobNum=0);
       
        /**
        * Return a sentence of a job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        * @param seq            Sequence number of the sentence.
        * @return               The specified sentence in the specified job.  If not such
        *                       job or sentence, returns "".
        */
        virtual QString getTextJobSentence(const uint jobNum=0, const uint seq=1);
       
        /**
        * Determine if kttsd is currently speaking any text jobs.
        * @return               True if currently speaking any text jobs.
        */
        virtual bool isSpeakingText();
        
        /**
        * Remove a text job from the queue.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        *
        * The job is deleted from the queue and the @ref textRemoved signal is emitted.
        *
        * If there is another job in the text queue, and it is marked speakable,
        * that job begins speaking.
        */
        virtual ASYNC removeText(const uint jobNum=0);

        /**
        * Start a text job at the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
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
        virtual ASYNC startText(const uint jobNum=0);

        /**
        * Stop a text job and rewind to the beginning.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        *
        * The job is marked not speakable and will not be speakable until @ref startText or @ref resumeText
        * is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the @ref textStopped signal is emitted and the job stops speaking.
        * Depending upon the speech engine and plugin used, speeking may not stop immediately
        * (it might finish the current sentence).
        */
        virtual ASYNC stopText(const uint jobNum=0);

        /**
        * Pause a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
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
        virtual ASYNC pauseText(const uint jobNum=0);

        /**
        * Start or resume a text job where it was paused.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        *
        * The job is marked speakable.
        *
        * If the job was not paused, it is the same as calling @ref startText.
        *
        * If there are speaking jobs preceeding this one in the queue, those jobs continue speaking and,
        * when finished this job will begin speaking where it left off.
        *
        * The @ref textResumed signal is emitted when the job resumes.
        * @see pauseText
        */
        virtual ASYNC resumeText(const uint jobNum=0);
        
        /**
        * Change the talker for a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        * @param talker         New code for the language to be spoken in.  Example "en".
        *                       If NULL, defaults to the user's default talker.
        *                       If no plugin has been configured for the specified language code,
        *                       defaults to the user's default talker.
        */
        virtual ASYNC changeTextTalker(const uint jobNum=0, const QString &talker=NULL);
        
        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        *
        * If the job is currently speaking, it is paused.
        * If the next job in the queue is speakable, it begins speaking.
        */
        virtual ASYNC moveTextLater(const uint jobNum=0);

        /**
        * Go to the previous paragraph in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        */
        virtual ASYNC prevParText(const uint jobNum=0);

        /**
        * Go to the previous sentence in the queue.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        */
        virtual ASYNC prevSenText(const uint jobNum=0);

        /**
        * Go to next sentence in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        */
        virtual ASYNC nextSenText(const uint jobNum=0);

        /**
        * Go to next paragraph in a text job.
        * @param jobNum         Job number of the text job.
        *                       If zero, applies to the last job queued by the application.
        */
        virtual ASYNC nextParText(const uint jobNum=0);
        
        /**
        * Add the clipboard contents to the text queue and begin speaking it.
        */
        virtual ASYNC speakClipboard();
        
        /**
        * Displays the %KTTS Manager dialog.  In this dialog, the user may backup or skip forward in
        * any text job by sentence or paragraph, rewind jobs, pause or resume jobs, or
        * delete jobs.
        */
        virtual void showDialog();

        /**
        * Stop the service.
        */
        virtual void kttsdExit();

        /**
        * Re-start %KTTSD.
        */
        virtual void reinit();
        
    private slots:
        /*
         * These functions are called whenever
         * the status of the speaker object has changed
         */
        void slotSentenceStarted(QString text, QString language, 
            const QCString& appId, const uint jobNum, const uint seq);
        void slotSentenceFinished(const QCString& appId, const uint jobNum, const uint seq);

        /*
         * These functions are called whenever
         * the status of the speechData object has changed
         */
        void slotTextSet(const QCString& appId, const uint jobNum);
        void slotTextStarted(const QCString& appId, const uint jobNum);
        void slotTextFinished(const QCString& appId, const uint jobNum);
        void slotTextStopped(const QCString& appId, const uint jobNum);
        void slotTextPaused(const QCString& appId, const uint jobNum);
        void slotTextResumed(const QCString& appId, const uint jobNum);
        void slotTextRemoved(const QCString& appId, const uint jobNum);

        /*
         * Fires whenever user clicks Apply or OK buttons in Settings dialog.
         */
        void configCommitted();
    
    private:
        /*
         * Initialize the speaker.
         */
        bool initializeSpeaker();

        /*
         * Returns the senderId (appId) of the DCOP application that called us.
         * @return appId         The DCOP sendId of calling application.  NULL if called internally by kttsd itself.
         */
        const QCString getAppId();
        
        /*
         * SpeechData containing all the data and the manipulating methods for all KTTSD
         */
        SpeechData *speechData;

        /*
         * Speaker that will be run as another thread, actually saying the messages, warnings, and texts
         */
        Speaker *speaker;

};

#endif // _KTTSD_H_
