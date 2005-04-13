/***************************************************** vim:set ts=4 sw=4 sts=4:
  KControl module for KTTSD configuration and Job Management
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

// Note to programmers.  There is a subtle difference between a plugIn name and a 
// synthesizer name.  The latter is a translated name, for example, "Festival Interactivo",
// while the former is alway an English name, example "Festival Interactive".

// C++ includes.
#include <math.h>

// Qt includes.
#include <qwidget.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qbuttongroup.h>

// KDE includes.
#include <dcopclient.h>
#include <klistview.h>
#include <kparts/componentfactory.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

// KTTS includes.
#include "talkercode.h"
#include "pluginconf.h"
#include "filterconf.h"
#include "testplayer.h"
#include "player.h"
#include "selecttalkerdlg.h"
#include "selectevent.h"
#include "notify.h"
#include "utils.h"

// KCMKttsMgr includes.
#include "kcmkttsmgr.h"
#include "kcmkttsmgr.moc"

// Some constants.
// Defaults set when clicking Defaults button.
const bool embedInSysTrayCheckBoxValue = true;
const bool showMainWindowOnStartupCheckBoxValue = true;

const bool autostartMgrCheckBoxValue = true;
const bool autoexitMgrCheckBoxValue = true;

const bool notifyEnableCheckBoxValue = false;
const bool notifyExcludeEventsWithSoundCheckBoxValue = true;

const bool textPreMsgCheckValue = true;
const QString textPreMsgValue = i18n("Text interrupted. Message.");

const bool textPreSndCheckValue = false;
const QString textPreSndValue = "";

const bool textPostMsgCheckValue = true;
const QString textPostMsgValue = i18n("Resuming text.");

const bool textPostSndCheckValue = false;
const QString textPostSndValue = "";

const int timeBoxValue = 100;

const bool keepAudioCheckBoxValue = false;

// Make this a plug in.
typedef KGenericFactory<KCMKttsMgr, QWidget> KCMKttsMgrFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kttsd, KCMKttsMgrFactory("kttsd") );

/**
* Constructor.
* Makes the list of plug ins.
* And the languages acording to the plug ins.
*/
KCMKttsMgr::KCMKttsMgr(QWidget *parent, const char *name, const QStringList &) :
    DCOPObject("kcmkttsmgr_kspeechsink"),
    KCModule(KCMKttsMgrFactory::instance(), parent, name)
{
    // kdDebug() << "KCMKttsMgr contructor running." << endl;

    // Initialize some variables.
    m_config = 0;
    m_jobMgrPart = 0;
    m_configDlg = 0;
    m_changed = false;
    m_suppressConfigChanged = false;

    // Add the KTTS Manager widget
    QGridLayout *layout = new QGridLayout(this, 0, 0);
    m_kttsmgrw = new KCMKttsMgrWidget(this, "kttsmgrw");
    layout->addWidget(m_kttsmgrw, 0, 0);

    // Give buttons icons.
    // Talkers tab.
    m_kttsmgrw->higherTalkerPriorityButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("up", KIcon::Small));
    m_kttsmgrw->lowerTalkerPriorityButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("down", KIcon::Small));
    m_kttsmgrw->removeTalkerButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small));
    m_kttsmgrw->configureTalkerButton->setIconSet(
        KGlobal::iconLoader()->loadIconSet("configure", KIcon::Small));

    // Filters tab.
    m_kttsmgrw->higherFilterPriorityButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("up", KIcon::Small));
    m_kttsmgrw->lowerFilterPriorityButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("down", KIcon::Small));
    m_kttsmgrw->removeFilterButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small));
    m_kttsmgrw->configureFilterButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("configure", KIcon::Small));

    // Notify tab.
    m_kttsmgrw->notifyActionComboBox->clear();
    for (int ndx = 0; ndx < NotifyAction::count(); ++ndx)
        m_kttsmgrw->notifyActionComboBox->insertItem( NotifyAction::actionDisplayName( ndx ) );
    m_kttsmgrw->notifyPresentComboBox->clear();
    for (int ndx = 0; ndx < NotifyPresent::count(); ++ndx)
        m_kttsmgrw->notifyPresentComboBox->insertItem( NotifyPresent::presentDisplayName( ndx ) );

    m_kttsmgrw->notifyRemoveButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small));
    m_kttsmgrw->notifyTestButton->setIconSet(
            KGlobal::iconLoader()->loadIconSet("speak", KIcon::Small));

    m_kttsmgrw->sinkComboBox->setEditable(false);

    // Construct a popup menu for the Sentence Boundary Detector buttons on Filter tab.
    m_sbdPopmenu = new QPopupMenu( m_kttsmgrw, "SbdPopupMenu" );
    m_sbdPopmenu->insertItem( i18n("&Edit..."), this, SLOT(slot_configureSbdFilter()), 0, sbdBtnEdit );
    m_sbdPopmenu->insertItem( KGlobal::iconLoader()->loadIconSet("up", KIcon::Small),
                              i18n("U&p"), this, SLOT(slot_higherSbdFilterPriority()), 0, sbdBtnUp );
    m_sbdPopmenu->insertItem( KGlobal::iconLoader()->loadIconSet("down", KIcon::Small),
                              i18n("Do&wn"), this, SLOT(slot_lowerSbdFilterPriority()), 0, sbdBtnDown );
    m_sbdPopmenu->insertItem( i18n("&Add..."), this, SLOT(slot_addSbdFilter()), 0, sbdBtnAdd );
    m_sbdPopmenu->insertItem( i18n("&Remove"), this, SLOT(slot_removeSbdFilter()), 0, sbdBtnRemove );
    m_kttsmgrw->sbdButton->setPopup( m_sbdPopmenu );

    // If GStreamer is available, enable its radio button.
    // Determine if available by loading its plugin.  If it fails, not available.
    TestPlayer* testPlayer = new TestPlayer();
    Player* player = testPlayer->createPlayerObject(1);
    if (player)
    {
        m_kttsmgrw->gstreamerRadioButton->setEnabled(true);
        m_kttsmgrw->sinkLabel->setEnabled(true);
        m_kttsmgrw->sinkComboBox->setEnabled(true);
        QStringList sinkList = player->getPluginList("Sink/Audio");
        // kdDebug() << "KCMKttsMgr::KCMKttsMgr: GStreamer sinkList = " << sinkList << endl;
        m_kttsmgrw->sinkComboBox->clear();
        m_kttsmgrw->sinkComboBox->insertStringList(sinkList);
    }
    delete player;
    delete testPlayer;

    // Set up Keep Audio Path KURLRequestor.
    m_kttsmgrw->keepAudioPath->setMode(KFile::Directory);
    m_kttsmgrw->keepAudioPath->setURL(locateLocal("data", "kttsd/audio/"));

    // Object for the KTTSD configuration.
    m_config = new KConfig("kttsdrc");

    // Load configuration.
    load();

    // Connect the signals from the KCMKtssMgrWidget to this class.

    // Talker tab.
    connect(m_kttsmgrw->addTalkerButton, SIGNAL(clicked()),
            this, SLOT(slot_addTalker()));
    connect(m_kttsmgrw->higherTalkerPriorityButton, SIGNAL(clicked()),
            this, SLOT(slot_higherTalkerPriority()));
    connect(m_kttsmgrw->lowerTalkerPriorityButton, SIGNAL(clicked()),
            this, SLOT(slot_lowerTalkerPriority()));
    connect(m_kttsmgrw->removeTalkerButton, SIGNAL(clicked()),
            this, SLOT(slot_removeTalker()));
    connect(m_kttsmgrw->configureTalkerButton, SIGNAL(clicked()),
            this, SLOT(slot_configureTalker()));
    connect(m_kttsmgrw->talkersList, SIGNAL(selectionChanged()),
            this, SLOT(updateTalkerButtons()));

    // Filter tab.
    connect(m_kttsmgrw->addFilterButton, SIGNAL(clicked()),
            this, SLOT(slot_addNormalFilter()));
    connect(m_kttsmgrw->higherFilterPriorityButton, SIGNAL(clicked()),
            this, SLOT(slot_higherNormalFilterPriority()));
    connect(m_kttsmgrw->lowerFilterPriorityButton, SIGNAL(clicked()),
            this, SLOT(slot_lowerNormalFilterPriority()));
    connect(m_kttsmgrw->removeFilterButton, SIGNAL(clicked()),
            this, SLOT(slot_removeNormalFilter()));
    connect(m_kttsmgrw->configureFilterButton, SIGNAL(clicked()),
            this, SLOT(slot_configureNormalFilter()));
    connect(m_kttsmgrw->filtersList, SIGNAL(selectionChanged()),
            this, SLOT(updateFilterButtons()));
    //connect(m_kttsmgrw->filtersList, SIGNAL(stateChanged()),
    //        this, SLOT(configChanged()));
    connect(m_kttsmgrw->sbdsList, SIGNAL(selectionChanged()),
            this, SLOT(updateSbdButtons()));

    // Audio tab.
    connect(m_kttsmgrw->gstreamerRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotGstreamerRadioButton_toggled(bool)));
    connect(m_kttsmgrw->timeBox, SIGNAL(valueChanged(int)),
            this, SLOT(timeBox_valueChanged(int)));
    connect(m_kttsmgrw->timeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeSlider_valueChanged(int)));
    connect(m_kttsmgrw->timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_kttsmgrw->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_kttsmgrw->keepAudioCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(keepAudioCheckBox_toggled(bool)));
    connect(m_kttsmgrw->keepAudioPath, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));

    // General tab.
    connect(m_kttsmgrw->enableKttsdCheckBox, SIGNAL(toggled(bool)),
            SLOT(enableKttsdToggled(bool)));

    // Notify tab.
    connect(m_kttsmgrw->notifyEnableCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotNotifyEnableCheckBox_toggled(bool)));
    connect(m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_kttsmgrw->notifyAddButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyAddButton_clicked()));
    connect(m_kttsmgrw->notifyRemoveButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyRemoveButton_clicked()));
    connect(m_kttsmgrw->notifyClearButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyClearButton_clicked()));
    connect(m_kttsmgrw->notifyLoadButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyLoadButton_clicked()));
    connect(m_kttsmgrw->notifySaveButton, SIGNAL(clicked()),
            this, SLOT(slotNotifySaveButton_clicked()));
    connect(m_kttsmgrw->notifyListView, SIGNAL(selectionChanged()),
            this, SLOT(slotNotifyListView_selectionChanged()));
    connect(m_kttsmgrw->notifyPresentComboBox, SIGNAL(activated(int)),
            this, SLOT(slotNotifyPresentComboBox_activated(int)));
    connect(m_kttsmgrw->notifyActionComboBox, SIGNAL(activated(int)),
            this, SLOT(slotNotifyActionComboBox_activated(int)));
    connect(m_kttsmgrw->notifyTestButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyTestButton_clicked()));
    connect(m_kttsmgrw->notifyMsgLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotNotifyMsgLineEdit_textChanged(const QString&)));
    connect(m_kttsmgrw->notifyTalkerButton, SIGNAL(clicked()),
            this, SLOT(slotNotifyTalkerButton_clicked()));

    // Others.
    connect(m_kttsmgrw, SIGNAL( configChanged() ),
            this, SLOT( configChanged() ) );
    connect(m_kttsmgrw->mainTab, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(slotTabChanged()));

    // Connect KTTSD DCOP signals to our slots.
    if (!connectDCOPSignal("kttsd", "KSpeech",
        "kttsdStarted()",
        "kttsdStarted()",
        false)) kdDebug() << "connectDCOPSignal failed" << endl;
    connectDCOPSignal("kttsd", "KSpeech",
        "kttsdExiting()",
        "kttsdExiting()",
        false);

    // See if KTTSD is already running, and if so, create jobs tab.
    if (kapp->dcopClient()->isApplicationRegistered("kttsd"))
        kttsdStarted();
    else
        // Start KTTSD if check box is checked.
        enableKttsdToggled(m_kttsmgrw->enableKttsdCheckBox->isChecked());

    // Switch to Talkers tab if none configured,
    // otherwise switch to Jobs tab if it is active.
    if (m_kttsmgrw->talkersList->childCount() == 0)
        m_kttsmgrw->mainTab->setCurrentPage(wpTalkers);
    else if (m_kttsmgrw->enableKttsdCheckBox->isChecked())
        m_kttsmgrw->mainTab->setCurrentPage(wpJobs);
} 

/**
* Destructor.
*/
KCMKttsMgr::~KCMKttsMgr(){
    // kdDebug() << "KCMKttsMgr::~KCMKttsMgr: Running" << endl;
    delete m_config;
}

/**
* This method is invoked whenever the module should read its 
* configuration (most of the times from a config file) and update the 
* user interface. This happens when the user clicks the "Reset" button in
* the control center, to undo all of his changes and restore the currently
* valid settings. NOTE that this is not called after the modules is loaded,
* so you probably want to call this method in the constructor.
*/
void KCMKttsMgr::load()
{
    // kdDebug() << "KCMKttsMgr::load: Running" << endl;

    m_changed = false;
    // Don't emit changed() signal while loading.
    m_suppressConfigChanged = true;

    // Set the group general for the configuration of kttsd itself (no plug ins)
    m_config->setGroup("General");

    // Load the configuration of the text interruption messages and sound
    m_kttsmgrw->textPreMsgCheck->setChecked(m_config->readBoolEntry("TextPreMsgEnabled", textPreMsgCheckValue));
    m_kttsmgrw->textPreMsg->setText(m_config->readEntry("TextPreMsg", textPreMsgValue));
    m_kttsmgrw->textPreMsg->setEnabled(m_kttsmgrw->textPreMsgCheck->isChecked());

    m_kttsmgrw->textPreSndCheck->setChecked(m_config->readBoolEntry("TextPreSndEnabled", textPreSndCheckValue));
    m_kttsmgrw->textPreSnd->setURL(m_config->readEntry("TextPreSnd", textPreSndValue));
    m_kttsmgrw->textPreSnd->setEnabled(m_kttsmgrw->textPreSndCheck->isChecked());

    m_kttsmgrw->textPostMsgCheck->setChecked(m_config->readBoolEntry("TextPostMsgEnabled", textPostMsgCheckValue));
    m_kttsmgrw->textPostMsg->setText(m_config->readEntry("TextPostMsg", textPostMsgValue));
    m_kttsmgrw->textPostMsg->setEnabled(m_kttsmgrw->textPostMsgCheck->isChecked());

    m_kttsmgrw->textPostSndCheck->setChecked(m_config->readBoolEntry("TextPostSndEnabled", textPostSndCheckValue));
    m_kttsmgrw->textPostSnd->setURL(m_config->readEntry("TextPostSnd", textPostSndValue));
    m_kttsmgrw->textPostSnd->setEnabled(m_kttsmgrw->textPostSndCheck->isChecked());

    // Overall settings.
    m_kttsmgrw->embedInSysTrayCheckBox->setChecked(m_config->readBoolEntry("EmbedInSysTray",
        m_kttsmgrw->embedInSysTrayCheckBox->isChecked()));
    m_kttsmgrw->showMainWindowOnStartupCheckBox->setChecked(m_config->readBoolEntry(
        "ShowMainWindowOnStartup", m_kttsmgrw->showMainWindowOnStartupCheckBox->isChecked()));
    m_kttsmgrw->showMainWindowOnStartupCheckBox->setEnabled(
        m_kttsmgrw->embedInSysTrayCheckBox->isChecked());

    m_kttsmgrw->enableKttsdCheckBox->setChecked(m_config->readBoolEntry("EnableKttsd",
        m_kttsmgrw->enableKttsdCheckBox->isChecked()));

    m_kttsmgrw->autostartMgrCheckBox->setChecked(m_config->readBoolEntry("AutoStartManager", true));
    m_kttsmgrw->autoexitMgrCheckBox->setChecked(m_config->readBoolEntry("AutoExitManager", true));

    // Notification settings.
    m_kttsmgrw->notifyEnableCheckBox->setChecked(m_config->readBoolEntry("Notify",
        m_kttsmgrw->notifyEnableCheckBox->isChecked()));
    m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->setChecked(
        m_config->readBoolEntry("ExcludeEventsWithSound",
        m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->isChecked()));
    slotNotifyClearButton_clicked();
    loadNotifyEventsFromFile( locateLocal("config", "kttsd_notifyevents.xml"), true );
    slotNotifyEnableCheckBox_toggled( m_kttsmgrw->notifyEnableCheckBox->isChecked() );
    // Auto-expand and position on the Default item.
    QListViewItem* item = m_kttsmgrw->notifyListView->findItem( "default", nlvcEventSrc );
    if ( item )
        if ( item->childCount() > 0 ) item = item->firstChild();
    if ( item ) m_kttsmgrw->notifyListView->ensureItemVisible( item );

    // Audio Output.
    int audioOutputMethod = 0;
    if (m_kttsmgrw->gstreamerRadioButton->isChecked()) audioOutputMethod = 1;
    audioOutputMethod = m_config->readNumEntry("AudioOutputMethod", audioOutputMethod);
    switch (audioOutputMethod)
    {
        case 0:
            m_kttsmgrw->artsRadioButton->setChecked(true);
            break;
        case 1:
            m_kttsmgrw->gstreamerRadioButton->setChecked(true);
    }
    m_kttsmgrw->timeBox->setValue(m_config->readNumEntry("AudioStretchFactor", timeBoxValue));
    timeBox_valueChanged(m_kttsmgrw->timeBox->value());
    m_kttsmgrw->keepAudioCheckBox->setChecked(
        m_config->readBoolEntry("KeepAudio",                                             m_kttsmgrw->keepAudioCheckBox->isChecked()));
    m_kttsmgrw->keepAudioPath->setURL(
        m_config->readEntry("KeepAudioPath",
        m_kttsmgrw->keepAudioPath->url()));
    m_kttsmgrw->keepAudioPath->setEnabled(m_kttsmgrw->keepAudioCheckBox->isChecked());

    // Last plugin ID.  Used to generate a new ID for an added talker.
    m_lastTalkerID = 0;

    // Last filter ID.  Used to generate a new ID for an added filter.
    m_lastFilterID = 0;

    // Dictionary mapping languages to language codes.
    m_languagesToCodes.clear();

    // Load existing Talkers into the listview.
    m_kttsmgrw->talkersList->clear();
    m_kttsmgrw->talkersList->setSortColumn(-1);
    QStringList talkerIDsList = m_config->readListEntry("TalkerIDs", ',');
    if (!talkerIDsList.isEmpty())
    {
        QListViewItem* talkerItem = 0;
        QStringList::ConstIterator itEnd = talkerIDsList.constEnd();
        for (QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it)
        {
            QString talkerID = *it;
            // kdDebug() << "KCMKttsMgr::load: talkerID = " << talkerID << endl;
            m_config->setGroup(QString("Talker_") + talkerID);
            QString talkerCode = m_config->readEntry("TalkerCode");
            QString fullLanguageCode;
            talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, fullLanguageCode);
            QString language = TalkerCode::languageCodeToLanguage(fullLanguageCode);
            QString desktopEntryName = m_config->readEntry("DesktopEntryName", QString::null);
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            QString synthName;
            if (desktopEntryName.isEmpty())
            {
                synthName = m_config->readEntry("PlugIn", QString::null);
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = TalkerCode::TalkerNameToDesktopEntryName(synthName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) m_config->writeEntry("DesktopEntryName", desktopEntryName);
            }
            synthName = TalkerCode::TalkerDesktopEntryNameToName(desktopEntryName);
            if (!synthName.isEmpty())
            {
                // kdDebug() << "KCMKttsMgr::load: talkerCode = " << talkerCode << endl;
                if (talkerItem)
                    talkerItem = new KListViewItem(m_kttsmgrw->talkersList, talkerItem,
                        talkerID, language, synthName);
                else
                    talkerItem = new KListViewItem(m_kttsmgrw->talkersList,
                        talkerID, language, synthName);
                updateTalkerItem(talkerItem, talkerCode);
                m_languagesToCodes[language] = fullLanguageCode;
                if (talkerID.toInt() > m_lastTalkerID) m_lastTalkerID = talkerID.toInt();
            }
        }
    }

    // Query for all the KCMKTTSD SynthPlugins and store the list in offers.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");

    // Iterate thru the possible plug ins getting their language support codes.
    for(unsigned int i=0; i < offers.count() ; ++i)
    {
        QString synthName = offers[i]->name();
        QStringList languageCodes = offers[i]->property("X-KDE-Languages").toStringList();
        // Add language codes to the language-to-language code map.
        QStringList::ConstIterator endLanguages(languageCodes.constEnd());
        for( QStringList::ConstIterator it = languageCodes.constBegin(); it != endLanguages; ++it )
        {
            QString language = TalkerCode::languageCodeToLanguage(*it);
            m_languagesToCodes[language] = *it;
        }

        // All plugins support "Other".
        // TODO: Eventually, this should not be necessary, since all plugins will know
        // the languages they support and report them in call to getSupportedLanguages().
        if (!languageCodes.contains("other")) languageCodes.append("other");

        // Add supported language codes to synthesizer-to-language map.
        m_synthToLangMap[synthName] = languageCodes;
    }

    // Add "Other" language.
    m_languagesToCodes[i18n("Other")] = "other";

    // Load Filters.
    QListViewItem* filterItem = 0;
    m_kttsmgrw->filtersList->clear();
    m_kttsmgrw->sbdsList->clear();
    m_kttsmgrw->filtersList->setSortColumn(-1);
    m_kttsmgrw->sbdsList->setSortColumn(-1);
    m_config->setGroup("General");
    QStringList filterIDsList = m_config->readListEntry("FilterIDs", ',');
    // kdDebug() << "KCMKttsMgr::load: FilterIDs = " << filterIDsList << endl;
    if (!filterIDsList.isEmpty())
    {
        QStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (QStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it)
        {
            QString filterID = *it;
            // kdDebug() << "KCMKttsMgr::load: filterID = " << filterID << endl;
            m_config->setGroup("Filter_" + filterID);
            QString desktopEntryName = m_config->readEntry("DesktopEntryName", QString::null);
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            QString filterPlugInName;
            if (desktopEntryName.isEmpty())
            {
                filterPlugInName = m_config->readEntry("PlugInName", QString::null);
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) m_config->writeEntry("DesktopEntryName", desktopEntryName);
            }
            filterPlugInName = FilterDesktopEntryNameToName(desktopEntryName);
            if (!filterPlugInName.isEmpty())
            {
                QString userFilterName = m_config->readEntry("UserFilterName", filterPlugInName);
                bool multiInstance = m_config->readBoolEntry("MultiInstance", false);
                // Determine if this filter is a Sentence Boundary Detector (SBD).
                bool isSbd = m_config->readBoolEntry("IsSBD", false);
                bool checked = m_config->readBoolEntry("Enabled", false);
                if (isSbd)
                {
                    filterItem = m_kttsmgrw->sbdsList->lastChild();
                    if (!filterItem)
                        filterItem = new KListViewItem(m_kttsmgrw->sbdsList,
                            userFilterName, filterID, filterPlugInName);
                    else
                        filterItem = new KListViewItem(m_kttsmgrw->sbdsList, filterItem,
                            userFilterName, filterID, filterPlugInName);
                } else {
                    filterItem = m_kttsmgrw->filtersList->lastChild();
                    if (!filterItem)
                        filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList,
                            userFilterName, QCheckListItem::CheckBox, this);
                    else
                        filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList, filterItem,
                            userFilterName, QCheckListItem::CheckBox, this);
                    dynamic_cast<QCheckListItem*>(filterItem)->setOn(checked);
                }
                filterItem->setText(flvcFilterID, filterID);
                filterItem->setText(flvcPlugInName, filterPlugInName);
                if (multiInstance)
                    filterItem->setText(flvcMultiInstance, "T");
                else
                    filterItem->setText(flvcMultiInstance, "F");
                if (filterID.toInt() > m_lastFilterID) m_lastFilterID = filterID.toInt();
            }
        }
    }

    // Add at least one unchecked instance of each available filter plugin if there is
    // not already at least one instance and the filter can autoconfig itself.
    offers = KTrader::self()->query("KTTSD/FilterPlugin");
    for (unsigned int i=0; i < offers.count() ; ++i)
    {
        QString filterPlugInName = offers[i]->name();
        QString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
        if (!desktopEntryName.isEmpty() && (countFilterPlugins(filterPlugInName) == 0))
        {
            // Must load plugin to determine if it supports multiple instances
            // and to see if it can autoconfigure itself.
            KttsFilterConf* filterPlugIn = loadFilterPlugin(filterPlugInName);
            if (filterPlugIn)
            {
                m_lastFilterID++;
                QString filterID = QString::number(m_lastFilterID);
                QString groupName = "Filter_" + filterID;
                filterPlugIn->load(m_config, groupName);
                QString userFilterName = filterPlugIn->userPlugInName();
                if (!userFilterName.isEmpty())
                {
                    kdDebug() << "KCMKttsMgr::load: auto-configuring filter " << userFilterName << endl;
                    // Determine if plugin is an SBD filter.
                    bool multiInstance = filterPlugIn->supportsMultiInstance();
                    bool isSbd = filterPlugIn->isSBD();
                    if (isSbd)
                    {
                        filterItem = m_kttsmgrw->sbdsList->lastChild();
                        if (!filterItem)
                            filterItem = new KListViewItem(m_kttsmgrw->sbdsList,
                                userFilterName, filterID, filterPlugInName);
                        else
                            filterItem = new KListViewItem(m_kttsmgrw->sbdsList, filterItem,
                                userFilterName, filterID, filterPlugInName);
                    } else {
                        filterItem = m_kttsmgrw->filtersList->lastChild();
                        if (!filterItem)
                            filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList,
                                userFilterName, QCheckListItem::CheckBox, this);
                        else
                            filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList, filterItem,
                                userFilterName, QCheckListItem::CheckBox, this);
                        dynamic_cast<QCheckListItem*>(filterItem)->setOn(false);
                    }
                    filterItem->setText(flvcFilterID, filterID);
                    filterItem->setText(flvcPlugInName, filterPlugInName);
                    if (multiInstance)
                        filterItem->setText(flvcMultiInstance, "T");
                    else
                        filterItem->setText(flvcMultiInstance, "F");
                    m_config->setGroup(groupName);
                    filterPlugIn->save(m_config, groupName);
                    m_config->setGroup(groupName);
                    m_config->writeEntry("DesktopEntryName", desktopEntryName);
                    m_config->writeEntry("UserFilterName", userFilterName);
                    m_config->writeEntry("Enabled", isSbd);
                    m_config->writeEntry("MultiInstance", multiInstance);
                    m_config->writeEntry("IsSBD", isSbd);
                    filterIDsList.append(filterID);
                } else m_lastFilterID--;
            } else
                kdDebug() << "KCMKttsMgr::load: Unable to load filter plugin " << filterPlugInName 
                    << " DesktopEntryName " << desktopEntryName << endl;
            delete filterPlugIn;
        }
    }
    // Rewrite list of FilterIDs in case we added any.
    QString filterIDs = filterIDsList.join(",");
    m_config->setGroup("General");
    m_config->writeEntry("FilterIDs", filterIDs);
    m_config->sync();

    // Uncheck and disable KTTSD checkbox if no Talkers are configured.
    if (m_kttsmgrw->talkersList->childCount() == 0)
    {
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(false);
        enableKttsdToggled(false);
    }

    // Enable ShowMainWindowOnStartup checkbox based on EmbedInSysTray checkbox.
    m_kttsmgrw->showMainWindowOnStartupCheckBox->setEnabled(
        m_kttsmgrw->embedInSysTrayCheckBox->isChecked());

    // GStreamer settings.
    m_config->setGroup("GStreamerPlayer");
    KttsUtils::setCbItemFromText(m_kttsmgrw->sinkComboBox, m_config->readEntry("SinkName", "osssink"));

    // Update controls based on new states.
    slotNotifyListView_selectionChanged();
    updateTalkerButtons();
    updateFilterButtons();
    updateSbdButtons();
    slotGstreamerRadioButton_toggled(m_kttsmgrw->gstreamerRadioButton->isChecked());

    m_changed = false;
    m_suppressConfigChanged = false;
}

/**
* This function gets called when the user wants to save the settings in 
* the user interface, updating the config files or wherever the 
* configuration is stored. The method is called when the user clicks "Apply" 
* or "Ok".
*/
void KCMKttsMgr::save()
{
    // kdDebug() << "KCMKttsMgr::save: Running" << endl;
    m_changed = false;

    // Clean up config.
    m_config->deleteGroup("General");

    // Set the group general for the configuration of kttsd itself (no plug ins)
    m_config->setGroup("General");

    // Set text interrumption messages and paths
    m_config->writeEntry("TextPreMsgEnabled", m_kttsmgrw->textPreMsgCheck->isChecked());
    m_config->writeEntry("TextPreMsg", m_kttsmgrw->textPreMsg->text());

    m_config->writeEntry("TextPreSndEnabled", m_kttsmgrw->textPreSndCheck->isChecked()); 
    m_config->writeEntry("TextPreSnd", PlugInConf::realFilePath(m_kttsmgrw->textPreSnd->url()));

    m_config->writeEntry("TextPostMsgEnabled", m_kttsmgrw->textPostMsgCheck->isChecked());
    m_config->writeEntry("TextPostMsg", m_kttsmgrw->textPostMsg->text());

    m_config->writeEntry("TextPostSndEnabled", m_kttsmgrw->textPostSndCheck->isChecked());
    m_config->writeEntry("TextPostSnd", PlugInConf::realFilePath(m_kttsmgrw->textPostSnd->url()));

    // Overall settings.
    m_config->writeEntry("EmbedInSysTray", m_kttsmgrw->embedInSysTrayCheckBox->isChecked());
    m_config->writeEntry("ShowMainWindowOnStartup",
        m_kttsmgrw->showMainWindowOnStartupCheckBox->isChecked());
    m_config->writeEntry("AutoStartManager", m_kttsmgrw->autostartMgrCheckBox->isChecked());
    m_config->writeEntry("AutoExitManager", m_kttsmgrw->autoexitMgrCheckBox->isChecked());

    // Uncheck and disable KTTSD checkbox if no Talkers are configured.
    // Enable checkbox if at least one Talker is configured.
    bool enableKttsdWasToggled = false;
    if (m_kttsmgrw->talkersList->childCount() == 0)
    {
        enableKttsdWasToggled = m_kttsmgrw->enableKttsdCheckBox->isChecked();
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(false);
        // Might as well zero LastTalkerID as well.
        m_lastTalkerID = 0;
    }
    else
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(true);

    m_config->writeEntry("EnableKttsd", m_kttsmgrw->enableKttsdCheckBox->isChecked());

    // Notification settings.
    m_config->writeEntry("Notify", m_kttsmgrw->notifyEnableCheckBox->isChecked());
    m_config->writeEntry("ExcludeEventsWithSound",
        m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->isChecked());
    saveNotifyEventsToFile( locateLocal("config", "kttsd_notifyevents.xml") );

    // Audio Output.
    int audioOutputMethod = 0;
    if (m_kttsmgrw->gstreamerRadioButton->isChecked()) audioOutputMethod = 1;
    m_config->writeEntry("AudioOutputMethod", audioOutputMethod);
    m_config->writeEntry("AudioStretchFactor", m_kttsmgrw->timeBox->value());
    m_config->writeEntry("KeepAudio", m_kttsmgrw->keepAudioCheckBox->isChecked());
    m_config->writeEntry("KeepAudioPath", m_kttsmgrw->keepAudioPath->url());

    // Get ordered list of all talker IDs.
    QStringList talkerIDsList;
    QListViewItem* talkerItem = m_kttsmgrw->talkersList->firstChild();
    while (talkerItem)
    {
        QListViewItem* nextTalkerItem = talkerItem->itemBelow();
        QString talkerID = talkerItem->text(tlvcTalkerID);
        talkerIDsList.append(talkerID);
        talkerItem = nextTalkerItem;
    }
    QString talkerIDs = talkerIDsList.join(",");
    m_config->writeEntry("TalkerIDs", talkerIDs);

    // Erase obsolete Talker_nn sections.
    QStringList groupList = m_config->groupList();
    int groupListCount = groupList.count();
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx)
    {
        QString groupName = groupList[groupNdx];
        if (groupName.left(7) == "Talker_")
        {
            QString groupTalkerID = groupName.mid(7);
            if (!talkerIDsList.contains(groupTalkerID)) m_config->deleteGroup(groupName);
        }
    }

    // Get ordered list of all filter IDs.  Record enabled state of each filter.
    QStringList filterIDsList;
    QListViewItem* filterItem = m_kttsmgrw->filtersList->firstChild();
    while (filterItem)
    {
        QListViewItem* nextFilterItem = filterItem->itemBelow();
        QString filterID = filterItem->text(flvcFilterID);
        filterIDsList.append(filterID);
        bool checked = dynamic_cast<QCheckListItem*>(filterItem)->isOn();
        m_config->setGroup("Filter_" + filterID);
        m_config->writeEntry("Enabled", checked);
        m_config->writeEntry("IsSBD", false);
        filterItem = nextFilterItem;
    }
    QListViewItem* sbdItem = m_kttsmgrw->sbdsList->firstChild();
    while (sbdItem)
    {
        QListViewItem* nextSbdItem = sbdItem->itemBelow();
        QString filterID = sbdItem->text(slvcFilterID);
        filterIDsList.append(filterID);
        m_config->setGroup("Filter_" + filterID);
        m_config->writeEntry("Enabled", true);
        m_config->writeEntry("IsSBD", true);
        sbdItem = nextSbdItem;
    }
    QString filterIDs = filterIDsList.join(",");
    m_config->setGroup("General");
    m_config->writeEntry("FilterIDs", filterIDs);

    // Erase obsolete Filter_nn sections.
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx)
    {
        QString groupName = groupList[groupNdx];
        if (groupName.left(7) == "Filter_")
        {
            QString groupFilterID = groupName.mid(7);
            if (!filterIDsList.contains(groupFilterID)) m_config->deleteGroup(groupName);
        }
    }

    // GStreamer settings.
    m_config->setGroup("GStreamerPlayer");
    m_config->writeEntry("SinkName", m_kttsmgrw->sinkComboBox->currentText());

    m_config->sync();

    // If we automatically unchecked the Enable KTTSD checkbox, stop KTTSD.
    if (enableKttsdWasToggled)
        enableKttsdToggled(false);
    else
    {
        // If KTTSD is running, reinitialize it.
        DCOPClient *client = kapp->dcopClient();
        bool kttsdRunning = (client->isApplicationRegistered("kttsd"));
        if (kttsdRunning)
        {
            kdDebug() << "Restarting KTTSD" << endl;
            QByteArray data;
            client->send("kttsd", "KSpeech", "reinit()", data);
        }
    }
}

void KCMKttsMgr::slotTabChanged()
{
    setButtons(buttons());
    int currentPageIndex = m_kttsmgrw->mainTab->currentPageIndex();
    if (currentPageIndex == wpJobs)
    {
        if (m_changed)
        {
            KMessageBox::information(m_kttsmgrw,
                i18n("You have made changes to the configuration but have not saved them yet.  "
                     "Click Apply to save the changes or Cancel to abandon the changes."));
        }
    }
}

/**
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.
*/
void KCMKttsMgr::defaults() {
    // kdDebug() << "Running: KCMKttsMgr::defaults: Running"<< endl;

    int currentPageIndex = m_kttsmgrw->mainTab->currentPageIndex();
    bool changed = false;
    switch (currentPageIndex)
    {
        case wpGeneral:
            if (m_kttsmgrw->embedInSysTrayCheckBox->isChecked() != embedInSysTrayCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->embedInSysTrayCheckBox->setChecked(embedInSysTrayCheckBoxValue);
            }
            if (m_kttsmgrw->showMainWindowOnStartupCheckBox->isChecked() !=
                showMainWindowOnStartupCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->showMainWindowOnStartupCheckBox->setChecked(
                    showMainWindowOnStartupCheckBoxValue);
            }
            if (m_kttsmgrw->autostartMgrCheckBox->isChecked() != autostartMgrCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->autostartMgrCheckBox->setChecked(
                    autostartMgrCheckBoxValue);
            }
            if (m_kttsmgrw->autoexitMgrCheckBox->isChecked() != autoexitMgrCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->autoexitMgrCheckBox->setChecked(
                    autoexitMgrCheckBoxValue);
            }
            break;

        case wpNotify:
            if (m_kttsmgrw->notifyEnableCheckBox->isChecked() != notifyEnableCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->notifyEnableCheckBox->setChecked(notifyEnableCheckBoxValue);
                m_kttsmgrw->notifyGroup->setChecked( notifyEnableCheckBoxValue );
            }
            if (m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->isChecked() !=
                notifyExcludeEventsWithSoundCheckBoxValue )
            {
                changed = true;
                m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->setChecked(
                    notifyExcludeEventsWithSoundCheckBoxValue );
            }
            break;

        case wpInterruption:
            if (m_kttsmgrw->textPreMsgCheck->isChecked() != textPreMsgCheckValue)
            {
                changed = true;
                m_kttsmgrw->textPreMsgCheck->setChecked(textPreMsgCheckValue);
            }
            if (m_kttsmgrw->textPreMsg->text() != i18n(textPreMsgValue.utf8()))
            {
                changed = true;
                m_kttsmgrw->textPreMsg->setText(i18n(textPreMsgValue.utf8()));
            }
            if (m_kttsmgrw->textPreSndCheck->isChecked() != textPreSndCheckValue)
            {
                changed = true;
                m_kttsmgrw->textPreSndCheck->setChecked(textPreSndCheckValue);
            }
            if (m_kttsmgrw->textPreSnd->url() != textPreSndValue)
            {
                changed = true;
                m_kttsmgrw->textPreSnd->setURL(textPreSndValue);
            }
            if (m_kttsmgrw->textPostMsgCheck->isChecked() != textPostMsgCheckValue)
            {
                changed = true;
                m_kttsmgrw->textPostMsgCheck->setChecked(textPostMsgCheckValue);
            }
            if (m_kttsmgrw->textPostMsg->text() != i18n(textPostMsgValue.utf8()))
            {
                changed = true;
                m_kttsmgrw->textPostMsg->setText(i18n(textPostMsgValue.utf8()));
            }
            if (m_kttsmgrw->textPostSndCheck->isChecked() != textPostSndCheckValue)
            {
                changed = true;
                m_kttsmgrw->textPostSndCheck->setChecked(textPostSndCheckValue);
            }
            if (m_kttsmgrw->textPostSnd->url() != textPostSndValue)
            {
                changed = true;
                m_kttsmgrw->textPostSnd->setURL(textPostSndValue);
            }
            break;

        case wpAudio:
            if (!m_kttsmgrw->artsRadioButton->isChecked())
            {
                changed = true;
                m_kttsmgrw->artsRadioButton->setChecked(true);
            }
            if (m_kttsmgrw->timeBox->value() != timeBoxValue)
            {
                changed = true;
                m_kttsmgrw->timeBox->setValue(timeBoxValue);
            }
            if (m_kttsmgrw->keepAudioCheckBox->isChecked() !=
                 keepAudioCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->keepAudioCheckBox->setChecked(keepAudioCheckBoxValue);
            }
            if (m_kttsmgrw->keepAudioPath->url() != locateLocal("data", "kttsd/audio/"))
            {
                changed = true;
                m_kttsmgrw->keepAudioPath->setURL(locateLocal("data", "kttsd/audio/"));
            }
            m_kttsmgrw->keepAudioPath->setEnabled(m_kttsmgrw->keepAudioCheckBox->isEnabled());
    }
    if (changed) configChanged();
}

/**
* This is a static method which gets called to realize the modules settings
* during the startup of KDE. NOTE that most modules do not implement this
* method, but modules like the keyboard and mouse modules, which directly
* interact with the X-server, need this method. As this method is static,
* it can avoid to create an instance of the user interface, which is often
* not needed in this case.
*/
void KCMKttsMgr::init(){
    // kdDebug() << "KCMKttsMgr::init: Running" << endl;
}

/**
* The control center calls this function to decide which buttons should
* be displayed. For example, it does not make sense to display an "Apply" 
* button for one of the information modules. The value returned can be set by 
* modules using setButtons.
*/
int KCMKttsMgr::buttons() {
    // kdDebug() << "KCMKttsMgr::buttons: Running"<< endl;
    return KCModule::Ok|KCModule::Apply|KCModule::Help|KCModule::Default;
}

/**
* This function returns the small quickhelp.
* That is displayed in the sidebar in the KControl
*/
QString KCMKttsMgr::quickHelp() const{
    // kdDebug() << "KCMKttsMgr::quickHelp: Running"<< endl;
    return i18n(
        "<h1>Text-to-Speech</h1>"
        "<p>This is the configuration for the text-to-speech dcop service</p>"
        "<p>This allows other applications to access text-to-speech resources</p>"
        "<p>Be sure to configure a default language for the language you are using as this will be the language used by most of the applications</p>");
}

const KAboutData* KCMKttsMgr::aboutData() const{
    KAboutData *about =
    new KAboutData(I18N_NOOP("kttsd"), I18N_NOOP("KCMKttsMgr"),
        0, 0, KAboutData::License_GPL,
        I18N_NOOP("(c) 2002, José Pablo Ezequiel Fernández"));

    about->addAuthor("José Pablo Ezequiel Fernández", I18N_NOOP("Author") , "pupeno@kde.org");
    about->addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer") , "garycramblitt@comcast.net");
    about->addAuthor("Olaf Schmidt", I18N_NOOP("Contributor"), "ojschmidt@kde.org");
    about->addAuthor("Paul Giannaros", I18N_NOOP("Contributor"), "ceruleanblaze@gmail.com");

    return about;
}

/**
* Loads the configuration plug in for a named talker plug in and type.
*/
PlugInConf* KCMKttsMgr::loadTalkerPlugin(const QString& name)
{
    // kdDebug() << "KCMKttsMgr::loadPlugin: Running"<< endl;

    // Find the plugin.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin",
        QString("Name == '%1'").arg(name));

    if (offers.count() == 1)
    {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if(factory){
            // If the factory is created successfully, instantiate the PlugInConf class for the
            // specific plug in to get the plug in configuration object.
            PlugInConf *plugIn = KParts::ComponentFactory::createInstanceFromLibrary<PlugInConf>(
                    offers[0]->library().latin1(), NULL, offers[0]->library().latin1());
            if(plugIn){
                // If everything went ok, return the plug in pointer.
                return plugIn;
            } else {
                // Something went wrong, returning null.
                kdDebug() << "KCMKttsMgr::loadTalkerPlugin: Unable to instantiate PlugInConf class for plugin " << name << endl;
                return NULL;
            }
        } else {
            // Something went wrong, returning null.
            kdDebug() << "KCMKttsMgr::loadTalkerPlugin: Unable to create Factory object for plugin "
                << name << endl;
            return NULL;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    kdDebug() << "KCMKttsMgr::loadTalkerPlugin: KTrader did not return an offer for plugin "
        << name << endl;
    return NULL;
}

/**
 * Loads the configuration plug in for a named filter plug in.
 */
KttsFilterConf* KCMKttsMgr::loadFilterPlugin(const QString& plugInName)
{
    // kdDebug() << "KCMKttsMgr::loadPlugin: Running"<< endl;

    // Find the plugin.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        QString("Name == '%1'").arg(plugInName));

    if (offers.count() == 1)
    {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if(factory){
            // If the factory is created successfully, instantiate the KttsFilterConf class for the
            // specific plug in to get the plug in configuration object.
            int errorNo;
            KttsFilterConf *plugIn =
                KParts::ComponentFactory::createInstanceFromLibrary<KttsFilterConf>(
                    offers[0]->library().latin1(), NULL, offers[0]->library().latin1(),
                    QStringList(), &errorNo);
            if(plugIn){
                // If everything went ok, return the plug in pointer.
                return plugIn;
            } else {
                // Something went wrong, returning null.
                kdDebug() << "KCMKttsMgr::loadFilterPlugin: Unable to instantiate KttsFilterConf class for plugin " << plugInName << " error: " << errorNo << endl;
                return NULL;
            }
        } else {
            // Something went wrong, returning null.
            kdDebug() << "KCMKttsMgr::loadFilterPlugin: Unable to create Factory object for plugin " << plugInName << endl;
            return NULL;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    kdDebug() << "KCMKttsMgr::loadFilterPlugin: KTrader did not return an offer for plugin " << plugInName << endl;
    return NULL;
}

/**
* Given an item in the talker listview and a talker code, sets the columns of the item.
* @param talkerItem       QListViewItem.
* @param talkerCode       Talker Code.
*/
void KCMKttsMgr::updateTalkerItem(QListViewItem* talkerItem, const QString &talkerCode)
{
    TalkerCode parsedTalkerCode(talkerCode);
    QString fullLanguageCode = parsedTalkerCode.fullLanguageCode();
    if (!fullLanguageCode.isEmpty())
    {
        QString language = TalkerCode::languageCodeToLanguage(fullLanguageCode);
        if (!language.isEmpty())
        {
            m_languagesToCodes[language] = fullLanguageCode;
            talkerItem->setText(tlvcLanguage, language);
        }
    }
    // Don't update the Synthesizer name with plugInName.  The former is a translated
    // name; the latter an English name.
    // if (!plugInName.isEmpty()) talkerItem->setText(tlvcSynthName, plugInName);
    if (!parsedTalkerCode.voice().isEmpty())
        talkerItem->setText(tlvcVoice, parsedTalkerCode.voice());
    if (!parsedTalkerCode.gender().isEmpty())
        talkerItem->setText(tlvcGender, TalkerCode::translatedGender(parsedTalkerCode.gender()));
    if (!parsedTalkerCode.volume().isEmpty())
        talkerItem->setText(tlvcVolume, TalkerCode::translatedVolume(parsedTalkerCode.volume()));
    if (!parsedTalkerCode.rate().isEmpty())
        talkerItem->setText(tlvcRate, TalkerCode::translatedRate(parsedTalkerCode.rate()));
}

/**
 * Add a talker.
 */
void KCMKttsMgr::slot_addTalker()
{
    AddTalker* addTalkerWidget = new AddTalker(m_synthToLangMap, this, "AddTalker_widget");
    KDialogBase* dlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Add Talker"),
        KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_kttsmgrw,
        "AddTalker_dlg",
        true,
        true);
    dlg->setMainWidget(addTalkerWidget);
    dlg->setHelp("select-plugin", "kttsd");
    int dlgResult = dlg->exec();
    QString languageCode = addTalkerWidget->getLanguageCode();
    QString synthName = addTalkerWidget->getSynthesizer();
    delete dlg;
    // TODO: Also delete addTalkerWidget?
    if (dlgResult != QDialog::Accepted) return;

    // If user chose "Other", must now get a language from him.
    if(languageCode == "other")
    {
        // Create a  QHBox to host KListView.
        QHBox* hBox = new QHBox(m_kttsmgrw, "SelectLanguage_hbox");
        // Create a KListView and fill with all known languages.
        KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
        langLView->addColumn(i18n("Language"));
        langLView->addColumn(i18n("Code"));
        QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
        QString locale;
        QString countryCode;
        QString charSet;
        QString language;
        int allLocalesCount = allLocales.count();
        for (int ndx=0; ndx < allLocalesCount; ndx++)
        {
            locale = allLocales[ndx];
            language = TalkerCode::languageCodeToLanguage(locale);
            new KListViewItem(langLView, language, locale);
        }
        // Sort by language.
        langLView->setSorting(0);
        langLView->sort();
        // Display the box in a dialog.
        KDialogBase* dlg = new KDialogBase(
            KDialogBase::Swallow,
            i18n("Select Language"),
            KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Cancel,
            m_kttsmgrw,
            "SelectLanguage_dlg",
            true,
            true);
        dlg->setMainWidget(hBox);
        dlg->setHelp("select-plugin", "kttsd");
        dlg->setInitialSize(QSize(200, 500), false);
        dlgResult = dlg->exec();
        languageCode = QString::null;
        if (langLView->selectedItem()) languageCode = langLView->selectedItem()->text(1);
        delete dlg;
        // TODO: Also delete KListView and QHBox?
        if (dlgResult != QDialog::Accepted) return;
    }

    if (languageCode.isEmpty()) return;
    QString language = TalkerCode::languageCodeToLanguage(languageCode);
    if (language.isEmpty()) return;

    m_languagesToCodes[language] = languageCode;

    // Assign a new Talker ID for the talker.  Wraps around to 1.
    QString talkerID = QString::number(m_lastTalkerID + 1);

    // Erase extraneous Talker configuration entries that might be there.
    m_config->deleteGroup(QString("Talker_")+talkerID);
    m_config->sync();

    // Convert translated plugin name to DesktopEntryName.
    QString desktopEntryName = TalkerCode::TalkerNameToDesktopEntryName(synthName);
    // This shouldn't happen, but just in case.
    if (desktopEntryName.isEmpty()) return;

    // Load the plugin.
    m_loadedTalkerPlugIn = loadTalkerPlugin(synthName);
    if (!m_loadedTalkerPlugIn) return;

    // Give plugin the user's language code and permit plugin to autoconfigure itself.
    m_loadedTalkerPlugIn->setDesiredLanguage(languageCode);
    m_loadedTalkerPlugIn->load(m_config, QString("Talker_")+talkerID);

    // If plugin was able to configure itself, it returns a full talker code.
    // If not, display configuration dialog for user to configure the plugin.
    QString talkerCode = m_loadedTalkerPlugIn->getTalkerCode();
    if (talkerCode.isEmpty())
    {
        // Display configuration dialog.
        configureTalker();
        // Did user Cancel?
        if (!m_loadedTalkerPlugIn)
        {
            m_configDlg->setMainWidget(0);
            delete m_configDlg;
            m_configDlg = 0;
            return;
        }
        talkerCode = m_loadedTalkerPlugIn->getTalkerCode();
    }

    // If still no Talker Code, abandon.
    if (!talkerCode.isEmpty())
    {
        // Let plugin save its configuration.
        m_config->setGroup(QString("Talker_")+talkerID);
        m_loadedTalkerPlugIn->save(m_config, QString("Talker_"+talkerID));

        // Record last Talker ID used for next add.
        m_lastTalkerID = talkerID.toInt();

        // Record configuration data.  Note, might as well do this now.
        m_config->setGroup(QString("Talker_")+talkerID);
        m_config->writeEntry("DesktopEntryName", desktopEntryName);
        talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, languageCode);
        m_config->writeEntry("TalkerCode", talkerCode);
        m_config->sync();

        // Add listview item.
        QListViewItem* talkerItem = m_kttsmgrw->talkersList->lastChild();
        if (talkerItem)
            talkerItem =  new KListViewItem(m_kttsmgrw->talkersList, talkerItem,
                QString::number(m_lastTalkerID), language, synthName);
        else
            talkerItem = new KListViewItem(m_kttsmgrw->talkersList,
                QString::number(m_lastTalkerID), language, synthName);

        // Set additional columns of the listview item.
        updateTalkerItem(talkerItem, talkerCode);

        // Make sure visible.
        m_kttsmgrw->talkersList->ensureItemVisible(talkerItem);

        // Select the new item, update buttons.
        m_kttsmgrw->talkersList->setSelected(talkerItem, true);
        updateTalkerButtons();

        // Inform Control Center that change has been made.
        configChanged();
    }

    // Don't need plugin in memory anymore.
    delete m_loadedTalkerPlugIn;
    m_loadedTalkerPlugIn = 0;
    if (m_configDlg)
    {
        m_configDlg->setMainWidget(0);
        delete m_configDlg;
        m_configDlg = 0;
    }

    // kdDebug() << "KCMKttsMgr::addTalker: done." << endl;
}

void KCMKttsMgr::slot_addNormalFilter()
{
    addFilter( false );
}

void KCMKttsMgr:: slot_addSbdFilter()
{
    addFilter( true );
}

/**
* Add a filter.
*/
void KCMKttsMgr::addFilter( bool sbd)
{
    // Build a list of filters that support multiple instances and let user choose.
    KListView* lView = m_kttsmgrw->filtersList;
    if (sbd) lView = m_kttsmgrw->sbdsList;

    QStringList filterPlugInNames;
    QListViewItem* item = lView->firstChild();
    while (item)
    {
        if (item->text(flvcMultiInstance) == "T")
        {
            if (!filterPlugInNames.contains(item->text(flvcPlugInName)))
                filterPlugInNames.append(item->text(flvcPlugInName));
        }
        item = item->nextSibling();
    }
    // Append those available plugins not yet in the list at all.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin");
    for (unsigned int i=0; i < offers.count() ; ++i)
    {
        QString filterPlugInName = offers[i]->name();
        if (countFilterPlugins(filterPlugInName) == 0)
        {
            KttsFilterConf* filterConf = loadFilterPlugin( filterPlugInName );
            if (filterConf)
            {
                if (filterConf->isSBD() == sbd)
                    filterPlugInNames.append(filterPlugInName);
                delete filterConf;
            }
        }
    }

    // If no choice (shouldn't happen), bail out.
    // kdDebug() << "KCMKttsMgr::addFilter: filterPluginNames = " << filterPlugInNames << endl;
    if (filterPlugInNames.count() == 0) return;

    // If exactly one choice, skip selection dialog, otherwise display list to user to select from.
    bool okChosen = false;
    QString filterPlugInName;
    if (filterPlugInNames.count() > 1)
    {
        filterPlugInName = KInputDialog::getItem(
            i18n("Select Filter"),
            i18n("Filter"),
            filterPlugInNames,
            0,
            false,
            &okChosen,
            m_kttsmgrw,
            "selectfilter_kttsd");
        if (!okChosen) return;
    } else
        filterPlugInName = filterPlugInNames[0];

    // Assign a new Filter ID for the filter.  Wraps around to 1.
    QString filterID = QString::number(m_lastFilterID + 1);

    // Erase extraneous Filter configuration entries that might be there.
    m_config->deleteGroup(QString("Filter_")+filterID);
    m_config->sync();

    // Get DesktopEntryName from the translated name.
    QString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
    // This shouldn't happen, but just in case.
    if (desktopEntryName.isEmpty()) return;

    // Load the plugin.
    m_loadedFilterPlugIn = loadFilterPlugin(filterPlugInName);
    if (!m_loadedFilterPlugIn) return;

    // Permit plugin to autoconfigure itself.
    m_loadedFilterPlugIn->load(m_config, QString("Filter_")+filterID);

    // Display configuration dialog for user to configure the plugin.
    configureFilter();

    // Did user Cancel?
    if (!m_loadedFilterPlugIn)
    {
        m_configDlg->setMainWidget(0);
        delete m_configDlg;
        m_configDlg = 0;
        return;
    }

    // Get user's name for Filter.
    QString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save its configuration.
    if ( !userFilterName.isEmpty() )
    {
        // Let plugin save its configuration.
        m_config->setGroup(QString("Filter_")+filterID);
        m_loadedFilterPlugIn->save(m_config, QString("Filter_"+filterID));

        // Record last Filter ID used for next add.
        m_lastFilterID = filterID.toInt();

        // Determine if filter supports multiple instances.
        bool multiInstance = m_loadedFilterPlugIn->supportsMultiInstance();

        // Record configuration data.  Note, might as well do this now.
        m_config->setGroup(QString("Filter_")+filterID);
        m_config->writeEntry("DesktopEntryName", desktopEntryName);
        m_config->writeEntry("UserFilterName", userFilterName);
        m_config->writeEntry("MultiInstance", multiInstance);
        m_config->writeEntry("Enabled", true);
        m_config->writeEntry("IsSBD", sbd);
        m_config->sync();

        // Add listview item.
        QListViewItem* filterItem = lView->lastChild();
        if (sbd)
        {
            if (filterItem)
                filterItem = new KListViewItem( lView, filterItem, userFilterName );
            else
                filterItem = new KListViewItem( lView, userFilterName );
        }
        else
        {
            if (filterItem)
                filterItem = new KttsCheckListItem(lView, filterItem,
                    userFilterName, QCheckListItem::CheckBox, this);
            else
                filterItem = new KttsCheckListItem(lView,
                    userFilterName, QCheckListItem::CheckBox, this);
            dynamic_cast<QCheckListItem*>(filterItem)->setOn(true);
        }
        filterItem->setText(flvcFilterID, QString::number(m_lastFilterID));
        filterItem->setText(flvcPlugInName, filterPlugInName);
        if (multiInstance)
            filterItem->setText(flvcMultiInstance, "T");
        else
            filterItem->setText(flvcMultiInstance, "F");

        // Make sure visible.
        lView->ensureItemVisible(filterItem);

        // Select the new item, update buttons.
        lView->setSelected(filterItem, true);
        if (sbd)
            updateSbdButtons();
        else
            updateFilterButtons();

        // Inform Control Center that change has been made.
        configChanged();
    }

    // Don't need plugin in memory anymore.
    delete m_loadedFilterPlugIn;
    m_loadedFilterPlugIn = 0;
    m_configDlg->setMainWidget(0);
    delete m_configDlg;
    m_configDlg = 0;

    // kdDebug() << "KCMKttsMgr::addFilter: done." << endl;
}

/**
* Remove talker.
*/
void KCMKttsMgr::slot_removeTalker(){
    // kdDebug() << "KCMKttsMgr::removeTalker: Running"<< endl;

    // Get the selected talker.
    QListViewItem *itemToRemove = m_kttsmgrw->talkersList->selectedItem();
    if (!itemToRemove) return;

    // Delete the talker from configuration file.
//    QString talkerID = itemToRemove->text(tlvcTalkerID);
//    m_config->deleteGroup("Talker_"+talkerID, true, false);

    // Delete the talker from list view.
    delete itemToRemove;

    updateTalkerButtons();

    // Emit configuraton changed.
    configChanged();
}

void KCMKttsMgr::slot_removeNormalFilter()
{
    removeFilter( false );
}

void KCMKttsMgr::slot_removeSbdFilter()
{
    removeFilter( true );
}

/**
* Remove filter.
*/
void KCMKttsMgr::removeFilter( bool sbd )
{
    // kdDebug() << "KCMKttsMgr::removeFilter: Running"<< endl;

    KListView* lView = m_kttsmgrw->filtersList;
    if (sbd) lView = m_kttsmgrw->sbdsList;
    // Get the selected filter.
    QListViewItem *itemToRemove = lView->selectedItem();
    if (!itemToRemove) return;

    // Delete the filter from configuration file.
//    QString filterID = itemToRemove->text(flvcFilterID);
//    m_config->deleteGroup("Filter_"+filterID, true, false);

    // Delete the filter from list view.
    delete itemToRemove;

    if (sbd)
        updateSbdButtons();
    else
        updateFilterButtons();

    // Emit configuraton changed.
    configChanged();
}

void KCMKttsMgr::slot_higherTalkerPriority()
{
    higherItemPriority( m_kttsmgrw->talkersList );
    updateTalkerButtons();
}

void KCMKttsMgr::slot_higherNormalFilterPriority()
{
    higherItemPriority( m_kttsmgrw->filtersList );
    updateFilterButtons();
}

void KCMKttsMgr::slot_higherSbdFilterPriority()
{
    higherItemPriority( m_kttsmgrw->sbdsList );
    updateSbdButtons();
}

/**
* This is called whenever user clicks the Up button.
*/
void KCMKttsMgr::higherItemPriority( KListView* lView )
{
    QListViewItem* item = lView->selectedItem();
    if (!item) return;
    QListViewItem* prevItem = item->itemAbove();
    if (!prevItem) return;
    prevItem->moveItem(item);
    lView->setSelected(item, true);
    lView->ensureItemVisible( item );
    configChanged();
}

void KCMKttsMgr::slot_lowerTalkerPriority()
{
    lowerItemPriority( m_kttsmgrw->talkersList );
    updateTalkerButtons();
}

void KCMKttsMgr::slot_lowerNormalFilterPriority()
{
    lowerItemPriority( m_kttsmgrw->filtersList );
    updateFilterButtons();
}

void KCMKttsMgr::slot_lowerSbdFilterPriority()
{
    lowerItemPriority( m_kttsmgrw->sbdsList );
    updateSbdButtons();
}

/**
* This is called whenever user clicks the Down button.
*/
void KCMKttsMgr::lowerItemPriority( KListView* lView )
{
    QListViewItem* item = lView->selectedItem();
    if (!item) return;
    QListViewItem* nextItem = item->itemBelow();
    if (!nextItem) return;
    item->moveItem(nextItem);
    lView->setSelected(item, true);
    lView->ensureItemVisible( item );
    configChanged();
}

/**
* Update the status of the Talker buttons.
*/
void KCMKttsMgr::updateTalkerButtons(){
    // kdDebug() << "KCMKttsMgr::updateTalkerButtons: Running"<< endl;
    if(m_kttsmgrw->talkersList->selectedItem()){
        m_kttsmgrw->removeTalkerButton->setEnabled(true);
        m_kttsmgrw->configureTalkerButton->setEnabled(true);
        m_kttsmgrw->higherTalkerPriorityButton->setEnabled(
            m_kttsmgrw->talkersList->selectedItem()->itemAbove() != 0);
        m_kttsmgrw->lowerTalkerPriorityButton->setEnabled(
            m_kttsmgrw->talkersList->selectedItem()->itemBelow() != 0);
    } else {
        m_kttsmgrw->removeTalkerButton->setEnabled(false);
        m_kttsmgrw->configureTalkerButton->setEnabled(false);
        m_kttsmgrw->higherTalkerPriorityButton->setEnabled(false);
        m_kttsmgrw->lowerTalkerPriorityButton->setEnabled(false);
    }
    // kdDebug() << "KCMKttsMgr::updateTalkerButtons: Exiting"<< endl;
}

/**
* Update the status of the normal Filter buttons.
*/
void KCMKttsMgr::updateFilterButtons(){
    // kdDebug() << "KCMKttsMgr::updateFilterButtons: Running"<< endl;
    QListViewItem* item = m_kttsmgrw->filtersList->selectedItem();
    if (item) {
        m_kttsmgrw->removeFilterButton->setEnabled(true);
        m_kttsmgrw->configureFilterButton->setEnabled(true);
        m_kttsmgrw->higherFilterPriorityButton->setEnabled(
                m_kttsmgrw->filtersList->selectedItem()->itemAbove() != 0);
        m_kttsmgrw->lowerFilterPriorityButton->setEnabled(
                m_kttsmgrw->filtersList->selectedItem()->itemBelow() != 0);
    } else {
        m_kttsmgrw->removeFilterButton->setEnabled(false);
        m_kttsmgrw->configureFilterButton->setEnabled(false);
        m_kttsmgrw->higherFilterPriorityButton->setEnabled(false);
        m_kttsmgrw->lowerFilterPriorityButton->setEnabled(false);
    }
    // kdDebug() << "KCMKttsMgr::updateFilterButtons: Exiting"<< endl;
}

/**
 * Update the status of the SBD buttons.
 */
void KCMKttsMgr::updateSbdButtons(){
    // kdDebug() << "KCMKttsMgr::updateSbdButtons: Running"<< endl;
    QListViewItem* item = m_kttsmgrw->sbdsList->selectedItem();
    if (item) {
        m_sbdPopmenu->setItemEnabled( sbdBtnEdit, true );
        m_sbdPopmenu->setItemEnabled( sbdBtnUp,
            m_kttsmgrw->sbdsList->selectedItem()->itemAbove() != 0 );
        m_sbdPopmenu->setItemEnabled( sbdBtnDown,
            m_kttsmgrw->sbdsList->selectedItem()->itemBelow() != 0 );
        m_sbdPopmenu->setItemEnabled( sbdBtnRemove, true );
    } else {
        m_sbdPopmenu->setItemEnabled( sbdBtnEdit, false );
        m_sbdPopmenu->setItemEnabled( sbdBtnUp, false );
        m_sbdPopmenu->setItemEnabled( sbdBtnDown, false );
        m_sbdPopmenu->setItemEnabled( sbdBtnRemove, false );
    }
    // kdDebug() << "KCMKttsMgr::updateSbdButtons: Exiting"<< endl;
}

/**
* This signal is emitted whenever user checks/unchecks the Enable TTS System check box.
*/
void KCMKttsMgr::enableKttsdToggled(bool)
{
    // Prevent re-entrancy.
    static bool reenter;
    if (reenter) return;
    reenter = true;
    // See if KTTSD is running.
    DCOPClient *client = kapp->dcopClient();
    bool kttsdRunning = (client->isApplicationRegistered("kttsd"));
    // kdDebug() << "KCMKttsMgr::enableKttsdToggled: kttsdRunning = " << kttsdRunning << endl;
    // If Enable KTTSD check box is checked and it is not running, then start KTTSD.
    if (m_kttsmgrw->enableKttsdCheckBox->isChecked())
    {
        if (!kttsdRunning)
        {
            // kdDebug() << "KCMKttsMgr::enableKttsdToggled:: Starting KTTSD" << endl;
            QString error;
            if (KApplication::startServiceByDesktopName("kttsd", QStringList(), &error))
            {
                kdDebug() << "Starting KTTSD failed with message " << error << endl;
                m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
            }
        }
    }
    else
    // If check box is not checked and it is running, then stop KTTSD.
    {
    if (kttsdRunning)
        {
            // kdDebug() << "KCMKttsMgr::enableKttsdToggled:: Stopping KTTSD" << endl;
            QByteArray data;
            client->send("kttsd", "KSpeech", "kttsdExit()", data);
        }
    }
    reenter = false;
}

/**
* This signal is emitted whenever user checks/unchecks the GStreamer radio button.
*/
void KCMKttsMgr::slotGstreamerRadioButton_toggled(bool state)
{
    m_kttsmgrw->sinkLabel->setEnabled(state);
    m_kttsmgrw->sinkComboBox->setEnabled(state);
}


/**
* This slot is called whenever KTTSD starts or restarts.
*/
void KCMKttsMgr::kttsdStarted()
{
    // kdDebug() << "KCMKttsMgr::kttsdStarted: Running" << endl;
    bool kttsdLoaded = (m_jobMgrPart != 0);
    // Load Job Manager Part library.
    if (!kttsdLoaded)
    {
        KLibFactory *factory = KLibLoader::self()->factory( "libkttsjobmgrpart" );
        if (factory)
        {
            // Create the Job Manager part
            m_jobMgrPart = (KParts::ReadOnlyPart *)factory->create( m_kttsmgrw->mainTab, "kttsjobmgr",
                "KParts::ReadOnlyPart" );
            if (m_jobMgrPart)
            {
                // Add the Job Manager part as a new tab.
                m_kttsmgrw->mainTab->addTab(m_jobMgrPart->widget(), i18n("&Jobs"));
                kttsdLoaded = true;
            }
            else
                kdDebug() << "Could not create kttsjobmgr part." << endl;
        }
        else kdDebug() << "Could not load libkttsjobmgrpart.  Is libkttsjobmgrpart installed?" << endl;
    }
    // Check/Uncheck the Enable KTTSD check box.
    if (kttsdLoaded)
        m_kttsmgrw->enableKttsdCheckBox->setChecked(true);
    else
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
}

/**
* This slot is called whenever KTTSD is about to exit.
*/
void KCMKttsMgr::kttsdExiting()
{
    // kdDebug() << "KCMKttsMgr::kttsdExiting: Running" << endl;
    if (m_jobMgrPart)
    {
        m_kttsmgrw->mainTab->removePage(m_jobMgrPart->widget());
        delete m_jobMgrPart;
        m_jobMgrPart = 0;
    }
    m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
}

/**
* User has requested display of talker configuration dialog.
*/
void KCMKttsMgr::slot_configureTalker()
{
    // Get highlighted plugin from Talker ListView and load into memory.
    QListViewItem* talkerItem = m_kttsmgrw->talkersList->selectedItem();
    if (!talkerItem) return;
    QString talkerID = talkerItem->text(tlvcTalkerID);
    QString synthName = talkerItem->text(tlvcSynthName);
    QString language = talkerItem->text(tlvcLanguage);
    QString languageCode = m_languagesToCodes[language];
    m_loadedTalkerPlugIn = loadTalkerPlugin(synthName);
    if (!m_loadedTalkerPlugIn) return;
    // kdDebug() << "KCMKttsMgr::slot_configureTalker: plugin for " << synthName << " loaded successfully." << endl;

    // Tell plugin to load its configuration.
    m_config->setGroup(QString("Talker_")+talkerID);
    m_loadedTalkerPlugIn->setDesiredLanguage(languageCode);
    // kdDebug() << "KCMKttsMgr::slot_configureTalker: about to call plugin load() method with Talker ID = " << talkerID << endl;
    m_loadedTalkerPlugIn->load(m_config, QString("Talker_")+talkerID);

    // Display configuration dialog.
    configureTalker();

    // Did user Cancel?
    if (!m_loadedTalkerPlugIn)
    {
        m_configDlg->setMainWidget(0);
        delete m_configDlg;
        m_configDlg = 0;
        return;
    }

    // Get Talker Code.  Note that plugin may return a code different from before.
    QString talkerCode = m_loadedTalkerPlugIn->getTalkerCode();

    // If plugin was successfully configured, save its configuration.
    if (!talkerCode.isEmpty())
    {
        m_config->setGroup(QString("Talker_")+talkerID);
        m_loadedTalkerPlugIn->save(m_config, QString("Talker_")+talkerID);
        m_config->setGroup(QString("Talker_")+talkerID);
        talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, languageCode);
        m_config->writeEntry("TalkerCode", talkerCode);
        m_config->sync();

        // Update display.
        updateTalkerItem(talkerItem, talkerCode);

        // Inform Control Center that configuration has changed.
        configChanged();
    }

    delete m_loadedTalkerPlugIn;
    m_loadedTalkerPlugIn = 0;
    m_configDlg->setMainWidget(0);
    delete m_configDlg;
    m_configDlg = 0;
}

void KCMKttsMgr::slot_configureNormalFilter()
{
    configureFilterItem( false );
}

void KCMKttsMgr::slot_configureSbdFilter()
{
    configureFilterItem( true );
}

/**
 * User has requested display of filter configuration dialog.
 */
void KCMKttsMgr::configureFilterItem( bool sbd )
{
    // Get highlighted plugin from Filter ListView and load into memory.
    KListView* lView = m_kttsmgrw->filtersList;
    if (sbd) lView = m_kttsmgrw->sbdsList;
    QListViewItem* filterItem = lView->selectedItem();
    if (!filterItem) return;
    QString filterID = filterItem->text(flvcFilterID);
    QString filterPlugInName = filterItem->text(flvcPlugInName);
    QString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
    if (desktopEntryName.isEmpty()) return;
    m_loadedFilterPlugIn = loadFilterPlugin(filterPlugInName);
    if (!m_loadedFilterPlugIn) return;
    // kdDebug() << "KCMKttsMgr::slot_configureFilter: plugin for " << filterPlugInName << " loaded successfully." << endl;

    // Tell plugin to load its configuration.
    m_config->setGroup(QString("Filter_")+filterID);
    // kdDebug() << "KCMKttsMgr::slot_configureFilter: about to call plugin load() method with Filter ID = " << filterID << endl;
    m_loadedFilterPlugIn->load(m_config, QString("Filter_")+filterID);

    // Display configuration dialog.
    configureFilter();

    // Did user Cancel?
    if (!m_loadedFilterPlugIn)
    {
        m_configDlg->setMainWidget(0);
        delete m_configDlg;
        m_configDlg = 0;
        return;
    }

    // Get user's name for the plugin.
    QString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save the configuration.
    if ( !userFilterName.isEmpty() )
    {

        // Let plugin save its configuration.
        m_config->setGroup(QString("Filter_")+filterID);
        m_loadedFilterPlugIn->save(m_config, QString("Filter_")+filterID);

        // Save configuration.
        m_config->setGroup("Filter_"+filterID);
        m_config->writeEntry("DesktopEntryName", desktopEntryName);
        m_config->writeEntry("UserFilterName", userFilterName);
        m_config->writeEntry("Enabled", true);
        m_config->writeEntry("MultiInstance", m_loadedFilterPlugIn->supportsMultiInstance());
        m_config->writeEntry("IsSBD", sbd);

        m_config->sync();

        // Update display.
        filterItem->setText(flvcUserName, userFilterName);
        if (!sbd)
            dynamic_cast<QCheckListItem*>(filterItem)->setOn(true);

        // Inform Control Center that configuration has changed.
        configChanged();
    }

    delete m_loadedFilterPlugIn;
    m_loadedFilterPlugIn = 0;
    m_configDlg->setMainWidget(0);
    delete m_configDlg;
    m_configDlg = 0;
}

/**
* Display talker configuration dialog.  The plugin is assumed already loaded into
* memory referenced by m_loadedTalkerPlugIn.
*/
void KCMKttsMgr::configureTalker()
{
    if (!m_loadedTalkerPlugIn) return;
    m_configDlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Talker Configuration"),
        KDialogBase::Help|KDialogBase::Default|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_kttsmgrw,
        "configureTalker_dlg",
        true,
        true);
    m_configDlg->setInitialSize(QSize(700, 300), false);
    m_configDlg->setMainWidget(m_loadedTalkerPlugIn);
    m_configDlg->setHelp("configure-plugin", "kttsd");
    m_configDlg->enableButtonOK(false);
    connect(m_loadedTalkerPlugIn, SIGNAL( changed(bool) ), this, SLOT( slotConfigTalkerDlg_ConfigChanged() ));
    connect(m_configDlg, SIGNAL( defaultClicked() ), this, SLOT( slotConfigTalkerDlg_DefaultClicked() ));
    connect(m_configDlg, SIGNAL( cancelClicked() ), this, SLOT (slotConfigTalkerDlg_CancelClicked() ));
    // Create a Player object for the plugin to use for testing.
    int playerOption = 0;
    if (m_kttsmgrw->gstreamerRadioButton->isChecked()) playerOption = 1;
    float audioStretchFactor = 1.0/(float(m_kttsmgrw->timeBox->value())/100.0);
    QString sinkName = m_kttsmgrw->sinkComboBox->currentText();
    // kdDebug() << "KCMKttsMgr::configureTalker: playerOption = " << playerOption << " audioStretchFactor = " << audioStretchFactor << " sink name = " << sinkName << endl;
    TestPlayer* testPlayer = new TestPlayer(this, "ktts_testplayer", 
        playerOption, audioStretchFactor, sinkName);
    m_loadedTalkerPlugIn->setPlayer(testPlayer);
    // Display the dialog.
    m_configDlg->exec();
    // Done with Player object.
    if (m_loadedTalkerPlugIn)
    {
        delete testPlayer;
        m_loadedTalkerPlugIn->setPlayer(0);
    }
}

/**
* Display filter configuration dialog.  The plugin is assumed already loaded into
* memory referenced by m_loadedFilterPlugIn.
*/
void KCMKttsMgr::configureFilter()
{
    if (!m_loadedFilterPlugIn) return;
    m_configDlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Filter Configuration"),
        KDialogBase::Help|KDialogBase::Default|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_kttsmgrw,
        "configureFilter_dlg",
        true,
        true);
    m_configDlg->setInitialSize(QSize(600, 450), false);
    m_loadedFilterPlugIn->setMinimumSize(m_loadedFilterPlugIn->minimumSizeHint());
    m_loadedFilterPlugIn->show();
    m_configDlg->setMainWidget(m_loadedFilterPlugIn);
    m_configDlg->setHelp("configure-filter", "kttsd");
    m_configDlg->enableButtonOK(false);
    connect(m_loadedFilterPlugIn, SIGNAL( changed(bool) ), this, SLOT( slotConfigFilterDlg_ConfigChanged() ));
    connect(m_configDlg, SIGNAL( defaultClicked() ), this, SLOT( slotConfigFilterDlg_DefaultClicked() ));
    connect(m_configDlg, SIGNAL( cancelClicked() ), this, SLOT (slotConfigFilterDlg_CancelClicked() ));
    // Display the dialog.
    m_configDlg->exec();
}

/**
* Count number of configured Filters with the specified plugin name.
*/
int KCMKttsMgr::countFilterPlugins(const QString& filterPlugInName)
{
    int cnt = 0;
    QListViewItem* item = m_kttsmgrw->filtersList->firstChild();
    while (item)
    {
        if (item->text(flvcPlugInName) == filterPlugInName) ++cnt;
        item = item->nextSibling();
    }
    item = m_kttsmgrw->sbdsList->firstChild();
    while (item)
    {
        if (item->text(slvcPlugInName) == filterPlugInName) ++cnt;
        item = item->nextSibling();
    }
    return cnt;
}

void KCMKttsMgr::keepAudioCheckBox_toggled(bool checked)
{
    m_kttsmgrw->keepAudioPath->setEnabled(checked);
    configChanged();
}

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int KCMKttsMgr::percentToSlider(int percentValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int KCMKttsMgr::sliderToPercent(int sliderValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void KCMKttsMgr::timeBox_valueChanged(int percentValue) {
    m_kttsmgrw->timeSlider->setValue (percentToSlider (percentValue));
}

void KCMKttsMgr::timeSlider_valueChanged(int sliderValue) {
    m_kttsmgrw->timeBox->setValue (sliderToPercent (sliderValue));
}

void KCMKttsMgr::slotConfigTalkerDlg_ConfigChanged()
{
    m_configDlg->enableButtonOK(!m_loadedTalkerPlugIn->getTalkerCode().isEmpty());
}

void KCMKttsMgr::slotConfigFilterDlg_ConfigChanged()
{
    m_configDlg->enableButtonOK( !m_loadedFilterPlugIn->userPlugInName().isEmpty() );
}

void KCMKttsMgr::slotConfigTalkerDlg_DefaultClicked()
{
    m_loadedTalkerPlugIn->defaults();
}

void KCMKttsMgr::slotConfigFilterDlg_DefaultClicked()
{
    m_loadedFilterPlugIn->defaults();
}

void KCMKttsMgr::slotConfigTalkerDlg_CancelClicked()
{
    delete m_loadedTalkerPlugIn;
    m_loadedTalkerPlugIn = 0;
}

void KCMKttsMgr::slotConfigFilterDlg_CancelClicked()
{
    delete m_loadedFilterPlugIn;
    m_loadedFilterPlugIn = 0;
}

/**
* This slot is called whenever user checks/unchecks item in Filters list.
*/
void KCMKttsMgr::slotFiltersList_stateChanged()
{
    // kdDebug() << "KCMKttsMgr::slotFiltersList_stateChanged: calling configChanged" << endl;
    configChanged();
}

/**
 * Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
 * @param name                   The translated plugin name.  From Name= line in .desktop file.
 * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
 *                               QString::null if not found.
 */
QString KCMKttsMgr::FilterNameToDesktopEntryName(const QString& name)
{
    if (name.isEmpty()) return QString::null;
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        QString("Name == '%1'").arg(name));

    if (offers.count() == 1)
        return offers[0]->desktopEntryName();
    else
        return QString::null;
}

/**
 * Uses KTrader to convert a DesktopEntryName into a translated Filter Plugin Name.
 * @param desktopEntryName       The DesktopEntryName.
 * @return                       The translated Name of the plugin, from Name= line in .desktop file.
 */
QString KCMKttsMgr::FilterDesktopEntryNameToName(const QString& desktopEntryName)
{
    if (desktopEntryName.isEmpty()) return QString::null;
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        QString("DesktopEntryName == '%1'").arg(desktopEntryName));

    if (offers.count() == 1)
        return offers[0]->name();
    else
        return QString::null;
}

/**
 * Loads notify events from a file.  Clearing listview if clear is True.
 */
QString KCMKttsMgr::loadNotifyEventsFromFile( const QString& filename, bool clear)
{
    // Open existing event list.
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
    {
        return i18n("Unable to open file.") + filename;
    }
    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        return i18n("File not in proper XML format.");
    }
    // kdDebug() << "StringReplacerConf::load: document successfully parsed." << endl;
    file.close();

    // Clear list view.
    if ( clear ) m_kttsmgrw->notifyListView->clear();

    // Event list.
    QDomNodeList eventList = doc.elementsByTagName("notifyEvent");
    int eventListCount = eventList.count();
    for (int eventIndex = 0; eventIndex < eventListCount; eventIndex++)
    {
        QDomNode eventNode = eventList.item(eventIndex);
        QDomNodeList propList = eventNode.childNodes();
        QString eventSrc;
        QString event;
        QString actionName;
        QString message;
        TalkerCode talkerCode;
        int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; propIndex++)
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "eventSrc") eventSrc = prop.text();
            if (prop.tagName() == "event") event = prop.text();
            if (prop.tagName() == "action") actionName = prop.text();
            if (prop.tagName() == "message") message = prop.text();
            if (prop.tagName() == "talker") talkerCode = TalkerCode(prop.text(), false);
        }
        addNotifyItem(eventSrc, event, NotifyAction::action( actionName ), message, talkerCode);
    }

    return QString::null;
}

/**
 * Saves notify events to a file.
 */
QString KCMKttsMgr::saveNotifyEventsToFile(const QString& filename)
{
    QFile file( filename );
    if ( !file.open( IO_WriteOnly ) )
        return i18n("Unable to open file ") + filename;

    QDomDocument doc( "" );

    QDomElement root = doc.createElement( "notifyEventList" );
    doc.appendChild( root );

    // Events.
    KListView* lv = m_kttsmgrw->notifyListView;
    QListViewItemIterator it(lv);
    while ( it.current() )
    {
        QListViewItem* item = *it;
        if ( item->depth() > 0 )
        {
            QDomElement wordTag = doc.createElement( "notifyEvent" );
            root.appendChild( wordTag );

            QDomElement propTag = doc.createElement( "eventSrc" );
            wordTag.appendChild( propTag);
            QDomText t = doc.createTextNode( item->text(nlvcEventSrc) );
            propTag.appendChild( t );

            propTag = doc.createElement( "event" );
            wordTag.appendChild( propTag);
            t = doc.createTextNode( item->text(nlvcEvent) );
            propTag.appendChild( t );

            propTag = doc.createElement( "action" );
            wordTag.appendChild( propTag);
            t = doc.createTextNode( item->text(nlvcAction) );
            propTag.appendChild( t );

            if ( item->text(nlvcAction) == NotifyAction::actionName( NotifyAction::SpeakCustom ) )
            {
                propTag = doc.createElement( "message" );
                wordTag.appendChild( propTag);
                QString msg = item->text(nlvcActionName);
                int msglen = msg.length();
                msg = msg.mid( 1, msglen-2 );
                t = doc.createCDATASection( msg );
                propTag.appendChild( t );
            }

            propTag = doc.createElement( "talker" );
            wordTag.appendChild( propTag);
            t = doc.createCDATASection( item->text(nlvcTalker) );
            propTag.appendChild( t );
        }
        ++it;
    }

    // Write it all out.
    QTextStream ts( &file );
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ts << doc.toString();
    file.close();

    return QString::null;
}

void KCMKttsMgr::slotNotifyEnableCheckBox_toggled(bool checked)
{
    m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->setEnabled( checked );
    m_kttsmgrw->notifyGroup->setEnabled( checked );
    configChanged();
}

void KCMKttsMgr::slotNotifyPresentComboBox_activated(int index)
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( !item ) return;        // should not happen
    item->setText( nlvcEvent, NotifyPresent::presentName( index ) );
    item->setText( nlvcEventName, NotifyPresent::presentDisplayName( index ) );
    bool enableIt = ( index != NotifyPresent::None);
    m_kttsmgrw->notifyActionComboBox->setEnabled( enableIt );
    m_kttsmgrw->notifyTalkerButton->setEnabled( enableIt );
    if (!enableIt)
    {
        m_kttsmgrw->notifyTalkerLineEdit->clear();
    } else {
        if ( m_kttsmgrw->notifyTalkerLineEdit->text().isEmpty() )
        {
            m_kttsmgrw->notifyTalkerLineEdit->setText( i18n("default") );
        }
    }
    configChanged();
}

void KCMKttsMgr::slotNotifyListView_selectionChanged()
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
    {
        bool topLevel = ( item->depth() == 0 );
        if ( topLevel )
        {
            m_kttsmgrw->notifyPresentComboBox->setEnabled( false );
            m_kttsmgrw->notifyActionComboBox->setEnabled( false );
            m_kttsmgrw->notifyTestButton->setEnabled( false );
            m_kttsmgrw->notifyMsgLineEdit->setEnabled( false );
            m_kttsmgrw->notifyMsgLineEdit->clear();
            m_kttsmgrw->notifyTalkerButton->setEnabled( false );
            m_kttsmgrw->notifyTalkerLineEdit->clear();
            bool defaultItem = ( item->text(nlvcEventSrc) == "default" );
            m_kttsmgrw->notifyRemoveButton->setEnabled( !defaultItem );
        } else {
            bool defaultItem = ( item->parent()->text(nlvcEventSrc) == "default" );
            m_kttsmgrw->notifyPresentComboBox->setEnabled( defaultItem );
            if ( defaultItem )
                m_kttsmgrw->notifyPresentComboBox->setCurrentItem( NotifyPresent::present( item->text( nlvcEvent ) ) );
            m_kttsmgrw->notifyActionComboBox->setEnabled( true );
            int action = NotifyAction::action( item->text( nlvcAction ) );
            m_kttsmgrw->notifyActionComboBox->setCurrentItem( action );
            m_kttsmgrw->notifyTalkerButton->setEnabled( true );
            TalkerCode talkerCode( item->text( nlvcTalker ) );
            m_kttsmgrw->notifyTalkerLineEdit->setText( talkerCode.getTranslatedDescription() );
            if ( action == NotifyAction::SpeakCustom )
            {
                m_kttsmgrw->notifyMsgLineEdit->setEnabled( true );
                QString msg = item->text( nlvcActionName );
                int msglen = msg.length();
                msg = msg.mid( 1, msglen-2 );
                m_kttsmgrw->notifyMsgLineEdit->setText( msg );
                m_kttsmgrw->notifyTestButton->setEnabled( 
                    !m_kttsmgrw->notifyMsgLineEdit->text().isEmpty() );
            } else {
                m_kttsmgrw->notifyTestButton->setEnabled( false );
                m_kttsmgrw->notifyMsgLineEdit->setEnabled( false );
                m_kttsmgrw->notifyMsgLineEdit->clear();
            }
            m_kttsmgrw->notifyRemoveButton->setEnabled( !defaultItem );
        }
    } else {
        m_kttsmgrw->notifyPresentComboBox->setEnabled( false );
        m_kttsmgrw->notifyActionComboBox->setEnabled( false );
        m_kttsmgrw->notifyTestButton->setEnabled( false );
        m_kttsmgrw->notifyMsgLineEdit->setEnabled( false );
        m_kttsmgrw->notifyMsgLineEdit->clear();
        m_kttsmgrw->notifyTalkerButton->setEnabled( false );
        m_kttsmgrw->notifyTalkerLineEdit->clear();
        m_kttsmgrw->notifyRemoveButton->setEnabled( false );
    }
}

void KCMKttsMgr::slotNotifyActionComboBox_activated(int index)
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
        if ( item->depth() == 0 ) item = 0;
    if ( !item ) return;  // This shouldn't happen.
    item->setText( nlvcAction, NotifyAction::actionName( index ) );
    item->setText( nlvcActionName, NotifyAction::actionDisplayName( index ) );
    if ( index == NotifyAction::SpeakCustom )
        item->setText( nlvcActionName, "\"" + m_kttsmgrw->notifyMsgLineEdit->text() + "\"" );
    if ( index == NotifyAction::DoNotSpeak )
        item->setPixmap( nlvcActionName, SmallIcon( "nospeak" ) );
    else
        item->setPixmap( nlvcActionName, SmallIcon( "speak" ) );
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyMsgLineEdit_textChanged(const QString& text)
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
        if ( item->depth() == 0 ) item = 0;
    if ( !item ) return;  // This shouldn't happen.
    if ( m_kttsmgrw->notifyActionComboBox->currentItem() != NotifyAction::SpeakCustom) return;
    item->setText( nlvcActionName, "\"" + text + "\"" );
    m_kttsmgrw->notifyTestButton->setEnabled( !text.isEmpty() );
    configChanged();
}

void KCMKttsMgr::slotNotifyTestButton_clicked()
{
    // TODO:
}

void KCMKttsMgr::slotNotifyTalkerButton_clicked()
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
        if ( item->depth() == 0 ) item = 0;
    if ( !item ) return;  // This shouldn't happen.
    QString talkerCode = item->text( nlvcTalker );
    SelectTalkerDlg dlg( m_kttsmgrw, "selecttalkerdialog", i18n("Select Talker"), talkerCode, true );
    int dlgResult = dlg.exec();
    if ( dlgResult != KDialogBase::Accepted ) return;
    item->setText( nlvcTalker, dlg.getSelectedTalkerCode() );
    QString talkerName = dlg.getSelectedTranslatedDescription();
    item->setText( nlvcTalkerName, talkerName );
    m_kttsmgrw->notifyTalkerLineEdit->setText( talkerName );
    configChanged();
}

/**
 * Adds an item to the notify listview.
 * message is only needed if action = naSpeakCustom.
 */
QListViewItem* KCMKttsMgr::addNotifyItem(
    const QString& eventSrc,
    const QString& event,
    int action,
    const QString& message,
    TalkerCode& talkerCode)
{
    KListView* lv = m_kttsmgrw->notifyListView;
    QListViewItem* item = 0;
    QString iconName;
    QString eventSrcName;
    if (eventSrc == "default")
        eventSrcName = i18n("Default (all other events)");
    else
        eventSrcName = NotifyEvent::getEventSrcName(eventSrc, iconName);
    QString eventName;
    if (eventSrc == "default")
        eventName = NotifyPresent::presentDisplayName( event );
    else
        eventName = NotifyEvent::getEventName(eventSrc, event);
    QString actionName = NotifyAction::actionName( action );
    QString actionDisplayName = NotifyAction::actionDisplayName( action );
    if (action == NotifyAction::SpeakCustom) actionName = "\"" + message + "\"";
    QString talkerName = talkerCode.getTranslatedDescription();
    if (!eventSrcName.isEmpty() && !eventName.isEmpty() && !actionName.isEmpty() && !talkerName.isEmpty())
    {
        QListViewItem* parentItem = lv->findItem(eventSrcName, nlvcEventSrcName);
        if (!parentItem)
        {
            item = lv->lastItem();
            if (!item)
                parentItem = new KListViewItem(lv, eventSrcName, QString::null, QString::null,
                    eventSrc);
            else
                parentItem = new KListViewItem(lv, item, eventSrcName, QString::null, QString::null,
                    eventSrc);
            if ( !iconName.isEmpty() )
                parentItem->setPixmap( nlvcEventSrcName, SmallIcon( iconName ) );
        }
        // No duplicates.
        item = lv->findItem( event, nlvcEvent );
        if ( !item || item->parent() != parentItem )
            item = new KListViewItem(parentItem, eventName, actionDisplayName, talkerName,
                eventSrc, event, actionName, talkerCode.getTalkerCode());
        if ( action == NotifyAction::DoNotSpeak )
            item->setPixmap( nlvcActionName, SmallIcon( "nospeak" ) );
        else
            item->setPixmap( nlvcActionName, SmallIcon( "speak" ) );
    }
    return item;
}

void KCMKttsMgr::slotNotifyAddButton_clicked()
{
    QListView* lv = m_kttsmgrw->notifyListView;
    QListViewItem* item = lv->selectedItem();
    QString eventSrc;
    if ( item ) eventSrc = item->text( nlvcEventSrc );
    SelectEvent* selectEventWidget = new SelectEvent( this, "SelectEvent_widget", 0, eventSrc );
    KDialogBase* dlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Select Event"),
        KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_kttsmgrw,
        "SelectEvent_dlg",
        true,
        true);
    dlg->setMainWidget( selectEventWidget );
    dlg->setInitialSize( QSize(500, 400) );
    // dlg->setHelp("select-plugin", "kttsd");
    int dlgResult = dlg->exec();
    eventSrc = selectEventWidget->getEventSrc();
    QString event = selectEventWidget->getEvent();
    delete dlg;
    if ( dlgResult != QDialog::Accepted ) return;
    if ( eventSrc.isEmpty() || event.isEmpty() ) return;
    // Use Default action, message, and talker.
    QString actionName;
    int action;
    QString msg;
    TalkerCode talkerCode;
    item = lv->findItem( "default", nlvcEventSrc );
    if ( item )
    {
        if ( item->childCount() > 0 ) item = item->firstChild();
        if ( item )
        {
            actionName = item->text( nlvcAction );
            action = NotifyAction::action( actionName );
            talkerCode = TalkerCode( item->text( nlvcTalker ) );
            if (action == NotifyAction::SpeakCustom )
            {
                msg = item->text(nlvcActionName);
                int msglen = msg.length();
                msg = msg.mid( 1, msglen-2 );
            }
        }
    }
    item = addNotifyItem( eventSrc, event, action, msg, talkerCode );
    lv->ensureItemVisible( item );
    lv->setSelected( item, true );
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyClearButton_clicked()
{
    m_kttsmgrw->notifyListView->clear();
    TalkerCode talkerCode( QString::null );
    QListViewItem* item = addNotifyItem(
        QString("default"),
        NotifyPresent::presentName(NotifyPresent::Passive),
        NotifyAction::SpeakEventName,
        QString::null,
        talkerCode );
    QListView* lv = m_kttsmgrw->notifyListView;
    lv->ensureItemVisible( item );
    lv->setSelected( item, true );
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyRemoveButton_clicked()
{
    QListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    delete item;
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyLoadButton_clicked()
{
    // QString dataDir = KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/stringreplacer/";
    QString dataDir = KGlobal::dirs()->findAllResources("data", "kttsd/notify/").last();
    QString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*.xml|" + i18n("file type", "Notification Event List") + " (*.xml)",
        m_kttsmgrw,
        "event_loadfile");
    if ( filename.isEmpty() ) return;
    QString errMsg = loadNotifyEventsFromFile( filename, true );
    slotNotifyListView_selectionChanged();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_kttsmgrw, errMsg, i18n("Error Opening File") );
    else
        configChanged();
}

void KCMKttsMgr::slotNotifySaveButton_clicked()
{
    QString filename = KFileDialog::getSaveFileName(
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/notify/", false ),
        "*.xml|" + i18n("file type", "Notification Event List") + " (*.xml)",
        m_kttsmgrw,
        "event_savefile");
    if ( filename.isEmpty() ) return;
    QString errMsg = saveNotifyEventsToFile( filename );
    slotNotifyListView_selectionChanged();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_kttsmgrw, errMsg, i18n("Error Opening File") );
}

// ----------------------------------------------------------------------------

KttsCheckListItem::KttsCheckListItem( QListView *parent, QListViewItem *after,
    const QString &text, Type tt,
    KCMKttsMgr* kcmkttsmgr ) :
        QCheckListItem(parent, after, text, tt),
        m_kcmkttsmgr(kcmkttsmgr) { }

KttsCheckListItem::KttsCheckListItem( QListView *parent,
    const QString &text, Type tt,
    KCMKttsMgr* kcmkttsmgr ) :
        QCheckListItem(parent, text, tt),
        m_kcmkttsmgr(kcmkttsmgr) { }

/*virtual*/ void KttsCheckListItem::stateChange(bool)
{
    if (m_kcmkttsmgr) m_kcmkttsmgr->slotFiltersList_stateChanged();
}

/*virtual*/ /*void resizeEvent( QResizeEvent ev )
{
    dynamic_cast<KCModule>(resizeEvent(ev));
    updateGeometry();
}
*/
