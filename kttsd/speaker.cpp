/***************************************************** vim:set ts=4 sw=4 sts=4:
  speaker.cpp
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
 
#include <qthread.h>

#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <ktrader.h>
#include <kapplication.h>

#include "speaker.h"
#include "speaker.moc"

/**
 * Constructor
 * Calls load plug ins
 */
Speaker::Speaker( SpeechData *speechData, QObject *parent, const char *name) : QObject(parent, name), QThread(), speechData(speechData){
    kdDebug() << "Running: Speaker::Speaker()" << endl;
    exitRequested = false;
    loadedPlugIns.setAutoDelete(true);
}

/**
 * Destructor
 */
Speaker::~Speaker(){
    kdDebug() << "Running: Speaker::~Speaker()" << endl;
}

/**
 * Base function, where the thread will start
 */
void Speaker::run()
{
    kdDebug() << "Running: Speaker::run()" << endl;
    kdDebug() << "Speaker thread started" << endl;

    while(!exitRequested)
    {
        kdDebug() << "Speaker going to sleep" << endl;
        speechData->newTMW.wait();
        kdDebug() << "Waking speaker up to process the new texts, messages, warnings" << endl;

        if (exitRequested)
        {
            kdDebug() << "Speaker::run: exiting due to request 1." << endl;
            return;
        }

        checkSayWarning();
        checkSayMessage();
        checkSayText();
    }
    kdDebug() << "Speaker::run: exiting due to request 2." << endl;
}

/**
 * Checks for warnings and if there's any, it says it.
 */
void Speaker::checkSayWarning(){
    kdDebug() << "Running: Speaker::checkSayWarning()" << endl;
    while(speechData->warningInQueue()){
        mlText temp = speechData->dequeueWarning();
        if(loadedPlugIns[temp.talker]){
            loadedPlugIns[temp.talker]->sayText(temp.text);
        } else {
            loadedPlugIns[speechData->defaultTalker]->sayText(temp.text);
        }
    }
}

/**
 * Checks for messages (and warnings) and if there's any, it says it.
 */
void Speaker::checkSayMessage(){
    while(speechData->messageInQueue() or speechData->warningInQueue()){
        checkSayWarning();
        mlText temp = speechData->dequeueMessage();
        if(loadedPlugIns[temp.talker]){
            loadedPlugIns[temp.talker]->sayText(temp.text);
        } else {
            loadedPlugIns[speechData->defaultTalker]->sayText(temp.text);
        }
    }
}

/**
 * Checks for playable texts (messages and warnings) and if there's any, it says it.
 */
void Speaker::checkSayText(){
    if (speechData->currentlyReading()) {
        emit readingStarted();
        emit paragraphStarted();
    }
    while(speechData->currentlyReading() or speechData->messageInQueue() or speechData->warningInQueue())
    {
        if(speechData->warningInQueue()){
            emit readingInterrupted();
            if(speechData->parPreMsgEnabled){
                loadedPlugIns[speechData->defaultTalker]->sayText(speechData->parPreMsg);
            }
            checkSayWarning();
            if(speechData->parPostMsgEnabled){
                loadedPlugIns[speechData->defaultTalker]->sayText(speechData->parPostMsg);
            }
            emit readingResumed();
        }
        mlText temp = speechData->getSentenceText();
        if (temp.text == "") {
            if (speechData->currentlyReading()) emit paragraphFinished();
            if(speechData->messageInQueue()){
                emit readingInterrupted();
                if(speechData->textPreMsgEnabled){
                    loadedPlugIns[speechData->defaultTalker]->sayText(speechData->textPreMsg);
                }
                checkSayMessage();
                if(speechData->textPostMsgEnabled){
                    loadedPlugIns[speechData->defaultTalker]->sayText(speechData->textPostMsg);
                }
                emit readingResumed();
            }
            if (speechData->currentlyReading()) emit paragraphStarted();
        } else {
            kdDebug() << "REALLY SAYING " << temp.text << endl;
            emit sentenceStarted(temp.text, temp.talker, temp.appId, temp.jobNum, temp.seq);
            if(loadedPlugIns[temp.talker]){
                loadedPlugIns[temp.talker]->sayText(temp.text);
            } else {
                loadedPlugIns[speechData->defaultTalker]->sayText(temp.text);
            }
            emit sentenceFinished(temp.appId, temp.jobNum, temp.seq);
        }
    }
    if (speechData->currentlyReading()) {
        emit paragraphFinished();
        emit readingStopped();
    }
}

/**
 * Load all the configured plug ins populating loadedPlugIns
 */
int Speaker::loadPlugIns(){
    kdDebug() << "Running: Speaker::loadPlugIns()" << endl;
    int good = 0, bad = 0;

    QStringList langs = speechData->config->groupList().grep("Lang_");
    KLibFactory *factory;
    for( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
        kdDebug() << "Loading plugInProc for: " << *it << endl;

        // Set the group for the language we're loading
        speechData->config->setGroup(*it);

        // Get the name of the plug in we will try to load
        QString plugInName = speechData->config->readEntry("PlugIn");

        // Query for all the KTTSD SynthPlugins and store the list in offers
        KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin", QString("Name == '%1'").arg(plugInName));

        if(offers.count() > 1){
            bad++;
            kdDebug() << "More than 1 plug in doesn't make any sense, well, let's use any" << endl;
        } else if(offers.count() < 1){
            bad++;
            kdDebug() << "Less than 1 plug in, nothing can be done" << endl;
        } else {
            kdDebug() << "Loading " << offers[0]->library() << endl;
            factory = KLibLoader::self()->factory(offers[0]->library());
            if(factory){
                PlugInProc *speech = KParts::ComponentFactory::createInstanceFromLibrary<PlugInProc>( offers[0]->library(), this, offers[0]->library());
                if(!speech){
                    kdDebug() << "Couldn't create the speech object from " << offers[0]->library() << endl;
                    bad++;
                } else {
                    speech->init((*it).right((*it).length()-5), speechData->config);
                    kdDebug() << "Plug in " << plugInName << " for language " <<  (*it).right((*it).length()-5) << " created succesfully." << endl;
                    loadedPlugIns.insert( (*it).right((*it).length()-5), speech);
                    good++;
                }
            } else {
                kdDebug() << "Couldn't create the factory object from " << offers[0]->library() << endl;
                bad++;
            }
        }
    }
    if(bad > 0){
        if(good == 0){
            // No plug in could be loeaded
            return -1;
        } else {
            // At least one plug in was loaded and one failed
            return 0;
        }
    } else {
        // All the plug in were loaded perfectly
        return 1;
    }
}

/**
 * Tells the thread to exit
 */
void Speaker::requestExit(){
    kdDebug() << "Speaker::requestExit: Running" << endl;
    exitRequested = true;
    speechData->newTMW.wakeOne();
}

/**
* SpeakerTerminator 
*
* A separate thread to request that the speaker thread exit, and when it does, emits a signal.
* We need to do this in a separate thread because the main thread cannot call speaker->wait(),
* otherwise it would block the QT event loop and hang the program.
*/
SpeakerTerminator::SpeakerTerminator(Speaker *speaker, QObject *parent, const char *name) :
    QObject(parent, name),
    QThread(),
    speaker(speaker)
{
    kdDebug() << "SpeakerTerminator::SpeakerTerminator: running" << endl;
}

void SpeakerTerminator::run()
{
    if (speaker)
    {
        speaker->requestExit();
        kdDebug() << "SpeakerTerminator::run: Waiting for speaker to finish" << endl;
        speaker->wait();
        kdDebug() << "SpeakerTerminator::run: speaker has finished." << endl;
        emit speakerFinished();
    }
    deleteLater();
}

