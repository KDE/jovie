/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.cpp
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

#include <qcstring.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "kttsd.h"
#include "speaker.h"

#include "kttsd.moc"

KTTSD::KTTSD( QObject *parent, const char *name) : QObject(parent, name), DCOPObject("kspeech"){
    kdDebug() << "Running: KTTSD::KTTSD( QObject *parent, const char *name)" << endl;
    // Do stuff here
    //setIdleTimeout(15); // 15 seconds idle timeout.
    kdDebug() << "Instantiating Speaker and running it as another thread" << endl;

    // By default, everything is ok, don't worry, be happy
    ok = true;

    // Create speechData object, and load configuration checking for the return
    speechData = new SpeechData();
    if(!speechData->readConfig()){
        KMessageBox::error(0, i18n("No default language defined. Without a default language the text to speech service cannot work. Text to speech service exiting."), i18n("Text To Speech Error"));
        ok = false;
        return;
    }

    // Create speaker object and load plug ins, checking for the return
    speaker = new Speaker(speechData);
    int load = speaker->loadPlugIns();
    if(load == -1){
        KMessageBox::error(0, i18n("No speech synthesizer plugin found. This program cannot run without a speech synthesizer. Text to speech service exiting."), i18n("Text To Speech Error"));
        ok = false;
        return;
    } else if(load == 0){
        KMessageBox::error(0, i18n("A speech synthesizer plugin was not found or is corrupt"), i18n("Text To Speech Error"));
    }

    // Let's rock!
    speaker->start();
}

/**
 * Destructor
 * Terminate speaker thread
 */
KTTSD::~KTTSD(){
    kdDebug() << "Running: KTTSD::~KTTSD()" << endl;
    kdDebug() << "Stoping KTTSD service" << endl;
    speaker->exit();
}

/**
 * DCOP exported function to say warnings
 */
void KTTSD::sayWarning(const QString &warning, const QString &language=NULL){
    kdDebug() << "Running: KTTSD::sayWarning(const QString &warning, const QString &language=NULL)" << endl;
    kdDebug() << "Adding '" << warning << "' to warning queue." << endl;
    speechData->enqueueWarning(warning, language);
}

/**
 * DCOP exported function to say messages
 */
void KTTSD::sayMessage(const QString &message, const QString &language=NULL){
    kdDebug() << "Running: KTTSD::sayMessage(const QString &message, const QString &language=NULL)" << endl;
    kdDebug() << "Adding '" << message << "' to message queue." << endl;
    speechData->enqueueMessage(message, language);
}

/**
 * DCOP exported function to sat text
 */
void KTTSD::setText(const QString &text, const QString &language=NULL){
    kdDebug() << "Running: setText(const QString &text, const QString &language=NULL)" << endl;
    kdDebug() << "Setting text: '" << text << "'" << endl;
    speechData->setText(text, language);
}

/**
 * Remove the text
 */
void KTTSD::removeText(){
    kdDebug() << "Running: KTTSD::removeText()" << endl;
    speechData->removeText();
}

/**
 * Previous paragrah
 */
void KTTSD::prevParText(){
    kdDebug() << "Running: KTTSD::prevParText()" << endl;
    speechData->prevParText();
}
  
/**
 * Previous sentence
 */  
void KTTSD::prevSenText(){
    kdDebug() << "Running: KTTSD::prevSenText()" << endl;
    speechData->prevSenText();
}

/**
 * Pause text
 */
void KTTSD::pauseText(){
    kdDebug() << "Running: KTTSD::pauseText()" << endl;
    speechData->pauseText();
}

/**
 * Pause text and go to the begining
 */
void KTTSD::stopText(){
    kdDebug() << "Running: KTTSD::stopText()" << endl;
    speechData->stopText();
}

/**
 * Start text
 */
void KTTSD::playText(){
    kdDebug() << "Running: KTTSD::playText()" << endl;
    speechData->playText();
}
 
/**
 * Next sentence
 */   
void KTTSD::nextSenText(){
    kdDebug() << "Running: KTTSD::nextSenText()" << endl;
    speechData->nextSenText();
}

/**
 * Next paragrah
 */
void KTTSD::nextParText(){
    kdDebug() << "Running: KTTSD::nextParText()" << endl;
    speechData->nextParText();
}

