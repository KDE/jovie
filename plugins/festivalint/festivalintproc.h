/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalintproc.h
  Main speaking functions for the Festival (Interactive) Plug in
  -------------------
  Copyright : (C) 2004 by Gary Cramblitt
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

#ifndef _FESTIVALINTPROC_H_
#define _FESTIVALINTPROC_H_

#include <qstringlist.h>

// #include <festival.h>

#include <pluginproc.h>
#include <kprocio.h>

class FestivalIntProc : public PlugInProc{
    Q_OBJECT 

    public:
        /**
         * Constructor
         */
        FestivalIntProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /**
         * Destructor
         */
        ~FestivalIntProc();

        /**
         * Initializate the speech
         */
        bool init(const QString &lang, KConfig *config);
        
        /**
         * Returns true when festival is ready to speak a sentence.
         */
        bool isReady();
        
        /** Say a text
         *  text: The text to be speech
         */
        void sayText(const QString &text);

        /**
         * Stop text
         */
        void stopText();
        
    private slots:
        void festProcExited(KProcess* proc);
        void festProcReceivedStdout(KProcess* proc, char* buffer, int buflen);
        void festProcReceivedStderr(KProcess* proc, char* buffer, int buflen);

    private:
        void waitTilReady();
        
        /**
         * To arts or not to arts ? (from configuration)
         */
        bool forceArts;

        /**
         * Selected voice
         */
        QString voiceCode;


        /**
         * Keep track of initialization;
         * It is static because festival has to be initializated only once (for all the application)
         */
        static bool initialized;
        
        /**
         * Festival process
         */
        KProcIO* festProc;
        
        /**
         * True when festival is ready to say another sentence.
         */
        bool ready;
};

#endif // _FESTIVALINTPROC_H_
