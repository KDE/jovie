/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.

  This class is in charge of getting the messages, warnings and text from
  the queue and call the plug ins function to actually speak the texts.
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

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

// Qt includes.
#include <qobject.h>
#include <qvaluevector.h>
#include <qevent.h>

// KTTSD includes.
#include <speechdata.h>
#include <pluginproc.h>
#include <stretcher.h>
#include <talkercode.h>

#if SUPPORT_SSML
#include <ssmlconvert.h>
#endif

class Player;
class QTimer;

/**
* Type of utterance.
*/
enum uttType
{
    utText,                      /**< Text */
    utInterruptMsg,              /**< Interruption text message */
    utInterruptSnd,              /**< Interruption sound file */
    utResumeMsg,                 /**< Resume text message */
    utResumeSnd,                 /**< Resume sound file */
    utMessage,                   /**< Message */
    utWarning,                   /**< Warning */
    utScreenReader,              /**< Screen Reader Output */
    utStartOfJob,                /**< Start-of-job */
    utEndOfJob                   /**< End-of-job */
};

/**
* Processing state of an utterance.
*/
enum uttState
{
    usNone,                      /**< Null state. Brand new utterance. */
#if SUPPORT_SSML
    usWaitingTransform,          /**< Waiting to be transformed (XSLT) */
    usTransforming,              /**< Transforming the utterance (XSLT). */
#endif 
    usWaitingSay,                /**< Waiting to start synthesis. */
    usWaitingSynth,              /**< Waiting to be synthesized and audibilized. */
    usWaitingSignal,             /**< Waiting to emit a textStarted or textFinished signal. */
    usSaying,                    /**< Plugin is synthesizing and audibilizing. */
    usSynthing,                  /**< Plugin is synthesizing only. */
    usSynthed,                   /**< Plugin has finished synthesizing.  Ready for stretch. */
    usStretching,                /**< Adjusting speed. */
    usStretched,                 /**< Speed adjustment finished.  Ready for playback. */
    usPlaying,                   /**< Playing on Audio Player. */
    usPaused,                    /**< Paused on Audio Player due to user action. */
    usPreempted,                 /**< Paused on Audio Player due to Screen Reader Output. */
    usFinished                   /**< Ready for deletion. */
};

/**
* Structure containing an utterance being synthesized or audibilized.
*/
struct Utt{
    mlText* sentence;            /* The text, talker, appId, and sequence num. */
    uttType utType;              /* The type of utterance (text, msg, screen reader) */
    uttState state;              /* Processing state of the utterance. */
#if SUPPORT_SSML
    SSMLConvert* transformer;    /* XSLT transformer. */
#endif
    PlugInProc* plugin;          /* The plugin that synthesizes the utterance. */
    Stretcher* audioStretcher;   /* Audio stretcher object.  Adjusts speed. */
    QString audioUrl;            /* Filename containing synthesized audio.  Null if
                                    plugin has not yet synthesized the utterance, or if
                                    plugin does not support synthesis. */
    Player* audioPlayer;         /* The audio player audibilizing the utterance.  Null
                                    if not currently audibilizing or if plugin doesn't
                                    support synthesis. */
};

/**
 * Structure containing information for a talker (plugin).
 */
struct TalkerInfo{
    PlugInProc* plugIn;                  /* Instance of the plugin, i.e., the Talker. */
    QString talkerID;                    /* ID of the talker. */
    QString talkerCode;                  /* The Talker's Talker Code in XML format. */
    TalkerCode parsedTalkerCode;         /* The Talker's Talker Code parsed into individual attributes. */
};

/**
* Iterator for queue of utterances.
*/
typedef QValueVector<Utt>::iterator uttIterator;

// Timer interval for checking whether audio playback is finished.
const int timerInterval = 500;

/**
 * This class is in charge of getting the messages, warnings and text from
 * the queue and call the plug ins function to actually speak the texts.
 * This class runs as another thread, using QThreads
 */
class Speaker : public QObject{
    Q_OBJECT

    public:
        /**
         * Constructor
         * Calls load plug ins
         */
        Speaker(SpeechData *speechData, QObject *parent = 0, const char *name = 0);

        /**
         * Destructor
         */
        ~Speaker();

        /**
         * Load all the configured plug ins populating loadedPlugIns
         */
        int loadPlugIns();

        /**
         * Tells the thread to exit
         */
        void requestExit();

        /**
        * Main processing loop.  Dequeues utterances and sends them to the
        * plugins and/or Audio Player.
        */
        void doUtterances();

        /**
        * Determine if kttsd is currently speaking any text jobs.
        * @return               True if currently speaking any text jobs.
        */
        bool isSpeakingText();

        /**
        * Get the job number of the current text job.
        * @return               Job number of the current text job. 0 if no jobs.
        *
        * Note that the current job may not be speaking. See @ref isSpeakingText.
        * @see getTextJobState.
        * @see isSpeakingText
        */
        uint getCurrentTextJob();

        /**
        * Remove a text job from the queue.
        * @param jobNum         Job number of the text job.
        *
        * The job is deleted from the queue and the @ref textRemoved signal is emitted.
        *
        * If there is another job in the text queue, and it is marked speakable,
        * that job begins speaking.
        */
        void removeText(const uint jobNum);

        /**
        * Start a text job at the beginning.
        * @param jobNum         Job number of the text job.
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
        void startText(const uint jobNum);

        /**
        * Stop a text job and rewind to the beginning.
        * @param jobNum         Job number of the text job.
        *
        * The job is marked not speakable and will not be speakable until @ref startText or @ref resumeText
        * is called.
        *
        * If there are speaking jobs preceeding this one in the queue, they continue speaking.
        * If the job is currently speaking, the @ref textStopped signal is emitted and the job stops speaking.
        * Depending upon the speech engine and plugin used, speeking may not stop immediately
        * (it might finish the current sentence).
        */
        void stopText(const uint jobNum);

        /**
        * Pause a text job.
        * @param jobNum         Job number of the text job.
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
        void pauseText(const uint jobNum);

        /**
        * Start or resume a text job where it was paused.
        * @param jobNum         Job number of the text job.
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
        void resumeText(const uint jobNum);

        /**
        * Move a text job down in the queue so that it is spoken later.
        * @param jobNum         Job number of the text job.
        *
        * If the job is currently speaking, it is paused.
        * If the next job in the queue is speakable, it begins speaking.
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
        * @return               Sequence number of the sentence actually moved to.
        *                       Sequence numbers are numbered starting at 1.
        *
        * If no such job, does nothing and returns 0.
        * If n is zero, returns the current sequence number of the job.
        * Does not affect the current speaking/not-speaking state of the job.
        */
        uint moveRelTextSentence(const int n, const uint jobNum);

        /**
        * Get a list of the talkers configured in KTTS.
        * @return               A QStringList of fully-specified talker codes, one
        *                       for each talker user has configured.
        */
        QStringList Speaker::getTalkers();

        /**
        * Given a Talker Code, returns the Talker ID of the talker that would speak
        * a text job with that Talker Code.
        * @param talkerCode     Talker Code.
        * @return               Talker ID of the talker that would speak the text job.
        */
        QString talkerCodeToTalkerId(const QString& talkerCode);

        /**
        * Get the user's default talker.
        * @return               A fully-specified talker code.
        *
        * @see talkers
        * @see getTalkers
        */
        QString userDefaultTalker();

    signals:
        /**
         * Emitted whenever reading a text was started or resumed
         */
        void readingStarted();

        /**
         * Emitted whenever reading a text was finished,
         * or paused, or stopped before it was finished
         */
        void readingStopped();

        /**
         * Emitted whenever a message or warning interrupts reading a text
         */
        void readingInterrupted();

        /**
         * Emitted whenever reading a text is resumed after it was interrupted
         * Note: In function resumeText, readingStarted is called instead
         */
        void readingResumed();

        /* The following signals correspond to the signals in the kspeech interface. */

        /**
        * This signal is emitted when the speech engine/plugin encounters a marker in the text.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param markerName     The name of the marker seen.
        * @see markers
        */
        void markerSeen(const QCString& appId, const QString& markerName);

        /**
        * This signal is emitted whenever a sentence begins speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        */
        void sentenceStarted(QString text, QString language, const QCString& appId,
            const uint jobNum, const uint seq);

        /**
        * This signal is emitted when a sentence has finished speaking.
        * @param appId          DCOP application ID of the application that queued the text.
        * @param jobNum         Job number of the text job.
        * @param seq            Sequence number of the text.
        */        
        void sentenceFinished(const QCString& appId, const uint jobNum, const uint seq);

        /**
        * This signal is emitted whenever speaking of a text job begins.
        * @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
        * @param jobNum         Job number of the text job.
        */
        void textStarted(const QCString& appId, const uint jobNum);

        /**
        * This signal is emitted whenever a text job is finished.  The job has
        * been marked for deletion from the queue and will be deleted when another
        * job reaches the Finished state. (Only one job in the text queue may be
        * in state Finished at one time.)  If @ref startText or @ref resumeText is
        * called before the job is deleted, it will remain in the queue for speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textFinished(const QCString& appId, const uint jobNum);

        /**
        * This signal is emitted whenever a speaking text job stops speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textStopped(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted whenever a speaking text job is paused.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textPaused(const QCString& appId, const uint jobNum);
        /**
        * This signal is emitted when a text job, that was previously paused, resumes speaking.
        * @param appId          The DCOP senderId of the application that created the job.
        * @param jobNum         Job number of the text job.
        */
        void textResumed(const QCString& appId, const uint jobNum);

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
        * Received from audio stretcher when stretching (speed adjustment) is finished.
        */
        void slotStretchFinished();
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
        void slotError(const bool keepGoing, const QString &msg);
        /**
        * Received from Timer when it fires.
        * Check audio player to see if it is finished.
        */
        void slotTimeout();

    private:
        /**
        * Given a talker code, returns pointer to the closest matching plugin.
        * @param talker          The talker (language) code.
        * @return                Index to m_loadedPlugins array of Talkers.
        *
        * If a plugin has not been loaded to match the talker, returns the default
        * plugin.
        */
        int talkerToPluginIndex(const QString& talker);

        /**
        * Given a talker code, returns pointer to the closest matching plugin.
        * @param talker          The talker (language) code.
        * @return                Pointer to closest matching plugin.
        *
        * If a plugin has not been loaded to match the talker, returns the default
        * plugin.
        */
        PlugInProc* talkerToPlugin(QString& talker);

        /**
         * Converts an utterance state enumerator to a displayable string.
        * @param state           Utterance state.
        * @return                Displayable string for utterance state.
        */
        QString uttStateToStr(uttState state);

        /**
        * Converts an utterance type enumerator to a displayable string.
        * @param utType          Utterance type.
        * @return                Displayable string for utterance type.
        */
        QString uttTypeToStr(uttType utType);

        /**
        * Converts a plugin state enumerator to a displayable string.
        * @param state           Plugin state.
        * @return                Displayable string for plugin state.
        */
        QString pluginStateToStr(pluginState state);

        /**
        * Converts a job state enumerator to a displayable string.
        * @param state           Job state.
        * @return                Displayable string for job state.
        */
        QString jobStateToStr(int state);

#if SUPPORT_SSML
        /**
        * Determines whether the given text is SSML markup.
        */
        bool isSsml(const QString &text);
#endif
        /**
        * Determines the initial state of an utterance.  If the utterance contains
        * SSML, the state is set to usWaitingTransform.  Otherwise, if the plugin
        * supports async synthesis, sets to usWaitingSynth, otherwise usWaitingSay.
        * If an utterance has already been transformed, usWaitingTransform is
        * skipped to either usWaitingSynth or usWaitingSay.
        * @param utt             The utterance.
        */
        void setInitialUtteranceState(Utt &utt);

        /**
        * Gets the next utterance to be spoken from speechdata and adds it to the queue.
        * @return                True if one or more utterances were added to the queue.
        *
        * Checks for waiting ScreenReaderOutput, Warnings, Messages, or Text,
        * in that order.
        * If Warning or Message and interruption messages have been configured,
        * adds those to the queue as well.
        * Determines which plugin should be used for the utterance.
        */
        bool getNextUtterance();

        /**
        * Given an iterator pointing to the m_uttQueue, deletes the utterance
        * from the queue.  If the utterance is currently being processed by a
        * plugin or the Audio Player, halts that operation and deletes Audio Player.
        * Also takes care of deleting temporary audio file.
        * @param it                      Iterator pointer to m_uttQueue.
        * @return                        Iterator pointing to the next utterance in the
        *                                queue, or m_uttQueue.end().
        */
        uttIterator deleteUtterance(uttIterator it);

        /**
        * Given an iterator pointing to the m_uttQueue, starts playing audio if
        *   1) An audio file is ready to be played, and
        *   2) It is not already playing.
        * If another audio player is already playing, pauses it before starting
        * the new audio player.
        * @param it                      Iterator pointer to m_uttQueue.
        * @return                        True if an utterance began playing or resumed.
        */
        bool startPlayingUtterance(uttIterator it);

        /**
        * Delete any utterances in the queue with this jobNum.
        * @param jobNum          The Job Number of the utterance(s) to delete.
        * If currently processing any deleted utterances, stop them.
        */
        void deleteUtteranceByJobNum(const uint jobNum);

        /**
        * Pause the utterance with this jobNum and if it is playing on the Audio Player,
        * pause the Audio Player.
        * @param jobNum          The Job Number of the utterance to pause.
        */
        void pauseUtteranceByJobNum(const uint jobNum);

        /**
        * Takes care of emitting reading interrupted/resumed and sentence started signals.
        * Should be called just before audibilizing an utterance.
        * @param it                      Iterator pointer to m_uttQueue.
        */
        void prePlaySignals(uttIterator it);

        /**
        * Takes care of emitting sentenceFinished signal.
        * Should be called immediately after an utterance has completed playback.
        * @param it                      Iterator pointer to m_uttQueue.
        */
        void postPlaySignals(uttIterator it);

        /**
        * Constructs a temporary filename for plugins to use as a suggested filename
        * for synthesis to write to.
        * @return                        Full pathname of suggested file.
        */
        QString makeSuggestedFilename();

        /**
        * Get the real path of a filename and convert it to local encoding.
        */
        QString getRealFilePath(const QString filename);

        /**
        * Creates and returns a player object based on user option.
        */
        Player* createPlayerObject();

        /**
         * Array of the loaded plug ins for different Talkers.
         */
        QValueVector<TalkerInfo> m_loadedPlugIns;

        /**
         * SpeechData local pointer
         */
        SpeechData* m_speechData;

        /**
         * True if the speaker was requested to exit.
         */
        volatile bool m_exitRequested;

        /**
        * Queue of utterances we are currently processing.
        */
        QValueVector<Utt> m_uttQueue;

        /**
        * True when text job reading has been interrupted.
        */
        bool m_textInterrupted;

        /**
        * Used to prevent doUtterances from prematurely exiting.
        */
        bool m_again;

        /**
        * Which audio player to use.
        *  0 = aRts
        *  1 = gstreamer
        */
        int m_playerOption;

        /**
        * Audio stretch factor (Speed).
        */
        float m_audioStretchFactor;

        /**
        * GStreamer sink name to use.
        */
        QString m_gstreamerSinkName;

        /**
        * Timer for monitoring audio player.
        */
        QTimer* m_timer;

        /**
        * Current Text job being processed.
        */
        uint m_currentJobNum;

        /**
        * Job Number, appId, and sequence number of the last text sentence queued.
        */
        uint m_lastJobNum;
        QCString m_lastAppId;
        uint m_lastSeq;

        /**
         * Cache of talker codes and index of closest matching Talker.
         */
        QMap<QString,int> m_talkerToPlugInCache;
};

#endif // _SPEAKER_H_
