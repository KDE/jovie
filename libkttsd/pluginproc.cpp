/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginproc.h
  This file is the template for the processing plug ins.
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
* Initializate the speech plugin.
*/
bool PlugInProc::init(const QString& /*lang*/, KConfig* /*config*/){
    kdDebug() << "Running: PlugInProc::init(const QString &lang)" << endl;
    return false;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void PlugInProc::sayText(const QString& /*text*/){
    kdDebug() << "Running: PlugInProc::sayText(const QString &text)" << endl;
}

/**
* Synthesize text into an audio file, but do not send to the audio device.
* @param text                    The text to be synthesized.
* @param suggestedFilename       Full pathname of file to create.  The plugin
*                                may ignore this parameter and choose its own
*                                filename.  KTTSD will query the generated
*                                filename using getFilename().
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void PlugInProc::synthText(const QString& /*text*/, const QString& /*suggestedFilename*/) { };

/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString PlugInProc::getFilename() { return QString::null; };

/**
* Stop current operation (saying or synthesizing text).
* This function only makes sense in asynchronus modes.
* The plugin should return to the psIdle state.
*/
void PlugInProc::stopText(){
    kdDebug() << "Running: PlugInProc::stopText()" << endl;
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @ref pluginState
*/
pluginState PlugInProc::getState() { return psIdle; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void PlugInProc::ackFinished() { }
        
/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*/
bool PlugInProc::supportsAsync() { return false; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool PlugInProc::supportsSynth() { return false; }

