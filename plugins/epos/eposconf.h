/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposconf.h
  Configuration widget and functions for Epos plug in
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
 
#ifndef _EPOSCONF_H_
#define _EPOSCONF_H_

// Qt includes.
#include <qstring.h>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>

// KTTS includes.
#include <pluginconf.h>

// Epos plugin includes.
#include "eposconfwidget.h"

class EposProc;
class KArtsServer;
namespace KDE {
    class PlayObject;
}

class EposConf : public EposConfWidget {
    Q_OBJECT 

    public:
        /** Constructor */
        EposConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~EposConf();

        /** This method is invoked whenever the module should read its 
        * configuration (most of the times from a config file) and update the 
        * user interface. This happens when the user clicks the "Reset" button in 
        * the control center, to undo all of his changes and restore the currently 
        * valid settings. NOTE that this is not called after the modules is loaded,
        * so you probably want to call this method in the constructor.
        */
        void load(KConfig *config, const QString &langGroup);

        /** This function gets called when the user wants to save the settings in 
        * the user interface, updating the config files or wherever the 
        * configuration is stored. The method is called when the user clicks "Apply" 
        * or "Ok". 
        */
        void save(KConfig *config, const QString &langGroup);

        /** This function is called to set the settings in the module to sensible
        * default values. It gets called when hitting the "Default" button. The 
        * default values should probably be the same as the ones the application 
        * uses when started without a config file. 
        */
        void defaults();

    private slots:
        void configChanged(){
            kdDebug() << "EposConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotEposTest_clicked();
        void slotSynthFinished();
   
    private:
        void buildCodecList ();
        
        // Epos synthesizer.
        EposProc* m_eposProc;
        // aRts server.
        KArtsServer* m_artsServer;
        // aRts player.
        KDE::PlayObject* m_playObj;
        // Synthesized wave file name.
        QString m_waveFile;
};
#endif // _EPOSCONF_H_
