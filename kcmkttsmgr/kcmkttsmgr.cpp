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

// TQt includes.
#include <tqwidget.h>
#include <tqtabwidget.h>
#include <tqcheckbox.h>
#include <tqvbox.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqslider.h>
#include <tqlabel.h>
#include <tqpopupmenu.h>
#include <tqbuttongroup.h>

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
const TQString textPreMsgValue = i18n("Text interrupted. Message.");

const bool textPreSndCheckValue = false;
const TQString textPreSndValue = "";

const bool textPostMsgCheckValue = true;
const TQString textPostMsgValue = i18n("Resuming text.");

const bool textPostSndCheckValue = false;
const TQString textPostSndValue = "";

const int timeBoxValue = 100;

const bool keepAudioCheckBoxValue = false;

// Make this a plug in.
typedef KGenericFactory<KCMKttsMgr, TQWidget> KCMKttsMgrFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kttsd, KCMKttsMgrFactory("kttsd") )

/**
* Constructor.
* Makes the list of plug ins.
* And the languages acording to the plug ins.
*/
KCMKttsMgr::KCMKttsMgr(TQWidget *tqparent, const char *name, const TQStringList &) :
    DCOPStub("kttsd", "KSpeech"),
    DCOPObject("kcmkttsmgr_kspeechsink"),
    KCModule(KCMKttsMgrFactory::instance(), tqparent, name)
{
    // kdDebug() << "KCMKttsMgr contructor running." << endl;

    // Initialize some variables.
    m_config = 0;
    m_jobMgrPart = 0;
    m_configDlg = 0;
    m_changed = false;
    m_suppressConfigChanged = false;

    // Add the KTTS Manager widget
    TQGridLayout *tqlayout = new TQGridLayout(this, 0, 0);
    m_kttsmgrw = new KCMKttsMgrWidget(this, "kttsmgrw");
    // m_kttsmgrw = new KCMKttsMgrWidget(this);
    tqlayout->addWidget(m_kttsmgrw, 0, 0);

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
    m_kttsmgrw->pcmComboBox->setEditable(false);

    // Construct a popup menu for the Sentence Boundary Detector buttons on Filter tab.
    m_sbdPopmenu = new TQPopupMenu( m_kttsmgrw, "SbdPopupMenu" );
    m_sbdPopmenu->insertItem( i18n("&Edit..."), this, TQT_SLOT(slot_configureSbdFilter()), 0, sbdBtnEdit );
    m_sbdPopmenu->insertItem( KGlobal::iconLoader()->loadIconSet("up", KIcon::Small),
                              i18n("U&p"), this, TQT_SLOT(slot_higherSbdFilterPriority()), 0, sbdBtnUp );
    m_sbdPopmenu->insertItem( KGlobal::iconLoader()->loadIconSet("down", KIcon::Small),
                              i18n("Do&wn"), this, TQT_SLOT(slot_lowerSbdFilterPriority()), 0, sbdBtnDown );
    m_sbdPopmenu->insertItem( i18n("&Add..."), this, TQT_SLOT(slot_addSbdFilter()), 0, sbdBtnAdd );
    m_sbdPopmenu->insertItem( i18n("&Remove"), this, TQT_SLOT(slot_removeSbdFilter()), 0, sbdBtnRemove );
    m_kttsmgrw->sbdButton->setPopup( m_sbdPopmenu );

    // If aRts is available, enable its radio button.
    // Determine if available by loading its plugin.  If it fails, not available.
    TestPlayer* testPlayer = new TestPlayer();
    Player* player = testPlayer->createPlayerObject(0);
    if (player)
        m_kttsmgrw->artsRadioButton->setEnabled(true);
    else
        m_kttsmgrw->artsRadioButton->setEnabled(false);
    delete player;
    delete testPlayer;

    // If GStreamer is available, enable its radio button.
    // Determine if available by loading its plugin.  If it fails, not available.
    testPlayer = new TestPlayer();
    player = testPlayer->createPlayerObject(1);
    if (player)
    {
        m_kttsmgrw->gstreamerRadioButton->setEnabled(true);
        m_kttsmgrw->sinkLabel->setEnabled(true);
        m_kttsmgrw->sinkComboBox->setEnabled(true);
        TQStringList sinkList = player->getPluginList("Sink/Audio");
        // kdDebug() << "KCMKttsMgr::KCMKttsMgr: GStreamer Sink List = " << sinkList << endl;
        m_kttsmgrw->sinkComboBox->clear();
        m_kttsmgrw->sinkComboBox->insertStringList(sinkList);
    }
    delete player;
    delete testPlayer;

    // If ALSA is available, enable its radio button.
    // Determine if available by loading its plugin.  If it fails, not available.
    testPlayer = new TestPlayer();
    player = testPlayer->createPlayerObject(2);
    if (player)
    {
        m_kttsmgrw->alsaRadioButton->setEnabled(true);
        m_kttsmgrw->pcmLabel->setEnabled(true);
        m_kttsmgrw->pcmComboBox->setEnabled(true);
        TQStringList pcmList = player->getPluginList("");
        pcmList.append("custom");
        kdDebug() << "KCMKttsMgr::KCMKttsMgr: ALSA pcmList = " << pcmList << endl;
        m_kttsmgrw->pcmComboBox->clear();
        m_kttsmgrw->pcmComboBox->insertStringList(pcmList);
    }
    delete player;
    delete testPlayer;

    // If aKode is available, enable its radio button.
    // Determine if available by loading its plugin.  If it fails, not available.
    testPlayer = new TestPlayer();
    player = testPlayer->createPlayerObject(3);
    if (player)
    {
        m_kttsmgrw->akodeRadioButton->setEnabled(true);
        m_kttsmgrw->akodeSinkLabel->setEnabled(true);
        m_kttsmgrw->akodeComboBox->setEnabled(true);
        TQStringList pcmList = player->getPluginList("");
        kdDebug() << "KCMKttsMgr::KCMKttsMgr: aKode Sink List = " << pcmList << endl;
        m_kttsmgrw->akodeComboBox->clear();
        m_kttsmgrw->akodeComboBox->insertStringList(pcmList);
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
    connect(m_kttsmgrw->addTalkerButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_addTalker()));
    connect(m_kttsmgrw->higherTalkerPriorityButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_higherTalkerPriority()));
    connect(m_kttsmgrw->lowerTalkerPriorityButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_lowerTalkerPriority()));
    connect(m_kttsmgrw->removeTalkerButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_removeTalker()));
    connect(m_kttsmgrw->configureTalkerButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_configureTalker()));
    connect(m_kttsmgrw->talkersList, TQT_SIGNAL(selectionChanged()),
            this, TQT_SLOT(updateTalkerButtons()));

    // Filter tab.
    connect(m_kttsmgrw->addFilterButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_addNormalFilter()));
    connect(m_kttsmgrw->higherFilterPriorityButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_higherNormalFilterPriority()));
    connect(m_kttsmgrw->lowerFilterPriorityButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_lowerNormalFilterPriority()));
    connect(m_kttsmgrw->removeFilterButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_removeNormalFilter()));
    connect(m_kttsmgrw->configureFilterButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slot_configureNormalFilter()));
    connect(m_kttsmgrw->filtersList, TQT_SIGNAL(selectionChanged()),
            this, TQT_SLOT(updateFilterButtons()));
    //connect(m_kttsmgrw->filtersList, TQT_SIGNAL(stateChanged()),
    //        this, TQT_SLOT(configChanged()));
    connect(m_kttsmgrw->sbdsList, TQT_SIGNAL(selectionChanged()),
            this, TQT_SLOT(updateSbdButtons()));

    // Audio tab.
    connect(m_kttsmgrw->gstreamerRadioButton, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(slotGstreamerRadioButton_toggled(bool)));
    connect(m_kttsmgrw->alsaRadioButton, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(slotAlsaRadioButton_toggled(bool)));
    connect(m_kttsmgrw->pcmComboBox, TQT_SIGNAL(activated(int)),
            this, TQT_SLOT(slotPcmComboBox_activated()));
    connect(m_kttsmgrw->akodeRadioButton, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(slotAkodeRadioButton_toggled(bool)));
    connect(m_kttsmgrw->timeBox, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(timeBox_valueChanged(int)));
    connect(m_kttsmgrw->timeSlider, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(timeSlider_valueChanged(int)));
    connect(m_kttsmgrw->timeBox, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_kttsmgrw->timeSlider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_kttsmgrw->keepAudioCheckBox, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(keepAudioCheckBox_toggled(bool)));
    connect(m_kttsmgrw->keepAudioPath, TQT_SIGNAL(textChanged(const TQString&)),
            this, TQT_SLOT(configChanged()));

    // General tab.
    connect(m_kttsmgrw->enableKttsdCheckBox, TQT_SIGNAL(toggled(bool)),
            TQT_SLOT(enableKttsdToggled(bool)));

    // Notify tab.
    connect(m_kttsmgrw->notifyEnableCheckBox, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(slotNotifyEnableCheckBox_toggled(bool)));
    connect(m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox, TQT_SIGNAL(toggled(bool)),
            this, TQT_SLOT(configChanged()));
    connect(m_kttsmgrw->notifyAddButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyAddButton_clicked()));
    connect(m_kttsmgrw->notifyRemoveButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyRemoveButton_clicked()));
    connect(m_kttsmgrw->notifyClearButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyClearButton_clicked()));
    connect(m_kttsmgrw->notifyLoadButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyLoadButton_clicked()));
    connect(m_kttsmgrw->notifySaveButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifySaveButton_clicked()));
    connect(m_kttsmgrw->notifyListView, TQT_SIGNAL(selectionChanged()),
            this, TQT_SLOT(slotNotifyListView_selectionChanged()));
    connect(m_kttsmgrw->notifyPresentComboBox, TQT_SIGNAL(activated(int)),
            this, TQT_SLOT(slotNotifyPresentComboBox_activated(int)));
    connect(m_kttsmgrw->notifyActionComboBox, TQT_SIGNAL(activated(int)),
            this, TQT_SLOT(slotNotifyActionComboBox_activated(int)));
    connect(m_kttsmgrw->notifyTestButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyTestButton_clicked()));
    connect(m_kttsmgrw->notifyMsgLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
            this, TQT_SLOT(slotNotifyMsgLineEdit_textChanged(const TQString&)));
    connect(m_kttsmgrw->notifyTalkerButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotNotifyTalkerButton_clicked()));

    // Others.
    connect(m_kttsmgrw, TQT_SIGNAL( configChanged() ),
            this, TQT_SLOT( configChanged() ) );
    connect(m_kttsmgrw->mainTab, TQT_SIGNAL(currentChanged(TQWidget*)),
            this, TQT_SLOT(slotTabChanged()));

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
    TQListViewItem* item = m_kttsmgrw->notifyListView->tqfindItem( "default", nlvcEventSrc );
    if ( item )
        if ( item->childCount() > 0 ) item = item->firstChild();
    if ( item ) m_kttsmgrw->notifyListView->ensureItemVisible( item );

    // Audio Output.
    int audioOutputMethod = 0;
    if (m_kttsmgrw->gstreamerRadioButton->isChecked()) audioOutputMethod = 1;
    if (m_kttsmgrw->alsaRadioButton->isChecked()) audioOutputMethod = 2;
    if (m_kttsmgrw->akodeRadioButton->isChecked()) audioOutputMethod = 3;
    audioOutputMethod = m_config->readNumEntry("AudioOutputMethod", audioOutputMethod);
    switch (audioOutputMethod)
    {
        case 0:
            m_kttsmgrw->artsRadioButton->setChecked(true);
            break;
        case 1:
            m_kttsmgrw->gstreamerRadioButton->setChecked(true);
            break;
        case 2:
            m_kttsmgrw->alsaRadioButton->setChecked(true);
            break;
        case 3:
            m_kttsmgrw->akodeRadioButton->setChecked(true);
            break;
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
    TQStringList talkerIDsList = m_config->readListEntry("TalkerIDs", ',');
    if (!talkerIDsList.isEmpty())
    {
        TQListViewItem* talkerItem = 0;
        TQStringList::ConstIterator itEnd = talkerIDsList.constEnd();
        for (TQStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it)
        {
            TQString talkerID = *it;
            // kdDebug() << "KCMKttsMgr::load: talkerID = " << talkerID << endl;
            m_config->setGroup(TQString("Talker_") + talkerID);
            TQString talkerCode = m_config->readEntry("TalkerCode");
            TQString fullLanguageCode;
            talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, fullLanguageCode);
            TQString language = TalkerCode::languageCodeToLanguage(fullLanguageCode);
            TQString desktopEntryName = m_config->readEntry("DesktopEntryName", TQString());
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            TQString synthName;
            if (desktopEntryName.isEmpty())
            {
                synthName = m_config->readEntry("PlugIn", TQString());
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
        TQString synthName = offers[i]->name();
        TQStringList languageCodes = offers[i]->property("X-KDE-Languages").toStringList();
        // Add language codes to the language-to-language code map.
        TQStringList::ConstIterator endLanguages(languageCodes.constEnd());
        for( TQStringList::ConstIterator it = languageCodes.constBegin(); it != endLanguages; ++it )
        {
            TQString language = TalkerCode::languageCodeToLanguage(*it);
            m_languagesToCodes[language] = *it;
        }

        // All plugins support "Other".
        // TODO: Eventually, this should not be necessary, since all plugins will know
        // the languages they support and report them in call to getSupportedLanguages().
        if (!languageCodes.tqcontains("other")) languageCodes.append("other");

        // Add supported language codes to synthesizer-to-language map.
        m_synthToLangMap[synthName] = languageCodes;
    }

    // Add "Other" language.
    m_languagesToCodes[i18n("Other")] = "other";

    // Load Filters.
    TQListViewItem* filterItem = 0;
    m_kttsmgrw->filtersList->clear();
    m_kttsmgrw->sbdsList->clear();
    m_kttsmgrw->filtersList->setSortColumn(-1);
    m_kttsmgrw->sbdsList->setSortColumn(-1);
    m_config->setGroup("General");
    TQStringList filterIDsList = m_config->readListEntry("FilterIDs", ',');
    // kdDebug() << "KCMKttsMgr::load: FilterIDs = " << filterIDsList << endl;
    if (!filterIDsList.isEmpty())
    {
        TQStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (TQStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it)
        {
            TQString filterID = *it;
            // kdDebug() << "KCMKttsMgr::load: filterID = " << filterID << endl;
            m_config->setGroup("Filter_" + filterID);
            TQString desktopEntryName = m_config->readEntry("DesktopEntryName", TQString());
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            TQString filterPlugInName;
            if (desktopEntryName.isEmpty())
            {
                filterPlugInName = m_config->readEntry("PlugInName", TQString());
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) m_config->writeEntry("DesktopEntryName", desktopEntryName);
            }
            filterPlugInName = FilterDesktopEntryNameToName(desktopEntryName);
            if (!filterPlugInName.isEmpty())
            {
                TQString userFilterName = m_config->readEntry("UserFilterName", filterPlugInName);
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
                            userFilterName, TQCheckListItem::CheckBox, this);
                    else
                        filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList, filterItem,
                            userFilterName, TQCheckListItem::CheckBox, this);
                    dynamic_cast<TQCheckListItem*>(filterItem)->setOn(checked);
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
        TQString filterPlugInName = offers[i]->name();
        TQString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
        if (!desktopEntryName.isEmpty() && (countFilterPlugins(filterPlugInName) == 0))
        {
            // Must load plugin to determine if it supports multiple instances
            // and to see if it can autoconfigure itself.
            KttsFilterConf* filterPlugIn = loadFilterPlugin(desktopEntryName);
            if (filterPlugIn)
            {
                ++m_lastFilterID;
                TQString filterID = TQString::number(m_lastFilterID);
                TQString groupName = "Filter_" + filterID;
                filterPlugIn->load(m_config, groupName);
                TQString userFilterName = filterPlugIn->userPlugInName();
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
                                userFilterName, TQCheckListItem::CheckBox, this);
                        else
                            filterItem = new KttsCheckListItem(m_kttsmgrw->filtersList, filterItem,
                                userFilterName, TQCheckListItem::CheckBox, this);
                        dynamic_cast<TQCheckListItem*>(filterItem)->setOn(false);
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
    TQString filterIDs = filterIDsList.join(",");
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

    // ALSA settings.
    m_config->setGroup("ALSAPlayer");
    KttsUtils::setCbItemFromText(m_kttsmgrw->pcmComboBox, m_config->readEntry("PcmName", "default"));
    m_kttsmgrw->pcmCustom->setText(m_config->readEntry("CustomPcmName", ""));

    // aKode settings.
    m_config->setGroup("aKodePlayer");
    KttsUtils::setCbItemFromText(m_kttsmgrw->akodeComboBox, m_config->readEntry("SinkName", "auto"));

    // Update controls based on new states.
    slotNotifyListView_selectionChanged();
    updateTalkerButtons();
    updateFilterButtons();
    updateSbdButtons();
    slotGstreamerRadioButton_toggled(m_kttsmgrw->gstreamerRadioButton->isChecked());
    slotAlsaRadioButton_toggled(m_kttsmgrw->alsaRadioButton->isChecked());
    slotAkodeRadioButton_toggled(m_kttsmgrw->akodeRadioButton->isChecked());

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
    if (m_kttsmgrw->alsaRadioButton->isChecked()) audioOutputMethod = 2;
    if (m_kttsmgrw->akodeRadioButton->isChecked()) audioOutputMethod = 3;
    m_config->writeEntry("AudioOutputMethod", audioOutputMethod);
    m_config->writeEntry("AudioStretchFactor", m_kttsmgrw->timeBox->value());
    m_config->writeEntry("KeepAudio", m_kttsmgrw->keepAudioCheckBox->isChecked());
    m_config->writeEntry("KeepAudioPath", m_kttsmgrw->keepAudioPath->url());

    // Get ordered list of all talker IDs.
    TQStringList talkerIDsList;
    TQListViewItem* talkerItem = m_kttsmgrw->talkersList->firstChild();
    while (talkerItem)
    {
        TQListViewItem* nextTalkerItem = talkerItem->itemBelow();
        TQString talkerID = talkerItem->text(tlvcTalkerID);
        talkerIDsList.append(talkerID);
        talkerItem = nextTalkerItem;
    }
    TQString talkerIDs = talkerIDsList.join(",");
    m_config->writeEntry("TalkerIDs", talkerIDs);

    // Erase obsolete Talker_nn sections.
    TQStringList groupList = m_config->groupList();
    int groupListCount = groupList.count();
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx)
    {
        TQString groupName = groupList[groupNdx];
        if (groupName.left(7) == "Talker_")
        {
            TQString groupTalkerID = groupName.mid(7);
            if (!talkerIDsList.tqcontains(groupTalkerID)) m_config->deleteGroup(groupName);
        }
    }

    // Get ordered list of all filter IDs.  Record enabled state of each filter.
    TQStringList filterIDsList;
    TQListViewItem* filterItem = m_kttsmgrw->filtersList->firstChild();
    while (filterItem)
    {
        TQListViewItem* nextFilterItem = filterItem->itemBelow();
        TQString filterID = filterItem->text(flvcFilterID);
        filterIDsList.append(filterID);
        bool checked = dynamic_cast<TQCheckListItem*>(filterItem)->isOn();
        m_config->setGroup("Filter_" + filterID);
        m_config->writeEntry("Enabled", checked);
        m_config->writeEntry("IsSBD", false);
        filterItem = nextFilterItem;
    }
    TQListViewItem* sbdItem = m_kttsmgrw->sbdsList->firstChild();
    while (sbdItem)
    {
        TQListViewItem* nextSbdItem = sbdItem->itemBelow();
        TQString filterID = sbdItem->text(slvcFilterID);
        filterIDsList.append(filterID);
        m_config->setGroup("Filter_" + filterID);
        m_config->writeEntry("Enabled", true);
        m_config->writeEntry("IsSBD", true);
        sbdItem = nextSbdItem;
    }
    TQString filterIDs = filterIDsList.join(",");
    m_config->setGroup("General");
    m_config->writeEntry("FilterIDs", filterIDs);

    // Erase obsolete Filter_nn sections.
    for (int groupNdx = 0; groupNdx < groupListCount; ++groupNdx)
    {
        TQString groupName = groupList[groupNdx];
        if (groupName.left(7) == "Filter_")
        {
            TQString groupFilterID = groupName.mid(7);
            if (!filterIDsList.tqcontains(groupFilterID)) m_config->deleteGroup(groupName);
        }
    }

    // GStreamer settings.
    m_config->setGroup("GStreamerPlayer");
    m_config->writeEntry("SinkName", m_kttsmgrw->sinkComboBox->currentText());

    // ALSA settings.
    m_config->setGroup("ALSAPlayer");
    m_config->writeEntry("PcmName", m_kttsmgrw->pcmComboBox->currentText());
    m_config->writeEntry("CustomPcmName", m_kttsmgrw->pcmCustom->text());

    // aKode settings.
    m_config->setGroup("aKodePlayer");
    m_config->writeEntry("SinkName", m_kttsmgrw->akodeComboBox->currentText());

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
            TQByteArray data;
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
TQString KCMKttsMgr::quickHelp() const{
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
* @param name             DesktopEntryName of the Synthesizer.
* @return                 Pointer to the configuration plugin for the Talker.
*/
PlugInConf* KCMKttsMgr::loadTalkerPlugin(const TQString& name)
{
    // kdDebug() << "KCMKttsMgr::loadPlugin: Running"<< endl;

    // Find the plugin.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin",
        TQString("DesktopEntryName == '%1'").tqarg(name));

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
 * @param plugInName       DesktopEntryName of the plugin.
 * @return                 Pointer to the configuration plugin for the Filter.
 */
KttsFilterConf* KCMKttsMgr::loadFilterPlugin(const TQString& plugInName)
{
    // kdDebug() << "KCMKttsMgr::loadPlugin: Running"<< endl;

    // Find the plugin.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        TQString("DesktopEntryName == '%1'").tqarg(plugInName));

    if (offers.count() == 1)
    {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if(factory){
            // If the factory is created successfully, instantiate the KttsFilterConf class for the
            // specific plug in to get the plug in configuration object.
            int errorNo = 0;
            KttsFilterConf *plugIn =
                KParts::ComponentFactory::createInstanceFromLibrary<KttsFilterConf>(
                    offers[0]->library().latin1(), NULL, offers[0]->library().latin1(),
                    TQStringList(), &errorNo);
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
* @param talkerItem       TQListViewItem.
* @param talkerCode       Talker Code.
*/
void KCMKttsMgr::updateTalkerItem(TQListViewItem* talkerItem, const TQString &talkerCode)
{
    TalkerCode parsedTalkerCode(talkerCode);
    TQString fullLanguageCode = parsedTalkerCode.fullLanguageCode();
    if (!fullLanguageCode.isEmpty())
    {
        TQString language = TalkerCode::languageCodeToLanguage(fullLanguageCode);
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
    TQString languageCode = addTalkerWidget->getLanguageCode();
    TQString synthName = addTalkerWidget->getSynthesizer();
    delete dlg;
    // TODO: Also delete addTalkerWidget?
    if (dlgResult != TQDialog::Accepted) return;

    // If user chose "Other", must now get a language from him.
    if(languageCode == "other")
    {
        // Create a  TQHBox to host KListView.
        TQHBox* hBox = new TQHBox(m_kttsmgrw, "SelectLanguage_hbox");
        // Create a KListView and fill with all known languages.
        KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
        langLView->addColumn(i18n("Language"));
        langLView->addColumn(i18n("Code"));
        TQStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
        TQString locale;
        TQString countryCode;
        TQString charSet;
        TQString language;
        const int allLocalesCount = allLocales.count();
        for (int ndx=0; ndx < allLocalesCount; ++ndx)
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
        dlg->setInitialSize(TQSize(200, 500), false);
        dlgResult = dlg->exec();
        languageCode = TQString();
        if (langLView->selectedItem()) languageCode = langLView->selectedItem()->text(1);
        delete dlg;
        // TODO: Also delete KListView and TQHBox?
        if (dlgResult != TQDialog::Accepted) return;
    }

    if (languageCode.isEmpty()) return;
    TQString language = TalkerCode::languageCodeToLanguage(languageCode);
    if (language.isEmpty()) return;

    m_languagesToCodes[language] = languageCode;

    // Assign a new Talker ID for the talker.  Wraps around to 1.
    TQString talkerID = TQString::number(m_lastTalkerID + 1);

    // Erase extraneous Talker configuration entries that might be there.
    m_config->deleteGroup(TQString("Talker_")+talkerID);
    m_config->sync();

    // Convert translated plugin name to DesktopEntryName.
    TQString desktopEntryName = TalkerCode::TalkerNameToDesktopEntryName(synthName);
    // This shouldn't happen, but just in case.
    if (desktopEntryName.isEmpty()) return;

    // Load the plugin.
    m_loadedTalkerPlugIn = loadTalkerPlugin(desktopEntryName);
    if (!m_loadedTalkerPlugIn) return;

    // Give plugin the user's language code and permit plugin to autoconfigure itself.
    m_loadedTalkerPlugIn->setDesiredLanguage(languageCode);
    m_loadedTalkerPlugIn->load(m_config, TQString("Talker_")+talkerID);

    // If plugin was able to configure itself, it returns a full talker code.
    // If not, display configuration dialog for user to configure the plugin.
    TQString talkerCode = m_loadedTalkerPlugIn->getTalkerCode();
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
        m_config->setGroup(TQString("Talker_")+talkerID);
        m_loadedTalkerPlugIn->save(m_config, TQString("Talker_"+talkerID));

        // Record last Talker ID used for next add.
        m_lastTalkerID = talkerID.toInt();

        // Record configuration data.  Note, might as well do this now.
        m_config->setGroup(TQString("Talker_")+talkerID);
        m_config->writeEntry("DesktopEntryName", desktopEntryName);
        talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, languageCode);
        m_config->writeEntry("TalkerCode", talkerCode);
        m_config->sync();

        // Add listview item.
        TQListViewItem* talkerItem = m_kttsmgrw->talkersList->lastChild();
        if (talkerItem)
            talkerItem =  new KListViewItem(m_kttsmgrw->talkersList, talkerItem,
                TQString::number(m_lastTalkerID), language, synthName);
        else
            talkerItem = new KListViewItem(m_kttsmgrw->talkersList,
                TQString::number(m_lastTalkerID), language, synthName);

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

    TQStringList filterPlugInNames;
    TQListViewItem* item = lView->firstChild();
    while (item)
    {
        if (item->text(flvcMultiInstance) == "T")
        {
            if (!filterPlugInNames.tqcontains(item->text(flvcPlugInName)))
                filterPlugInNames.append(item->text(flvcPlugInName));
        }
        item = item->nextSibling();
    }
    // Append those available plugins not yet in the list at all.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin");
    for (unsigned int i=0; i < offers.count() ; ++i)
    {
        TQString filterPlugInName = offers[i]->name();
        if (countFilterPlugins(filterPlugInName) == 0)
        {
            TQString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
            KttsFilterConf* filterConf = loadFilterPlugin(desktopEntryName);
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
    TQString filterPlugInName;
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
    TQString filterID = TQString::number(m_lastFilterID + 1);

    // Erase extraneous Filter configuration entries that might be there.
    m_config->deleteGroup(TQString("Filter_")+filterID);
    m_config->sync();

    // Get DesktopEntryName from the translated name.
    TQString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
    // This shouldn't happen, but just in case.
    if (desktopEntryName.isEmpty()) return;

    // Load the plugin.
    m_loadedFilterPlugIn = loadFilterPlugin(desktopEntryName);
    if (!m_loadedFilterPlugIn) return;

    // Permit plugin to autoconfigure itself.
    m_loadedFilterPlugIn->load(m_config, TQString("Filter_")+filterID);

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
    TQString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save its configuration.
    if ( !userFilterName.isEmpty() )
    {
        // Let plugin save its configuration.
        m_config->setGroup(TQString("Filter_")+filterID);
        m_loadedFilterPlugIn->save(m_config, TQString("Filter_"+filterID));

        // Record last Filter ID used for next add.
        m_lastFilterID = filterID.toInt();

        // Determine if filter supports multiple instances.
        bool multiInstance = m_loadedFilterPlugIn->supportsMultiInstance();

        // Record configuration data.  Note, might as well do this now.
        m_config->setGroup(TQString("Filter_")+filterID);
        m_config->writeEntry("DesktopEntryName", desktopEntryName);
        m_config->writeEntry("UserFilterName", userFilterName);
        m_config->writeEntry("MultiInstance", multiInstance);
        m_config->writeEntry("Enabled", true);
        m_config->writeEntry("IsSBD", sbd);
        m_config->sync();

        // Add listview item.
        TQListViewItem* filterItem = lView->lastChild();
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
                    userFilterName, TQCheckListItem::CheckBox, this);
            else
                filterItem = new KttsCheckListItem(lView,
                    userFilterName, TQCheckListItem::CheckBox, this);
            dynamic_cast<TQCheckListItem*>(filterItem)->setOn(true);
        }
        filterItem->setText(flvcFilterID, TQString::number(m_lastFilterID));
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
    TQListViewItem *itemToRemove = m_kttsmgrw->talkersList->selectedItem();
    if (!itemToRemove) return;

    // Delete the talker from configuration file.
//    TQString talkerID = itemToRemove->text(tlvcTalkerID);
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
    TQListViewItem *itemToRemove = lView->selectedItem();
    if (!itemToRemove) return;

    // Delete the filter from configuration file.
//    TQString filterID = itemToRemove->text(flvcFilterID);
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
    TQListViewItem* item = lView->selectedItem();
    if (!item) return;
    TQListViewItem* prevItem = item->itemAbove();
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
    TQListViewItem* item = lView->selectedItem();
    if (!item) return;
    TQListViewItem* nextItem = item->itemBelow();
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
    TQListViewItem* item = m_kttsmgrw->filtersList->selectedItem();
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
    TQListViewItem* item = m_kttsmgrw->sbdsList->selectedItem();
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
            TQString error;
            if (KApplication::startServiceByDesktopName("kttsd", TQStringList(), &error))
            {
                kdDebug() << "Starting KTTSD failed with message " << error << endl;
                m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
                m_kttsmgrw->notifyTestButton->setEnabled(false);
            }
        }
    }
    else
    // If check box is not checked and it is running, then stop KTTSD.
    {
    if (kttsdRunning)
        {
            // kdDebug() << "KCMKttsMgr::enableKttsdToggled:: Stopping KTTSD" << endl;
            TQByteArray data;
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
* This signal is emitted whenever user checks/unchecks the ALSA radio button.
*/
void KCMKttsMgr::slotAlsaRadioButton_toggled(bool state)
{
    m_kttsmgrw->pcmLabel->setEnabled(state);
    m_kttsmgrw->pcmComboBox->setEnabled(state);
    m_kttsmgrw->pcmCustom->setEnabled(state && m_kttsmgrw->pcmComboBox->currentText() == "custom");
}

/**
* This is emitted whenever user activates the ALSA pcm combobox.
*/
void KCMKttsMgr::slotPcmComboBox_activated()
{
    m_kttsmgrw->pcmCustom->setEnabled(m_kttsmgrw->pcmComboBox->currentText() == "custom");
}

/**
* This signal is emitted whenever user checks/unchecks the aKode radio button.
*/
void KCMKttsMgr::slotAkodeRadioButton_toggled(bool state)
{
    m_kttsmgrw->akodeSinkLabel->setEnabled(state);
    m_kttsmgrw->akodeComboBox->setEnabled(state);
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
            m_jobMgrPart = (KParts::ReadOnlyPart *)factory->create( TQT_TQOBJECT(m_kttsmgrw->mainTab), "kttsjobmgr",
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
    {
        m_kttsmgrw->enableKttsdCheckBox->setChecked(true);
        // Enable/disable notify Test button.
        slotNotifyListView_selectionChanged();
    } else {
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
        m_kttsmgrw->notifyTestButton->setEnabled(false);
    }
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
    m_kttsmgrw->notifyTestButton->setEnabled(false);
}

/**
* User has requested display of talker configuration dialog.
*/
void KCMKttsMgr::slot_configureTalker()
{
    // Get highlighted plugin from Talker ListView and load into memory.
    TQListViewItem* talkerItem = m_kttsmgrw->talkersList->selectedItem();
    if (!talkerItem) return;
    TQString talkerID = talkerItem->text(tlvcTalkerID);
    TQString synthName = talkerItem->text(tlvcSynthName);
    TQString language = talkerItem->text(tlvcLanguage);
    TQString languageCode = m_languagesToCodes[language];
    TQString desktopEntryName = TalkerCode::TalkerNameToDesktopEntryName(synthName);
    m_loadedTalkerPlugIn = loadTalkerPlugin(desktopEntryName);
    if (!m_loadedTalkerPlugIn) return;
    // kdDebug() << "KCMKttsMgr::slot_configureTalker: plugin for " << synthName << " loaded successfully." << endl;

    // Tell plugin to load its configuration.
    m_config->setGroup(TQString("Talker_")+talkerID);
    m_loadedTalkerPlugIn->setDesiredLanguage(languageCode);
    // kdDebug() << "KCMKttsMgr::slot_configureTalker: about to call plugin load() method with Talker ID = " << talkerID << endl;
    m_loadedTalkerPlugIn->load(m_config, TQString("Talker_")+talkerID);

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
    TQString talkerCode = m_loadedTalkerPlugIn->getTalkerCode();

    // If plugin was successfully configured, save its configuration.
    if (!talkerCode.isEmpty())
    {
        m_config->setGroup(TQString("Talker_")+talkerID);
        m_loadedTalkerPlugIn->save(m_config, TQString("Talker_")+talkerID);
        m_config->setGroup(TQString("Talker_")+talkerID);
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
    TQListViewItem* filterItem = lView->selectedItem();
    if (!filterItem) return;
    TQString filterID = filterItem->text(flvcFilterID);
    TQString filterPlugInName = filterItem->text(flvcPlugInName);
    TQString desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
    if (desktopEntryName.isEmpty()) return;
    m_loadedFilterPlugIn = loadFilterPlugin(desktopEntryName);
    if (!m_loadedFilterPlugIn) return;
    // kdDebug() << "KCMKttsMgr::slot_configureFilter: plugin for " << filterPlugInName << " loaded successfully." << endl;

    // Tell plugin to load its configuration.
    m_config->setGroup(TQString("Filter_")+filterID);
    // kdDebug() << "KCMKttsMgr::slot_configureFilter: about to call plugin load() method with Filter ID = " << filterID << endl;
    m_loadedFilterPlugIn->load(m_config, TQString("Filter_")+filterID);

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
    TQString userFilterName = m_loadedFilterPlugIn->userPlugInName();

    // If user properly configured the plugin, save the configuration.
    if ( !userFilterName.isEmpty() )
    {

        // Let plugin save its configuration.
        m_config->setGroup(TQString("Filter_")+filterID);
        m_loadedFilterPlugIn->save(m_config, TQString("Filter_")+filterID);

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
            dynamic_cast<TQCheckListItem*>(filterItem)->setOn(true);

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
    m_configDlg->setInitialSize(TQSize(700, 300), false);
    m_configDlg->setMainWidget(m_loadedTalkerPlugIn);
    m_configDlg->setHelp("configure-plugin", "kttsd");
    m_configDlg->enableButtonOK(false);
    connect(m_loadedTalkerPlugIn, TQT_SIGNAL( changed(bool) ), this, TQT_SLOT( slotConfigTalkerDlg_ConfigChanged() ));
    connect(m_configDlg, TQT_SIGNAL( defaultClicked() ), this, TQT_SLOT( slotConfigTalkerDlg_DefaultClicked() ));
    connect(m_configDlg, TQT_SIGNAL( cancelClicked() ), this, TQT_SLOT (slotConfigTalkerDlg_CancelClicked() ));
    // Create a Player object for the plugin to use for testing.
    int playerOption = 0;
    TQString sinkName;
    if (m_kttsmgrw->gstreamerRadioButton->isChecked()) {
        playerOption = 1;
        sinkName = m_kttsmgrw->sinkComboBox->currentText();
    }
    if (m_kttsmgrw->alsaRadioButton->isChecked()) {
        playerOption = 2;
        if (m_kttsmgrw->pcmComboBox->currentText() == "custom")
            sinkName = m_kttsmgrw->pcmCustom->text();
        else
            sinkName = m_kttsmgrw->pcmComboBox->currentText();
    }
    if (m_kttsmgrw->akodeRadioButton->isChecked()) {
        playerOption = 3;
        sinkName = m_kttsmgrw->akodeComboBox->currentText();
    }
    float audioStretchFactor = 1.0/(float(m_kttsmgrw->timeBox->value())/100.0);
    // kdDebug() << "KCMKttsMgr::configureTalker: playerOption = " << playerOption << " audioStretchFactor = " << audioStretchFactor << " sink name = " << sinkName << endl;
    TestPlayer* testPlayer = new TestPlayer(TQT_TQOBJECT(this), "ktts_testplayer", 
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
    m_configDlg->setInitialSize(TQSize(600, 450), false);
    m_loadedFilterPlugIn->setMinimumSize(m_loadedFilterPlugIn->tqminimumSizeHint());
    m_loadedFilterPlugIn->show();
    m_configDlg->setMainWidget(m_loadedFilterPlugIn);
    m_configDlg->setHelp("configure-filter", "kttsd");
    m_configDlg->enableButtonOK(false);
    connect(m_loadedFilterPlugIn, TQT_SIGNAL( changed(bool) ), this, TQT_SLOT( slotConfigFilterDlg_ConfigChanged() ));
    connect(m_configDlg, TQT_SIGNAL( defaultClicked() ), this, TQT_SLOT( slotConfigFilterDlg_DefaultClicked() ));
    connect(m_configDlg, TQT_SIGNAL( cancelClicked() ), this, TQT_SLOT (slotConfigFilterDlg_CancelClicked() ));
    // Display the dialog.
    m_configDlg->exec();
}

/**
* Count number of configured Filters with the specified plugin name.
*/
int KCMKttsMgr::countFilterPlugins(const TQString& filterPlugInName)
{
    int cnt = 0;
    TQListViewItem* item = m_kttsmgrw->filtersList->firstChild();
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
 *                               TQString() if not found.
 */
TQString KCMKttsMgr::FilterNameToDesktopEntryName(const TQString& name)
{
    if (name.isEmpty()) return TQString();
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin");
    for (uint ndx = 0; ndx < offers.count(); ++ndx)
        if (offers[ndx]->name() == name) return offers[ndx]->desktopEntryName();
    return TQString();
}

/**
 * Uses KTrader to convert a DesktopEntryName into a translated Filter Plugin Name.
 * @param desktopEntryName       The DesktopEntryName.
 * @return                       The translated Name of the plugin, from Name= line in .desktop file.
 */
TQString KCMKttsMgr::FilterDesktopEntryNameToName(const TQString& desktopEntryName)
{
    if (desktopEntryName.isEmpty()) return TQString();
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        TQString("DesktopEntryName == '%1'").tqarg(desktopEntryName));

    if (offers.count() == 1)
        return offers[0]->name();
    else
        return TQString();
}

/**
 * Loads notify events from a file.  Clearing listview if clear is True.
 */
TQString KCMKttsMgr::loadNotifyEventsFromFile( const TQString& filename, bool clear)
{
    // Open existing event list.
    TQFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
    {
        return i18n("Unable to open file.") + filename;
    }
    // TQDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    TQDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        return i18n("File not in proper XML format.");
    }
    // kdDebug() << "StringReplacerConf::load: document successfully parsed." << endl;
    file.close();

    // Clear list view.
    if ( clear ) m_kttsmgrw->notifyListView->clear();

    // Event list.
    TQDomNodeList eventList = doc.elementsByTagName("notifyEvent");
    const int eventListCount = eventList.count();
    for (int eventIndex = 0; eventIndex < eventListCount; ++eventIndex)
    {
        TQDomNode eventNode = eventList.item(eventIndex);
        TQDomNodeList propList = eventNode.childNodes();
        TQString eventSrc;
        TQString event;
        TQString actionName;
        TQString message;
        TalkerCode talkerCode;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex)
        {
            TQDomNode propNode = propList.item(propIndex);
            TQDomElement prop = propNode.toElement();
            if (prop.tagName() == "eventSrc") eventSrc = prop.text();
            if (prop.tagName() == "event") event = prop.text();
            if (prop.tagName() == "action") actionName = prop.text();
            if (prop.tagName() == "message") message = prop.text();
            if (prop.tagName() == "talker") talkerCode = TalkerCode(prop.text(), false);
        }
        addNotifyItem(eventSrc, event, NotifyAction::action( actionName ), message, talkerCode);
    }

    return TQString();
}

/**
 * Saves notify events to a file.
 */
TQString KCMKttsMgr::saveNotifyEventsToFile(const TQString& filename)
{
    TQFile file( filename );
    if ( !file.open( IO_WriteOnly ) )
        return i18n("Unable to open file ") + filename;

    TQDomDocument doc( "" );

    TQDomElement root = doc.createElement( "notifyEventList" );
    doc.appendChild( root );

    // Events.
    KListView* lv = m_kttsmgrw->notifyListView;
    TQListViewItemIterator it(lv);
    while ( it.current() )
    {
        TQListViewItem* item = *it;
        if ( item->depth() > 0 )
        {
            TQDomElement wordTag = doc.createElement( "notifyEvent" );
            root.appendChild( wordTag );

            TQDomElement propTag = doc.createElement( "eventSrc" );
            wordTag.appendChild( propTag);
            TQDomText t = doc.createTextNode( item->text(nlvcEventSrc) );
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
                TQString msg = item->text(nlvcActionName);
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
    TQTextStream ts( &file );
    ts.setEncoding( TQTextStream::UnicodeUTF8 );
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ts << doc.toString();
    file.close();

    return TQString();
}

void KCMKttsMgr::slotNotifyEnableCheckBox_toggled(bool checked)
{
    m_kttsmgrw->notifyExcludeEventsWithSoundCheckBox->setEnabled( checked );
    m_kttsmgrw->notifyGroup->setEnabled( checked );
    configChanged();
}

void KCMKttsMgr::slotNotifyPresentComboBox_activated(int index)
{
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
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
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
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
            bool defaultItem = ( item->tqparent()->text(nlvcEventSrc) == "default" );
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
                TQString msg = item->text( nlvcActionName );
                int msglen = msg.length();
                msg = msg.mid( 1, msglen-2 );
                m_kttsmgrw->notifyMsgLineEdit->setText( msg );
            } else {
                m_kttsmgrw->notifyMsgLineEdit->setEnabled( false );
                m_kttsmgrw->notifyMsgLineEdit->clear();
            }
            m_kttsmgrw->notifyRemoveButton->setEnabled( !defaultItem );
            m_kttsmgrw->notifyTestButton->setEnabled(
                action != NotifyAction::DoNotSpeak &&
                m_kttsmgrw->enableKttsdCheckBox->isChecked());
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
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
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

void KCMKttsMgr::slotNotifyMsgLineEdit_textChanged(const TQString& text)
{
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
        if ( item->depth() == 0 ) item = 0;
    if ( !item ) return;  // This shouldn't happen.
    if ( m_kttsmgrw->notifyActionComboBox->currentItem() != NotifyAction::SpeakCustom) return;
    item->setText( nlvcActionName, "\"" + text + "\"" );
    m_kttsmgrw->notifyTestButton->setEnabled(
        !text.isEmpty() && m_kttsmgrw->enableKttsdCheckBox->isChecked());
    configChanged();
}

void KCMKttsMgr::slotNotifyTestButton_clicked()
{
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if (item)
    {
        TQString msg;
        int action = NotifyAction::action(item->text(nlvcAction));
        switch (action)
        {
            case NotifyAction::SpeakEventName:
                msg = item->text(nlvcEventName);
                break;
            case NotifyAction::SpeakMsg:
                msg = i18n("sample notification message");
                break;
            case NotifyAction::SpeakCustom:
                msg = m_kttsmgrw->notifyMsgLineEdit->text();
                msg.tqreplace("%a", i18n("sample application"));
                msg.tqreplace("%e", i18n("sample event"));
                msg.tqreplace("%m", i18n("sample notification message"));
                break;
        }
        if (!msg.isEmpty()) sayMessage(msg, item->text(nlvcTalker));
    }
}

void KCMKttsMgr::slotNotifyTalkerButton_clicked()
{
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if ( item )
        if ( item->depth() == 0 ) item = 0;
    if ( !item ) return;  // This shouldn't happen.
    TQString talkerCode = item->text( nlvcTalker );
    SelectTalkerDlg dlg( m_kttsmgrw, "selecttalkerdialog", i18n("Select Talker"), talkerCode, true );
    int dlgResult = dlg.exec();
    if ( dlgResult != KDialogBase::Accepted ) return;
    item->setText( nlvcTalker, dlg.getSelectedTalkerCode() );
    TQString talkerName = dlg.getSelectedTranslatedDescription();
    item->setText( nlvcTalkerName, talkerName );
    m_kttsmgrw->notifyTalkerLineEdit->setText( talkerName );
    configChanged();
}

/**
 * Adds an item to the notify listview.
 * message is only needed if action = naSpeakCustom.
 */
TQListViewItem* KCMKttsMgr::addNotifyItem(
    const TQString& eventSrc,
    const TQString& event,
    int action,
    const TQString& message,
    TalkerCode& talkerCode)
{
    KListView* lv = m_kttsmgrw->notifyListView;
    TQListViewItem* item = 0;
    TQString iconName;
    TQString eventSrcName;
    if (eventSrc == "default")
        eventSrcName = i18n("Default (all other events)");
    else
        eventSrcName = NotifyEvent::getEventSrcName(eventSrc, iconName);
    TQString eventName;
    if (eventSrc == "default")
        eventName = NotifyPresent::presentDisplayName( event );
    else
    {
        if (event == "default")
            eventName = i18n("All other %1 events").tqarg(eventSrcName);
        else
            eventName = NotifyEvent::getEventName(eventSrc, event);
    }
    TQString actionName = NotifyAction::actionName( action );
    TQString actionDisplayName = NotifyAction::actionDisplayName( action );
    if (action == NotifyAction::SpeakCustom) actionDisplayName = "\"" + message + "\"";
    TQString talkerName = talkerCode.getTranslatedDescription();
    if (!eventSrcName.isEmpty() && !eventName.isEmpty() && !actionName.isEmpty() && !talkerName.isEmpty())
    {
        TQListViewItem* parentItem = lv->tqfindItem(eventSrcName, nlvcEventSrcName);
        if (!parentItem)
        {
            item = lv->lastItem();
            if (!item)
                parentItem = new KListViewItem(lv, eventSrcName, TQString(), TQString(),
                    eventSrc);
            else
                parentItem = new KListViewItem(lv, item, eventSrcName, TQString(), TQString(),
                    eventSrc);
            if ( !iconName.isEmpty() )
                parentItem->setPixmap( nlvcEventSrcName, SmallIcon( iconName ) );
        }
        // No duplicates.
        item = lv->tqfindItem( event, nlvcEvent );
        if ( !item || item->tqparent() != parentItem )
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
    TQListView* lv = m_kttsmgrw->notifyListView;
    TQListViewItem* item = lv->selectedItem();
    TQString eventSrc;
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
    dlg->setInitialSize( TQSize(500, 400) );
    // dlg->setHelp("select-plugin", "kttsd");
    int dlgResult = dlg->exec();
    eventSrc = selectEventWidget->getEventSrc();
    TQString event = selectEventWidget->getEvent();
    delete dlg;
    if ( dlgResult != TQDialog::Accepted ) return;
    if ( eventSrc.isEmpty() || event.isEmpty() ) return;
    // Use Default action, message, and talker.
    TQString actionName;
    int action = NotifyAction::DoNotSpeak;
    TQString msg;
    TalkerCode talkerCode;
    item = lv->tqfindItem( "default", nlvcEventSrc );
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
    TalkerCode talkerCode( TQString::null );
    TQListViewItem* item = addNotifyItem(
        TQString("default"),
        NotifyPresent::presentName(NotifyPresent::Passive),
        NotifyAction::SpeakEventName,
        TQString(),
        talkerCode );
    TQListView* lv = m_kttsmgrw->notifyListView;
    lv->ensureItemVisible( item );
    lv->setSelected( item, true );
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyRemoveButton_clicked()
{
    TQListViewItem* item = m_kttsmgrw->notifyListView->selectedItem();
    if (!item) return;
    TQListViewItem* parentItem = item->tqparent();
    delete item;
    if (parentItem)
    {
        if (parentItem->childCount() == 0) delete parentItem;
    }
    slotNotifyListView_selectionChanged();
    configChanged();
}

void KCMKttsMgr::slotNotifyLoadButton_clicked()
{
    // TQString dataDir = KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/stringreplacer/";
    TQString dataDir = KGlobal::dirs()->findAllResources("data", "kttsd/notify/").last();
    TQString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*.xml|" + i18n("file type", "Notification Event List") + " (*.xml)",
        m_kttsmgrw,
        "event_loadfile");
    if ( filename.isEmpty() ) return;
    TQString errMsg = loadNotifyEventsFromFile( filename, true );
    slotNotifyListView_selectionChanged();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_kttsmgrw, errMsg, i18n("Error Opening File") );
    else
        configChanged();
}

void KCMKttsMgr::slotNotifySaveButton_clicked()
{
    TQString filename = KFileDialog::getSaveFileName(
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/notify/", false ),
        "*.xml|" + i18n("file type", "Notification Event List") + " (*.xml)",
        m_kttsmgrw,
        "event_savefile");
    if ( filename.isEmpty() ) return;
    TQString errMsg = saveNotifyEventsToFile( filename );
    slotNotifyListView_selectionChanged();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_kttsmgrw, errMsg, i18n("Error Opening File") );
}

// ----------------------------------------------------------------------------

KttsCheckListItem::KttsCheckListItem( TQListView *tqparent, TQListViewItem *after,
    const TQString &text, Type tt,
    KCMKttsMgr* kcmkttsmgr ) :
        TQCheckListItem(tqparent, after, text, tt),
        m_kcmkttsmgr(kcmkttsmgr) { }

KttsCheckListItem::KttsCheckListItem( TQListView *tqparent,
    const TQString &text, Type tt,
    KCMKttsMgr* kcmkttsmgr ) :
        TQCheckListItem(tqparent, text, tt),
        m_kcmkttsmgr(kcmkttsmgr) { }

/*virtual*/ void KttsCheckListItem::stateChange(bool)
{
    if (m_kcmkttsmgr) m_kcmkttsmgr->slotFiltersList_stateChanged();
}

/*virtual*/ /*void resizeEvent( TQResizeEvent ev )
{
    dynamic_cast<KCModule>(resizeEvent(ev));
    updateGeometry();
}
*/
