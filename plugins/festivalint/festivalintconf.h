/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalconf.h
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

// $Id$
 
#ifndef _FESTIVALINTCONF_H_
#define _FESTIVALINTCONF_H_

// Qt includes.
#include <qstringlist.h>
#include <qvaluelist.h>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>

// KTTS includes.
#include <pluginconf.h>

// FestivalInt includes.
#include "festivalintconfwidget.h"

class FestivalIntProc;
class KArtsServer;
namespace KDE {
    class PlayObject;
}

typedef struct voiceStruct{
    QString code;
    QString name;
    QString comment;
    QString path;
    QString languageCode;
} voice;

class FestivalIntConf : public PlugInConf {
    Q_OBJECT 

    public:
        /** Constructor */
        FestivalIntConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~FestivalIntConf();

        /** This method is invoked whenever the module should read its 
        *  configuration (most of the times from a config file) and update the 
        *  user interface. This happens when the user clicks the "Reset" button in 
        *  the control center, to undo all of his changes and restore the currently 
        *  valid settings. NOTE that this is not called after the modules is loaded,
        *  so you probably want to call this method in the constructor.
        */
        void load(KConfig *config, const QString &langGroup);

        /** This function gets called when the user wants to save the settings in 
        *  the user interface, updating the config files or wherever the 
        *  configuration is stored. The method is called when the user clicks "Apply" 
        *  or "Ok". 
        */
        void save(KConfig *config, const QString &langGroup);

        /** This function is called to set the settings in the module to sensible
        *  default values. It gets called when hitting the "Default" button. The 
        *  default values should probably be the same as the ones the application 
        *  uses when started without a config file. */
        void defaults();

    private slots:
        /** Scan for the different voices in festivalPath/lib */
        void scanVoices();
        void configChanged(){
            kdDebug() << "FestivalIntConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotTest_clicked();
        void slotSynthFinished();
        void timeBox_valueChanged(int percentValue);
        void timeSlider_valueChanged(int sliderValue);
      
   private:
        int percentToSlider(int percentValue);
        int sliderToPercent(int sliderValue);
        void setDefaultVoice();
        QString getDefaultVoicesPath();
        
        // Configuration Widget.
        FestivalIntConfWidget* m_widget;
       
        // Language group.
        QString m_langGroup;
        /** List of voices */
        QValueList<voice> voiceList;
        // Festival synthesizer.
        FestivalIntProc* m_festProc;
        // aRts server.
        KArtsServer* m_artsServer;
        // aRts player.
        KDE::PlayObject* m_playObj;
        // Synthesized wave file name.
        QString m_waveFile;

};
#endif // _FESTIVALINTCONF_H_
