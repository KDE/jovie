/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginconf.cpp
  This file is the templates for the configuration plug ins.
  ------------------- 
  Copyright : (C) 2002 by Jos� Pablo Ezequiel "Pupeno" Fern�ndez
  -------------------
  Original author: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org>
  Current Maintainer: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org> 
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
 ******************************************************************************/

// $Id$

#include "pluginconf.h"
#include "pluginconf.moc"

/**
 * Constructor 
 */
PlugInConf::PlugInConf( QWidget *parent, const char *name) : QWidget(parent, name){
   kdDebug() << "Running: PlugInConf::PlugInConf( QWidget *parent, const char *name)" << endl;
}

/**
 * Destructor 
 */
PlugInConf::~PlugInConf(){
   kdDebug() << "Running: PlugInConf::~PlugInConf()" << endl;
}

/**
 * This method is invoked whenever the module should read its 
 * configuration (most of the times from a config file) and update the 
 * user interface. This happens when the user clicks the "Reset" button in 
 * the control center, to undo all of his changes and restore the currently 
 * valid settings. NOTE that this is not called after the modules is loaded,
 * so you probably want to call this method in the constructor.
 */
void PlugInConf::load(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: PlugInConf::load(KConfig *config, const QString &langGroup)" << endl;
}

/**
 * This function gets called when the user wants to save the settings in 
 * the user interface, updating the config files or wherever the 
 * configuration is stored. The method is called when the user clicks "Apply" 
 * or "Ok". 
 */
void PlugInConf::save(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: PlugInConf::save(KConfig *config, const QString &langGroup)" << endl;
}

/** 
 * This function is called to set the settings in the module to sensible
 * default values. It gets called when hitting the "Default" button. The 
 * default values should probably be the same as the ones the application 
 * uses when started without a config file.
 */
void PlugInConf::defaults(){
   kdDebug() << "Running: PlugInConf::defaults()" << endl;
}
