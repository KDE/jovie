/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginproc.h
  This file is the template for the processing plug ins.
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

#include <qstring.h>

#include <kdebug.h>

#include "pluginproc.h"
#include "pluginproc.moc"

/**
 * Constructor
 */
PlugInProc::PlugInProc( QObject *parent, const char *name) : QObject(parent, name){
    kdDebug() << "Running: PlugInProc::PlugInProc( QObject *parent, const char *name)" << endl;
}

/**
 * Destructor
 */
PlugInProc::~PlugInProc(){
    kdDebug() << "Running: PlugInProc::~PlugInProc()" << endl;
}

/**
 * Initializate the speech
 */
bool PlugInProc::init(const QString &lang, KConfig *config){
    kdDebug() << "Running: PlugInProc::init(const QString &lang)" << endl;
    return false;
}

/** 
 * Say a text
 * text: The text to be speech
 */
void PlugInProc::sayText(const QString &text){
    kdDebug() << "Running: PlugInProc::sayText(const QString &text)" << endl;
}

/**
 * Stop text
 * This function only makes sense in asynchronus modes
 */
void PlugInProc::stopText(){
    kdDebug() << "Running: PlugInProc::stopText()" << endl;
}
