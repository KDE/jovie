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

#include "festivalintproc.h"
#include "festivalintproc.moc"
 
/** Constructor */
FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList& ) : 
    PlugInProc( parent, name ){
    kdDebug() << "Running: FestivalIntProc::FestivalIntProc( QObject* parent, const char* name, const QStringList &args)" << endl;
    ready = true;
    festProc = 0;
}

/** Destructor */
FestivalIntProc::~FestivalIntProc(){
    kdDebug() << "Running: FestivalIntProc::~FestivalIntProc()" << endl;
    if (festProc)
    {
        stopText();
        if (festProc) delete festProc;
    }
}

/** Initializate the speech */
bool FestivalIntProc::init(const QString &lang, KConfig *config){
    kdDebug() << "Running: FestivalIntProc::init(const QString &lang)" << endl;
    kdDebug() << "Initializing plug in: Festival" << endl;

    // To save resources, this function should get a KConfig too
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
void FestivalIntProc::sayText(const QString &text)
{
    kdDebug() << "Running: FestivalIntProc::sayText(const QString &text)" << endl;

    // Initialize Festival only if it's not initialized
    if(!festProc)
    {
        kdDebug()<< "FestivalIntProc::sayText: Creating Festival object" << endl;
        festProc = new KProcIO;
        connect(festProc, SIGNAL(processExited(KProcess*)), this, SLOT(festProcExited(KProcess*)));
        connect(festProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(festProcReceivedStdout(KProcess*, char*, int)));
        connect(festProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(festProcReceivedStderr(KProcess*, char*, int)));
    }
    if (!festProc->isRunning())
    {
        kdDebug()<< "FestivalIntProc::sayText: Starting Festival process" << endl;
        if (forceArts) *festProc << "artsdsp";
        *festProc << "festival";
        *festProc << "--interactive";
        if (festProc->start(KProcess::NotifyOnExit, KProcess::All))
        {
            ready = false;
            // Selecting the voice
            waitTilReady();
            ready = false;
            festProc->writeStdin(voiceCode, true);
        }
        else
        {
            kdDebug() << "FestivalIntProc::sayText: Error starting Festival process.  Is festival in the PATH?" << endl;
            return;
        }
    }
    kdDebug()<< "FestivalIntProc:sayText: Festival initialized" << endl;

/*
        // Setting output thru arts if necessary.
        // Note: Force Arts does not currently work correctly because the artsplay command returns
        // immediately, causing Festival to send prompt, whereupon we send another sentence...etc.
        // As a result, the sentences get spoken on top of each other.
        if(forceArts){
            kdDebug() << "Forcing Arts output" << endl;
            waitTilReady();
            ready = false;
            // On my system, artsplay won't play a file unless the file extension is set.
            // Problem with this solution is it leaves files in /tmp directory.  audio_xxxx.snd
            // Need a better solution.
            festProc->writeStdin(QString("(Parameter.set 'Audio_Command \"mv $FILE $FILE.snd && artsplay $FILE.snd\")"), true);
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(Parameter.set 'Audio_Method 'Audio_Command)"), true);
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(Parameter.set 'Audio_Required_Format 'snd)"), true);
            // Make Festival wait until sentence is spoken before sending prompt.
            waitTilReady();
            ready = false;
            festProc->writeStdin(QString("(audio_mode 'sync)"), true);
        }
*/
    
    // Encode quotation characters.
    QString saidText = text;
    saidText.replace("\\\"", "#!#!");
    saidText.replace("\"", "\\\"");
    saidText.replace("#!#!", "\\\"");
    // Remove certain comment characters.
    saidText.replace("--", "");

    // Ok, let's rock
    waitTilReady();
    ready = false;
    kdDebug() << "Saying text: '" << saidText << "' using Festival plug in with voice " << voiceCode << endl;
    festProc->writeStdin("(SayText \"" + saidText + "\")", true);
    waitTilReady();
    // If using artsdsp, wait an additional second between sentences to prevent the beginning of one
    // sentence from overlapping the end of the previous sentence.  Not sure *why* this is necessary,
    // but there it is.
    if (forceArts) festProc->wait(1);
    kdDebug() << "Finished saying text" << endl;
}

/**
 * Stop text
 */
void FestivalIntProc::stopText(){
    kdDebug() << "Running: FestivalIntProc::stopText()" << endl;
    if (festProc)
    {
        if (ready)
        {
            kdDebug() << "FestivalIntProc::stopText: telling Festival to quit." << endl;
            festProc->writeStdin(QString("(quit)"), true);
        }
        else
        {
            kdDebug() << "FestivalIntProc::stopText: killing Festival." << endl;
            festProc->kill();
        }
        if (festProc)
        {
            kdDebug() << "FestivalIntProc::stopText: waiting for Festival to exit." << endl;
            festProc->wait(1);
        }
    }
    kdDebug() << "FestivalIntProc::stopText: Festival stopped." << endl;
}

void FestivalIntProc::festProcExited(KProcess*)
{
    kdDebug() << "FestivalIntProc:festProcExited: Festival process has exited." << endl;
    ready = true;
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
