/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalintproc.cpp
  Main speaking functions for the Festival (Interactive) Plug in
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
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
#include <qthread.h>

// #include <festival.h>

#include <unistd.h>

#include "festivalintproc.h"
#include "festivalintproc.moc"

 
bool FestivalIntProc::initialized = false;

/** Constructor */
FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList& ) : 
    PlugInProc( parent, name ){
    kdDebug() << "Running: FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList &args)" << endl;
    ready = true;
}

/** Destructor */
FestivalIntProc::~FestivalIntProc(){
    kdDebug() << "Running: FestivalIntProc::~FestivalIntProc()" << endl;
}

/** Initializate the speech */
bool FestivalIntProc::init(const QString &lang, KConfig *config){
    kdDebug() << "Running: FestivalIntProc::init(const QString &lang)" << endl;
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
void FestivalIntProc::sayText(const QString &text){
    kdDebug() << "Running: FestivalIntProc::sayText(const QString &text)" << endl;

    // Initialize Festival only if it's not initialized
    if(initialized == false){
        kdDebug()<< "Initializing Festival" << endl;
        festProc = new KProcIO;
        *festProc << "festival";
        *festProc << "--interactive";
        connect(festProc, SIGNAL(processExited(KProcess*)), this, SLOT(festProcExited(KProcess*)));
        connect(festProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(festProcReceivedStdout(KProcess*, char*, int)));
        connect(festProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(festProcReceivedStderr(KProcess*, char*, int)));
        ready = false;
        if (!festProc->start(KProcess::NotifyOnExit, KProcess::All))
        {
            kdDebug() << "Error starting Festival process.  Is festival in the PATH?" << endl;
            return;
        }
        kdDebug()<< "Festival initialized" << endl;
        initialized = true;

        // Setting output thru arts if necessary.
        if(forceArts){
            kdDebug() << "Forcing Arts output" << endl;
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(Parameter.set 'Audio_Command \"artsplay $FILE\")"), true);
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(Parameter.set 'Audio_Method 'Audio_Command)"), true);
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(Parameter.set 'Audio_Required_Format 'snd)"), true);
        }
        // Selecting the voice
        waitTilReady();
        ready = false;
        festProc->writeStdin(voiceCode, true);
    }
    
    // Encode quotation characters.
    QString saidText = text;
    saidText.replace("\\\"", "#!#!");
    saidText.replace("\"", "\\\"");
    saidText.replace("#!#!", "\\\"");
    // Remove certain comment lines.
    saidText.replace("--", "");

    // Ok, let's rock
    waitTilReady();
    ready = false;
    kdDebug() << "Saying text: '" << saidText << "' using Festival plug in with voice " << voiceCode << endl;
    festProc->writeStdin("(SayText \"" + saidText + "\")", true);
    waitTilReady();
    kdDebug() << "Finished saying text" << endl;
}

/**
 * Stop text
 */
void FestivalIntProc::stopText(){
    kdDebug() << "Running: FestivalIntProc::stopText()" << endl;
    if (initialized)
    {
      festProc->writeStdin(QString("(audio_mode 'shutup)"), true);
      festProc->writeStdin(QString("(quit)"), true);
    }
    kdDebug() << "Festival stopped." << endl;
}

void FestivalIntProc::festProcExited(KProcess*)
{
    initialized = false;
    ready = true;
    delete festProc;
    festProc = 0;
}

void FestivalIntProc::festProcReceivedStdout(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "Received from Festival: " << buf << endl;
    if (buf.contains("festival>") > 0) ready = true;
    if (ready) kdDebug() << "Festival is ready for another sentence." << endl;
}

void FestivalIntProc::festProcReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "Received error from Festival: " << buf << endl;
}

bool FestivalIntProc::isReady() { return ready; }

void FestivalIntProc::waitTilReady() { while (!ready) festProc->wait(1); }
