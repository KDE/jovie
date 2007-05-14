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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <QFile>
#include <QFileInfo>

#include <QDomDocument>
#include <QFile>
#include <QRadioButton>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kregexpeditorinterface.h>
#include <kparts/componentfactory.h>
#include <kfiledialog.h>
#include <kvbox.h>
#include <kservicetypetrader.h>

// KTTS includes.
#include "filterconf.h"
#include "selectlanguagedlg.h"

// SBD includes.
#include "sbdconf.h"
#include "sbdconf.moc"

/**
* Constructor
*/
SbdConf::SbdConf( QWidget *parent, const QStringList &args) :
    KttsFilterConf(parent)
{
    Q_UNUSED(args);
    // kDebug() << "SbdConf::SbdConf: Running" << endl;

    // Create configuration widget.
    setupUi(this);

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KServiceTypeTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();

    reButton->setEnabled( m_reEditorInstalled );
    if ( m_reEditorInstalled )
        connect( reButton, SIGNAL(clicked()), this, SLOT(slotReButton_clicked()) );

    connect( reLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( sbLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( nameLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( appIdLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect(languageBrowseButton, SIGNAL(clicked()),
         this, SLOT(slotLanguageBrowseButton_clicked()));
    connect(loadButton, SIGNAL(clicked()),
         this, SLOT(slotLoadButton_clicked()));
    connect(saveButton, SIGNAL(clicked()),
         this, SLOT(slotSaveButton_clicked()));
    connect(clearButton, SIGNAL(clicked()),
         this, SLOT(slotClearButton_clicked()));

    // Set up defaults.
    defaults();
}

/**
* Destructor.
*/
SbdConf::~SbdConf(){
    // kDebug() << "SbdConf::~SbdConf: Running" << endl;
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
void SbdConf::load(KConfig* c, const QString& configGroup){
    // kDebug() << "SbdConf::load: Running" << endl;
    KConfigGroup config( c, configGroup );
    nameLineEdit->setText(
        config.readEntry("UserFilterName", nameLineEdit->text()) );
    reLineEdit->setText(
        config.readEntry("SentenceDelimiterRegExp", reLineEdit->text()) );
    sbLineEdit->setText(
        config.readEntry("SentenceBoundary", sbLineEdit->text()) );
    QStringList langCodeList = config.readEntry("LanguageCodes", QStringList(), ',');
    if (!langCodeList.isEmpty())
        m_languageCodeList = langCodeList;
    QString language = "";
    for ( int ndx=0; ndx < m_languageCodeList.count(); ++ndx)
    {
        if (!language.isEmpty()) language += ',';
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    languageLineEdit->setText(language);
    appIdLineEdit->setText(
            config.readEntry("AppID", appIdLineEdit->text()) );
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
void SbdConf::save(KConfig* c, const QString& configGroup){
    // kDebug() << "SbdConf::save: Running" << endl;
    KConfigGroup config( c, configGroup );
    config.writeEntry("UserFilterName", nameLineEdit->text() );
    config.writeEntry("SentenceDelimiterRegExp", reLineEdit->text() );
    config.writeEntry("SentenceBoundary", sbLineEdit->text() );
    config.writeEntry("LanguageCodes", m_languageCodeList );
    config.writeEntry("AppID", appIdLineEdit->text().replace(" ", "") );
}

/**
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The
* default values should probably be the same as the ones the application
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void SbdConf::defaults(){
    // kDebug() << "SbdConf::defaults: Running" << endl;
    nameLineEdit->setText( i18n("Standard Sentence Boundary Detector") );
    reLineEdit->setText( "([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))" );
    sbLineEdit->setText( "\\1\\t" );
    m_languageCodeList.clear();
    languageLineEdit->setText( "" );
    appIdLineEdit->setText( "" );
    // kDebug() << "SbdConf::defaults: Exiting" << endl;
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
    if ( reLineEdit->text().isEmpty() )
        return QString();
    else
        return nameLineEdit->text();
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
            KServiceTypeTrader::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );
    if ( editorDialog )
    {
        // kdeutils was installed, so the dialog was found.  Fetch the editor interface.
        KRegExpEditorInterface *reEditor =
            dynamic_cast<KRegExpEditorInterface *>(editorDialog);
        Q_ASSERT( reEditor ); // This should not fail!// now use the editor.
        reEditor->setRegExp( reLineEdit->text() );
        int dlgResult = editorDialog->exec();
        if ( dlgResult == QDialog::Accepted )
        {
            QString re = reEditor->regExp();
            reLineEdit->setText( re );
            configChanged();
        }
        delete editorDialog;
    } else return;
}

void SbdConf::slotLanguageBrowseButton_clicked()
{
    SelectLanguageDlg* dlg = new SelectLanguageDlg(
        this,
        i18n("Select Languages"),
        QStringList(m_languageCodeList),
        SelectLanguageDlg::MultipleSelect,
        SelectLanguageDlg::BlankAllowed);
    int dlgResult = dlg->exec();
    if (dlgResult == QDialog::Accepted)
        m_languageCodeList = dlg->selectedLanguageCodes();
    delete dlg;
    if (dlgResult != QDialog::Accepted) return;
    QString language("");
    for ( int ndx=0; ndx < m_languageCodeList.count(); ++ndx)
    {
        if (!language.isEmpty()) language += ',';
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    languageLineEdit->setText(language);
    configChanged();
}

void SbdConf::slotLoadButton_clicked()
{
    QStringList dataDirs = KGlobal::dirs()->findAllResources("data", "kttsd/sbd/");
    QString dataDir;
    if (!dataDirs.isEmpty()) dataDir = dataDirs.last();
    QString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*rc|SBD Config (*rc)",
        this,
        "sbd_loadfile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename, KConfig::NoGlobals );
    load( cfg, "Filter" );
    delete cfg;
    configChanged();
}

void SbdConf::slotSaveButton_clicked()
{
    QString filename = KFileDialog::getSaveFileName(
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/sbd/", false ),
        "*rc|SBD Config (*rc)",
        this,
        "sbd_savefile");
    if ( filename.isEmpty() ) return;
    KConfig* cfg = new KConfig( filename );
    save( cfg, "Filter" );
    delete cfg;
}

void SbdConf::slotClearButton_clicked()
{
    nameLineEdit->setText( QString() );
    reLineEdit->setText( QString() );
    sbLineEdit->setText( QString() );
    m_languageCodeList.clear();
    languageLineEdit->setText( QString() );
    appIdLineEdit->setText( QString() );
    configChanged();
}
