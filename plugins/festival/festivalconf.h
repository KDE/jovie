/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalconf.h
  Configuration widget and functions for Festival plug in
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
 
#ifndef _FESTIVALCONF_H_
#define _FESTIVALCONF_H_

#include <qstringlist.h>
#include <qvaluelist.h>

#include <kconfig.h>
#include <kdebug.h>

#include <pluginconf.h>

#include "festivalconfwidget.h"

typedef struct voiceStruct{
  QString code;
  QString name;
  QString comment;
  QString path;
} voice;

class FestivalConf : public FestivalConfWidget {
   Q_OBJECT 

   public:
      /** Constructor */
      FestivalConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());

      /** Destructor */
      ~FestivalConf();

      /** This method is invoked whenever the module should read its 
         configuration (most of the times from a config file) and update the 
         user interface. This happens when the user clicks the "Reset" button in 
         the control center, to undo all of his changes and restore the currently 
         valid settings. NOTE that this is not called after the modules is loaded,
         so you probably want to call this method in the constructor.*/
      void load(KConfig *config, const QString &langGroup);

      /** This function gets called when the user wants to save the settings in 
         the user interface, updating the config files or wherever the 
         configuration is stored. The method is called when the user clicks "Apply" 
         or "Ok". */
      void save(KConfig *config, const QString &langGroup);

      /** This function is called to set the settings in the module to sensible
         default values. It gets called when hitting the "Default" button. The 
         default values should probably be the same as the ones the application 
         uses when started without a config file. */
      void defaults();

   private:
      /** List of voices */
      QValueList<voice> voiceList;

   private slots:
      /** Scan for the diferent voices in festivalPath/lib */
      void scanVoices();
      void configChanged(){
         kdDebug() << "Running: FestivalConf::configChanged()"<< endl;
         emit changed(true);
      };
};
#endif // _FESTIVALCONF_H_
