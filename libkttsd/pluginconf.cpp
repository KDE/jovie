/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginconf.cpp
  This file is the templates for the configuration plug ins.
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

// C++ library includes.
#include <stdlib.h>
#include <sys/param.h>

// Qt includes.
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>

// PluginConf includes.
#include "pluginconf.h"
#include "pluginconf.moc"

/**
* Constructor 
*/
PlugInConf::PlugInConf( QWidget *parent, const char *name) : QWidget(parent, name){
    kdDebug() << "PlugInConf::PlugInConf: Running" << endl;
    QString systemPath(getenv("PATH"));
    // kdDebug() << "Path is " << systemPath << endl;
    m_path = QStringList::split(":", systemPath);
    m_player = 0;
}

/**
* Destructor.
*/
PlugInConf::~PlugInConf(){
    kdDebug() << "PlugInConf::~PlugInConf: Running" << endl;
    delete m_player;
}

/**
* This method is invoked whenever the module should read its 
* configuration (most of the times from a config file) and update the 
* user interface. This happens when the user clicks the "Reset" button in 
* the control center, to undo all of his changes and restore the currently 
* valid settings.  Note that kttsmgr calls this when the plugin is
* loaded, so it not necessary to call it in your constructor.
* The plugin should read its configuration from the specified group
* in the specified config file.
* @param config      Pointer to a KConfig object.
* @param configGroup Call config->setGroup with this argument before
*                    loading your configuration.
*/
void PlugInConf::load(KConfig* /*config*/, const QString& /*configGroup*/){
    kdDebug() << "PlugInConf::load: Running" << endl;
}

/**
* This function gets called when the user wants to save the settings in 
* the user interface, updating the config files or wherever the 
* configuration is stored. The method is called when the user clicks "Apply" 
* or "Ok". The plugin should save its configuration in the specified
* group of the specified config file.
* @param config      Pointer to a KConfig object.
* @param configGroup Call config->setGroup with this argument before
*                    saving your configuration.
*/
void PlugInConf::save(KConfig* /*config*/, const QString& /*configGroup*/){
    kdDebug() << "PlugInConf::save: Running" << endl;
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void PlugInConf::defaults(){
    kdDebug() << "PlugInConf::defaults: Running" << endl;
}

/**
* Indicates whether the plugin supports multiple instances.  Return
* False if only one instance of the plugin can run at a time.
* @return            True if multiple instances are possible.
*
* It is assumed that most plugins can support multiple instances.
* A plugin must override this method and return false if it
* cannot support multiple instances.
*/
bool PlugInConf::supportsMultiInstance() { return true; }

/**
* This function informs the plugin of the desired language to be spoken
* by the plugin.  The plugin should attempt to adapt itself to the
* specified language code, choosing sensible defaults if necessary.
* If the passed-in code is QString::null, no specific language has
* been chosen.
* @param lang        The desired language code or Null if none.
*
* If the plugin is unable to support the desired language, that is OK.
*/
void PlugInConf::setDesiredLanguage(const QString& /*lang*/ ) { }

/**
* Return fully-specified talker code for the configured plugin.  This code
* uniquely identifies the configured instance of the plugin and distinquishes
* one instance from another.  If the plugin has not been fully configured,
* i.e., cannot yet synthesize, return QString::null.
* @return            Fully-specified talker code.
*/
QString PlugInConf::getTalkerCode() { return QString::null; }

/**
* Return a list of all the languages currently supported by the plugin.
* Note that as the user configures your plugin, the language choices may become
* narrower.  For example, once the user has picked a voice file, the language
* may be determined.  If your plugin has just been added and no configuration
* choices have yet been made, return a list of all possible languages the
* plugin might support.
* If your plugin cannot yet determine the languages supported, return Null.
* If your plugin can support any language, return Null.
* @return            A QStringList of supported language codes, or Null if unknown.
*/
QStringList PlugInConf::getSupportedLanguages() { return QStringList(); }

/**
* Return the full path to any program in the $PATH environmental variable
* @param name        The name of the file to search for.
* @returns           The path to the file on success, a blank QString
*                    if its not found.
*/
QString PlugInConf::getLocation(const QString &name) {
    // Iterate over the path and see if 'name' exists in it. Return the
    // full path to it if it does. Else return an empty QString.
    if (QFile::exists(name)) return name;
    kdDebug() << "PluginConf::getLocation: Searching for " << name << " in the path.." << endl;
    kdDebug() << m_path << endl;
    for(QStringList::iterator it = m_path.begin(); it != m_path.end(); ++it) {
        QString fullName = *it;
        fullName += "/";
        fullName += name;
        // The user either has the directory of the file in the path...
        if(QFile::exists(fullName)) {
            return fullName;
            kdDebug() << "PluginConf:getLocation: " << fullName << endl;
        }
        // ....Or the file itself
        else if(QFileInfo(*it).baseName().append(QString(".").append(QFileInfo(*it).extension())) == name) {
            return fullName;
            kdDebug() << "PluginConf:getLocation: " << fullName << endl;
        }
    }
    return "";
}

/**
* Breaks a language code into the language code and country code (if any).
* @param languageCode   Language code.
* @return countryCode   Just the country code part (if any).
* @return               Just the language code part.
*/
QString PlugInConf::splitLanguageCode(const QString& languageCode, QString& countryCode)
{
    QString locale = languageCode;
    QString langCode;
    QString charSet;
    KGlobal::locale()->splitLocale(locale, langCode, countryCode, charSet);
    return langCode;
}

QString PlugInConf::realFilePath(const QString &filename)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(filename).data(), realpath_buffer) != 0) {
        //succes, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }
    return filename;
}

/**
* Player object that can be used by the plugin for testing playback of synthed files.
*/
void PlugInConf::setPlayer(TestPlayer* player) { m_player = player; }
TestPlayer* PlugInConf::getPlayer() { return m_player; }
