/***************************************************** vim:set ts=4 sw=4 sts=4:
  kcmkttsmgr.h
  KControl module for KTTSD configuration and job management
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$

#ifndef KCMKTTSMGR_H
#define KCMKTTSMGR_H

#include <kcmodule.h>
#include <qdict.h>
#include <qmap.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kparts/part.h>

#include "kcmkttsmgrwidget.h"
#include "kspeechsink.h"

class PlugInConf;
class KListViewItem;
class KAboutData;
class KConfig;
class KAboutApplication;

/**
* This strcture is used to keep track of the objects asociated with a talker.
* From the widget of the plug in to the object in the list
*/
struct languageRelatedObjects {
   PlugInConf *plugIn;
   QString plugInName;
   QString plugInLib;
   KListViewItem *listItem;
};

/**
* @author José Pablo Ezequiel "Pupeno" Fernández
* @author Gary Cramblitt
*/
class KCMKttsMgr :
    public KCModule,
    virtual public KSpeechSink
{
    Q_OBJECT

    public:
        KCMKttsMgr(QWidget *parent, const char *name, const QStringList &);

        ~KCMKttsMgr();
      
        /**
        * This method is invoked whenever the module should read its 
        * configuration (most of the times from a config file) and update the 
        * user interface. This happens when the user clicks the "Reset" button in 
        * the control center, to undo all of his changes and restore the currently 
        * valid settings. NOTE that this is not called after the modules is loaded,
        * so you probably want to call this method in the constructor.
        */
        void load();
    
        /**
        * This function gets called when the user wants to save the settings in 
        * the user interface, updating the config files or wherever the 
        * configuration is stored. The method is called when the user clicks "Apply" 
        * or "Ok".
        */
        void save();
    
        /**
        * This function is called to set the settings in the module to sensible
        * default values. It gets called when hitting the "Default" button. The 
        * default values should probably be the same as the ones the application 
        * uses when started without a config file.
        */
        void defaults();
    
        /**
        * This is a static method which gets called to realize the modules settings
        * durign the startup of KDE. NOTE that most modules do not implement this 
        * method, but modules like the keyboard and mouse modules, which directly 
        * interact with the X-server, need this method. As this method is static, 
        * it can avoid to create an instance of the user interface, which is often 
        * not needed in this case.
        */
        static void init();
    
        /**
        * The control center calls this function to decide which buttons should
        * be displayed. For example, it does not make sense to display an "Apply" 
        * button for one of the information modules. The value returned can be set by 
        * modules using setButtons.
        */
        int buttons();
    
        /**
        * This function returns the small quickhelp.
        * That is displayed in the sidebar in the KControl
        */
        QString quickHelp() const;
        
        /**
        * Return the about information for this module
        */
        const KAboutData* aboutData() const;
        
    protected:
        /** DCOP Methods connected to DCOP Signals emitted by KTTSD. */
        /** Most of these are not used */
        
        /**
        * This signal is emitted when KTTSD starts or restarts after a call to reinit.
        */
        virtual void kttsdStarted();
        /**
        * This signal is emitted just before KTTSD exits.
        */
        virtual void kttsdExiting();
    
    private:
        /**
        * Add a language to the pool of languages for the lang/plugIn pair
        */
        void addLanguage(const QString &language, const QString &plugInName);
    
        /**
        * Initializes the language with the given language code by determining
        * its name.
        */
        void initLanguageCode(const QString &code);
    
        /**
        * Remove a language named lang
        */
        void removeLanguage(const QString &language);
    
        /**
        * Set the default language
        */
        void setDefaultLanguage(const QString &defaultLanguage);
    
        /**
        * Loads the configuration plug in for a specific plug in
        */
        PlugInConf *loadPlugIn(const QString &plugInName);
        
        /**
        * Main widget
        */
        KCMKttsMgrWidget *m_kttsmgrw;
        
        /**
        * Current Plugin widget.
        */
        PlugInConf* m_pluginWidget;
        
        /**
        * List of the objects related to each loaded language
        */
        QDict<languageRelatedObjects> m_loadedLanguages;
    
        /**
        * List of languages codes maped to language names
        */
        QMap<QString, QString> m_languagesMap;
    
        /**
        * List of languages names maped to language codes
        */
        QMap<QString, QString> m_reverseLanguagesMap;
    
        /**
        * List of available plug ins
        */
        KTrader::OfferList m_offers;
    
        /**
        * Object holding all the configuration
        */
        KConfig *m_config;
        
        /**
        * KTTS Job Manager.
        */
        KParts::ReadOnlyPart *m_jobMgrPart;
    
        /**
        * About dialog.
        */
        KAboutApplication *m_aboutDlg;
    
    private slots:
        /**
        * Add a language
        * This is a wrapper function that takes the parameters for the real addLanguage from the
        * widgets to later call it.
        */
        void addLanguage();
    
        /**
        * Remove language 
        * This is a wrapper function that takes the parameters for the real removeLanguage from the
        * widgets to later call it.
        */
        void removeLanguage();
    
        /**
        * Set default langauge
        * This is a wrapper function that takes the parameters for the real setDefaultLanguage from the
        * widgets to later call it.
        */
        void setDefaultLanguage();
    
        /**
        * Update the status of the Remove button
        */
        void updateRemoveButton();
    
        /**
        * Update the status of the Default button
        */
        void updateDefaultButton();
        
        /**
        * This slot is used to emit the signal changed when any widget changes the configuration 
        */
        void configChanged(){
            kdDebug() << "Running: KCMKttsMgr::configChanged()"<< endl;
            emit changed(true);
        };
        
        /**
        * This signal is emitted whenever user checks/unchecks the Enable TTS System check box.
        */
        void enableKttsdToggled(bool checked);

        /**
        * Displays about dialog.
        */
        void aboutSelected();

};

#endif
