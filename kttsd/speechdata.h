/*************************************************** vim:set ts=4 sw=4 sts=4:
  speechdata.h
  This contains the SpeechData class which is in charge of maintaining
  all the data on the memory.
  It maintains queues, mutex, a wait condition and has methods to enque
  messages and warnings and manage the text that is thread safe.
  We could say that this is the common repository between the KTTSD class
  (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
  functions)
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
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

#ifndef _SPEECHDATA_H_
#define _SPEECHDATA_H_

#include <qptrqueue.h>
#include <qptrlist.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <kconfig.h>

/**
 * Struct containing a text cell, for messages, warnings, and texts.
 * Contains the text itself, the associated language, and the ID of the application that requested it be spoken.
 */
struct mlText{
   QString language;    /* Language code for the sentence. */
   QString text;        /* Text of sentence. */
   QCString appId;      /* DCOP senderId of application that requested the speech. */
   uint seq;    /* Sentence sequence number. */
};

/**
 * SpeechData class which is in charge of maintaining all the data on the memory.
 * It maintains queues, mutex, a wait condition and has methods to enque
 * messages and warnings and manage the text that is thread safe.
 * We could say that this is the common repository between the KTTSD class
 * (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
 * functions)
 */
class SpeechData : public QObject {
   Q_OBJECT

   public:
      /**
       * Constructor
       * Sets text to be stoped and warnings and messages queues to be autodelete (thread safe)
       */
      SpeechData();

      /**
       * Destructor
       */
      ~SpeechData();

      /**
       * Read the configuration
       */
      bool readConfig();

      /**
       * Add a new warning to the queue (thread safe)
       */
      void enqueueWarning( const QString &, const QString &language=NULL, const QCString &appId=NULL );

      /**
       * Pop (get and erase) a warning from the queue (thread safe)
       */
      mlText dequeueWarning();

      /**
       * Are there any Warning (thread safe)
       */
      bool warningInQueue();

      /**
       * Add a new message to the queue (thread safe)
       */
      void enqueueMessage( const QString &, const QString &language=NULL, const QCString &appId=NULL );

      /**
       * Pop (get and erase) a message from the queue (thread safe)
       */
      mlText dequeueMessage();

      /**
       * Are there any Message (thread safe)
       */
      bool messageInQueue();

      /**
       * Sets a text to say it and navigate it (thread safe) (see also resumeText, stopText, etc)
       * @param text           The message to be spoken.
       * @param language       Code for the language to be spoken in.  Example "en".
       *                       If NULL, the text is spoken in the default languange.
       * @param appId          DCOP senderId of the application.
       * @return               Sequence number of the last sentence added to the text queue.
       *                       The first sentence ever added by an application is sequence number 1.
       */
      uint setText( const QString &, const QString &language=NULL, const QCString &appId=NULL);
      
      /**
       * Returns the next sequence number that will be assigned to the application when it calls
       * setText.
       * @param appId          DCOP senderId of the application.
       * @return               Sequence number that will be assigned to the application on next call
       *                       to setText.
       */
      uint getNextSequenceNum(const QCString& appId);

      /**
       * Remove the text (thread safe)
       */
      void removeText();

      /**
       * Get a sentence to speak it.
       */
      mlText getSentenceText();

      /**
       * Returns true if the text has not to be speaked (thread safe)
       */
      bool currentlyReading();

      /**
       * Jump to the previous paragrah (thread safe)
       */
      void prevParText();

      /**
       * Jump to the previous sentence (thread safe)
       */
      void prevSenText();

      /**
       * Stop playing text (thread safe)
       */
      void pauseText();

      /**
       * Stop playing text and go to the begining (thread safe)
       */
      void stopText();

      /**
       * Start text at the beginning (thread safe)
       * Note: If no text is set, the call is ignored.
       */
      void startText();

      /**
       * Resume text if the text was paused
       * Start text if the text was not started yet or if it was finished or stopped
       * Otherwise ignored.
       */
      void resumeText();

      /**
       * Next sentence (thread safe)
       */
      void nextSenText();

      /**
       * Next paragrah (thread safe)
       */
      void nextParText();

      /**
       * Wait condition for new text, messages or warnings.
       * When there's no text, messages or warnings this wait condition
       * will prevent KTTSD from doing useless and CPU consuming loops.
       */
      QWaitCondition newTMW;

      /**
       * Text pre message
       */
      QString textPreMsg;

      /**
       * Text pre message enabled ?
       */
      bool textPreMsgEnabled;

      /**
       * Text pre sound
       */
      QString textPreSnd;

      /**
       * Text pre sound enabled ?
       */
      bool textPreSndEnabled;

      /**
       * Text post message
       */
      QString textPostMsg;

      /**
       * Text post message enabled ?
       */
      bool textPostMsgEnabled;

      /**
       * Text post sound
       */
      QString textPostSnd;

      /**
       * Text post sound enabled ?
       */
      bool textPostSndEnabled;

      /**
       * Paragraph pre message
       */
      QString parPreMsg;

      /**
       * Paragraph pre message enabled ?
       */
      bool parPreMsgEnabled;

      /**
       * Paragraph pre sound
       */
      QString parPreSnd;

      /**
       * Paragraph pre sound enabled ?
       */
      bool parPreSndEnabled;

      /**
       * Paragraph post message
       */
      QString parPostMsg;

      /**
       * Paragraph post message enabled ?
       */
      bool parPostMsgEnabled;

      /**
       * Paragraph post sound
       */
      QString parPostSnd;

      /**
       * Paragraph post sound enabled ?
       */
      bool parPostSndEnabled;

      /**
       * Default language
       */
      QString defaultLanguage;

      /**
       * Configuration
       */
      KConfig *config;

   signals:
      /**
       * Emitted after reading a text was started by function startText or resumeText
       */
      void textStarted();

      /**
       * Emitted after reading a text was finished
       */
      void textFinished();

      /**
       * Emitted whenever reading a text was stopped before it was finished
       */
      void textStopped();

      /**
       * Emitted whenever reading a text was paused by function pauseText
       */
      void textPaused();

      /**
       * Emitted whenever reading a text was resumed after having been paused
       */
      void textResumed();

      /**
       * Emitted after a text was set by function setText
       */
      void textSet();

      /**
       * Emitted after a text was removed by function removeText
       */
      void textRemoved();

   private:
      /**
       * List of warnings
       */
      QPtrQueue<mlText> warnings;

      /**
       * Mutex for reading/writing warnings
       */
      QMutex warningsMutex;

      /**
       * List of messages
       */
      QPtrQueue<mlText> messages;

      /**
       * Mutex for reading/writing warnings
       */
      QMutex messagesMutex;

      /**
       * List of sentences of the text
       */
      QPtrList<mlText> textSents;

      /**
       * Mutex for reading/writing text
       */
      QMutex textMutex;

      /**
       * Iterator of the sentences of the text
       */
      QPtrListIterator<mlText>* textIterator;
      
      /**
       * Dictionary of text sequence numbers indexed by appId.
       * Contains the last sequence number assigned to each application.
       */
      QMap<QCString, uint> appSeq;
      
      /**
       * Mutex for reading/writing of appSeq dictionary.
       */
      QMutex appSeqMutex;
      
      /**
       * Language of the text
       */
      QString textLanguage;

      /**
       * True if text is being spoken.
       */
      bool reading;
};

#endif // _SPEECHDATA_H_
