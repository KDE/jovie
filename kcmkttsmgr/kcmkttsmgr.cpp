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

// $Id$

#include <dcopclient.h>

#include <qtabwidget.h>
//#include <qwidgetstack.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qwaitcondition.h>

#include <klistview.h>
#include <kcombobox.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kgenericfactory.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kaboutapplication.h>
#include <kpopupmenu.h>

#include "kcmkttsmgr.moc"
#include "kcmkttsmgr.h"
#include "pluginconf.h"

// Some constants
const bool textPreMsgCheckValue = true;
const QString textPreMsgValue = i18n("Text interrupted. Message.");

const bool textPreSndCheckValue = false;
const QString textPreSndValue = "";

const bool textPostMsgCheckValue = true;
const QString textPostMsgValue = i18n("Resuming text.");

const bool textPostSndCheckValue = false;
const QString textPostSndValue = "";

// Make this a plug in.
typedef KGenericFactory<KCMKttsMgr, QWidget> KCMKttsMgrFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kttsmgr, KCMKttsMgrFactory("kcmkttsmgr") );

/**
* Constructor
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
    m_pluginWidget = 0;
    
    //Defaults
    //textPreMsgValue = i18n("Paragraph interrupted. Message.");

    // Add the KTTS Manager widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_kttsmgrw = new KCMKttsMgrWidget(this, "kttsmgrw");
    layout->addWidget(m_kttsmgrw);
    
    // Start out with Properties tab disabled.  It will be enabled when user selects a language.
//    m_kttsmgrw->mainTab->setTabEnabled(m_kttsmgrw->PluginWidgetStack, false);
//    m_kttsmgrw->PluginWidgetStack->setEnabled(false);

    // Connect the signals from the KCMKtssMgrWidget to this class
    connect( m_kttsmgrw, SIGNAL( addLanguage() ), this, SLOT( addLanguage() ) );
    connect( m_kttsmgrw, SIGNAL( configChanged() ), this, SLOT( configChanged() ) );
    connect( m_kttsmgrw, SIGNAL( removeLanguage() ), this, SLOT( removeLanguage() ) );
    connect( m_kttsmgrw, SIGNAL( setDefaultLanguage() ), this, SLOT( setDefaultLanguage() ) );
    connect( m_kttsmgrw, SIGNAL( updateRemoveButton() ), this, SLOT( updateRemoveButton() ) );
    connect( m_kttsmgrw, SIGNAL( updateDefaultButton() ), this, SLOT( updateDefaultButton() ) );
    connect( m_kttsmgrw->enableKttsdCheckBox, SIGNAL(toggled(bool)), SLOT( enableKttsdToggled(bool) ) );

    // List of languages to be added, true add, false, leave.
    QMap<QString, bool> languagesToBeAdded;

    // Initializate the list of codes
    m_languagesMap["other"] = i18n("Other");
    QMap<QString, QString>::ConstIterator endLanguagesMap(m_languagesMap.constEnd());
    for( QMap<QString, QString>::ConstIterator it = m_languagesMap.constBegin(); it != endLanguagesMap; ++it ){
        m_reverseLanguagesMap[it.data()] = it.key();
        languagesToBeAdded[it.key()] = false;
    }
    languagesToBeAdded["other"] = true;

    // Object for the KTTSD configuration
    m_config = new KConfig("kttsdrc");

    // Set autoDelete to the list of structures of languages/plugins
    m_loadedLanguages.setAutoDelete(true);

    // Query for all the KCMKTTSD SynthPlugins and store the list in m_offers
    m_offers = KTrader::self()->query("KTTSD/SynthPlugin");

    // Iterate thru the posible plug ins and have them added to the combo box plugInSelection and make the list of posible languages
    for(unsigned int i=0; i < m_offers.count() ; ++i){
        m_kttsmgrw->plugInSelection->insertItem(m_offers[i]->name(), i);

        // Add the plug in to the combo box
        QStringList languages = m_offers[i]->property("X-KDE-Languages").toStringList();

        QStringList::ConstIterator endLanguages(languages.constEnd());
        for( QStringList::ConstIterator it = languages.constBegin(); it != endLanguages; ++it ) {
            languagesToBeAdded[*it] = true;
            initLanguageCode (*it);
        }
    }

    // Insert the list of languages into the comobo box for language selection
    QMap<QString, bool>::ConstIterator endLanguagesToBeAdded(languagesToBeAdded.constEnd());
    for( QMap<QString, bool>::ConstIterator it = languagesToBeAdded.constBegin(); it != endLanguagesToBeAdded; ++it ) {
        if(it.data()){
            m_kttsmgrw->languageSelection->insertItem(m_languagesMap[it.key()]);
        }
    }

    // Load the configuration from the file to the widgets :P
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
* Destructor
*/
KCMKttsMgr::~KCMKttsMgr(){
    // kdDebug() << "Running: KCMKttsMgr::~KCMKttsMgr()" << endl;
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
    // kdDebug() << "Running: KCMKttsMgr::load()"<< endl;

    // Get rid of everything first for the sake of the reset button
    // Maybe we should do something here to jump to the right tab if the reset button was presed being in tab, anyway, that's not so bad
    QStringList languagesToRemove;
    for( QDictIterator<languageRelatedObjects> it( m_loadedLanguages ) ; it.current(); ++it ){
        languagesToRemove << it.currentKey();
    }
    for( QStringList::Iterator it = languagesToRemove.begin(); it != languagesToRemove.end(); ++it ) {
        removeLanguage(*it);
    }

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
    
    QString defaultLanguage = m_config->readEntry("DefaultLanguage");

    // Iterate thru loaded languages and load them and their configuration
    QStringList langs = m_config->groupList().grep("Lang_");
    QStringList::ConstIterator endLangs(langs.constEnd());
    for( QStringList::ConstIterator it = langs.constBegin(); it != endLangs; ++it ) {
        QString langcode = (*it).right((*it).length()-5);
        kdDebug() << "Loading: " << *it << " Langcode: " << langcode << endl;
        m_config->setGroup(*it);
        addLanguage( langcode, m_config->readEntry("PlugIn"));
        if(!m_loadedLanguages.isEmpty()){
            m_loadedLanguages[langcode]->plugIn->load(m_config, *it);
        }
    }

    // we need the languages loaded to set the default one, chicken and egg, never liked chicken
    setDefaultLanguage(defaultLanguage);
    updateDefaultButton();
    updateRemoveButton();
} 

/**
* This function gets called when the user wants to save the settings in 
* the user interface, updating the config files or wherever the 
* configuration is stored. The method is called when the user clicks "Apply" 
* or "Ok".
*/
void KCMKttsMgr::save()
{
    kdDebug() << "Running: KCMKttsMgr::save()"<< endl;
    // Clean up config before saving everything (as kconfig merges, when a language is removed from memory we have to ensure that is removed from disk too)
    QStringList allGroups = m_config->groupList();
    for( QStringList::Iterator it = allGroups.begin(); it != allGroups.end(); ++it )
    {
        m_config->deleteGroup(*it);
    }

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
    
    // Iterate thru loaded languages and store their configuration
    for(QDictIterator<languageRelatedObjects> it(m_loadedLanguages); it.current() ; ++it){
        // kdDebug() << "Saving: " << it.currentKey() << endl;
        
        // Let's check if this is the default language
        if(it.current()->listItem->pixmap(2)){
            // If it is, store it, and store it in General
            m_config->setGroup("General");
            m_config->writeEntry("DefaultLanguage", it.currentKey());
        }
        m_config->setGroup(QString("Lang_")+it.currentKey());
        m_config->writeEntry("PlugIn", it.current()->plugInName);
        it.current()->plugIn->save(m_config, QString("Lang_")+it.currentKey());
    }
    
    m_config->sync();
    
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

/**
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.
*/
void KCMKttsMgr::defaults() {
    // kdDebug() << "Running: KCMKttsMgr::defaults()"<< endl;
    
    m_kttsmgrw->textPreMsgCheck->setChecked(textPreMsgCheckValue);
    m_kttsmgrw->textPreMsg->setText(textPreMsgValue);
    
    m_kttsmgrw->textPreSndCheck->setChecked(textPreSndCheckValue);
    m_kttsmgrw->textPreSnd->setURL(textPreSndValue);
    
    m_kttsmgrw->textPostMsgCheck->setChecked(textPostMsgCheckValue);
    m_kttsmgrw->textPostMsg->setText(textPostMsgValue);
    
    m_kttsmgrw->textPostSndCheck->setChecked(textPostSndCheckValue);
    m_kttsmgrw->textPostSnd->setURL(textPostSndValue);
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
    // kdDebug() << "Running: KCMKttsMgr::init()"<< endl;
} 

/**
* The control center calls this function to decide which buttons should
* be displayed. For example, it does not make sense to display an "Apply" 
* button for one of the information modules. The value returned can be set by 
* modules using setButtons.
*/
int KCMKttsMgr::buttons() {
    // kdDebug() << "Running: KCMKttsMgr::buttons()"<< endl;
    return KCModule::Ok|KCModule::Apply|KCModule::Help|KCModule::Reset;
} 

/**
* This function returns the small quickhelp.
* That is displayed in the sidebar in the KControl
*/
QString KCMKttsMgr::quickHelp() const{
    // kdDebug() << "Running: KCMKttsMgr::quickHelp()"<< endl;
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
    about->addAuthor("Gary Cramblitt", I18N_NOOP("Author") , "garycramblitt@comcast.net");
    about->addAuthor("Olaf Schmidt", I18N_NOOP("Author"), "<ojschmidt@kde.org>");

    return about;
}

/**
* Loads the configuration plug in for a specific plug in
*/
PlugInConf *KCMKttsMgr::loadPlugIn(const QString &plugInName){
    // kdDebug() << "Running: KCMKttsMgr::loadPlugIn(const QString &plugInName)"<< endl;

    // Iterate thru the plug in m_offers to find the plug in that matches the plugInName
    for(unsigned int i=0; i < m_offers.count() ; ++i){
        // Compare the plug in to be loeaded with the entry in m_offers[i]
        // kdDebug() << "Comparing " << m_offers[i]->name() << " to " << plugInName << endl;
        if(m_offers[i]->name() == plugInName){
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(m_offers[i]->library());
        if(factory){
            // If the factory is created succesfully, isntatiate the PlugInConf class for the specific plug in to get the plug in configuration object.
            PlugInConf *plugIn = KParts::ComponentFactory::createInstanceFromLibrary<PlugInConf>(m_offers[i]->library(), NULL, m_offers[i]->library());
            if(plugIn){
                // If everything went ok, return the plug in pointer
                return plugIn;
            } else {
                // Something went wront, returning null
                return NULL;
            }
        } else {
            // Something went wront, returning null
            return NULL;
        }
        break;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null)
    return NULL;
}
    
/**
* Add a language
* This is a wrapper function that takes the parameters for the real addLanguage from the
* widgets to later call it.
*/
void KCMKttsMgr::addLanguage(){
    // kdDebug() << "Running: KCMKttsMgr::addLanguage()"<< endl;
    // kdDebug() << "Adding plug in " << m_kttsmgrw->plugInSelection->currentText() << " for language " << m_kttsmgrw->languageSelection->currentText() << endl;
    
    if(m_loadedLanguages.find(m_reverseLanguagesMap[m_kttsmgrw->languageSelection->currentText()]) == 0){
        if(m_reverseLanguagesMap[m_kttsmgrw->languageSelection->currentText()] == "other"){
            QString customLanguage = KLineEditDlg::getText(i18n("Create custom language"), i18n("Please enter the code for the custom language:"));
            addLanguage( customLanguage, m_kttsmgrw-> plugInSelection->currentText());
        } else {
            addLanguage( m_reverseLanguagesMap[m_kttsmgrw->languageSelection->currentText()], m_kttsmgrw->plugInSelection->currentText());
        }
    } else {
        KMessageBox::error(0, i18n("This language already has a plugin assigned; please remove it before re-assigning it"), i18n("Language not valid"));
    }
}

/**
* Add a language to the pool of languages for the lang/plugIn pair
*/
void KCMKttsMgr::addLanguage(const QString &language, const QString &plugInName){
    // kdDebug() << "Running: KCMKttsMgr::addLanguage(const QString &lang, const QString &plugInName)"<< endl;
    // kdDebug() << "Adding plug in " << plugInName << " for language " << language << endl;
    
    // If the language is not in the map, let's add it.
    if(!m_languagesMap.contains(language)){
        initLanguageCode (language);
    }
    
    // This object will contain pointers to all the objects related to a single language/plug in pair.
    // kdDebug() << "Creating newLanguage" << endl;
    languageRelatedObjects *newLanguage = new languageRelatedObjects();
    
    // Load the plug in
    // kdDebug() << "Creating the new plugIn" << endl;
    newLanguage->plugIn = loadPlugIn(plugInName);
    
    // If there wasn't any error, let's go on
    if(newLanguage->plugIn){
        // Create the item in the list and add it and store the pointer in the structure
        // kdDebug() << "Creating the new listItem" << endl;
    
        newLanguage->listItem = new KListViewItem(m_kttsmgrw->languagesList, m_languagesMap[language], plugInName);
    
        // Store the name of the plug in in the structure
        // kdDebug() << "Storing the plug in name" << endl;
        newLanguage->plugInName = plugInName;
    
        // Add the plugin configuration widget to the Properties tab WidgetStack.
        // kdDebug() << "Adding the tab" << endl;
//        m_kttsmgrw->PluginWidgetStack->addWidget(newLanguage->plugIn, -1);

        // Let plug in changes be as global changed to show apply button.
        // kdDebug() << "Connecting" << endl;
        connect(  newLanguage->plugIn, SIGNAL( changed(bool) ), this, SLOT( configChanged() )  );
    
        // Let's insert it in the QDict to keep track of language structure
        // kdDebug() << "Inserting the newLanguage in the QDict" << endl;
        m_loadedLanguages.insert( language, newLanguage);
    
        // kdDebug() << "Done" <<endl;
    } else {
        KMessageBox::error(0, i18n("Speech syntheziser plugin library not found or corrupted."), i18n("Plugin not found"));
    }
}

/**
* Initializes the language with the given language code by determining
* its name.
*/
void KCMKttsMgr::initLanguageCode (const QString &code) {
    if(!m_languagesMap.contains(code)) {
        QString name = QString::null;
    
        QString file = locate("locale", QString::fromLatin1("%1/entry.desktop").arg(code));
        if (!file.isNull() && !file.isEmpty()) {
            KSimpleConfig entry(file);
            entry.setGroup("KCM Locale");
            name = entry.readEntry("Name", QString::null);
        }
    
        if (name.isNull() || name.isEmpty())
            name = code;
        else
            name = name + QString::fromLatin1(" (%1)").arg(code);
    
        m_languagesMap[code] = name;
        m_reverseLanguagesMap[name] = code;
    }
}

/**
* Remove language 
* This is a wrapper function that takes the parameters for the real removeLanguage from the
* widgets to later call it.
*/
void KCMKttsMgr::removeLanguage(){
    // kdDebug() << "Running: KCMKttsMgr::removeLanguage()"<< endl;
    
    // Get the selected language
    QListViewItem *itemToRemove = m_kttsmgrw->languagesList->selectedItem();
    
    // Call the real removeLanguage function
    if(itemToRemove){
        removeLanguage(m_reverseLanguagesMap[itemToRemove->text(0)]);
    }
}

/**
* Remove a language named lang
*/
void KCMKttsMgr::removeLanguage(const QString &language){
    // kdDebug() << "Running: KCMKttsMgr::removeLanguage(const QString &language)"<< endl;
    if(m_loadedLanguages[language]){
        // Remove the KListViewItem object
        m_kttsmgrw->languagesList->takeItem(m_loadedLanguages[language]->listItem);
    
        // Remove the configuration widget
        delete m_loadedLanguages[language]->plugIn;
    
        // And finish removing the wholestructure from memory
        m_loadedLanguages.remove(language);
    }
}

/**
* Set default langauge
* This is a wrapper function that takes the parameters for the real setDefaultLanguage from the
* widgets to later call it.
*/
void KCMKttsMgr::setDefaultLanguage(){
    // kdDebug() << "Running: KCMKttsMgr::setDefaultLanguage()"<< endl;
    // Get the selected language
    QListViewItem *defaultLanguageItem = m_kttsmgrw->languagesList->selectedItem();
    
    // Call the real setDefaultLanguage function
    if(defaultLanguageItem){
        setDefaultLanguage(m_reverseLanguagesMap[defaultLanguageItem->text(0)]);
    }
}

/**
* Set the default language
*/
void KCMKttsMgr::setDefaultLanguage(const QString &defaultLanguage){
    // kdDebug() << "Running: KCMKttsMgr::setDefaultLanguage(const QString &defaultLanguage)"<< endl;
    // Yes should be a beautiful tick icon
    for(QDictIterator<languageRelatedObjects> it(m_loadedLanguages); it.current() ; ++it){
        if(it.currentKey() == defaultLanguage){
            kdDebug() << "Setting " << it.currentKey() << " as the default language" << endl;
            it.current()->listItem->setPixmap(2, DesktopIcon("ok", 16));
            m_kttsmgrw->languagesList->setSelected(it.current()->listItem, true);
//            it.current()->listItem->setSelected(true);
        } else {
            kdDebug() << "UN-Setting " << it.currentKey() << " as the default language" << endl;
            it.current()->listItem->setPixmap(2, NULL);
        }
    }
    updateDefaultButton();
}

/**
* Update the status of the Remove button
*/
void KCMKttsMgr::updateRemoveButton(){
    // kdDebug() << "Running: KCMKttsMgr::updateRemoveButton()"<< endl;
    if(m_kttsmgrw->languagesList->selectedItem()){
        m_kttsmgrw->removeLanguageButton->setEnabled(true);
        QString language = m_kttsmgrw->languagesList->selectedItem()->text(0);
//        m_kttsmgrw->mainTab->setTabEnabled(m_kttsmgrw->PluginWidgetStack, true);
//        m_kttsmgrw->PluginWidgetStack->setEnabled(true);
//        m_kttsmgrw->PluginWidgetStack->raiseWidget(m_loadedLanguages[m_reverseLanguagesMap[language]]->plugIn);
        int currentTab = m_kttsmgrw->mainTab->currentPageIndex();
        if (m_pluginWidget)
            m_kttsmgrw->mainTab->removePage(m_pluginWidget);
        m_pluginWidget = m_loadedLanguages[m_reverseLanguagesMap[language]]->plugIn;
        m_kttsmgrw->mainTab->insertTab(m_pluginWidget, "Properties", 1);
        if (currentTab) m_kttsmgrw->mainTab->setCurrentPage(currentTab);
    } else {
        m_kttsmgrw->removeLanguageButton->setEnabled(false);
//        m_kttsmgrw->PluginWidgetStack->setEnabled(false);
//        m_kttsmgrw->mainTab->setTabEnabled(m_kttsmgrw->PluginWidgetStack, false);
        if (m_pluginWidget) m_kttsmgrw->mainTab->setTabEnabled(m_pluginWidget, false);
    }
}

/**
* Update the status of the Default button
*/
void KCMKttsMgr::updateDefaultButton(){
    // kdDebug() << "Running: KCMKttsMgr::updateDefaultButton()"<< endl;
    if(m_kttsmgrw->languagesList->selectedItem()){
        if(!m_kttsmgrw->languagesList->selectedItem()->pixmap(2)){
            m_kttsmgrw->makeDefaultLanguage->setEnabled(true);
        } else {
            m_kttsmgrw->makeDefaultLanguage->setEnabled(false);
        }
    } else {
        m_kttsmgrw->makeDefaultLanguage->setEnabled(false);
    }
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
    // kdDebug() << "Running enableKttsdToggled" << endl;
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
    // kdDebug() << "KCMKttsMgr::kttsdStarted running" << endl;
    bool kttsdLoaded = (m_jobMgrPart != 0);
    // Load Job Manager Part library.
    if (!kttsdLoaded)
    {
        KLibFactory *factory = KLibLoader::self()->factory( "libkttsjobmgr" );
        if (factory)
        {
            // Create the Job Manager part
            m_jobMgrPart = (KParts::ReadOnlyPart *)factory->create( m_kttsmgrw->mainTab, "kttsjobmgr",
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
        else kdDebug() << "Could not load libkttsjobmgr.  Is libkttsjobmgr installed?" << endl;
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
    // kdDebug() << "KCMKttsMgr::kttsdExiting running" << endl;
    if (m_jobMgrPart)
    {
        m_kttsmgrw->mainTab->removePage(m_jobMgrPart->widget());
        delete m_jobMgrPart;
        m_jobMgrPart = 0;
    }
    m_kttsmgrw->enableKttsdCheckBox->setChecked(false);
}

// System tray context menu entries
void KCMKttsMgr::aboutSelected(){
    m_aboutDlg->show();
}
