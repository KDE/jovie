/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalproc.cpp
  Main speaking functions for the Festival Plug in
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _FESTIVALPROC_H_
#define _FESTIVALPROC_H_

#include <qstringlist.h>

#include <festival.h>

#include <pluginproc.h>

class FestivalProc : public PlugInProc{
    Q_OBJECT 

    public:
        /**
         * Constructor
         */
        FestivalProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /**
         * Destructor
         */
        ~FestivalProc();

        /**
         * Initializate the speech
         */
        bool init(KConfig *config, const QString &configGroup);

        /** Say a text
         *  text: The text to be speech
         */
        void sayText(const QString &text);

        /**
         * Stop text
         */
        void stopText(); 

    private:
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
};

#endif // _FESTIVALPROC_H_
