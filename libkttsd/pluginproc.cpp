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

// Qt includes. 
#include <qstring.h>

// KDE includes.
#include <kdebug.h>
#include <kstandarddirs.h>

// PlugInProc includes.
#include "pluginproc.h"
#include "pluginproc.moc"

/**
* Constructor
*/
PlugInProc::PlugInProc( QObject *parent, const char *name) : QObject(parent, name){
    // kdDebug() << "PlugInProc::PlugInProc: Running" << endl;
}

/**
* Destructor
*/
PlugInProc::~PlugInProc(){
    // kdDebug() << "PlugInProc::~PlugInProc: Running" << endl;
}

/**
* Initializate the speech plugin.
*/
bool PlugInProc::init(KConfig* /*config*/, const QString& /*configGroup*/){
    // kdDebug() << "PlugInProc::init: Running" << endl;
    return false;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void PlugInProc::sayText(const QString& /*text*/){
    // kdDebug() << "PlugInProc::sayText: Running" << endl;
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
    // kdDebug() << "PlugInProc::stopText: Running" << endl;
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

/**
* Returns the name of an XSLT stylesheet that will convert a valid SSML file
* into a format that can be processed by the synth.  For example,
* The Festival plugin returns a stylesheet that will convert SSML into
* SABLE.  Any tags the synth cannot handle should be stripped (leaving
* their text contents though).  The default stylesheet strips all
* tags and converts the file to plain text.
* @return            Name of the XSLT file.
*/
QString PlugInProc::getSsmlXsltFilename()
{
    return KGlobal::dirs()->resourceDirs("data").last() + "kttsd/xslt/SSMLtoPlainText.xsl";
}
