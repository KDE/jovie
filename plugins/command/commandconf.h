/***************************************************** vim:set ts=4 sw=4 sts=4:
  commandconf.h
  Configuration for the Command Plug in
  -------------------
  Copyright : (C) 2002,2004 by Gunnar Schmi Dt and Gary Cramblitt
  -------------------
  Original author: Gunnar Schmi Dt <kmouth@schmi-dt.de>
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

#ifndef _COMMANDCONF_H_
#define _COMMANDCONF_H_

// Qt includes.
#include <qstring.h>
#include <qstringlist.h>

// KDE includes.
#include <kconfig.h>

// KTTS includes.
#include <pluginconf.h>

// Command Plugin includes.
#include "commandconfwidget.h"

class CommandProc;
class KArtsServer;
namespace KDE {
    class PlayObject;
}

class CommandConf : public PlugInConf {
    Q_OBJECT 
  
    public:
        /** Constructor */
        CommandConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());
    
        /** Destructor */
        ~CommandConf();
    
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
            kdDebug() << "CommandConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotCommandTest_clicked();
        void slotSynthFinished();
   
    private:
        // Fill the Codec combobox.
        void buildCodecList();
        
        // Configuration Widget.
        CommandConfWidget* m_widget;

        // Language Group.
        QString m_language;
        // Command synthesizer.
        CommandProc* m_commandProc;
        // aRts server.
        KArtsServer* m_artsServer;
        // aRts player.
        KDE::PlayObject* m_playObj;
        // Synthesized wave file name.
        QString m_waveFile;
};
#endif      // _COMMANDCONF_H_
