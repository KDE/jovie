/***************************************************** vim:set ts=4 sw=4 sts=4:
  KControl module for KTTSD configuration and job management
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: 2004 by Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef KCMKTTSMGR_H
#define KCMKTTSMGR_H

#include "config.h"

// Qt includes.
#include <qmap.h>
#include <qlistview.h>

// KDE includes.
#include <kcmodule.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kparts/part.h>

// KTTS includes.
#include "addtalker.h"
#include "kcmkttsmgrwidget.h"
#include "kspeechsink.h"

class PlugInConf;
class KttsFilterConf;
class KListViewItem;
class KAboutData;
class KConfig;
class KAboutApplication;

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

    public slots:
        /**
        * This slot is used to emit the signal changed when any widget changes the configuration 
        */
        void configChanged(){
            // kdDebug() << "KCMKttsMgr::configChanged: Running"<< endl;
            emit changed(true);
        };
        /**
        * This slot is called whenever user checks/unchecks item in Filters list.
        */
        void slotFiltersList_stateChanged();

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

        // virtual void resizeEvent( QResizeEvent ev );

    private:
        enum widgetPages
        {
            wpGeneral = 0,          // General tab.
            wpFilters = 1,          // Filters tab.
            wpTalkers = 2,          // Talkers tab.
            wpInterruption = 3,     // Interruption tab.
            wpAudio = 4,            // Audio tab.
            wpJobs = 5              // Jobs tab.
        };

        enum TalkerListViewColumn
        {
            tlvcTalkerID,
            tlvcLanguage,
            tlvcSynthName,
            tlvcVoice,
            tlvcGender,
            tlvcVolume,
            tlvcRate,
        };

        enum FilterListViewColumn
        {
            flvcUserName,           // Name of filter as set by user and displayed.
            flvcFilterID,           // Internal ID assigned to the filter (hidden).
            flvcPlugInName,         // Name of the filter plugin (from .desktop file, hidden).
            flvcMultiInstance       // True if multiple instances of this plugin are possible. (hidden)
        };

        enum SbdListViewColumn
        {
            slvcUserName,           // Name of filter as set by user and displayed.
            slvcFilterID,           // Internal ID assigned to the filter (hidden).
            slvcPlugInName,         // Name of the filter plugin (from .desktop file, hidden).
        };

        /**
        * These functions return translated Talker Code attributes.
        */
        QString translatedGender(const QString &gender);
        QString translatedVolume(const QString &volume);
        QString translatedRate(const QString &rate);

        /**
        * Conversion functions for percent boxes to/from sliders.
        */
        int percentToSlider(int percentValue);
        int sliderToPercent(int sliderValue);

        /**
        * Given a language code and plugin name, returns a normalized default talker code.
        * @param languageCode     Language code.
        * @param plugInName       Name of the plugin.
        * @return                 Full normalized talker code.
        *
        * Example returned from defaultTalkerCode("en", "Festival")
        *   <voice lang="en" name="fixed" gender="neutral"/>
        *   <prosody volume="medium" rate="medium"/>
        *   <kttsd synthesizer="Festival" />
        */
        QString defaultTalkerCode(const QString &languageCode, const QString &plugInName);

        /**
        * Given an item in the talker listview and a talker code, sets the columns of the item.
        * @param talkerItem       QListViewItem.
        * @param talkerCode       Talker Code.
        */
        void updateTalkerItem(QListViewItem* talkerItem, const QString &talkerCode);

        /**
        * Loads the configuration plugin for a named Talker plugin.
        * @param name             Name of the Synthesizer.  This is a translated name, and not
        *                         necessarily the same as the plugIn name.
        *                         Example, "Festival Interactivo".
        */
        PlugInConf* loadTalkerPlugin(const QString& name);

        /**
        * Loads the configuration plugin for a named Filter plugin.
        * @param plugInName       Name of the plugin.
        */
        KttsFilterConf* loadFilterPlugin(const QString& plugInName);

        /**
        * Display the Talker Configuration Dialog.
        */
        void configureTalker();

        /**
        * Display the Filter Configuration Dialog.
        */
        void configureFilter();

        /**
        * Count number of configured Filters with the specified plugin name.
        */
        int countFilterPlugins(const QString& filterPlugInName);

        /**
        * Main widget
        */
        KCMKttsMgrWidget *m_kttsmgrw;

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

        /**
        * Plugin configuration dialog.
        */
        KDialogBase* m_configDlg;

        /**
        * Talker(synth) Plugin currently loaded into configuration dialog.
        */
        PlugInConf *m_loadedTalkerPlugIn;

        /**
        * Filter Plugin currently loaded into configuration dialog.
        */
        KttsFilterConf *m_loadedFilterPlugIn;

        /**
        * Last talker ID.  Used to generate a new ID.
        */
        int m_lastTalkerID;

        /**
        * Last filter ID.  Used to generate a new ID.
        */
        int m_lastFilterID;

        /**
        * Last SBD filter ID.  Used to generate to new ID.
         */
        int m_lastSbdID;

        /**
        * Dictionary mapping language names to codes.
        */
        QMap<QString, QString> m_languagesToCodes;

        /**
        * A QMap of languages codes indexed by synthesizer that supports them.
        */
        SynthToLangMap m_synthToLangMap;

    private slots:
        /**
        * Add a talker/filter.
        * This is a wrapper function that takes the parameters for the real talker from the
        * widgets to later call it.
        */
        void addTalker();
        void addFilter();

        /**
        * Remove talker/filter.
        * This is a wrapper function that takes the parameters for the real removeTalker from the
        * widgets to later call it.
        */
        void removeTalker();
        void removeFilter();

        /**
        * This slot is called whenever user clicks the higher*Priority button (up).
        */
        void higherTalkerPriority();
        void higherFilterPriority();
        void higherSbdPriority();

        /**
        * This slot is called whenever user clicks the lower*Priority button (down).
        */
        void lowerTalkerPriority();
        void lowerFilterPriority();
        void lowerSbdPriority();

        /**
        * Update the status of the Talker/Filter buttons.
        */
        void updateTalkerButtons();
        void updateFilterButtons();
        void updateSbdButtons();

        /**
        * This signal is emitted whenever user checks/unchecks the Enable TTS System check box.
        */
        void enableKttsdToggled(bool checked);

        /**
        * This signal is emitted whenever user checks/unchecks the GStreamer radio button.
        */
        void slotGstreamerRadioButton_toggled(bool state);

        /**
        * User has requested to display the Talker/Filter Configuration Dialog.
        */
        void slot_configureTalker();
        void slot_configureFilter();
        void slot_configureSbd();

        /**
        * Displays about dialog.
        */
        void aboutSelected();

        /**
        * Slots for the Talker/Filter Configuration dialogs.
        */
        void slotConfigTalkerDlg_ConfigChanged();
        void slotConfigFilterDlg_ConfigChanged();
        void slotConfigTalkerDlg_DefaultClicked();
        void slotConfigFilterDlg_DefaultClicked();
        void slotConfigTalkerDlg_CancelClicked();
        void slotConfigFilterDlg_CancelClicked();

        /**
        * Slots for Speed setting.
        */
        void timeBox_valueChanged(int percentValue);
        void timeSlider_valueChanged(int sliderValue);

        /**
        * Other slots.
        */
        void slotTabChanged();
};

/// This is a small helper class to detect when user checks/unchecks a Filter in Filters tab
/// and emit changed() signal.
class KttsCheckListItem : public QCheckListItem
{
    public:
        KttsCheckListItem( QListView *parent,
            const QString &text, Type tt = RadioButtonController,
            KCMKttsMgr* kcmkttsmgr = 0);
        KttsCheckListItem( QListView *parent, QListViewItem *after,
            const QString &text, Type tt = RadioButtonController,
            KCMKttsMgr* kcmkttsmgr = 0);

    protected:
        virtual void stateChange(bool);

    private:
        KCMKttsMgr* m_kcmkttsmgr;
};

#endif
