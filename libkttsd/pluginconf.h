/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginconf.h
  This file is the template for the configuration plug ins.
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

#ifndef _PLUGINCONF_H_
#define _PLUGINCONF_H_

#include <qwidget.h>

#include <kconfig.h>
#include <kdebug.h>

/**
 * Base class for the configuration plug ins
 */
class PlugInConf : public QWidget{
   Q_OBJECT

   public:
      /**
      * Constructor 
      */
      PlugInConf( QWidget *parent = 0, const char *name = 0);

      /**
      * Destructor 
      */
      virtual ~PlugInConf();

      /**
      * This method is invoked whenever the module should read its 
      * configuration (most of the times from a config file) and update the 
      * user interface. This happens when the user clicks the "Reset" button in 
      * the control center, to undo all of his changes and restore the currently 
      * valid settings. NOTE that this is not called after the modules is loaded,
      * so you probably want to call this method in the constructor.
      */
      virtual void load(KConfig *config, const QString &langGroup);

      /**
      * This function gets called when the user wants to save the settings in 
      * the user interface, updating the config files or wherever the 
      * configuration is stored. The method is called when the user clicks "Apply" 
      * or "Ok". 
      */
      virtual void save(KConfig *config, const QString &langGroup);

      /** 
      * This function is called to set the settings in the module to sensible
      * default values. It gets called when hitting the "Default" button. The 
      * default values should probably be the same as the ones the application 
      * uses when started without a config file.
      */
      virtual void defaults();

   public slots:
      /**
      * This slot is called when the configuration is changed
      */
      void configChanged(){
         kdDebug() << "Running: PlugInConf::configChanged()"<< endl;
         emit changed(true);
      };

   signals:
      /**
      * This signal indicates that the configuration has been changed
      */
      void changed(bool);
};

#endif  //_PLUGINCONF_H_
