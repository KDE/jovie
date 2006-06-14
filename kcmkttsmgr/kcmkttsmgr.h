/***************************************************** vim:set ts=4 sw=4 sts=4:
  KControl module for KTTSD configuration and job management
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

/**
* @author José Pablo Ezequiel "Pupeno" Fernández
* @author Gary Cramblitt
*/

#ifndef KCMKTTSMGR_H
#define KCMKTTSMGR_H

#include "config.h"

// Qt includes.
#include <QMap>
#include <QAbstractListModel>
#include <QModelIndex>

// KDE includes.
#include <kcmodule.h>
#include <kdebug.h>
#include <kparts/part.h>

// KTTS includes.
#include "talkercode.h"
#include "talkerlistmodel.h"
#include "addtalker.h"
#include "ui_kcmkttsmgrwidget.h"
#include "kspeechinterface.h"

class PlugInConf;
class KttsFilterConf;
class QListWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class KAboutData;
class KConfig;
class QAction;

class FilterItem
{
public:
    QString id;
    QString userFilterName;
    QString plugInName;
    QString desktopEntryName;
    bool enabled;
    bool multiInstance;
};

typedef QList<FilterItem> FilterList;

class FilterListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    FilterListModel(FilterList filters = FilterList(), QObject *parent = 0);

    FilterList datastore() { return m_filters; }
    void setDatastore(FilterList filters = FilterList()) { m_filters = filters; }
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index ) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRow(int row, const QModelIndex & parent = QModelIndex());

    FilterItem getRow(int row) const;
    bool appendRow(FilterItem& filter);
    bool updateRow(int row, FilterItem& filter);
    bool swap(int i, int j);
    void clear();
protected:
    FilterList m_filters;
};

class SbdFilterListModel : public FilterListModel
{
    Q_OBJECT
public:
    SbdFilterListModel(FilterList filters = FilterList(), QObject *parent = 0);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
};

class KCMKttsMgr :
    public KCModule,
    private Ui::KCMKttsMgrWidget
{
    Q_OBJECT

    public:
        KCMKttsMgr(QWidget *parent, const QStringList &);

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
        void configChanged()
        {
            if (!m_suppressConfigChanged)
            {
                // kDebug() << "KCMKttsMgr::configChanged: Running"<< endl;
                m_changed = true;
                emit changed(true);
            }
        };

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
        * Loads the configuration plugin for a named Talker plugin.
        * @param name             DesktopEntryName of the Synthesizer.
        * @return                 Pointer to the configuration plugin for the Talker.
        */
        PlugInConf* loadTalkerPlugin(const QString& name);

        /**
        * Loads the configuration plugin for a named Filter plugin.
        * @param plugInName       DesktopEntryName of the plugin.
        * @return                 Pointer to the configuration plugin for the Filter.
        */
        KttsFilterConf* loadFilterPlugin(const QString& plugInName);

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
        * Count number of configured Filters with the specified plugin name.
        */
        int countFilterPlugins(const QString& filterPlugInName);

        /**
         * Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
         * @param name                   The translated plugin name.  From Name= line in .desktop file.
         * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
         *                               QString() if not found.
         */
        QString FilterNameToDesktopEntryName(const QString& name);

        /**
         * Uses KTrader to convert a DesktopEntryName into a translated Filter Plugin Name.
         * @param desktopEntryName       The DesktopEntryName.
         * @return                       The translated Name of the plugin, from Name= line in .desktop file.
         */
        QString FilterDesktopEntryNameToName(const QString& desktopEntryName);

        /**
         * Loads notify events from a file.  Clearing listview if clear is True.
         */
        QString loadNotifyEventsFromFile( const QString& filename, bool clear);

        /**
         * Saves notify events to a file.
         */
        QString saveNotifyEventsToFile(const QString& filename);

        /**
         * Adds an item to the notify treeview.
         * message is only needed if action = nactSpeakCustom.
         */
        QTreeWidgetItem* addNotifyItem(
            const QString& eventSrc,
            const QString& event,
            int action,
            const QString& message,
            TalkerCode& talkerCode);

        /**
        * A convenience method that finds an item in a TreeViewWidget, assuming there is at most
        * one occurrence of the item.  Returns 0 if not found.
        * @param tw                     The TreeViewWidget to search.
        * @param sought                 The string sought.
        * @param col                    Column of the TreeViewWidget to search.
        * @return                       The item of the TreeViewWidget found or null if not found.
        *
        * An exact match is performed.
        */
        QTreeWidgetItem* findTreeWidgetItem(QTreeWidget* tw, const QString& sought, int col);
        
        /**
        * DBUS KSpeech Interface.
        */
        org::kde::KSpeech* m_kspeech;

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
        KDialog* m_configDlg;

        /**
        * Sentence Boundary Detector button popup menu actions.
        */
        QAction* m_sbdBtnEdit;
        QAction* m_sbdBtnUp;
        QAction* m_sbdBtnDown;
        QAction* m_sbdBtnAdd;
        QAction* m_sbdBtnRemove;

        /**
        * Talker(synth) Plugin currently loaded into configuration dialog.
        */
        PlugInConf *m_loadedTalkerPlugIn;

        /**
        * Filter Plugin currently loaded into configuration dialog.
        */
        KttsFilterConf *m_loadedFilterPlugIn;

        /**
        * Model containing list of Talker Codes.
        */
        TalkerListModel m_talkerListModel;

        /**
        * Last talker ID.  Used to generate a new ID.
        */
        int m_lastTalkerID;

        /**
        * Models containing normal and SBD filters.
        */
        FilterListModel m_filterListModel;
        SbdFilterListModel m_sbdFilterListModel;

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
        QMap<QString, QString> m_languagesToCodes;

        /**
        * A QMap of languages codes indexed by synthesizer that supports them.
        */
        SynthToLangMap m_synthToLangMap;

        /**
        * Default Talker Code for notifications.
        */
        QString m_defaultNotifyTalkerCode;

    private slots:
        /**
        * Update the status of the Talker/Filter buttons.
        */
        void updateTalkerButtons();
        void updateFilterButtons();
        void updateSbdButtons();

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
        * General tab slots.
        */
        void slotEnableKttsd_toggled(bool checked);
        void slotAutoStartMgrCheckBox_toggled(bool checked);

        /**
        * Talker tab slots.
        */
        void slotAddTalkerButton_clicked();
        void slotRemoveTalkerButton_clicked();
        void slotHigherTalkerPriorityButton_clicked();
        void slotLowerTalkerPriorityButton_clicked();
        void slotConfigureTalkerButton_clicked();

        /**
        * Notify tab slots.
        */
        void slotNotifyEnableCheckBox_toggled(bool checked);
        void slotNotifyAddButton_clicked();
        void slotNotifyRemoveButton_clicked();
        void slotNotifyClearButton_clicked();
        void slotNotifyLoadButton_clicked();
        void slotNotifySaveButton_clicked();
        void slotNotifyListView_currentItemChanged();
        void slotNotifyPresentComboBox_activated(int index);
        void slotNotifyActionComboBox_activated(int index);
        void slotNotifyTestButton_clicked();
        void slotNotifyMsgLineEdit_textChanged(const QString& text);
        void slotNotifyTalkerButton_clicked();

        /**
        * Filters tab slots.
        */
        void slotFilterListView_clicked(const QModelIndex & index);
        void slotAddNormalFilterButton_clicked();
        void slotAddSbdFilterButton_clicked();
        void slotRemoveNormalFilterButton_clicked();
        void slotRemoveSbdFilterButton_clicked();
        void slotHigherNormalFilterPriorityButton_clicked();
        void slotHigherSbdFilterPriorityButton_clicked();
        void slotLowerNormalFilterPriorityButton_clicked();
        void slotLowerSbdFilterPriorityButton_clicked();
        void slotConfigureNormalFilterButton_clicked();
        void slotConfigureSbdFilterButton_clicked();

        /**
        * Interruption tab slots.
        */
        void slotTextPreMsgCheck_toggled(bool checked);
        void slotTextPreSndCheck_toggled(bool checked);
        void slotTextPostMsgCheck_toggled(bool checked);
        void slotTextPostSndCheck_toggled(bool checked);

        /**
        * Audio tab slots.
        */
        void timeBox_valueChanged(int percentValue);
        void timeSlider_valueChanged(int sliderValue);
        void keepAudioCheckBox_toggled(bool checked);
        void slotPhononRadioButton_toggled(bool state);
        void slotAlsaRadioButton_toggled(bool state);
        void slotPcmComboBox_activated();

        /**
        * Other slots.
        */
        void slotTabChanged();
};

#endif
