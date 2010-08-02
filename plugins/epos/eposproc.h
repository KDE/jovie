/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposproc.h
  Main speaking functions for the Epos Plug in
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef _EPOSPROC_H_
#define _EPOSPROC_H_

// Qt includes.
#include <tqstringlist.h>
#include <tqmutex.h>

// KTTS includes.
#include <pluginproc.h>

class KProcess;
class TQTextCodec;

class EposProc : public PlugInProc{
    Q_OBJECT 

    public:
        /**
         * Constructor
         */
        EposProc( TQObject* parent = 0, const char* name = 0, const TQStringList &args = TQStringList());

        /**
         * Destructor
         */
        virtual ~EposProc();

        /**
         * Initializate the speech engine.
         * @param config          Settings object.
         * @param configGroup     Settings group.
         */
        virtual bool init(KConfig *config, const TQString &configGroup);

        /**
         * Say a text string.
         * @param text            The text to speak.
         */
        virtual void sayText(const TQString &text);

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
        virtual void synthText(const TQString& text, const TQString& suggestedFilename);

        /**
        * Get the generated audio filename from synthText.
        * @return                        Name of the audio file the plugin generated.
        *                                Null if no such file.
        *
        * The plugin must not re-use the filename.
        */
        virtual TQString getFilename();

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
        * @param text                    The text to be synthesized.
        * @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
        *                                synthesize and audibilize the text.
        * @param eposServerExePath       Path to the Epos server executable.
        * @param eposClientExePath       Path to the Epos client executable.
        * @param eposServerOptions       Options passed to Epos server executable.
        * @param eposClientOptions       Options passed to Epos client executable (don't include -o).
        * @param codec                   Codec for encoding of text.
        * @param eposLanguage            Epos language setting.  "czech", "slovak",
        *                                or null (default language).
        * @param time                    Speed percentage. 50 to 200. 200% = 2x normal.
        * @param pitch                   Pitch persentage.  50 to 200.
        */
        void synth(
            const TQString &text,
            const TQString &suggestedFilename,
            const TQString& eposServerExePath,
            const TQString& eposClientExePath,
            const TQString& eposServerOptions,
            const TQString& eposClientOptions,
            TQTextCodec *codec,
            const TQString& eposLanguage,
            int time,
            int pitch);

    private slots:
        void slotProcessExited(KProcess* proc);
        void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
        void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);
        void slotWroteStdin(KProcess* proc);

    private:

        /**
        * Path to epos executables (from config).
        */
        TQString m_eposServerExePath;
        TQString m_eposClientExePath;

        /**
        * User options passed to executables (from config).
        */
        TQString m_eposServerOptions;
        TQString m_eposClientOptions;

        /**
        * Epos Server process.
        */
        KProcess* m_eposServerProc;

        /**
         * Epos Client process
         */
        KProcess* m_eposProc;

        /**
        * Epos language setting.  "czech", "slovak", or Null (use default language).
        */
        TQString m_eposLanguage;

        /**
        * Rate (speed) from config file.
        */
        int m_time;

        /**
        * Pitch from the config file.
        */
        int m_pitch;

        /**
        * Codec.
        */
        TQTextCodec* m_codec;

        /**
        * Encoded buffer to be sent to Epos client.
         */
        TQCString m_encText;

        /**
        * Synthesis filename.
        */
        TQString m_synthFilename;

        /**
        * Plugin state.
        */
        pluginState m_state;

        /**
        * True when stopText has been called.  Used to force transition to psIdle when
        * Epos exits.
        */
        bool m_waitingStop;

};

#endif // _EPOSPROC_H_
