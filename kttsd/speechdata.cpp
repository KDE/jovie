/***************************************************** vim:set ts=4 sw=4 sts=4:
  speechdata.cpp
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

#include <stdlib.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kapplication.h>
#include <qregexp.h>

#include "speechdata.h"
#include "speechdata.moc"

/**
 * Constructor
 * Sets text to be stoped and warnings and messages queues to be autodelete.
 * Loads configuration
 */
SpeechData::SpeechData(){
    kdDebug() << "Running: SpeechData::SpeechData()" << endl;
    // The text should be stoped at the beggining (thread safe)
    textMutex.lock();
    reading = false;
    textMutex.unlock();

    // Warnings queue to be autodelete  (thread safe)
    warningsMutex.lock();
    warnings.setAutoDelete(true);
    warningsMutex.unlock();

    // Messages queue to be autodelete (thread safe)
    messagesMutex.lock();
    messages.setAutoDelete(true);
    messagesMutex.unlock();

    // Load configuration
    //config = KGlobal::config();
    config = new KConfig("kttsdrc");
}

bool SpeechData::readConfig(){
    // Load configuration
    // Set the group general for the configuration of KTTSD itself (no plug ins)
    config->setGroup("General");

    // Load the configuration of the text interruption messages and sound
    textPreMsgEnabled = config->readBoolEntry("TextPreMsgEnabled", false);
    textPreMsg = config->readEntry("TextPreMsg");

    textPreSndEnabled = config->readBoolEntry("TextPreSndEnabled", false);
    textPreSnd = config->readPathEntry("TextPreSnd");

    textPostMsgEnabled = config->readBoolEntry("TextPostMsgEnabled", false);
    textPostMsg = config->readEntry("TextPostMsg");

    textPostSndEnabled = config->readBoolEntry("TextPostSndEnabled", false);
    textPostSnd = config->readPathEntry("TextPostSnd");

    // Load the configuration of the par interruption messages and sound
    parPreMsgEnabled = config->readBoolEntry("ParPreMsgEnabled", false);
    parPreMsg = config->readEntry("ParPreMsg");

    parPreSndEnabled = config->readBoolEntry("ParPreSndEnabled", false);
    parPreSnd = config->readPathEntry("ParPreSnd");

    parPostMsgEnabled = config->readBoolEntry("ParPostMsgEnabled", false);
    parPostMsg = config->readEntry("ParPostMsg");

    parPostSndEnabled = config->readBoolEntry("ParPostSndEnabled", false);
    parPostSnd = config->readPathEntry("ParPostSnd");

    if(config->hasKey("DefaultLanguage")){
        defaultLanguage = config->readEntry("DefaultLanguage");
        return true;
    } else {
        return false;
    }
}
/**
 * Destructor
 */
SpeechData::~SpeechData(){
    kdDebug() << "Running: SpeechData::~SpeechData()" << endl;
}

/**
 * Add a new warning to the queue (thread safe)
 */
void SpeechData::enqueueWarning( const QString &warning, const QString &language ){
    kdDebug() << "Running: SpeechData::enqueueWarning( const QString &warning )" << endl;
    mlText *temp = new mlText();
    temp->text = warning;
    temp->language = language;
    warningsMutex.lock();
    warnings.enqueue( temp );
    uint count = warnings.count();
    warningsMutex.unlock();
    kdDebug() << "Adding '" << temp->text << "' with language '" << temp->language << "' to the warnings queue leaving a total of " << count << " items." << endl;
    newTMW.wakeOne();
}

/**
 * Pop (get and erase) a warning from the queue (thread safe)
 */
mlText SpeechData::dequeueWarning(){
    kdDebug() << "Running: SpeechData::dequeueWarning()" << endl;
    warningsMutex.lock();
    mlText *temp = warnings.dequeue();
    uint count = warnings.count();
    warningsMutex.unlock();
    kdDebug() << "Removing '" << temp->text << "' with language '" << temp->language << "' from the warnings queue leaving a total of " << count << " items." << endl;
    return *temp;
}

/**
 * Are there any Warning (thread safe)
 */
bool SpeechData::warningInQueue(){
    kdDebug() << "Running: SpeechData::warningInQueue() const" << endl;
    warningsMutex.lock();
    bool temp = !warnings.isEmpty();
    warningsMutex.unlock();
    if(temp){
        kdDebug() << "The warnings queue is NOT empty" << endl;
    } else {
        kdDebug() << "The warnings queue is empty" << endl;
    }
    return temp;
}

/**
 * Add a new message to the queue (thread safe)
 */
void SpeechData::enqueueMessage( const QString &message, const QString &language ){
    kdDebug() << "Running: SpeechData::enqueueMessage( const QString &message )" << endl;
    mlText *temp = new mlText();
    temp->text = message;
    temp->language = language;
    messagesMutex.lock();
    messages.enqueue( temp );
    uint count = warnings.count();
    messagesMutex.unlock();
    kdDebug() << "Adding '" << temp->text << "' with language '" << temp->language << "' to the messages queue leaving a total of " << count << " items." << endl;
    newTMW.wakeOne();
}

/**
 * Pop (get and erase) a message from the queue (thread safe)
 */
mlText SpeechData::dequeueMessage(){
    kdDebug() << "Running: SpeechData::dequeueMessage()" << endl;
    messagesMutex.lock();
    mlText *temp = messages.dequeue();
    uint count = warnings.count();
    messagesMutex.unlock();
    kdDebug() << "Removing '" << temp->text << "' with language '" << temp->language << "' from the messages queue leaving a total of " << count << " items." << endl;
    return *temp;
}

/**
 * Are there any Message (thread safe)
 */
bool SpeechData::messageInQueue(){
    kdDebug() << "Running: SpeechData::messageInQueue() const" << endl;
    messagesMutex.lock();
    bool temp = !messages.isEmpty();
    messagesMutex.unlock();
    if(temp){
        kdDebug() << "The messages queue is NOT empty" << endl;
    } else {
        kdDebug() << "The messages queue is empty" << endl;
    }
    return temp;
}

/**
 * Sets a text to say it and navigate it (thread safe) (see also resumeText, stopText, etc)
 */
void SpeechData::setText( const QString &text, const QString &language ){
    kdDebug() << "Running: SpeechData::setText" << endl;
    // There has to be a better way
    kdDebug() << "I'm getting: " << endl << text  << endl;
    QString temp = text;
    QStringList tempList = QStringList::split(QRegExp("([\\.\\?\\:\\;]\\s)|(\\n\\n)"), temp, false);
/*    
    // This should be something better, like "[a-zA-Z]\. " (a regexp of course) The dot (.) is used for more than ending a sentence.
    temp.replace('.', '\n');
    QStringList tempList = QStringList::split('\n', temp, true);*/
    
    for ( QStringList::Iterator it = tempList.begin(); it != tempList.end(); ++it ) {
        kdDebug() << "'" << *it << "'" << endl;
    }
    

    textMutex.lock();
    bool wasReading = reading;
    reading = false;
    textLanguage = language;
    textSents = tempList;
    textIterator = textSents.begin();
    textMutex.unlock();
    if (wasReading)
        emit textStopped();
    emit textSet();
}

/**
 * Remove the text (thread safe)
 */
void SpeechData::removeText(){
    kdDebug() << "Running: SpeechData::removeText()" << endl;
    textMutex.lock();
    bool wasReading = reading;
    reading = false;
    textIterator = textSents.begin();
    textSents.clear();
    textMutex.unlock();
    if (wasReading)
        emit textStopped();
    emit textRemoved();
}

 /**
  * Get a sentence to speak it.
  */
mlText SpeechData::getSentenceText(){
    kdDebug() << "Running: QString getSentenceText()" << endl;
    mlText *temp = new mlText();
    textMutex.lock();
    temp->text = *textIterator;
    temp->language = textLanguage;
    textIterator++;
    if(textIterator == textSents.end()){
        reading = false;
        textIterator = textSents.begin();
    }
    textMutex.unlock();
    if (!reading)
        emit textFinished();
    return *temp;
 }

/**
 * Returns true if the text has not to be speaked (thread safe)
 */
bool SpeechData::currentlyReading(){
    kdDebug() << "Running: SpeechData::currentlyReading()" << endl;
    textMutex.lock();
    bool temp = reading;
    textMutex.unlock();
    return temp;
}

/**
 * Jump to the previous paragrah (thread safe)
 */
void SpeechData::prevParText(){
    kdDebug() << "Running: SpeechData::prevParText()" << endl;
    textMutex.lock();
    while(*textIterator != "" and textIterator != textSents.begin()){
        textIterator--;
    }
    textMutex.unlock();
}

/**
 * Jump to the previous sentence (thread safe)
 */
void SpeechData::prevSenText(){
    kdDebug() << "Running: SpeechData::prevSenText()" << endl;
    textMutex.lock();
    if (textIterator != textSents.begin())
        textIterator--;
    textMutex.unlock();
}

/**
 * Stop playing text (thread safe)
 */
void SpeechData::pauseText(){
    kdDebug() << "Running: SpeechData::pauseText()" << endl;
    textMutex.lock();
    reading = false;
    textMutex.unlock();
    emit textPaused();
}

/**
 * Stop playing text and go to the begining (thread safe)
 */
void SpeechData::stopText(){
    kdDebug() << "Running: SpeechData::stopText()" << endl;
    textMutex.lock();
    reading = false;
    textIterator = textSents.begin();
    textMutex.unlock();
    emit textStopped();
}

/**
 * Start text at the beginning (thread safe)
 */
void SpeechData::startText(){
    kdDebug() << "Running: SpeechData::startText()" << endl;
    textMutex.lock();
    bool wasReading = reading;
    reading = true;
    textIterator = textSents.begin();
    textMutex.unlock();
    newTMW.wakeOne();
    if (wasReading)
        emit textStopped();
    emit textStarted();
}

/**
 * Resume text if paused, otherwise start at the beginning
 */
void SpeechData::resumeText(){
    kdDebug() << "Running: SpeechData::resumeText()" << endl;
    textMutex.lock();
    bool wasReading = reading;
    bool wasStarted = (textIterator == textSents.begin());
    reading = true;
    textMutex.unlock();
    newTMW.wakeOne();

    if (!wasReading) {
        if (wasStarted)
            emit textStarted();
        else
            emit textResumed();
    }
}

/**
 * Next sentence (thread safe)
 */
void SpeechData::nextSenText(){
    kdDebug() << "Running: SpeechData::nextSenText()" << endl;
    textMutex.lock();
    if (textIterator != textSents.end())
        textIterator++;
    textMutex.unlock();
}

/**
 * Next paragrah (thread safe)
 */
void SpeechData::nextParText(){
    kdDebug() << "Running: SpeechData::nextParText()" << endl;
    textMutex.lock();
    while(*textIterator != "" and textIterator != textSents.end()){
        textIterator++;
    }
    textMutex.unlock();
}
