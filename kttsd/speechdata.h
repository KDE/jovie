/*************************************************** vim:set ts=4 sw=4 sts=4:
  speechdata.h
  This contains the SpeechData class which is in charge of maintaining
  all the data on the memory.
  It maintains queues, mutex, a wait condition and has methods to enque 
  messages and warnings and manage the text that is thread safe.
  We could say that this is the common repository between the Proklam class
  (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
  functions)
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
 
#ifndef _SPEECHDATA_H_
#define _SPEECHDATA_H_

#include <qptrqueue.h> 
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kconfig.h>

/**
 * Struct containing a text cell, for messages and warnings
 * Contains the text itself and the language asosiated
 */
struct mlText{
   QString language;
   QString text;
};

/**
 * SpeechData class which is in charge of maintaining all the data on the memory.
 * It maintains queues, mutex, a wait condition and has methods to enque 
 * messages and warnings and manage the text that is thread safe.
 * We could say that this is the common repository between the Proklam class
 * (dcop service) and the Speaker class (speaker, loads plug ins, call plug in
 * functions)
 */
class SpeechData{
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
      void enqueueWarning( const QString &, const QString &language=NULL );

      /**
       * Pop (get and erase) a warning from the queue (thread safe)
       */
      mlText dequeueWarning();

      /**
       * Is there any Warning (thread safe)
       */
      bool isEmptyWarning();

      /**
       * Add a new message to the queue (thread safe)
       */
      void enqueueMessage( const QString &, const QString &language=NULL );

      /**
       * Pop (get and erase) a message from the queue (thread safe)
       */
      mlText dequeueMessage();

      /**
       * Is there any Message (thread safe)
       */
      bool isEmptyMessage();

      /**
       * Sets a text to say it and navigate it (thread safe) (see also playText, stopText, etc)
       */
      void setText( const QString &, const QString &language=NULL  );

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
      bool isStopedText();

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
       * Start text (thread safe)
       */
      void playText();

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
       * will prevent Proklam from doing useless and CPU consuming loops.
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
       * List of sentenses of the text
       */
      QStringList textSents;

      /**
       * Language of the text
       */
      QString textLanguage;

      /**
       * Mutex for reading/writing text
       */
      QMutex textMutex;

      /**
       * holds true if the text is stoped
       */ 
      bool textPaused;    

      /**
       * Iterator of the sentenses of the text
       */
      QStringList::Iterator textIterator;
};

#endif // _SPEECHDATA_H_
