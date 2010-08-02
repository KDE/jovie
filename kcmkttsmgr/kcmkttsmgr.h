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
#include <tqmap.h>
#include <tqlistview.h>

// KDE includes.
#include <kcmodule.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kparts/part.h>

// KTTS includes.
#include "addtalker.h"
#include "kcmkttsmgrwidget.h"
#include "kspeech_stub.h"
#include "kspeechsink.h"

class PlugInConf;
class KttsFilterConf;
class KListViewItem;
class KAboutData;
class KConfig;
class TQPopupMenu;

/**
* @author José Pablo Ezequiel "Pupeno" Fernández
* @author Gary Cramblitt
*/

class KCMKttsMgr :
    public KCModule,
    public KSpeech_stub,
    virtual public KSpeechSink
{
    Q_OBJECT

    public:
        KCMKttsMgr(TQWidget *parent, const char *name, const TQStringList &);

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
        TQString quickHelp() const;

        /**
        * Return the about information for this module
        */
        const KAboutData* aboutData() const;

    public slots:
        /**
        * This slot is used to emit the signal changed when any widget changes the configuration 
        */
        void configChanged()
        {
            if (!m_suppressConfigChanged)
            {
                // kdDebug() << "KCMKttsMgr::configChanged: Running"<< endl;
                m_changed = true;
                emit changed(true);
            }
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

        // virtual void resizeEvent( TQResizeEvent ev );

    private:
        enum widgetPages
        {
            wpGeneral = 0,          // General tab.
            wpTalkers = 1,          // Talkers tab.
            wpNotify = 2,           // Notify tab.
            wpFilters = 3,          // Filters tab.
            wpInterruption = 4,     // Interruption tab.
            wpAudio = 5,            // Audio tab.
            wpJobs = 6              // Jobs tab.
        };

        enum NotifyListViewColumn
        {
            nlvcEventSrcName = 0,
            nlvcEventName = 0,
            nlvcActionName = 1,
            nlvcTalkerName = 2,
            nlvcEventSrc = 3,      // hidden
            nlvcEvent = 4,         // hidden
            nlvcAction = 5,        // hidden
            nlvcTalker = 6         // hidden
        };

        enum TalkerListViewColumn
        {
            tlvcTalkerID,
            tlvcLanguage,
            tlvcSynthName,
            tlvcVoice,
            tlvcGender,
            tlvcVolume,
            tlvcRate
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
            slvcMultiInstance       // True if multiple instances of this plugin are possible. (hidden)
        };

        enum SbdButtonIDs
        {
            sbdBtnEdit = 1,
            sbdBtnUp = 2,
            sbdBtnDown = 3,
            sbdBtnAdd = 4,
            sbdBtnRemove = 5
        };

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
        TQString defaultTalkerCode(const TQString &languageCode, const TQString &plugInName);

        /**
        * Given an item in the talker listview and a talker code, sets the columns of the item.
        * @param talkerItem       TQListViewItem.
        * @param talkerCode       Talker Code.
        */
        void updateTalkerItem(TQListViewItem* talkerItem, const TQString &talkerCode);

        /**
        * Loads the configuration plugin for a named Talker plugin.
        * @param name             DesktopEntryName of the Synthesizer.
        * @return                 Pointer to the configuration plugin for the Talker.
        */
        PlugInConf* loadTalkerPlugin(const TQString& name);

        /**
        * Loads the configuration plugin for a named Filter plugin.
        * @param plugInName       DesktopEntryName of the plugin.
        * @return                 Pointer to the configuration plugin for the Filter.
        */
        KttsFilterConf* loadFilterPlugin(const TQString& plugInName);

        /**
        * Display the Talker Configuration Dialog.
        */
        void configureTalker();

        /**
        * Display the Filter Configuration Dialog.
        */
        void configureFilterItem( bool sbd );
        void configureFilter();

        /**
        * Add a filter.
        */
        void addFilter( bool sbd );

        /**
        * Remove a filter.
        */
        void removeFilter( bool sbd );

        /**
        * Move an item in a KListView up or down.
        */
        void lowerItemPriority( KListView* lView );
        void higherItemPriority( KListView* lView );

        /**
        * Count number of configured Filters with the specified plugin name.
        */
        int countFilterPlugins(const TQString& filterPlugInName);

        /**
         * Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
         * @param name                   The translated plugin name.  From Name= line in .desktop file.
         * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
         *                               TQString::null if not found.
         */
        TQString FilterNameToDesktopEntryName(const TQString& name);

        /**
         * Uses KTrader to convert a DesktopEntryName into a translated Filter Plugin Name.
         * @param desktopEntryName       The DesktopEntryName.
         * @return                       The translated Name of the plugin, from Name= line in .desktop file.
         */
        TQString FilterDesktopEntryNameToName(const TQString& desktopEntryName);

        /**
         * Loads notify events from a file.  Clearing listview if clear is True.
         */
        TQString loadNotifyEventsFromFile( const TQString& filename, bool clear);

        /**
         * Saves notify events to a file.
         */
        TQString saveNotifyEventsToFile(const TQString& filename);

        /**
         * Adds an item to the notify listview.
         * message is only needed if action = nactSpeakCustom.
         */
        TQListViewItem* addNotifyItem(
            const TQString& eventSrc,
            const TQString& event,
            int action,
            const TQString& message,
            TalkerCode& talkerCode);

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
        * Plugin configuration dialog.
        */
        KDialogBase* m_configDlg;

        /**
        * Sentence Boundary Detector button popup menu.
        */
        TQPopupMenu* m_sbdPopmenu;

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
        * True if the configuration has been changed.
        */
        bool m_changed;

        /**
        * When True, suppresses emission of changed() signal.  Used to suppress this
        * signal while loading configuration.
        */
        bool m_suppressConfigChanged;

        /**
        * Dictionary mapping language names to codes.
        */
        TQMap<TQString, TQString> m_languagesToCodes;

        /**
        * A TQMap of languages codes indexed by synthesizer that supports them.
        */
        SynthToLangMap m_synthToLangMap;

        /**
        * Default Talker Code for notifications.
        */
        TQString m_defaultNotifyTalkerCode;

    private slots:
        /**
        * Add a talker/filter.
        * This is a wrapper function that takes the parameters for the real talker from the
        * widgets to later call it.
        */
        void slot_addTalker();
        void slot_addNormalFilter();
        void slot_addSbdFilter();

        /**
        * Remove talker/filter.
        * This is a wrapper function that takes the parameters for the real removeTalker from the
        * widgets to later call it.
        */
        void slot_removeTalker();
        void slot_removeNormalFilter();
        void slot_removeSbdFilter();

        /**
        * This slot is called whenever user clicks the higher*Priority button (up).
        */
        void slot_higherTalkerPriority();
        void slot_higherNormalFilterPriority();
        void slot_higherSbdFilterPriority();

        /**
        * This slot is called whenever user clicks the lower*Priority button (down).
        */
        void slot_lowerTalkerPriority();
        void slot_lowerNormalFilterPriority();
        void slot_lowerSbdFilterPriority();

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
        * This signal is emitted whenever user checks/unchecks the ALSA radio button.
        */
        void slotAlsaRadioButton_toggled(bool state);

        /**
        * This is emitted whenever user activates the ALSA pcm combobox.
        */
        void slotPcmComboBox_activated();

        /**
        * This signal is emitted whenever user checks/unchecks the aKode radio button.
        */
        void slotAkodeRadioButton_toggled(bool state);

        /**
        * User has requested to display the Talker/Filter Configuration Dialog.
        */
        void slot_configureTalker();
        void slot_configureNormalFilter();
        void slot_configureSbdFilter();

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
        * Keep Audio CheckBox slot.
        */
        void keepAudioCheckBox_toggled(bool checked);

        /**
        * Notify tab slots.
        */
        void slotNotifyEnableCheckBox_toggled(bool checked);
        void slotNotifyAddButton_clicked();
        void slotNotifyRemoveButton_clicked();
        void slotNotifyClearButton_clicked();
        void slotNotifyLoadButton_clicked();
        void slotNotifySaveButton_clicked();
        void slotNotifyListView_selectionChanged();
        void slotNotifyPresentComboBox_activated(int index);
        void slotNotifyActionComboBox_activated(int index);
        void slotNotifyTestButton_clicked();
        void slotNotifyMsgLineEdit_textChanged(const TQString& text);
        void slotNotifyTalkerButton_clicked();

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
        KttsCheckListItem( TQListView *parent,
            const TQString &text, Type tt = RadioButtonController,
            KCMKttsMgr* kcmkttsmgr = 0);
        KttsCheckListItem( TQListView *parent, TQListViewItem *after,
            const TQString &text, Type tt = RadioButtonController,
            KCMKttsMgr* kcmkttsmgr = 0);

    protected:
        virtual void stateChange(bool);

    private:
        KCMKttsMgr* m_kcmkttsmgr;
};

#endif
