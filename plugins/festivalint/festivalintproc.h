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
#include <kprocio.h>
#include <pluginproc.h>

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
         * Initializate the speech engine.
         * @param lang            Code giving the language to speak text in.  Example "en".
         * @param config          Settings
         */
        bool init(const QString &lang, KConfig *config);
        
        /**
         * Returns true when festival is ready to speak a sentence.
         */
        bool isReady();
        
        /**
         * Say a text string.
         * @param text            The text to speak.
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
         * Festival process
         */
        KProcIO* festProc;
        
        /**
         * True when festival is ready to say another sentence.
         */
        bool ready;
};

#endif // _FESTIVALINTPROC_H_
