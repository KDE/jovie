/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalconf.h
  Configuration widget and functions for Festival plug in
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
      void load(KConfig *config, const QString &configGroup);

      /** This function gets called when the user wants to save the settings in 
         the user interface, updating the config files or wherever the 
         configuration is stored. The method is called when the user clicks "Apply" 
         or "Ok". */
      void save(KConfig *config, const QString &configGroup);

      /** This function is called to set the settings in the module to sensible
         default values. It gets called when hitting the "Default" button. The 
         default values should probably be the same as the ones the application 
         uses when started without a config file. */
      void defaults();

       /**
       * This function informs the plugin of the desired language to be spoken
       * by the plugin.  The plugin should attempt to adapt itself to the
       * specified language code, choosing sensible defaults if necessary.
       * If the passed-in code is QString::null, no specific language has
       * been chosen.
       * @param lang        The desired language code or Null if none.
       *
       * If the plugin is unable to support the desired language, that is OK.
       * Language codes are given by ISO 639-1 and are in lowercase.
       * The code may also include an ISO 3166 country code in uppercase
       * separated from the language code by underscore (_).  For
       * example, en_GB.  If your plugin supports the given language, but
       * not the given country, treat it as though the country
       * code were not specified, i.e., adapt to the given language.
       */
      void setDesiredLanguage(const QString &lang);

       /**
       * Return fully-specified talker code for the configured plugin.  This code
       * uniquely identifies the configured instance of the plugin and distinquishes
       * one instance from another.  If the plugin has not been fully configured,
       * i.e., cannot yet synthesize, return QString::null.
       * @return            Fully-specified talker code.
       */
      QString getTalkerCode();

   private:
       QString m_languageCode;

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
