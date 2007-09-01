/***************************************************** vim:set ts=4 sw=4 sts=4:
  Main speaking functions for the Command Plug in
  -------------------
  Copyright : (C) 2002 by Gunnar Schmi Dt and 2004 by Gary Cramblitt
  -------------------
  Original author: Gunnar Schmi Dt <kmouth@schmi-dt.de>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMMANDPROC_H_
#define _COMMANDPROC_H_

// Qt includes.
#include <QStringList> 

// KTTS includes.
#include <pluginproc.h>

class K3Process;
class QTextCodec;

class CommandProc : public PlugInProc{
    Q_OBJECT 

    public:
        /** Constructor */
        explicit CommandProc( QObject* parent = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~CommandProc();

        /** Initializate the speech */
        bool init (KConfig *config, const QString &configGroup);

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
        * If the plugin supports asynchronous operation, it should return immediately.
        */
        virtual void synthText(const QString& text, const QString& suggestedFilename);

        /**
        * Get the generated audio filename from synthText.
        * @return                        Name of the audio file the plugin generated.
        *                                Null if no such file.
        *
        * The plugin must not re-use the filename.
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
        */
        virtual bool supportsSynth();

        /**
        * Say or Synthesize text.
        * @param inputText               The text that shall be spoken
        * @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
        *                                synthesize and audibilize the text.
        * @param userCmd                 The program that shall be executed for speaking
        * @param stdIn                   True if the program shall receive its data via
        *                                standard input.
        * @param codec                   Codec for encoding the text.
        * @param language                The language code (used for the %l macro)
        */
        void synth(const QString& inputText, const QString& suggestedFilename,
            const QString& userCmd, bool stdIn,
            QTextCodec *codec, QString& language);

    private slots:
        void slotProcessExited(K3Process* proc);
        void slotReceivedStdout(K3Process* proc, char* buffer, int buflen);
        void slotReceivedStderr(K3Process* proc, char* buffer, int buflen);
        void slotWroteStdin(K3Process* proc);

    private:

        /**
        * True if the plugin supports separate synthesis (option set by user).
        */
        bool m_supportsSynth;

        /**
        * TTS command
        */
        QString m_ttsCommand;

        /**
        * True if process should use Stdin.
        */
        bool m_stdin;

        /**
        * Language Group.
        */
        QString m_language;

        /**
        * Codec.
        */
        QTextCodec* m_codec;

        /**
         * Flite process
         */
        K3Process* m_commandProc;

        /**
        * Name of temporary file containing text.
        */
        QString m_textFilename;

        /**
        * Synthesis filename.
        */
        QString m_synthFilename;

        /**
        * Plugin state.
        */
        pluginState m_state;

        /**
        * True when stopText has been called.  Used to force transition to psIdle when
        * Flite exits.
        */
        bool m_waitingStop;
};

#endif  // _COMMANDPROC_H_
