/***************************************************** vim:set ts=4 sw=4 sts=4:
  kcmkttsmgr.cpp
  KControl module for KTTSD configuration and Job Management
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>

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
#include <kaboutapplication.h>

// KTTS includes.
#include "kcmkttsmgr.moc"
#include "kcmkttsmgr.h"
#include "pluginconf.h"

// Some constants.
// Defaults set when clicking Defaults button.
const bool enableNotifyCheckBoxValue = false;
const bool enablePassiveOnlyCheckBoxValue = false;

const bool textPreMsgCheckValue = true;
const QString textPreMsgValue = i18n("Text interrupted. Message.");

const bool textPreSndCheckValue = false;
const QString textPreSndValue = "";

const bool textPostMsgCheckValue = true;
const QString textPostMsgValue = i18n("Resuming text.");

const bool textPostSndCheckValue = false;
const QString textPostSndValue = "";

// Make this a plug in.
// Provide two identical modules.  Once all apps have stopped using
// kcm_kttsmgr, remove it.
typedef KGenericFactory<KCMKttsMgr, QWidget> KCMKttsMgrFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kttsd, KCMKttsMgrFactory("kcm_kttsd") );
K_EXPORT_COMPONENT_FACTORY( kcm_kttsmgr, KCMKttsMgrFactory("kcm_kttsmgr") );

/**
* Constructor.
* Makes the list of plug ins.
* And the languages acording to the plug ins.
*/
KCMKttsMgr::KCMKttsMgr(QWidget *parent, const char *name, const QStringList &) :
    DCOPObject("kttsmgr_kspeechsink"),
    KCModule(KCMKttsMgrFactory::instance(), parent, name)
{
    // kdDebug() << "KCMKttsMgr contructor running." << endl;

    // Initialize some variables.
    m_jobMgrPart = 0;
    m_configDlg = 0;

    // Add the KTTS Manager widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_kttsmgrw = new KCMKttsMgrWidget(this, "kttsmgrw");
    layout->addWidget(m_kttsmgrw);

    // Give buttons icons.
    m_kttsmgrw->lowerTalkerPriorityButton->setIconSet(
        KGlobal::iconLoader()->loadIconSet("down", KIcon::Small));
    m_kttsmgrw->removeTalkerButton->setIconSet(
        KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small));
    m_kttsmgrw->configureTalkerButton->setIconSet(
        KGlobal::iconLoader()->loadIconSet("configure", KIcon::Small));

    // Connect the signals from the KCMKtssMgrWidget to this class.
    connect(m_kttsmgrw->addTalkerButton, SIGNAL(clicked()),
        this, SLOT(addTalker()));
    connect(m_kttsmgrw->lowerTalkerPriorityButton, SIGNAL(clicked()),
        this, SLOT(lowerTalkerPriority()));
    connect(m_kttsmgrw->removeTalkerButton, SIGNAL(clicked()),
        this, SLOT(removeTalker()));
    connect(m_kttsmgrw->configureTalkerButton, SIGNAL(clicked()),
        this, SLOT(slot_configureTalker()));
    connect(m_kttsmgrw->talkersList, SIGNAL(selectionChanged()),
        this, SLOT(updateTalkerButtons()));
    connect(m_kttsmgrw, SIGNAL( configChanged() ),
        this, SLOT( configChanged() ) );
    connect(m_kttsmgrw->enableKttsdCheckBox, SIGNAL(toggled(bool)),
        SLOT(enableKttsdToggled(bool)));
    connect(m_kttsmgrw->mainTab, SIGNAL(currentChanged(QWidget*)),
        this, SLOT(slotTabChanged()));

    // Object for the KTTSD configuration.
    m_config = new KConfig("kttsdrc");

    // Load configuration.
    load();

    // Register DCOP client.
    DCOPClient *client = kapp->dcopClient();
    if (!client->isRegistered())
    {
        client->attach();
        client->registerAs(kapp->name());    
    }

    // Connect KTTSD DCOP signals to our slots.
    if (!connectDCOPSignal("kttsd", "kspeech",
        "kttsdStarted()",
        "kttsdStarted()",
        false)) kdDebug() << "connectDCOPSignal failed" << endl;
    connectDCOPSignal("kttsd", "kspeech",
        "kttsdExiting()",
        "kttsdExiting()",
        false);

    // See if KTTSD is already running.
    if (client->isApplicationRegistered("kttsd"))
        kttsdStarted();
    else
        // Start KTTSD if check box is checked.
        enableKttsdToggled(false);

    // About Dialog.
    m_aboutDlg = new KAboutApplication (aboutData(), m_kttsmgrw, "KDE Text-to-Speech Manager", false);

} 

/**
* Destructor.
*/
KCMKttsMgr::~KCMKttsMgr(){
    // kdDebug() << "KCMKttsMgr::~KCMKttsMgr: Running" << endl;
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

    // Set the group general for the configuration of kttsd itself (no plug ins)
    m_config->setGroup("General");

    // Load the configuration of the text interruption messages and sound
    m_kttsmgrw->textPreMsgCheck->setChecked(m_config->readBoolEntry("TextPreMsgEnabled", textPreMsgCheckValue));
    m_kttsmgrw->textPreMsg->setText(m_config->readEntry("TextPreMsg", textPreMsgValue));
    m_kttsmgrw->textPreMsg->setEnabled(m_kttsmgrw->textPreMsgCheck->isChecked());

    m_kttsmgrw->textPreSndCheck->setChecked(m_config->readBoolEntry("TextPreSndEnabled", textPreSndCheckValue));
    m_kttsmgrw->textPreSnd->setURL(m_config->readPathEntry("TextPreSnd", textPreSndValue));
    m_kttsmgrw->textPreSnd->setEnabled(m_kttsmgrw->textPreSndCheck->isChecked());

    m_kttsmgrw->textPostMsgCheck->setChecked(m_config->readBoolEntry("TextPostMsgEnabled", textPostMsgCheckValue));
    m_kttsmgrw->textPostMsg->setText(m_config->readEntry("TextPostMsg", textPostMsgValue));
    m_kttsmgrw->textPostMsg->setEnabled(m_kttsmgrw->textPostMsgCheck->isChecked());

    m_kttsmgrw->textPostSndCheck->setChecked(m_config->readBoolEntry("TextPostSndEnabled", textPostSndCheckValue));
    m_kttsmgrw->textPostSnd->setURL(m_config->readPathEntry("TextPostSnd", textPostSndValue));
    m_kttsmgrw->textPostSnd->setEnabled(m_kttsmgrw->textPostSndCheck->isChecked());

    // Overall settings.
    m_kttsmgrw->enableKttsdCheckBox->setChecked(m_config->readBoolEntry("EnableKttsd",
        m_kttsmgrw->enableKttsdCheckBox->isChecked()));

    // Notification settings.
    m_kttsmgrw->enableNotifyCheckBox->setChecked(m_config->readBoolEntry("Notify",
        m_kttsmgrw->enableNotifyCheckBox->isChecked()));
    m_kttsmgrw->enablePassiveOnlyCheckBox->setChecked(m_config->readBoolEntry("NotifyPassivePopupsOnly",
        m_kttsmgrw->enablePassiveOnlyCheckBox->isChecked()));

    // Last plugin ID.  Used to generate a new ID for an added talker.
    m_lastTalkerID = m_config->readEntry("LastTalkerID", "0");

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
            QString languageCode;
            talkerCode = normalizeTalkerCode(talkerCode, languageCode);
            QString language = languageCodeToLanguage(languageCode);
            QString plugInName = m_config->readEntry("PlugIn", "");
            // kdDebug() << "KCMKttsMgr::load: talkerCode = " << talkerCode << endl;
            if (talkerItem)
                talkerItem =
                    new KListViewItem(m_kttsmgrw->talkersList, talkerItem, talkerID, language, plugInName);
            else
                talkerItem =
                    new KListViewItem(m_kttsmgrw->talkersList, talkerID, language, plugInName);
            updateTalkerItem(talkerItem, talkerCode);
            m_languagesToCodes[language] = languageCode;
        }
    }

    // Query for all the KCMKTTSD SynthPlugins and store the list in m_offers.
    m_offers = KTrader::self()->query("KTTSD/SynthPlugin");

    // Iterate thru the posible plug ins getting their language support codes.
    for(unsigned int i=0; i < m_offers.count() ; ++i)
    {
        QString plugInName = m_offers[i]->name();
        QStringList languageCodes = m_offers[i]->property("X-KDE-Languages").toStringList();
        // Add language codes to the language-to-language code map.
        QStringList::ConstIterator endLanguages(languageCodes.constEnd());
        for( QStringList::ConstIterator it = languageCodes.constBegin(); it != endLanguages; ++it )
        {
            QString language = languageCodeToLanguage(*it);
            m_languagesToCodes[language] = *it;
        }

        // All plugins support "Other".
        // TODO: Eventually, this should not be necessary, since all plugins will know
        // the languages they support and report them in call to getSupportedLanguages().
        if (!languageCodes.contains("other")) languageCodes.append("other");

        // Add supported language codes to synthesizer-to-language map.
        m_synthToLangMap[plugInName] = languageCodes;
    }

    // Add "Other" language.
    m_languagesToCodes[i18n("Other")] = "other";

    // Uncheck and disable KTTSD checkbox if no Talkers are configured.
    if (m_kttsmgrw->talkersList->childCount() == 0)
    {
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(false);
        enableKttsdToggled(false);
    }

    updateTalkerButtons();
}

/**
* Converts a language code plus optional country code to language description.
*/
QString KCMKttsMgr::languageCodeToLanguage(const QString &languageCode)
{
    QString twoAlpha;
    QString countryCode;
    QString charSet;
    QString language;
    if (languageCode == "other")
        language = i18n("Other");
    else
    {
        KGlobal::locale()->splitLocale(languageCode, twoAlpha, countryCode, charSet);
        language = KGlobal::locale()->twoAlphaToLanguageName(twoAlpha);
    }
    if (!countryCode.isEmpty())
        language += " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode) + ")";
    return language;
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
    // Clean up config.
    m_config->deleteGroup("General");

    // Set the group general for the configuration of kttsd itself (no plug ins)
    m_config->setGroup("General");

    // Set text interrumption messages and paths
    m_config->writeEntry("TextPreMsgEnabled", m_kttsmgrw->textPreMsgCheck->isChecked());
    m_config->writeEntry("TextPreMsg", m_kttsmgrw->textPreMsg->text());

    m_config->writeEntry("TextPreSndEnabled", m_kttsmgrw->textPreSndCheck->isChecked()); 
    m_config->writePathEntry("TextPreSnd", m_kttsmgrw->textPreSnd->url());

    m_config->writeEntry("TextPostMsgEnabled", m_kttsmgrw->textPostMsgCheck->isChecked());
    m_config->writeEntry("TextPostMsg", m_kttsmgrw->textPostMsg->text());

    m_config->writeEntry("TextPostSndEnabled", m_kttsmgrw->textPostSndCheck->isChecked());
    m_config->writePathEntry("TextPostSnd", m_kttsmgrw->textPostSnd->url());

    // Overall settings.
    m_config->writeEntry("EnableKttsd", m_kttsmgrw->enableKttsdCheckBox->isChecked());

    // Notification settings.
    m_config->writeEntry("Notify", m_kttsmgrw->enableNotifyCheckBox->isChecked());
    m_config->writeEntry("NotifyPassivePopupsOnly", m_kttsmgrw->enablePassiveOnlyCheckBox->isChecked());

    // Get ordered list of all talker IDs.
    QStringList talkerIDsList;
    QListViewItem* talkerItem = m_kttsmgrw->talkersList->firstChild();
    while (talkerItem)
    {
        QListViewItem* nextTalkerItem = talkerItem->nextSibling();
        QString talkerID = talkerItem->text(tlvcTalkerID);
        talkerIDsList.append(talkerID);
        talkerItem = nextTalkerItem;
    }
    QString talkerIDs = talkerIDsList.join(",");
    m_config->writeEntry("TalkerIDs", talkerIDs);
    m_config->writeEntry("LastTalkerID", m_lastTalkerID);

    m_config->sync();

    // Uncheck and disable KTTSD checkbox if no Talkers are configured.
    // Enable checkbox if at least one Talker is configured.
    if (m_kttsmgrw->talkersList->childCount() == 0)
    {
        m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(false);
        enableKttsdToggled(false);
    }
    else
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(true);

    // If KTTSD is running, reinitialize it.
    DCOPClient *client = kapp->dcopClient();
    bool kttsdRunning = (client->isApplicationRegistered("kttsd"));
    if (kttsdRunning)
    {
        kdDebug() << "Restarting KTTSD" << endl;
        QByteArray data;
        client->send("kttsd", "kspeech", "reinit()", data);
    }
}

void KCMKttsMgr::slotTabChanged()
{
    setButtons(buttons());
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
            if (m_kttsmgrw->enableNotifyCheckBox->isChecked() != enableNotifyCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->enableNotifyCheckBox->setChecked(enableNotifyCheckBoxValue);
            }
            if (m_kttsmgrw->enablePassiveOnlyCheckBox->isChecked() != enablePassiveOnlyCheckBoxValue)
            {
                changed = true;
                m_kttsmgrw->enablePassiveOnlyCheckBox->setChecked(enablePassiveOnlyCheckBoxValue);
            }
            break;

        case wpInterruption:
            if (m_kttsmgrw->textPreMsgCheck->isChecked() != textPreMsgCheckValue)
            {
                changed = true;
                m_kttsmgrw->textPreMsgCheck->setChecked(textPreMsgCheckValue);
            }
            if (m_kttsmgrw->textPreMsg->text() != textPreMsgValue)
            {
                changed = true;
                m_kttsmgrw->textPreMsg->setText(textPreMsgValue);
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
            if (m_kttsmgrw->textPostMsg->text() != textPostMsgValue)
            {
                changed = true;
                m_kttsmgrw->textPostMsg->setText(textPostMsgValue);
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
    }
    if (changed) configChanged();
}

/**
* This is a static method which gets called to realize the modules settings
* durign the startup of KDE. NOTE that most modules do not implement this
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
    if (!m_kttsmgrw)
        return KCModule::Ok|KCModule::Apply|KCModule::Help;
    else
    {
        // TODO: This isn't working.  We prefer to hide (or disable) Defaults button
        // on all except Interruption tab.
        int currentPageIndex = m_kttsmgrw->mainTab->currentPageIndex();
        if (currentPageIndex == wpInterruption)
            return KCModule::Ok|KCModule::Apply|KCModule::Help|KCModule::Default;
        else
            return KCModule::Ok|KCModule::Apply|KCModule::Help;
    }
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
        I18N_NOOP("(c) 2002, Jos�Pablo Ezequiel Fern�dez"));

    about->addAuthor("Jos�Pablo Ezequiel Fern�dez", I18N_NOOP("Author") , "pupeno@kde.org");
    about->addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer") , "garycramblitt@comcast.net");
    about->addAuthor("Olaf Schmidt", I18N_NOOP("Contributor"), "ojschmidt@kde.org");
    about->addAuthor("Paul Giannaros", I18N_NOOP("Contributor"), "ceruleanblaze@gmail.com");

    return about;
}

/**
* Given a talker code, normalizes it into a standard form, and extracts language code.
* @param talkerCode      Unnormalized talker code.
* @param languageCode    Parsed language code.
* @return                Normalized talker code.
*/
QString KCMKttsMgr::normalizeTalkerCode(const QString &talkerCode, QString& languageCode)
{
    QString voice;
    QString gender;
    QString volume;
    QString rate;
    QString plugInName;
    parseTalkerCode(talkerCode, languageCode, voice, gender, volume, rate, plugInName);
    if (voice.isEmpty()) voice = "fixed";
    if (gender.isEmpty()) gender = "neutral";
    if (volume.isEmpty()) volume = "medium";
    if (rate.isEmpty()) rate = "medium";
    QString normalTalkerCode = QString(
        "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
        "<prosody volume=\"%4\" rate=\"%5\" />"
        "<kttsd synthesizer=\"%6\" />")
        .arg(languageCode)
        .arg(voice)
        .arg(gender)
        .arg(volume)
        .arg(rate)
        .arg(plugInName);
    return normalTalkerCode;
}

/**
* Given a talker code, parses out the attributes.
* @param talkerCode       The talker code.
* @return languageCode    Language Code.
* @return voice           Voice name.
* @return gender          Gender.
* @return volume          Volume.
* @return rate            Rate.
* @return plugInName      Name of Synthesizer.
*/
void KCMKttsMgr::parseTalkerCode(const QString &talkerCode,
    QString &languageCode,
    QString &voice,
    QString &gender,
    QString &volume,
    QString &rate,
    QString &plugInName)
{
    languageCode = talkerCode.section("lang=", 1, 1);
    languageCode = languageCode.section('"', 1, 1);
    voice = talkerCode.section("name=", 1, 1);
    voice = voice.section('"', 1, 1);
    gender = talkerCode.section("gender=", 1, 1);
    gender = gender.section('"', 1, 1);
    volume = talkerCode.section("volume=", 1, 1);
    volume = volume.section('"', 1, 1);
    rate = talkerCode.section("rate=", 1, 1);
    rate = rate.section('"', 1, 1);
    plugInName = talkerCode.section("synthesizer=", 1, 1);
    plugInName = plugInName.section('"', 1, 1);
}

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
QString KCMKttsMgr::defaultTalkerCode(const QString &languageCode, const QString &plugInName)
{
    QString talkerCode = QString(
        "<voice lang=\"%1\" name=\"fixed\" gender=\"neutral\" />"
        "<prosody volume=\"medium\" rate=\"medium\" />"
        "<kttsd synthesizer=\"%1\" />")
        .arg(languageCode)
        .arg(plugInName);
    return talkerCode;
}

/**
* Loads the configuration plug in for a named plug in.
*/
PlugInConf *KCMKttsMgr::loadPlugin(const QString &plugInName)
{
    // kdDebug() << "KCMKttsMgr::loadPlugin: Running"<< endl;

    // Iterate thru the plug in m_offers to find the plug in that matches the plugInName.
    for(unsigned int i=0; i < m_offers.count() ; ++i){
        // Compare the plug in to be loaded with the entry in m_offers[i]
        // kdDebug() << "Comparing " << m_offers[i]->name() << " to " << plugInName << endl;
        if(m_offers[i]->name() == plugInName){
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(m_offers[i]->library());
        if(factory){
            // If the factory is created successfully, instantiate the PlugInConf class for the
            // specific plug in to get the plug in configuration object.
            PlugInConf *plugIn = KParts::ComponentFactory::createInstanceFromLibrary<PlugInConf>(m_offers[i]->library(), NULL, m_offers[i]->library());
            if(plugIn){
                // If everything went ok, return the plug in pointer.
                return plugIn;
            } else {
                // Something went wrong, returning null.
                return NULL;
            }
        } else {
            // Something went wrong, returning null.
            return NULL;
        }
        break;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    return NULL;
}

/**
* Given an item in the talker listview and a talker code, sets the columns of the item.
* @param talkerItem       QListViewItem.
* @param talkerCode       Talker Code.
*/
void KCMKttsMgr::updateTalkerItem(QListViewItem* talkerItem, const QString &talkerCode)
{
    QString languageCode;
    QString voice;
    QString gender;
    QString volume;
    QString rate;
    QString plugInName;
    parseTalkerCode(talkerCode, languageCode, voice, gender, volume, rate, plugInName);
    if (!languageCode.isEmpty())
    {
        QString language = languageCodeToLanguage(languageCode);
        if (!language.isEmpty())
        {
            m_languagesToCodes[language] = languageCode;
            talkerItem->setText(tlvcLanguage, language);
        }
    }
    if (!plugInName.isEmpty()) talkerItem->setText(tlvcPlugInName, plugInName);
    if (!voice.isEmpty()) talkerItem->setText(tlvcVoice, voice);
    if (!gender.isEmpty()) talkerItem->setText(tlvcGender, gender);
    if (!volume.isEmpty()) talkerItem->setText(tlvcVolume, volume);
    if (!rate.isEmpty()) talkerItem->setText(tlvcRate, rate);
}

/**
* Add a talker.
*/
void KCMKttsMgr::addTalker(){
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
    QString plugInName = addTalkerWidget->getSynthesizer();
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
            KGlobal::locale()->splitLocale(locale, languageCode, countryCode, charSet);
            language = KGlobal::locale()->twoAlphaToLanguageName(languageCode);
            if (!countryCode.isEmpty()) language +=
                        " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode)+")";
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
        if (langLView->currentItem()) languageCode = langLView->currentItem()->text(1);
        delete dlg;
        // TODO: Also delete KListView and QHBox?
        if (dlgResult != QDialog::Accepted) return;
    }

    if (languageCode.isEmpty()) return;
    QString language = languageCodeToLanguage(languageCode);
    if (language.isEmpty()) return;

    m_languagesToCodes[language] = languageCode;

    // Assign a new Talker ID for the talker.  Wraps around to 1.
    QString talkerID = QString::number(m_lastTalkerID.toInt()+1);

    // Erase extraneous Talker configuration entries that might be there.
    m_config->deleteGroup(QString("Talker_")+talkerID);
    m_config->sync();

    // Load the plugin.
    m_loadedPlugIn = loadPlugin(plugInName);
    if (!m_loadedPlugIn) return;

    // Give plugin the user's language code and permit plugin to autoconfigure itself.
    m_loadedPlugIn->setDesiredLanguage(languageCode);
    m_loadedPlugIn->load(m_config, QString("Talker_")+talkerID);

    // If plugin was able to configure itself, it returns a full talker code.
    // If not, display configuration dialog for user to configure the plugin.
    QString talkerCode = m_loadedPlugIn->getTalkerCode();
    if (talkerCode.isEmpty())
    {
        // Display configuration dialog.
        configureTalker();
        // Did user Cancel?
        if (!m_loadedPlugIn) return;
        talkerCode = m_loadedPlugIn->getTalkerCode();
    }

    // If still no Talker Code, abandon.
    if (!talkerCode.isEmpty())
    {
        // Let plugin save its configuration.
        m_config->setGroup(QString("Talker_")+talkerID);
        m_loadedPlugIn->save(m_config, QString("Talker_"+talkerID));

        // Record last Talker ID used for next add.
        m_lastTalkerID = talkerID;

        // Record configuration data.  Note, might as well do this now.
        m_config->setGroup(QString("Talker_")+talkerID);
        m_config->writeEntry("PlugIn", plugInName);
        talkerCode = normalizeTalkerCode(talkerCode, languageCode);
        m_config->writeEntry("TalkerCode", talkerCode);
        m_config->sync();

        // Add listview item.
        QListViewItem* talkerItem = m_kttsmgrw->talkersList->lastChild();
        if (talkerItem)
            talkerItem =
                new KListViewItem(m_kttsmgrw->talkersList, talkerItem, m_lastTalkerID, language, plugInName);
        else
            talkerItem =
                new KListViewItem(m_kttsmgrw->talkersList, m_lastTalkerID, language, plugInName);

        // Set additional columns of the listview item.
        updateTalkerItem(talkerItem, talkerCode);

        // Make sure visible.
        m_kttsmgrw->talkersList->ensureItemVisible(talkerItem);

        // Enable Start KTTS checkbox.
        m_kttsmgrw->enableKttsdCheckBox->setEnabled(true);

        // Select the new item, update buttons.
        m_kttsmgrw->talkersList->setSelected(talkerItem, true);
        updateTalkerButtons();

        // Inform Control Center that change has been made.
        configChanged();
    }

    // Don't need plugin in memory anymore.
    delete m_loadedPlugIn;
    m_loadedPlugIn = 0;

    // kdDebug() << "KCMKttsMgr::addTalker: done." << endl;
}

/**
* Remove talker. 
*/
void KCMKttsMgr::removeTalker(){
    // kdDebug() << "KCMKttsMgr::removeTalker: Running"<< endl;

    // Get the selected talker.
    QListViewItem *itemToRemove = m_kttsmgrw->talkersList->selectedItem();
    if (!itemToRemove) return;

    // Delete the talker from configuration file.
    QString talkerID = itemToRemove->text(tlvcTalkerID);
    m_config->deleteGroup("Talker_"+talkerID, true, false);

    // Delete the talker from list view.
    delete itemToRemove;

    updateTalkerButtons();

    // Emit configuraton changed.
    configChanged();
}

/**
* This slot is called whenever user clicks the lowerTalkerPriority button.
*/
void KCMKttsMgr::lowerTalkerPriority()
{
    QListViewItem* talkerItem = m_kttsmgrw->talkersList->selectedItem();
    if (!talkerItem) return;
    QListViewItem* nextTalkerItem = talkerItem->nextSibling();
    if (!nextTalkerItem) return;
    talkerItem->moveItem(nextTalkerItem);
    m_kttsmgrw->talkersList->setSelected(talkerItem, true);
    updateTalkerButtons();
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
        m_kttsmgrw->lowerTalkerPriorityButton->setEnabled(
            m_kttsmgrw->talkersList->selectedItem()->nextSibling() != 0);
    } else {
        m_kttsmgrw->removeTalkerButton->setEnabled(false);
        m_kttsmgrw->configureTalkerButton->setEnabled(false);
        m_kttsmgrw->lowerTalkerPriorityButton->setEnabled(false);
    }
    // kdDebug() << "KCMKttsMgr::updateTalkerButtons: Exiting"<< endl;
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
    // kdDebug() << "KCMKttsMgr::enableKttsdToggled: Running" << endl;
    // See if KTTSD is running.
    DCOPClient *client = kapp->dcopClient();
    bool kttsdRunning = (client->isApplicationRegistered("kttsd"));
    // If Enable KTTSD check box is checked and it is not running, then start KTTSD.
    if (m_kttsmgrw->enableKttsdCheckBox->isChecked())
    {
        if (!kttsdRunning)
        {
            QString error;
            if (KApplication::startServiceByName("KTTSD", QStringList(), &error))
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
            QByteArray data;
            client->send("kttsd", "kspeech", "kttsdExit()", data);
        }
    }
    reenter = false;
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
            m_jobMgrPart = (KParts::ReadOnlyPart *)factory->create( m_kttsmgrw->mainTab, "kttsjobmgrpart",
                "KParts::ReadOnlyPart" );
            if (m_jobMgrPart)
            {
                // Add the Job Manager part as a new tab.
                m_kttsmgrw->mainTab->addTab(m_jobMgrPart->widget(), i18n("Jobs"));
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
* User has requested display talker configuration dialog.
*/
void KCMKttsMgr::slot_configureTalker()
{
    // Get highlighted plugin from Talker ListView and load into memory.
    QListViewItem* talkerItem = m_kttsmgrw->talkersList->selectedItem();
    if (!talkerItem) return;
    QString talkerID = talkerItem->text(tlvcTalkerID);
    QString plugInName = talkerItem->text(tlvcPlugInName);
    QString language = talkerItem->text(tlvcLanguage);
    QString languageCode = m_languagesToCodes[language];
    m_loadedPlugIn = loadPlugin(plugInName);
    if (!m_loadedPlugIn) return;

    // Tell plugin to load its configuration.
    m_config->setGroup(QString("Talker_")+talkerID);
    m_loadedPlugIn->setDesiredLanguage(languageCode);
    m_loadedPlugIn->load(m_config, QString("Talker_")+talkerID);

    // Display configuration dialog.
    configureTalker();

    // Did user Cancel?
    if (!m_loadedPlugIn) return;

    // Get Talker Code.  Note that plugin may return a code different from before.
    QString talkerCode = m_loadedPlugIn->getTalkerCode();

    // If plugin was successfully configured, save its configuration.
    if (!talkerCode.isEmpty())
    {
        m_config->setGroup(QString("Talker_")+talkerID);
        m_loadedPlugIn->save(m_config, QString("Talker_")+talkerID);
        m_config->setGroup(QString("Talker_")+talkerID);
        talkerCode = normalizeTalkerCode(talkerCode, languageCode);
        m_config->writeEntry("TalkerCode", talkerCode);
        m_config->sync();

        // Update display.
        updateTalkerItem(talkerItem, talkerCode);

        // Inform Control Center that configuration has changed.
        configChanged();
    }

    delete m_loadedPlugIn;
    m_loadedPlugIn = 0;
}

/**
* Display talker configuration dialog.  The plugin is assumed already loaded into
* memory referenced by m_loadedPlugIn.
*/
void KCMKttsMgr::configureTalker()
{
    if (!m_loadedPlugIn) return;
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
    m_configDlg->setMainWidget(m_loadedPlugIn);
    m_configDlg->setHelp("configure-plugin", "kttsd");
    m_configDlg->enableButtonOK(false);
    connect(m_loadedPlugIn, SIGNAL( changed(bool) ), this, SLOT( slotConfigDlg_ConfigChanged() ));
    connect(m_configDlg, SIGNAL( defaultClicked() ), this, SLOT( slotConfigDlg_DefaultClicked() ));
    connect(m_configDlg, SIGNAL( okClicked() ), this, SLOT( slotConfigDlg_OkClicked() ));
    connect(m_configDlg, SIGNAL( cancelClicked() ), this, SLOT (slotConfigDlg_CancelClicked() ));
    m_configDlg->exec();
}

void KCMKttsMgr::slotConfigDlg_ConfigChanged()
{
    m_configDlg->enableButtonOK(!m_loadedPlugIn->getTalkerCode().isEmpty());
}

void KCMKttsMgr::slotConfigDlg_DefaultClicked()
{
    m_loadedPlugIn->defaults();
}

void KCMKttsMgr::slotConfigDlg_OkClicked()
{
    kdDebug() << "KCMKttsMgr::slotConfigDlg_OkClicked: Running" << endl;
}

void KCMKttsMgr::slotConfigDlg_CancelClicked()
{
    delete m_loadedPlugIn;
    m_loadedPlugIn = 0;
}

// System tray context menu entries.
void KCMKttsMgr::aboutSelected(){
    m_aboutDlg->show();
}
