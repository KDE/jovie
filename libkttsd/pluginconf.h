/***************************************************** vim:set ts=4 sw=4 sts=4:
  This file is the template for the configuration plug ins.
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

#ifndef _PLUGINCONF_H_
#define _PLUGINCONF_H_

// Qt includes.
#include <QWidget>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>
#include <kdemacros.h>

// KTTS includes.
#include "testplayer.h"

/**
* @interface PlugInConf
*
* pluginconf - the KDE Text-to-Speech Deamon Plugin Configuration API.
*
* @version 1.0 Draft 2
*
* This class defines the interface that plugins to KTTSMGR must implement.
*
* @warning The pluginconf interface is still being developed and is likely
* to change in the future.
*
* A KTTSD Plugin interfaces between KTTSD and a speech engine.  
* A PlugInConf provides an on-screen widget for configuring the plugin for use
* with KTTSD.
*
* @section guidelines General Guidelines
*
* - The configuration widget should be no larger than TODO pixels.
* - Do not supply Load, Save, Cancel, or OK buttons.  Those are provided by KTTSMGR.
* - Try to supply a Test button so that users can test the configuration before
*   saving it.
* - Your configuration widget will be running inside a KPart.
* - Whenever the user changes something in your on-screen widget, emit the
*   @ref changed signal.
* - If a plugin can automatically configure itself, i.e., locate voice files,
*   set default values, etc., it should do so when it is first added to KTTSMGR.
*
* @section multiinstance Multiple Instances
*
* If it is possible to run multiple simultaneous instances of your synthesis engine,
* return True from the @ref supportsMultiInstance method.  The user will be able to
* configure multiple instances of your plugin, each with a different set of
* talker attributes.
*
* If you cannot run multiple simultaneous instances of your synthesis engine,
* or your plugin has a fixed set of talker attributes (only one language, voice,
* gender, volume, and speed), return False from @ref supportsMultiInstance.
* 
* @section language Language Support
*
* Some plugins support only one language.  For them, return the appropriate language
* code when @ref getSupportedLanguages is called.
*
* If your plugin can support multiple languages, your task is a little more
* complicated.  The best way to handle this is to install a @e voices file with
* your plugin that lists all the supported languages, voice files, genders, etc.
* that are possible.  When your plugin is added to KTTSMGR,
* @ref getSupportedLanguages will be called.  Return a list of all the possible
* languages supported, even if the user hasn't yet picked a voice file in your
* configuration, or even told your where the voice files are.
*
* There are three ways that users and applications pick a language code for your
* plugin:
* - The user picks a code from among the languages you returned in
*   @ref getSupportedLanguages, or
* - The user picks your plugin and uses your configuration widget to pick a voice
*   file or other configuration option that determines the language, or
* - An application requests a plugin with specific language support.
*
* If possible, avoid making the user pick a language code in your plugin.
*
* In the first and third cases, the chosen language code will be passed to your
* plugin when @ref setDesiredLanguage is called.  If you can satisfy this
* language code, good, but it is possible that once the user has begun
* configuring your plugin, you find that you cannot support the desired
* language.  Perhaps a needed voice file is missing.  That is OK.
* You'll inform KTTSMGR of the actual language code when KTTSMGR
* calls @ref getTalkerCode (see below).  Note that you should not limit the
* users choices based on the @ref setDesiredLanguage.  A user might start
* out configuring your plugin for one language, and then change his or her
* mind to a different language.
*
* Also note that language codes may also include an appended country code.
* For example, "en_GB" for British English.  When @ref getSupportedLanguages is
* called, you should return as specific a list as possible.  For example,
* if your plugin supports both American and British English, your returned
* list would include "en_GB" and "en_US".  When @ref setDesiredLanguage is
* called, a country code may or may not be included.  If included and your
* plugin supports the language, but not the specific country variant,
* your plugin should nevertheless attempt to satisfy the request, returning
* the actual supported language and country when @ref getTalkerCode is called.
*
* @section talkercodes Talker Codes
*
* Review the section on Talkers in kspeech.h.
*
* When your plugin is added to the KTTSMGR, @ref getSupportedLanguages
* will be called followed by @ref setDesiredLanguage and @ref load.
* Note that the configuration file will most likely be empty when
* @ref load is called.

* Next, @ref getTalkerCode
* will be called.  If your plugin can automatically configure itself to the desired
* language, it should do so and return a fully-specified talker code.  If your
* plugin is not yet ready and requires user help, return QString(). Note that
* @ref setDesiredLanguage may be Null, in which case, you should allow the
* user to configure your plugin to any of your supported languages.
*
* When your plugin has been configured enough to begin synthesis, return a
* fully-specified talker code in @ref getTalkerCode().
*
* Here is guidance for what you should return for each of the talker attributes
* when @ref getTalkerCode is called:
*
* - @e lang.         If user has completed configuring your plugin, i.e., it is
*                    ready to begin synthesizing, return the ISO 639-1 language code
*                    for the language it can synthesize.  If your plugin is not yet 
*                    fully configured, you should return QString() for the entire
*                    talker code.  If your plugin supports a specific national version
*                    of a language, that should also be included using the ISO 3166
*                    country code separated from the language code by underscore (_).
*                    For example, if your plugin synthesizes American English, you would
*                    return "en_US".  If British English, "en_BR".  And if
*                    non-specific English, just "en".
* - @e synthesizer.  The name of your plugin.  Keep short, but long enough to
*                    distinquish different implementations.  For example,
*                    Festival Int, Flite, Hadifix.  Use only letters, numbers
*                    spaces, and underscores (_) in your plugin name.
* - @e gender.       May be "male", "female", or "neutral".
* - @e name.         The voice code.  If your plugin does not support voices,
*                    return "fixed".
* - @e volume.       May be "medium", "loud", or "soft".  If your plugin does not support
*                    configurable volume, return "medium".
* - @e rate.         May be "medium", "fast", or "slow".  If your plugin does not support
*                    configurable speed, return "medium".
*
* The order of the attributes you return does not matter.  Here is an example of
* a fully-specified talker code.
*
*   lang="en" name="Kal" gender="male" volume="soft" rate="fast"
*   synthesizer="Festival Interactive"
*
* Do not return translated values for the Talker Code attributes.  All English.
*
* Each time your plugin emits the @ref changed signal, @ref getTalkerCode will be called.
* The configuration dialog OK button will be disabled until you return a non-null
* Talker Code.
*
* It is possible that your plugin does not know the language supported.  The generic
* Command plugin is example of such a case, since the user enters an arbitrary command.
* In this case, return the value from the @ref setDesiredLanguage call.  It is possible
* that @ref setDesiredLanguage is Null.  That is OK.  In this case, KTTSMGR will prompt
* the user for the language code.
*
* @section loadandsavemethods Load and Save Methods
*
* The @ref load and @ref save methods are called by KTTSMGR so that your plugin
* can load and save configuration options from the configuration file.
* These methods have two parameters, a @e config object and a @e configGroup string.
*
* Plugins that do not support multiple instances (return False from 
* @ref supportsMultiInstance), should simply call config->setGroup(configGroup)
* before loading or saving their configuration.
*
* If your plugin supports multiple instances, it is slightly more complicated.
* Typically, there will be configuration options that apply to all instances
* of the plugin and there will be options that apply only to the specific
* configured instance of the plugin.  To load or save the instance-specific
* options, call config->setGroup(configGroup).  For options that apply
* to all instances of the plugin, call config->setGroup() with a group name
* that contains your plugin's name.  For example,
* config->setGroup("Festival Defaults").
*
* For example, when first added to KTTSMGR, the Festival plugin needs to know the path to
* the directory containing all the installed voice files.  It is best for a plugin
* to try to locate these resources automatically, but if it can't find them,
* when the user has told it where they are, it is a good idea to save this information
* in the all-instances group.  In this way, the next time the plugin
* is added to KTTSMGR, or another instance is added, it will be able to find them
* automatically.
*
* @ref setDesiredLanguage is always called just prior to @ref load, therefore
* it is not necessary to save the language code, unless your plugin needs it in
* order to synthesize speech.
*/

class KDE_EXPORT PlugInConf : public QWidget{
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
        * valid settings.  Note that KTTSMGR calls this when the plugin is
        * loaded, so it not necessary to call it in your constructor.
        * The plugin should read its configuration from the specified group
        * in the specified config file.
        * @param config      Pointer to a KConfig object.
        * @param configGroup Call config->setGroup with this argument before
        *                    loading your configuration.
        *
        * When a plugin is first added to KTTSMGR, @e load will be called with
        * a Null @e configGroup.  In this case, the plugin will not have
        * any instance-specific parameters to load, but it may still wish
        * to load parameters that apply to all instances of the plugin.
        * 
        * @see loadandsavemethods
        */
        virtual void load(KConfig *config, const QString &configGroup);

        /**
        * This function gets called when the user wants to save the settings in 
        * the user interface, updating the config files or wherever the 
        * configuration is stored. The method is called when the user clicks "Apply" 
        * or "Ok". The plugin should save its configuration in the specified
        * group of the specified config file.
        * @param config      Pointer to a KConfig object.
        * @param configGroup Call config->setGroup with this argument before
        *                    saving your configuration.
        *
        * @ref setDesiredLanguage is always called just prior to @ref load, therefore
        * it is not necessary to save the language code, unless your plugin needs it in
        * order to synthesize speech.
        */
        virtual void save(KConfig *config, const QString &configGroup);

        /** 
        * This function is called to set the settings in the module to sensible
        * default values. It gets called when hitting the "Default" button. The 
        * default values should probably be the same as the ones the application 
        * uses when started without a config file.  Note that defaults should
        * be applied to the on-screen widgets; not to the config file.
        */
        virtual void defaults();
      
        /**
        * Indicates whether the plugin supports multiple instances.  Return
        * False if only one instance of the plugin can run at a time, or
        * if your plugin is limited to a single language, voice, gender, volume,
        * and speed.
        * @return            True if multiple instances are possible.
        */
        virtual bool supportsMultiInstance();

        /**
        * This function informs the plugin of the desired language to be spoken
        * by the plugin.  The plugin should attempt to adapt itself to the
        * specified language code, choosing sensible defaults if necessary.
        * If the passed-in code is QString(), no specific language has
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
        virtual void setDesiredLanguage(const QString &lang);

        /**
        * Return fully-specified talker code for the configured plugin.  This code
        * uniquely identifies the configured instance of the plugin and distinquishes
        * one instance from another.  If the plugin has not been fully configured,
        * i.e., cannot yet synthesize, return QString().
        * @return            Fully-specified talker code.
        */
        virtual QString getTalkerCode();

        /**
        * Return a list of all the languages possibly supported by the plugin.
        * If your plugin can support any language, return Null.
        * @return            A QStringList of supported language and optional country
        *                    codes, or Null if any.
        *
        * The languge codes are given in ISO 639-1.  Lowercase should be used.
        * If your plugin supports various national forms of a language, ISO 3166
        * country codes should also be include in upperase and separated from
        * the language code with underscore (_).  Examples:
        *   en
        *   en_US
        *   en_GB
        *   es
        *   es_CL
        * The list you return should be as specific as practicable.
        */
        virtual QStringList getSupportedLanguages();

        /**
        * Player object that can be used by the plugin for testing playback of synthed files.
        */
        void setPlayer(TestPlayer* player);
        TestPlayer* getPlayer();

        static QString realFilePath(const QString &filename);

        static QString testMessage(const QString& languageCode);

    public slots:
        /**
        * This slot is used internally when the configuration is changed.  It is
        * typically connected to signals from the widgets of the configuration
        * and should emit the @ref changed signal.
        */
        void configChanged(){
            kDebug() << "PlugInConf::configChanged: Running"<< endl;
            emit changed(true);
        };

    signals:
        /**
        * This signal indicates that the configuration has been changed.
        * It should be emitted whenever user changes something in the configuration widget.
        */
        void changed(bool);

    protected:
        /**
        * Searches the $PATH variable for any file. If that file exists in the PATH, or
        * is contained in any directory in the PATH, it returns the full path to it.
        * @param name        The name of the file to search for.
        * @returns           The path to the file on success, a blank QString
        *                    if its not found.
        */
        QString getLocation(const QString &name);

        /**
        * Breaks a language code into the language code and country code (if any).
        * @param languageCode   Language code.
        * @return countryCode   Just the country code part (if any).
        * @return               Just the language code part.
        */
        QString splitLanguageCode(const QString& languageCode, QString& countryCode);

        /// The system path in a QStringList.
        QStringList m_path;

        TestPlayer* m_player;
};

#endif  //_PLUGINCONF_H_
