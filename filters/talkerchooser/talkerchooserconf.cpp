/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic Talker Chooser Filter Configuration class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; version 2 of the License.                 *
 *                                                                            *
 ******************************************************************************/

// Qt includes.
#include <qstring.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qcheckbox.h>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kregexpeditorinterface.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

// KTTS includes.
#include "filterconf.h"
#include "talkercode.h"

// StringReplacer includes.
#include "talkerchooserconf.h"
#include "talkerchooserconf.moc"

/**
* Constructor 
*/
TalkerChooserConf::TalkerChooserConf( QWidget *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterConf(parent, name)
{
    // kdDebug() << "TalkerChooserConf::TalkerChooserConf: Running" << endl;

    // Create configuration widget.
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "TalkerChooserConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new TalkerChooserConfWidget(this, "TalkerChooserConfigWidget");
    layout->addWidget(m_widget);

    // Fill combo boxes.
    KComboBox* cb = m_widget->genderComboBox;
    cb->insertItem( "" );
    cb->insertItem( TalkerCode::translatedGender("male") );
    cb->insertItem( TalkerCode::translatedGender("female") );
    cb->insertItem( TalkerCode::translatedGender("neutral") );

    cb = m_widget->volumeComboBox;
    cb->insertItem( "" );
    cb->insertItem( TalkerCode::translatedGender("medium") );
    cb->insertItem( TalkerCode::translatedGender("loud") );
    cb->insertItem( TalkerCode::translatedGender("soft") );

    cb = m_widget->rateComboBox;
    cb->insertItem( "" );
    cb->insertItem( TalkerCode::translatedGender("medium") );
    cb->insertItem( TalkerCode::translatedGender("fast") );
    cb->insertItem( TalkerCode::translatedGender("slow") );

    cb = m_widget->synthComboBox;
    cb->insertItem( "" );
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");
    for(unsigned int i=0; i < offers.count() ; ++i)
        cb->insertItem(offers[i]->name());

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();
    m_widget->reEditorButton->setEnabled(m_reEditorInstalled);

    connect(m_widget->nameLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->reLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->reEditorButton, SIGNAL(clicked()),
            this, SLOT(slotReEditorButton_clicked()));
    connect(m_widget->appIdLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));

    connect(m_widget->languageBrowseButton, SIGNAL(clicked()),
            this, SLOT(slotLanguageBrowseButton_clicked()));

    connect(m_widget->synthComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(slotSynthCheckBox_activated(const QString&)));
    connect(m_widget->genderComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(slotGenderCheckBox_activated(const QString&)));
    connect(m_widget->volumeComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(slotVolumeCheckBox_activated(const QString&)));
    connect(m_widget->rateComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(slotRateCheckBox_activated(const QString&)));

    connect(m_widget->synthCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->genderCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->volumeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->rateCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));

    connect(m_widget->loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoadButton_clicked()));
    connect(m_widget->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveButton_clicked()));
    connect(m_widget->clearButton, SIGNAL(clicked()),
            this, SLOT(slotClearButton_clicked()));

    // Set up defaults.
    defaults();
}

/**
* Destructor.
*/
TalkerChooserConf::~TalkerChooserConf(){
    // kdDebug() << "TalkerChooserConf::~TalkerChooserConf: Running" << endl;
}

QString TalkerChooserConf::readTalkerSetting(KConfig* config, const QString& key, bool* preferred)
{
    QString val = config->readEntry( key );
    *preferred = (val.startsWith("*"));
    if (*preferred) val = val.mid(1);
    return val;
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
void TalkerChooserConf::load(KConfig* config, const QString& configGroup){
    // kdDebug() << "TalkerChooserConf::load: Running" << endl;
    config->setGroup( configGroup );
    m_widget->nameLineEdit->setText( config->readEntry( "UserFilterName", m_widget->nameLineEdit->text() ) );
    m_widget->reLineEdit->setText(
            config->readEntry("MatchRegExp", m_widget->reLineEdit->text()) );
    m_widget->appIdLineEdit->setText(
            config->readEntry("AppIDs", m_widget->appIdLineEdit->text()) );

    bool preferred = false;
    QString val;

    m_languageCode = readTalkerSetting( config, "LanguageCode", &preferred );
    QString language = "";
    if (!m_languageCode.isEmpty())
    {
        language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCode);
        m_widget->languageLineEdit->setText( language );
    }
    m_widget->languageCheckBox->setChecked( preferred );

    val = readTalkerSetting( config, "SynthName", &preferred );
    m_widget->synthComboBox->setCurrentItem( val );
    m_widget->synthCheckBox->setChecked( preferred );
    m_widget->synthCheckBox->setEnabled( !val.isEmpty() );

    val = TalkerCode::translatedGender( readTalkerSetting( config, "Gender", &preferred ) );
    m_widget->genderComboBox->setCurrentItem( val );
    m_widget->genderCheckBox->setChecked( preferred );
    m_widget->genderCheckBox->setEnabled( !val.isEmpty() );

    val = TalkerCode::translatedVolume( readTalkerSetting( config, "Volume", &preferred ) );
    m_widget->volumeComboBox->setCurrentItem( val );
    m_widget->volumeCheckBox->setChecked( preferred );
    m_widget->volumeCheckBox->setEnabled( !val.isEmpty() );

    val = TalkerCode::translatedRate( readTalkerSetting( config, "Rate", &preferred ) );
    m_widget->rateComboBox->setCurrentItem( val );
    m_widget->rateCheckBox->setChecked( preferred );
    m_widget->rateCheckBox->setEnabled( !val.isEmpty() );
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
void TalkerChooserConf::save(KConfig* config, const QString& configGroup){
    // kdDebug() << "TalkerChooserConf::save: Running" << endl;
    config->setGroup( configGroup );
    config->writeEntry( "UserFilterName", m_widget->nameLineEdit->text() );
    config->writeEntry( "MatchRegExp", m_widget->reLineEdit->text() );
    config->writeEntry( "AppIDs", m_widget->appIdLineEdit->text().replace(" ", "") );
    QString val;

    val = m_languageCode;
    if ( m_widget->languageCheckBox->isChecked() ) val.prepend( "*" );
    config->writeEntry( "LanguageCode", m_languageCode );

    val = m_widget->synthComboBox->currentText();
    if ( m_widget->synthCheckBox->isChecked() ) val.prepend( "*" );
    config->writeEntry( "SynthName", val );

    val = TalkerCode::untranslatedGender( m_widget->genderComboBox->currentText() );
    if ( m_widget->genderCheckBox->isChecked() ) val.prepend( "*" );
    config->writeEntry( "Gender", val );

    val = TalkerCode::untranslatedVolume( m_widget->volumeComboBox->currentText() );
    if ( m_widget->volumeCheckBox->isChecked() ) val.prepend( "*" );
    config->writeEntry( "Volume", val );

    val = TalkerCode::untranslatedRate( m_widget->rateComboBox->currentText() );
    if ( m_widget->rateCheckBox->isChecked() ) val.prepend( "*" );
    config->writeEntry( "Rate", val );
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void TalkerChooserConf::defaults(){
    // kdDebug() << "TalkerChooserConf::defaults: Running" << endl;
    // Default name.
    m_widget->nameLineEdit->setText( i18n("Talker Chooser") );
    // Default regular expression is blank.
    m_widget->reLineEdit->setText( "" );
    // Default App ID is blank.
    m_widget->appIdLineEdit->setText( "" );
    // kdDebug() << "TalkerChooserConf::defaults: Exiting" << endl;
    m_languageCode = QString::null;
    m_widget->languageLineEdit->setText( "" );
    m_widget->languageCheckBox->setChecked( false );
    m_widget->languageCheckBox->setEnabled( false );
    m_widget->synthComboBox->setCurrentItem( 0 );
    m_widget->synthCheckBox->setChecked( false );
    m_widget->synthCheckBox->setEnabled( false );
    m_widget->genderComboBox->setCurrentItem( 0 );
    m_widget->genderCheckBox->setChecked( false );
    m_widget->genderCheckBox->setEnabled( false );
    m_widget->volumeComboBox->setCurrentItem( 0 );
    m_widget->volumeCheckBox->setChecked( false );
    m_widget->volumeCheckBox->setEnabled( false );
    m_widget->rateComboBox->setCurrentItem( 0 );
    m_widget->rateCheckBox->setChecked( false );
    m_widget->rateCheckBox->setEnabled( false );
}

/**
 * Indicates whether the plugin supports multiple instances.  Return
 * False if only one instance of the plugin can be configured.
 * @return            True if multiple instances are possible.
 */
bool TalkerChooserConf::supportsMultiInstance() { return true; }

/**
 * Returns the name of the plugin.  Displayed in Filters tab of KTTSMgr.
 * If there can be more than one instance of a filter, it should return
 * a unique name for each instance.  The name should be TalkerCode::translated for
 * the user if possible.  If the plugin is not correctly configured,
 * return an empty string.
 * @return          Filter instance name.
 */
QString TalkerChooserConf::userPlugInName()
{
    QString instName = m_widget->nameLineEdit->text();
    return instName;
}

void TalkerChooserConf::slotLanguageBrowseButton_clicked()
{
    // Create a  QHBox to host KListView.
    QHBox* hBox = new QHBox(m_widget, "SelectLanguage_hbox");
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    langLView->setSelectionMode(QListView::Single);
    QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    QString locale;
    QString languageCode;
    QString countryCode;
    QString charSet;
    QString language;
    // Blank line so user can select no language.
    QListViewItem* item = new KListViewItem(langLView, "", "");
    if (m_languageCode.isEmpty()) item->setSelected(true);
    int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ndx++)
    {
        locale = allLocales[ndx];
        KGlobal::locale()->splitLocale(locale, languageCode, countryCode, charSet);
        language = KGlobal::locale()->twoAlphaToLanguageName(languageCode);
        if (!countryCode.isEmpty()) language +=
            " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode)+")";
        item = new KListViewItem(langLView, language, locale);
        if (m_languageCode == locale) item->setSelected(true);
    }
    // Sort by language.
    langLView->setSorting(0);
    langLView->sort();
    // Display the box in a dialog.
    KDialogBase* dlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Select Languages"),
        KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_widget,
        "SelectLanguage_dlg",
        true,
        true);
    dlg->setMainWidget(hBox);
    dlg->setHelp("", "kttsd");
    dlg->setInitialSize(QSize(300, 500), false);
    // TODO: This isn't working.  Furthermore, item appears selected but is not.
    langLView->ensureItemVisible(langLView->selectedItem());
    int dlgResult = dlg->exec();
    if (dlgResult == QDialog::Accepted)
    {
        if (langLView->selectedItem())
        {
            language = langLView->selectedItem()->text(0);
            m_languageCode = langLView->selectedItem()->text(1);
        }
    }
    delete dlg;
    // TODO: Also delete KListView and QHBox?
    language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCode);
    m_widget->languageLineEdit->setText(language);
    m_widget->languageCheckBox->setChecked( !language.isEmpty() );
    configChanged();
}

void TalkerChooserConf::slotReEditorButton_clicked()
{
    // Show Regular Expression Editor dialog if it is installed.
    if ( !m_reEditorInstalled ) return;
    QDialog *editorDialog = 
        KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );
    if ( editorDialog )
    {
        // kdeutils was installed, so the dialog was found.  Fetch the editor interface.
        KRegExpEditorInterface *reEditor =
            static_cast<KRegExpEditorInterface *>(editorDialog->qt_cast( "KRegExpEditorInterface" ) );
        Q_ASSERT( reEditor ); // This should not fail!// now use the editor.
        reEditor->setRegExp( m_widget->reLineEdit->text() );
        int dlgResult = editorDialog->exec();
        if ( dlgResult == QDialog::Accepted )
        {
            QString re = reEditor->regExp();
            m_widget->reLineEdit->setText( re );
        }
        delete editorDialog;
    } else return;
}

void TalkerChooserConf::slotSynthCheckBox_activated( const QString& text )
{
    m_widget->synthCheckBox->setEnabled( !text.isEmpty() );
    if ( text.isEmpty() ) m_widget->synthCheckBox->setChecked( false );
    configChanged();
}
void TalkerChooserConf::slotGenderCheckBox_activated( const QString& text )
{
    m_widget->genderCheckBox->setEnabled( !text.isEmpty() );
    if ( text.isEmpty() ) m_widget->genderCheckBox->setChecked( false );
    configChanged();
}
void TalkerChooserConf::slotVolumeCheckBox_activated( const QString& text )
{
    m_widget->volumeCheckBox->setEnabled( !text.isEmpty() );
    if ( text.isEmpty() ) m_widget->volumeCheckBox->setChecked( false );
    configChanged();
}
void TalkerChooserConf::slotRateCheckBox_activated( const QString& text )
{
    m_widget->rateCheckBox->setEnabled( !text.isEmpty() );
    if ( text.isEmpty() ) m_widget->rateCheckBox->setChecked( false );
    configChanged();
}

void TalkerChooserConf::slotLoadButton_clicked()
{
    QString dataDir = KGlobal::dirs()->findAllResources("data", "kttsd/talkerchooser/").last();
    QString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*rc|Talker Chooser Config (*rc)",
        m_widget,
        "talkerchooser_loadfile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename, true, false, 0 );
    load( cfg, "Filter" );
    delete cfg;
    configChanged();
}

void TalkerChooserConf::slotSaveButton_clicked()
{
    QString filename = KFileDialog::getSaveFileName(
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/talkerchooser/", false ),
       "*rc|Talker Chooser Config (*rc)",
        m_widget,
        "talkerchooser_savefile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename, false, false, 0 );
    save( cfg, "Filter" );
    delete cfg;
}

void TalkerChooserConf::slotClearButton_clicked()
{
    m_widget->nameLineEdit->setText( QString::null );
    m_widget->reLineEdit->setText( QString::null );
    m_widget->appIdLineEdit->setText( QString::null );
    m_languageCode = QString::null;
    m_widget->languageLineEdit->setText( QString::null );
    m_widget->languageCheckBox->setChecked( false );
    m_widget->languageCheckBox->setEnabled( false );
    m_widget->synthComboBox->setCurrentItem( 0 );
    m_widget->synthCheckBox->setChecked( false );
    m_widget->synthCheckBox->setEnabled( false );
    m_widget->genderComboBox->setCurrentItem( 0 );
    m_widget->genderCheckBox->setChecked( false );
    m_widget->genderCheckBox->setEnabled( false );
    m_widget->volumeComboBox->setCurrentItem( 0 );
    m_widget->volumeCheckBox->setChecked( false );
    m_widget->volumeCheckBox->setEnabled( false );
    m_widget->rateComboBox->setCurrentItem( 0 );
    m_widget->rateCheckBox->setChecked( false );
    m_widget->rateCheckBox->setEnabled( false );
    configChanged();
}
