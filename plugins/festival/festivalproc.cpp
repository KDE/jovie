/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalproc.cpp
  Main speaking functions for the Festival Plug in
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

#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <festival.h>

#include <unistd.h>

#include "festivalproc.h"
#include "festivalproc.moc"

 
bool FestivalProc::initialized = false;

/** Constructor */
FestivalProc::FestivalProc( QObject* parent, const char* name, const QStringList &args) : 
    PlugInProc( parent, name ){
    kdDebug() << "Running: FestivalProc::FestivalProc( QObject* parent, const char* name, const QStringList &args)" << endl;
}

/** Desctructor */
FestivalProc::~FestivalProc(){
    kdDebug() << "Running: FestivalProc::~FestivalProc()" << endl;
}

/** Initializate the speech */
bool FestivalProc::init(const QString &lang, KConfig *config){
    kdDebug() << "Running: FestivalProc::init(const QString &lang)" << endl;
    kdDebug() << "Initializing plug in: Festival" << endl;

    // To save resources, this founction should get a KConfig too
    // This KConfig will be passed to this function (already opened) from speaker.cpp
    // KConfig *config = new KConfig("kttsdrc");
    // KConfig *config = KGlobal::config();
    config->setGroup(QString("Lang_")+lang);
    forceArts = config->readBoolEntry("Arts");

    // Get the code for the selected voice
    KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festival/voices", true, false);

    voices.setGroup(config->readEntry("Voice"));
    voiceCode = "("+voices.readEntry("Code")+")";
    kdDebug() << "---- The code for the selected voice " << config->readEntry("Voice") << " is " << voiceCode << endl;
    
    return true;
}

/** Say a text
    text: The text to be speech
*/
void FestivalProc::sayText(const QString &text){
    kdDebug() << "Running: FestivalProc::sayText(const QString &text)" << endl;

    // Initialize Festival only if it's not initialized
    if(initialized == false){
        kdDebug()<< "Initializing Festival" << endl;
        int heap_size = 210000;  // default scheme heap size
        int load_init_files = 1; // we want the festival init files loaded

        festival_initialize(load_init_files,heap_size);
        kdDebug()<< "Festival initialized" << endl;
        initialized = true;
    } 

    // Seting output thru arts if necessary.
    if(forceArts){
        kdDebug() << "Forcing Arts output" << endl;
        if(!festival_eval_command(EST_String("(Parameter.set 'Audio_Command \"artsplay $FILE\")"))){
            kdDebug() << "Error while running (Parameter.set 'Audio_Command \"artsplay $FILE\")" << endl;
        }
        if(!festival_eval_command(EST_String("(Parameter.set 'Audio_Method 'Audio_Command)"))){
            kdDebug() << "Error while running (Parameter.set 'Audio_Method 'Audio_Command)" << endl;
        }
        if(!festival_eval_command(EST_String("(Parameter.set 'Audio_Required_Format 'snd)"))){
            kdDebug() << "Error while running (Parameter.set 'Audio_Required_Format 'snd)" << endl;
        }
    }

    // Selecting the voice
    if(!festival_eval_command(EST_String(voiceCode.latin1()))){
        kdDebug() << "Error selecting the voice" << endl;
    }

    // Ok, let's rock
    kdDebug() << "Saying text: '" << text << "' using Festival plug in with voice " << voiceCode << endl;
    festival_say_text(text.latin1());
    //   //festival_say_text("hello world");
    kdDebug() << "Finished saying text" << endl;
}

/**
 * Stop text
 */
void FestivalProc::stopText(){
    kdDebug() << "Running: FestivalProc::stopText()" << endl;
    // Bogus implementation until Festival can be run in async mode.
}
