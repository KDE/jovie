/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.h
  KTTSD main class
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$

#ifndef _KTTSD_H_
#define _KTTSD_H_

#include <dcopobject.h>

#include "speechdata.h"
#include "speaker.h"

class KTTSD : public QObject, public DCOPObject{
    Q_OBJECT
    K_DCOP

    public:
        /**
         * Constructor
         * Create objects, speechData and speaker
         * Start thread
         */
        KTTSD(QObject *parent = 0, const char *name = 0);

        /**
         * Destructor
         * Terminate speaker thread
         */
        ~KTTSD();
      
        /**
         * Holds if we are ok to go or not
         */
        bool ok;

    private:
        /**
         * SpeechData containing all the data and the manipulating methods for all KTTSD
         */
        SpeechData *speechData;

        /**
         * Speaker that will be run as another thread, actually saying the messages, warning, texts
         */
        Speaker *speaker;

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
         * Stop text
         */
        void pauseText();

        /**
         * Stop text and go to the begining
         */
        void stopText();

        /**
         * Start text
         */
        void playText();

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

#endif // _KTTSD_H_
