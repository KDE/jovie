/***************************************************** vim:set ts=4 sw=4 sts=4:
  Main speaking functions for the Festival (Interactive) Plug in
  -------------------
  Copyright : (C) 2004 by Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _FESTIVALINTPROC_H_
#define _FESTIVALINTPROC_H_

#include <qstringlist.h>
#include <qmutex.h>

#include <kprocess.h>

#include <pluginproc.h>

class QTextCodec;

class FestivalIntProc : public PlugInProc{
    Q_OBJECT 

    public:
        /**
         * Constructor
         */
        FestivalIntProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /**
         * Destructor
         */
        virtual ~FestivalIntProc();

        /**
         * Initializate the speech engine.
         * @param config          Settings object.
         * @param configGroup     Settings group.
         */
        virtual bool init(KConfig *config, const QString &configGroup);

        /**
         * Returns true when festival is ready to speak a sentence.
         */
        bool isReady();

        /**
         * Say a text string.
         * @param text            The text to speak.
         */
        virtual void sayText(const QString &text);

        /**
        * Synthesize text into an audio file, but do not send to the audio device.
        * @param text                    The text to be synthesized.
        * @param suggestedFilename       Full pathname of file to create.  The plugin
        *                                may ignore this parameter and choose its own
        *                                filename.  KTTSD will query the generated
        *                                filename using getFilename().
        *
        * If the plugin supports asynchronous operation, it should return immediately
        * and emit @ref synthFinished signal when synthesis is completed.
        * It must also implement the @ref getState method, which must return
        * psFinished, when synthesis is completed.
        */
        virtual void synthText(const QString &text, const QString &suggestedFilename);

        /**
        * Get the generated audio filename from call to @ref synthText.
        * @return                        Name of the audio file the plugin generated.
        *                                Null if no such file.
        *
        * The plugin must not re-use or delete the filename.  The file may not
        * be locked when this method is called.  The file will be deleted when
        * KTTSD is finished using it.
        */
        virtual QString getFilename();

        /**
        * Stop current operation (saying or synthesizing text).
        * Important: This function may be called from a thread different from the
        * one that called sayText or synthText.
        * If the plugin cannot stop an in-progress @ref sayText or
        * @ref synthText operation, it must not block waiting for it to complete.
        * Instead, return immediately.
        *
        * If a plugin returns before the operation has actually been stopped,
        * the plugin must emit the @ref stopped signal when the operation has
        * actually stopped.
        *
        * The plugin should change to the psIdle state after stopping the
        * operation.
        */
        virtual void stopText();

        /**
        * Return the current state of the plugin.
        * This function only makes sense in asynchronous mode.
        * @return                        The pluginState of the plugin.
        *
        * @see pluginState
        */
        virtual pluginState getState();

        /**
        * Acknowledges a finished state and resets the plugin state to psIdle.
        *
        * If the plugin is not in state psFinished, nothing happens.
        * The plugin may use this call to do any post-processing cleanup,
        * for example, blanking the stored filename (but do not delete the file).
        * Calling program should call getFilename prior to ackFinished.
        */
        virtual void ackFinished();

        /**
        * Returns True if the plugin supports asynchronous processing,
        * i.e., returns immediately from sayText or synthText.
        * @return                        True if this plugin supports asynchronous processing.
        *
        * If the plugin returns True, it must also implement @ref getState .
        * It must also emit @ref sayFinished or @ref synthFinished signals when
        * saying or synthesis is completed.
        */
        virtual bool supportsAsync();

        /**
        * Returns True if the plugin supports synthText method,
        * i.e., is able to synthesize text to a sound file without
        * audibilizing the text.
        * @return                        True if this plugin supports synthText method.
        *
        * If the plugin returns True, it must also implement the following methods:
        * - @ref synthText
        * - @ref getFilename
        * - @ref ackFinished
        *
        * If the plugin returns True, it need not implement @ref sayText .
        */
        virtual bool supportsSynth();

        /**
        * Say or Synthesize text with the given voice code.
        * @param festivalExePath         Path to the Festival executable, or just "festival".
        * @param text                    The text to be synthesized.
        * @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
        *                                synthesize and audibilize the text.
        * @param voiceCode               Voice code.
        * @param time                    Speed percentage. 50 to 200. 200% = 2x normal.
        * @param pitch                   Pitch persentage.  50 to 200.
        * @param volume                  Volume percentage.  50 to 200.
        * @param languageCode            Language code, for example, "en".
        */
        void synth(const QString &festivalExePath, const QString &text,
            const QString &synthFilename, const QString& voiceCode,
            const int time, const int pitch, const int volume, const QString &languageCode,
            QTextCodec* codec);

        /**
        * Sends commands to Festival to query for a list of supported voice codes.
        * Fires queryVoicesFinished when completed.
        * @return                       False if busy doing something else and therefore cannot
        *                               do the query.
        */
        bool queryVoices(const QString &festivalExePath);

        /**
        * Returns the name of an XSLT stylesheet that will convert a valid SSML file
        * into a format that can be processed by the synth.  For example,
        * The Festival plugin returns a stylesheet that will convert SSML into
        * SABLE.  Any tags the synth cannot handle should be stripped (leaving
        * their text contents though).  The default stylesheet strips all
        * tags and converts the file to plain text.
        * @return            Name of the XSLT file.
        */
        QString getSsmlXsltFilename();

    signals:
        /**
        * This signal fires upon completion of a queryVoices operation.
        * The list of voice codes do not have "voice_" prefix.
        */
        void queryVoicesFinished(const QStringList &voiceCodes);

    private slots:
        void slotProcessExited(KProcess* proc);
        void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
        void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);
        void slotWroteStdin(KProcess* proc);

    private:
        /**
        * Start Festival engine.
        * @param festivalExePath         Path to the Festival executable, or just "festival".
        * @param voiceCode               Voice code in which to speak text.
        * @param languageCode            Language code, for example, "en".
        */
        void startEngine(const QString &festivalExePath, const QString &voiceCode,
            const QString &languageCode, QTextCodec* codec);

        /**
        * If ready for more output, sends the given text to Festival process, otherwise,
        * puts it in the queue.
        * @param text                    Text to send or queue.
        */
        void sendToFestival(const QString& text);

        /**
        * If Festival is ready for more input and there is more output to send, send it.
        * To be ready for more input, the Stdin buffer must be empty and the "festival>"
        * prompt must have been received (m_ready = true).
        * @return                        False when Festival is ready for more input
        *                                but there is nothing to be sent, or if Festival
        *                                has exited.
        */
        bool sendIfReady();

        /**
        * Determine if the text has SABLE tags.  If so, we will have to use a different
        * synthesis method.
        */
        bool isSable(const QString &text);

        /**
        * Path to the Festival executable.
        */
        QString m_festivalExePath;

        /**
        * Selected voice (from config).
        */
        QString m_voiceCode;

        /**
        * True if the voice is preloaded.  Also used as a flag to supress killing
        * Festival, since startup time will be excessive.
        */
        bool m_preload;

        /**
        * Selected speed (from config).
        */
        int m_time;

        /**
        * Selected pitch (frequency) (from config).
        */
        int m_pitch;

        /**
         * Selected volume (from config).
         */
        int m_volume;

        /**
        * Running voice.
        */
        QString m_runningVoiceCode;

        /**
        * Running time (speed).
        */
        int m_runningTime;

        /**
        * Running pitch (frequency).
        */
        int m_runningPitch;

        /**
         * Festival process
         */
        KProcess* m_festProc;

        /**
        * Synthesis filename.
        */
        QString m_synthFilename;

        /**
         * True when festival is ready for another input.
         */
        volatile bool m_ready;

        /**
        * Plugin state.
        */
        pluginState m_state;

        /**
        * True when stopText has been called.  Used to force transition to psIdle when
        * Festival exits.
        */
        bool m_waitingStop;

        /**
        * True when queryVoices has been called.
        */
        bool m_waitingQueryVoices;

        /**
        * A queue of outputs to be sent to the Festival process.
        * Since Festival requires us to wait until the "festival>" prompt before
        * sending the next command, this queue allows us to queue up multiple
        * commands and send each one when the ReceivedStdOut signal fires.
        */
        QStringList m_outputQueue;

        bool m_writingStdin;

        /**
        * Language code.
        */
        QString m_languageCode;

        /**
        * Codec.
        */
        QTextCodec* m_codec;
};

#endif // _FESTIVALINTPROC_H_
