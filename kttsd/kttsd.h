/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.h
  KTTSD main class
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
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

class KTTSD : public kttsdUI, public DCOPObject {
    Q_OBJECT
    K_DCOP

    public:
        /**
         * Constructor
         * Create objects, speechData and speaker
         * Start thread
         */
        KTTSD(QWidget *parent = 0, const char *name = 0);

        /**
         * Destructor
         * Terminate speaker thread
         */
        ~KTTSD();

        /**
         * Holds if we are ok to go or not
         */
        bool ok;
    public slots:
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
        void closeSelected();
        void quitSelected();

        void configureSelected();
        void aboutSelected();

        /**
         * These functions are called whenever
         * the status of the speaker object has changed
         */
        void sentenceStarted(QString text, QString language);
        void sentenceFinished();

        /**
         * These functions are called whenever
         * the status of the speechData object has changed
         */
        void textStarted();
        void textFinished();
        void textStopped();
        void textPaused();
        void textResumed();
        void textSet();
        void textRemoved();

    private:
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

    k_dcop:
        /**
         * DCOP exported function to say warnings
         */
        void sayWarning(const QString &warning, const QString &language);

        /**
         * DCOP exported function to say messages
         */
        void sayMessage(const QString &message, const QString &language);

        /**
         * DCOP exported function to sat text
         */
        void setText(const QString &text, const QString &language);

        /**
         * Remove the text
         */
        void removeText();

        /**
         * Previous paragrah
         */
        void prevParText();

        /**
         * Previous sentence
         */
        void prevSenText();

        /**
         * Pause text
         */
        void pauseText();

        /**
         * Stop text and go to the begining
         */
        void stopText();

        /**
         * Start text at the beginning
         */
        void startText();

        /**
         * Start or resume text where it was paused
         */
        void resumeText();

        /**
         * Next sentence
         */
        void nextSenText();

        /**
         * Next paragrah
         */
        void nextParText();

        /**
         * Function exported in dcop to let other ones stop the service
         */
        //void exit();

        /**
         * This function is to re-start KTTSD
         */
        //void reinit();

};

class KTTSDTray : public KSystemTray {
    Q_OBJECT

    public:
        KTTSDTray (QWidget *parent=0, const char *name=0);
        ~KTTSDTray();

    signals:
        /**
         * These functions are called whenever an entry in system tray contect menu is selected
         */
        void configureSelected();
        void aboutSelected();
        void helpSelected();
};
#endif // _KTTSD_H_
