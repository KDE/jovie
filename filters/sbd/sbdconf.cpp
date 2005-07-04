/***************************************************** vim:set ts=4 sw=4 sts=4:
  Sentence Boundary Detection Filter Configuration class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qdom.h>
#include <qfile.h>
#include <qradiobutton.h>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kregexpeditorinterface.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kfiledialog.h>

// KTTS includes.
#include "filterconf.h"

// SBD includes.
#include "sbdconf.h"
#include "sbdconf.moc"

/**
* Constructor 
*/
SbdConf::SbdConf( QWidget *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterConf(parent, name)
{
    // kdDebug() << "SbdConf::SbdConf: Running" << endl;

    // Create configuration widget.
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "SbdConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new SbdConfWidget(this, "SbdConfigWidget");
    layout->addWidget(m_widget);

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();

    m_widget->reButton->setEnabled( m_reEditorInstalled );
    if ( m_reEditorInstalled )
        connect( m_widget->reButton, SIGNAL(clicked()), this, SLOT(slotReButton_clicked()) );

    connect( m_widget->reLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->sbLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->nameLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->appIdLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect(m_widget->languageBrowseButton, SIGNAL(clicked()),
         this, SLOT(slotLanguageBrowseButton_clicked()));
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
SbdConf::~SbdConf(){
    // kdDebug() << "SbdConf::~SbdConf: Running" << endl;
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
void SbdConf::load(KConfig* config, const QString& configGroup){
    // kdDebug() << "SbdConf::load: Running" << endl;
    config->setGroup( configGroup );
    m_widget->nameLineEdit->setText( 
        config->readEntry("UserFilterName", m_widget->nameLineEdit->text()) );
    m_widget->reLineEdit->setText(
        config->readEntry("SentenceDelimiterRegExp", m_widget->reLineEdit->text()) );
    m_widget->sbLineEdit->setText(
        config->readEntry("SentenceBoundary", m_widget->sbLineEdit->text()) );
    QStringList langCodeList = config->readListEntry("LanguageCodes");
    if (!langCodeList.isEmpty())
        m_languageCodeList = langCodeList;
    QString language = "";
    for ( uint ndx=0; ndx < m_languageCodeList.count(); ++ndx)
    {
        if (!language.isEmpty()) language += ",";
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    m_widget->languageLineEdit->setText(language);
    m_widget->appIdLineEdit->setText(
            config->readEntry("AppID", m_widget->appIdLineEdit->text()) );
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
void SbdConf::save(KConfig* config, const QString& configGroup){
    // kdDebug() << "SbdConf::save: Running" << endl;
    config->setGroup( configGroup );
    config->writeEntry("UserFilterName", m_widget->nameLineEdit->text() );
    config->writeEntry("SentenceDelimiterRegExp", m_widget->reLineEdit->text() );
    config->writeEntry("SentenceBoundary", m_widget->sbLineEdit->text() );
    config->writeEntry("LanguageCodes", m_languageCodeList );
    config->writeEntry("AppID", m_widget->appIdLineEdit->text().replace(" ", "") );
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void SbdConf::defaults(){
    // kdDebug() << "SbdConf::defaults: Running" << endl;
    m_widget->nameLineEdit->setText( i18n("Standard Sentence Boundary Detector") );
    m_widget->reLineEdit->setText( "([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))" );
    m_widget->sbLineEdit->setText( "\\1\\t" );
    m_languageCodeList.clear();
    m_widget->languageLineEdit->setText( "" );
    m_widget->appIdLineEdit->setText( "" );
    // kdDebug() << "SbdConf::defaults: Exiting" << endl;
}

/**
 * Indicates whether the plugin supports multiple instances.  Return
 * False if only one instance of the plugin can be configured.
 * @return            True if multiple instances are possible.
 */
bool SbdConf::supportsMultiInstance() { return true; }

/**
 * Returns the name of the plugin.  Displayed in Filters tab of KTTSMgr.
 * If there can be more than one instance of a filter, it should return
 * a unique name for each instance.  The name should be translated for
 * the user if possible.  If the plugin is not correctly configured,
 * return an empty string.
 * @return          Filter instance name.
 */
QString SbdConf::userPlugInName()
{
    if ( m_widget->reLineEdit->text().isEmpty() )
        return QString::null;
    else
        return m_widget->nameLineEdit->text();
}

/**
 * Returns True if this filter is a Sentence Boundary Detector.
 * @return          True if this filter is a SBD.
 */
bool SbdConf::isSBD() { return true; }

void SbdConf::slotReButton_clicked()
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
            configChanged();
        }
        delete editorDialog;
    } else return;
}

void SbdConf::slotLanguageBrowseButton_clicked()
{
    // Create a  QHBox to host KListView.
    QHBox* hBox = new QHBox(m_widget, "SelectLanguage_hbox");
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    langLView->setSelectionMode(QListView::Extended);
    QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    QString locale;
    QString languageCode;
    QString countryCode;
    QString charSet;
    QString language;
    // Blank line so user can select no language.
    QListViewItem* item = new KListViewItem(langLView, "", "");
    if (m_languageCodeList.isEmpty()) item->setSelected(true);
    const int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ++ndx)
    {
        locale = allLocales[ndx];
        KGlobal::locale()->splitLocale(locale, languageCode, countryCode, charSet);
        language = KGlobal::locale()->twoAlphaToLanguageName(languageCode);
        if (!countryCode.isEmpty()) language +=
            " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode)+")";
        QListViewItem* item = new KListViewItem(langLView, language, locale);
        if (m_languageCodeList.contains(locale)) item->setSelected(true);
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
    int dlgResult = dlg->exec();
    languageCode = QString::null;
    if (dlgResult == QDialog::Accepted)
    {
        m_languageCodeList.clear();
        QListViewItem* item = langLView->firstChild();
        while (item)
        {
            if (item->isSelected()) m_languageCodeList += item->text(1);
            item = item->nextSibling();
        }
    }
    delete dlg;
    // TODO: Also delete KListView and QHBox?
    if (dlgResult != QDialog::Accepted) return;
    language = "";
    for ( uint ndx=0; ndx < m_languageCodeList.count(); ++ndx)
    {
        if (!language.isEmpty()) language += ",";
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    m_widget->languageLineEdit->setText(language);
    configChanged();
}

void SbdConf::slotLoadButton_clicked()
{
    // QString dataDir = KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/stringreplacer/";
    QString dataDir = KGlobal::dirs()->findAllResources("data", "kttsd/sbd/").last();
    QString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*rc|SBD Config (*rc)",
        m_widget,
        "sbd_loadfile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename, true, false, 0 );
    load( cfg, "Filter" );
    delete cfg;
    configChanged();
}

void SbdConf::slotSaveButton_clicked()
{
    QString filename = KFileDialog::getSaveFileName(
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/sbd/", false ),
        "*rc|SBD Config (*rc)",
        m_widget,
        "sbd_savefile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename, false, false, 0 );
    save( cfg, "Filter" );
    delete cfg;
}

void SbdConf::slotClearButton_clicked()
{
    m_widget->nameLineEdit->setText( QString::null );
    m_widget->reLineEdit->setText( QString::null );
    m_widget->sbLineEdit->setText( QString::null );
    m_languageCodeList.clear();
    m_widget->languageLineEdit->setText( QString::null );
    m_widget->appIdLineEdit->setText( QString::null );
    configChanged();
}
