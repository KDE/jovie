/***************************************************** vim:set ts=4 sw=4 sts=4:
  speaker.h
  Speaker class.
  This class is in charge of getting the messages, warnings and text from
  the queue and call the plug ins function to actually speak the texts.
  This class runs as another thread, using QThreads
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

        /**
         * Tells the thread to exit
         */
        void requestExit();
        
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

        /**
         * Emitted whenever reading a sentence was started or finished
         */
        void sentenceStarted(QString text, QString language, const QCString& appId, const uint jobNum, const uint seq);
        void sentenceFinished(const QCString& appId, const uint jobNum, const uint seq);

    protected:
        /**
         * Base function, where the thread will start
         */
        virtual void run();

    private:
        /**
        * Checks for Screen Reader Output, if there's any, and says it.
        */
        void checkSayScreenReaderOutput();
        
        /**
         * Checks for warnings (and Screen Reader Output) and if there's any, it says it.
         */
        void checkSayWarning();

        /**
         * Checks for messages (and Screen Reader and warnings) and if there's any, it says it.
         */
        void checkSayMessage();

        /**
         * Checks for playable texts (Screen Reader, messages, warnings, and text jobs)
         * and if there's any, it says it.
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
        
        /**
         * True if the thread was requested to exit
         */
        bool exitRequested;
};

/**
* SpeakerTerminator 
*
* A separate thread to request that the speaker thread exit, and when it does, emits a signal.
* We need to do this in a separate thread because the main thread cannot call speaker->wait(),
* otherwise it would block the QT event loop and hang the program.
*/
class SpeakerTerminator: public QObject, public QThread 
{
    Q_OBJECT

    public:
        SpeakerTerminator(Speaker *speaker, QObject *parent = 0, const char *name = 0);
    
    signals:
        void speakerFinished();
    
    protected:   
        virtual void run();
    
    private:
        Speaker* speaker;
};

#endif // _SPEAKER_H_
