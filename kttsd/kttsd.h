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

#include <kaboutapplication.h>
#include <dcopobject.h>
#include <ksystemtray.h>

#include "speechdata.h"
#include "speaker.h"
#include "kttsdui.h"

class KTTSDTray;

/**
 * KTTSD - the KDE Text-to-speech Deamon.
 *
 * Provides the capability for applications to speak text.
 * Applications interface to KTTSD via DCOP.
 *
 * @warning The DCOP interface to KTTSD is still being developed and is likely to change in the future.
 *
 * @section Features
 *
 *   - Priority system for warnings and messages, while still playing regular texts.
 *   - Long text is parsed into sentences.  User may backup by sentence or paragraph,
 *     replay, pause, and stop playing.
 *   - Speak contents of clipboard.
 *
 * @section Requirements
 *
 * KDE 3.2.x and a speech synthesis engine, such as Festival.  Festival can be
 * obtained from 
 * <a href="http://www.cstr.ed.ac.uk/projects/festival/">http://www.cstr.ed.ac.uk/projects/festival/</a>.
 * Festival is distributed with most Linux distros.  Check your distro CDs.  Also works
 * with Hadifax or any command that can speak text, such as Festival Lite
 * (flite).
 *
 * @section Using
 *
 * Make sure your speech engine is working.
 *
 * You may need to grant Festival write access to the audio device.
 *
   @verbatim
     chmod a+rw /dev/dsp*
   @endverbatim
 *
 * To configure the speech plugin for KTTSD
 *
   @verbatim
     kcmshell kcmkttsd
   @endverbatim
 *
 * If using the Festival or Festival Interactive plugins, you'll need to
 * specify the path to the voices.  On most systems, this will be
 *
   @verbatim
     /usr/share/festival/voices
   @endverbatim
 *
 * or
 *
   @verbatim
    /usr/local/share/festival/voices
   @endverbatim
 * 
 * Be sure to click the Default button after configuring a speech plugin!
 *
 * To run KTTSD
 *
   @verbatim
     kttsd
   @endverbatim
 *
 * Click on the system tray icon.  Click "Open text to read out" button, 
 * pick a plain text file, then click "Start reading" button.
 *
 * To set the text to be spoken
 *
   @verbatim
     dcop kttsd kspeech setText <text> <language>
   @endverbatim
 *
 * where <text> is the text to be spoken, and <language> is a language code
 * such as "en", "cy", etc.
 *
 * Example.
 *
   @verbatim
     dcop kttsd kspeech setText "This is a test." "en"
   @endverbatim
 *
 * To speak the current text
 *
   @verbatim
     dcop kttsd kspeech startText
   @endverbatim
 *
 * To stop speaking
 *
   @verbatim
     dcop kttsd kspeech stopText
   @endverbatim
 *
 * Depending upon the speech plugin used, speaking may not immediately stop.
 *
 * Festival (Client/Server), festivalcs, is not currently working.
 *
 * @section Using with Festival Lite (flite)
 *
 * Obtain Festival Lite here:
 *
 * <a href="http://www.speech.cs.cmu.edu/flite/index.html">http://www.speech.cs.cmu.edu/flite/index.html</a>.
 *
 * Build and install following the instructions in the README that comes with flite.
 *
 * Start KTTSD settings dialog
 *
   @verbatim
     kcmshell kcmkttsd
   @endverbatim
 *
 * Choose the Command plugin and enter the following as the command
 *
   @verbatim
     flite -t "%t"
   @endverbatim
 *
 * @bug 17 Mar 2004: The Festival plugin does not work for me.  Crashes as soon
 * as call to festival_initialize is made.  I suspect that the problem is
 * incompatible gcc compilers used to compile kttsd and the libfestival.a
 * static library, which on my system, is version 1.42.  So I added the
 * Festival (Interactive) plugin that interfaces with Festival interactively
 * via pipes ("festival --interactive").  If someone can confirm or deny
 * this, I'd be appreciative.. Gary Cramblitt <garycramblitt@comcast.net>.

 * @author José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 * @author Olaf Schmidt <ojschmidt@kde.org>
 * @author Gary Cramblitt <garycramblitt@comcast.net>
 */

class KTTSD : public kttsdUI, public DCOPObject {
    Q_OBJECT
    K_DCOP

    public:
        /**
         * @internal
         * Constructor.
         *
         * Create objects, speechData and speaker.
         * Start thread
         */
        KTTSD(QWidget *parent = 0, const char *name = 0);

        /** 
         * @internal
         * Destructor.
         *
         * Terminate speaker thread.
         */
        ~KTTSD();

        /**
         * @internal
         * Holds if we are ok to go or not.
         */
        bool ok;
    
    k_dcop:
        /**
         * DCOP exported function to say a warning.  The warning will be spoken when the current sentence
         * stops speaking. 
         * @param warning        The warning to be spoken.
         * @param language       Code for the language to be spoken in.  Example "en".
         */
        void sayWarning(const QString &warning, const QString &language);

        /**
         * DCOP exported function to say a message.  The message will be spoken when the current sentence
         * stops speaking.
         * @param message        The message to be spoken.
         * @param language       Code for the language to be spoken in.  Example "en".
         */
        void sayMessage(const QString &message, const QString &language);

        /**
         * DCOP exported function to set the text queue.  Does not start speaking the text.
         * The text is parsed into individual sentences.
         * @param text           The message to be spoken.
         * @param language       Code for the language to be spoken in.  Example "en".
         *                       If NULL, the text is spoken in the default languange.
         */
        void setText(const QString &text, const QString &language = NULL);
        
        /**
         * DCOP exported function to set text queue to contents of a file.  Does not start speaking the text.
         * @param filename       Full path to the file to be spoken.  May be a URL.
         * @param language       Code for the language to be spoken in.  Example "en".
         *                       If NULL, the text is spoken in the default languange.
         */
        void setFile(const QString &filename, const QString &language);

        /**
         * DCOP exported function to remove the text from the queue.
         */
        void removeText();

        /**
         * DCOP exported function to go to the previous paragraph in the queue.
         */
        void prevParText();

        /**
         * DCOP exported function to go to the previous sentence in the queue.
         */
        void prevSenText();

        /**
         * DCOP exported function to pause text.
         */
        void pauseText();

        /**
         * DCOP exported function to stop text and go to the beginning.
         */
        void stopText();

        /**
         * DCOP exported function to start text at the beginning.
         */
        void startText();

        /**
         * DCOP exported function to start or resume text where it was paused.
         */
        void resumeText();

        /**
         * DCOP exported function to go to next sentence in the text queue.
         */
        void nextSenText();

        /**
         * DCOP exported function to go to next paragraph in the text queue.
         */
        void nextParText();
        
        /**
         * DCOP exported function to add the clipboard contents to the text queue and begin speaking it.
         */
        void speakClipboard();

        /**
         * DCOP exported function to stop the service.
         */
        //void exit();

        /**
         * DCOP exported function to re-start KTTSD.
         */
        void reinit();
        
    k_dcop_signals:
        /**
         * This DCOP signal is emitted whenever a sentence begins speaking.
         */
        void sentenceStarted();
        /**
         * This DCOP signal is emitted when a sentence has finished speaking.
         */        
        void sentenceFinished();
    
        /**
         * This DCOP signal is emitted whenever speaking of normal text begins.
         */
        void textStarted();
        /**
         * This DCOP signal is emitted whenever speaking is finished.
         */
        void textFinished();
        /**
         * This DCOP signal is emitted whenever speaking is stopped.
         */
        void textStopped();
        /**
         * This DCOP signal is emitted whenever speaking is paused.
         */
        void textPaused();
        /**
         * This DCOP signal is emitted when speaking resumes that was previously paused.
         */
        void textResumed();
        /**
         * This DCOP signal is emitted whenver text in the queue is changed.
         */
        void textSet();
        /**
         * This DCOP signal is emitted whenever text in the queue is removed.
         */
        void textRemoved();
        
    private slots:
        /**
         * These functions are called whenever either
         * a button in the main window or a system tray
         * contect menu entry is selected
         */
        void openSelected();
        void startSelected();
        void stopSelected();
        void pauseSelected(bool paused);

        void nextSentenceSelected();
        void prevSentenceSelected();
        void nextParagraphSelected();
        void prevParagraphSelected();

        void helpSelected();
        void speakClipboardSelected();
        void closeSelected();
        void quitSelected();

        void configureSelected();
        void aboutSelected();

        /**
         * These functions are called whenever
         * the status of the speaker object has changed
         */
        void slotSentenceStarted(QString text, QString language);
        void slotSentenceFinished();

        /**
         * These functions are called whenever
         * the status of the speechData object has changed
         */
        void slotTextStarted();
        void slotTextFinished();
        void slotTextStopped();
        void slotTextPaused();
        void slotTextResumed();
        void slotTextSet();
        void slotTextRemoved();

        /**
         * Fires whenever user clicks Apply or OK buttons in Settings dialog.
         */
        void configCommitted();
    
    private:
        /**
         * Initialize the speaker.
         */
        bool initializeSpeaker();

        /**
         * Send a DCOP signal with no parameters.
         */
        void emitDcopSignalNoParams(const QCString& signalName);
        
        /**
         * SpeechData containing all the data and the manipulating methods for all KTTSD
         */
        SpeechData *speechData;

        /**
         * Speaker that will be run as another thread, actually saying the messages, warning, texts
         */
        Speaker *speaker;

        KTTSDTray *tray;
        KAboutApplication *aboutDlg;
        
};

class KTTSDTray : public KSystemTray {
    Q_OBJECT

    public:
        KTTSDTray (QWidget *parent=0, const char *name=0);
        ~KTTSDTray();

    signals:
        /**
         * @internal
         * These functions are called whenever an entry in system tray contect menu is selected
         */
        void configureSelected();
        void speakClipboardSelected();
        void aboutSelected();
        void helpSelected();
};
#endif // _KTTSD_H_
