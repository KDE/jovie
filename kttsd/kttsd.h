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
#include <ksystemtray.h>

#include "speechdata.h"
#include "speaker.h"
#include "kttsdui.h"
#include "kspeech.h"

class KTTSDTray;

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

class KTTSD : public kttsdUI, virtual public kspeech {
    Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * Create objects, speechData and speaker.
         * Start thread
         */
        KTTSD(QWidget *parent = 0, const char *name = 0);

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
        
        ASYNC sayWarning(const QString &warning, const QString &language);
        ASYNC sayMessage(const QString &message, const QString &language);
        uint setText(const QString &text, const QString &language = NULL);
        uint setFile(const QString &filename, const QString &language);
        uint getNextSequenceNum();
        ASYNC removeText();
        ASYNC prevParText();
        ASYNC prevSenText();
        ASYNC pauseText();
        ASYNC stopText();
        ASYNC startText();
        ASYNC resumeText();
        ASYNC nextSenText();
        ASYNC nextParText();
        ASYNC speakClipboard();
        //void exit();
        void reinit();
        
    private slots:
        /*
         * These functions are called whenever either
         * a button in the main window or a system tray
         * context menu entry is selected
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

        /*
         * These functions are called whenever
         * the status of the speaker object has changed
         */
        void slotSentenceStarted(QString text, QString language, const QCString& appId, const uint seq);
        void slotSentenceFinished(const QCString& appId, const uint seq);

        /*
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
         * Send a DCOP signal with no parameters.
         */
        void emitDcopSignalNoParams(const QCString& signalName);
        
        /*
         * SpeechData containing all the data and the manipulating methods for all KTTSD
         */
        SpeechData *speechData;

        /*
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
        /*
         * These functions are called whenever an entry in system tray contect menu is selected
         */
        void configureSelected();
        void speakClipboardSelected();
        void aboutSelected();
        void helpSelected();
};
#endif // _KTTSD_H_
