/***************************************************** vim:set ts=4 sw=4 sts=4:
  fliteconf.h
  Configuration widget and functions for Festival (Interactive) plug in
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

#ifndef _FLITECONF_H_
#define _FLITECONF_H_

// Qt includes.
#include <qstring.h>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>

// KTTS includes.
#include <pluginconf.h>

// Flite plugin includes.
#include "fliteconfwidget.h"

class FliteProc;
class KArtsServer;
namespace KDE {
    class PlayObject;
}

class FliteConf : public PlugInConf {
    Q_OBJECT 

    public:
        /** Constructor */
        FliteConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~FliteConf();

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
            kdDebug() << "FliteConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotFliteTest_clicked();
        void slotSynthFinished();
   
    private:
        // Configuration Widget.
        FliteConfWidget* m_widget;
        // Flite synthesizer.
        FliteProc* m_fliteProc;
        // aRts server.
        KArtsServer* m_artsServer;
        // aRts player.
        KDE::PlayObject* m_playObj;
        // Synthesized wave file name.
        QString m_waveFile;
};
#endif // _FLITECONF_H_
