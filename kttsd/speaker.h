/***************************************************** vim:set ts=4 sw=4 sts=4:
  speaker.h
  Speaker class.
  This class is in charge of getting the messages, warnings and text from
  the queue and call the plug ins function to actually speak the texts.
  This class runs as another thread, using QThreads
  ------------------- 
  Copyright : (C) 2002 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org> 
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
 ******************************************************************************/
 
// $Id$

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include <qthread.h>
#include <qobject.h>
#include <qdict.h>

#include <speechdata.h>
#include <pluginproc.h>

/**
 * This class is in charge of getting the messages, warnings and text from
 * the queue and call the plug ins function to actually speak the texts.
 * This class runs as another thread, using QThreads
 */
class Speaker : public QObject, public QThread{
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

    protected:
        /**
         * Base function, where the thread will start
         */
        virtual void run();

    private:
        /**
         * Checks for warnings and if there's any, it says it.
         */
        void checkSayWarning();

        /**
         * Checks for messages (and warnings) and if there's any, it says it.
         */
        void checkSayMessage();

        /**
         * Checks for playable texts (messages and warnings) and if there's any, it says it.
         */
        void checkSayText();

        /**
         * QDict of the loaded plug ins for diferent languages
         */
        QDict<PlugInProc> loadedPlugIns;

        /**
         * SpeechData local pointer
         */
        SpeechData *speechData;
};

#endif // _SPEAKER_H_
