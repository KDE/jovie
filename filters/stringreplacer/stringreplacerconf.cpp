/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Configuration class.
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

// KTTS includes.
#include "filterconf.h"

// StringReplacer includes.
#include "stringreplacerconf.h"
#include "stringreplacerconf.moc"
#include "editreplacementwidget.h"

/**
* Constructor 
*/
StringReplacerConf::StringReplacerConf( QWidget *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterConf(parent, name),
    m_editDlg(0),
    m_editWidget(0)
{
    // kdDebug() << "StringReplacerConf::StringReplacerConf: Running" << endl;

    // Create configuration widget.
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "StringReplacerConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new StringReplacerConfWidget(this, "StringReplacerConfigWidget");
    layout->addWidget(m_widget);

    connect(m_widget->nameLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->languageBrowseButton, SIGNAL(clicked()),
        this, SLOT(slotLanguageBrowseButton_clicked()));
    connect(m_widget->addButton, SIGNAL(clicked()),
        this, SLOT(slotAddButton_clicked()));
    connect(m_widget->editButton, SIGNAL(clicked()),
        this, SLOT(slotEditButton_clicked()));
    connect(m_widget->removeButton, SIGNAL(clicked()),
        this, SLOT(slotRemoveButton_clicked()));
    connect(m_widget->substLView, SIGNAL(selectionChanged()),
        this, SLOT(enableDisableButtons()));
    connect(m_widget->appIdLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();

    // Set up defaults.
    defaults();
}

/**
* Destructor.
*/
StringReplacerConf::~StringReplacerConf(){
    // kdDebug() << "StringReplacerConf::~StringReplacerConf: Running" << endl;
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
void StringReplacerConf::load(KConfig* /*config*/, const QString& configGroup){
    // kdDebug() << "StringReplacerConf::load: Running" << endl;
    QString wordsFilename =
       KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", false );
    if ( wordsFilename.isEmpty() ) return;
    wordsFilename += configGroup;

    // Open existing word list.
    QFile file( wordsFilename );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdDebug() << "StringReplacerConf::load: unable to open file " << wordsFilename << endl;
        return;
    }
    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        kdDebug() << "StringReplacerConf::load: unable to parse file " << wordsFilename << endl;
        file.close();
        return;
    }
    // kdDebug() << "StringReplacerConf::load: document successfully parsed." << endl;
    file.close();

    // Clear list view.
    m_widget->substLView->clear();

    // Name setting.
    QDomNodeList nameList = doc.elementsByTagName( "name" );
    QDomNode nameNode = nameList.item( 0 );
    m_widget->nameLineEdit->setText( nameNode.toElement().text() );
    // kdDebug() << "StringReplacerConf::load: name = " << nameNode.toElement().text() << endl;

    // Language Code setting.
    QDomNodeList languageList = doc.elementsByTagName( "language-code" );
    QDomNode languageNode = languageList.item( 0 );
    m_languageCode = languageNode.toElement().text();
    QString language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCode);
    m_widget->languageLineEdit->setText( language );

    // AppId.  Apply this filter only if DCOP appId of application that queued
    // the text contains this string.
    QDomNodeList appIdList = doc.elementsByTagName( "appid" );
    if (appIdList.count() > 0)
    {
        QDomNode appIdNode = appIdList.item( 0 );
        m_widget->appIdLineEdit->setText( appIdNode.toElement().text().latin1() );
    }

    // Word list.
    QListViewItem* item = 0;
    QDomNodeList wordList = doc.elementsByTagName("word");
    int wordListCount = wordList.count();
    for (int wordIndex = 0; wordIndex < wordListCount; wordIndex++)
    {
        // kdDebug() << "StringReplacerConf::load: start parsing of word " << wordIndex << endl;
        QDomNode wordNode = wordList.item(wordIndex);
        QDomNodeList propList = wordNode.childNodes();
        QString wordType;
        QString match;
        QString subst;
        int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; propIndex++)
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "type") wordType = prop.text();
            if (prop.tagName() == "match") match = prop.text();
            if (prop.tagName() == "subst") subst = prop.text();
        }
        if (!item)
            item = new KListViewItem(m_widget->substLView, wordType, match, subst);
        else
            item = new KListViewItem(m_widget->substLView, item, wordType, match, subst);
    }
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
void StringReplacerConf::save(KConfig* /*config*/, const QString& configGroup){
    // kdDebug() << "StringReplacerConf::save: Running" << endl;
    QString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", true );
    if ( wordsFilename.isEmpty() )
    {
        kdDebug() << "StringReplacerConf::save: no save location" << endl;
        return;
    }
    wordsFilename += configGroup;

    QFile file( wordsFilename );
    if ( !file.open( IO_WriteOnly ) )
    {
        kdDebug() << "StringReplacerConf::save: unable to open file " << wordsFilename << endl;
        return;
    }

    // kdDebug() << "StringReplacerConf::save: saving to file " << wordsFilename << endl;

    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );

    QDomElement root = doc.createElement( "wordlist" );
    doc.appendChild( root );

    // Name.
    QDomElement name = doc.createElement( "name" );
    root.appendChild( name );
    QDomText t = doc.createTextNode( m_widget->nameLineEdit->text() );
    name.appendChild( t );

    // Language code.
    QDomElement languageCode = doc.createElement( "language-code" );
    root.appendChild( languageCode );
    t = doc.createTextNode( m_languageCode );
    languageCode.appendChild( t );

    // Application ID
    QString appId = m_widget->appIdLineEdit->text();
    if ( !appId.isEmpty() )
    {
        QDomElement appIdElem = doc.createElement( "appid" );
        root.appendChild( appIdElem );
        t = doc.createTextNode( appId );
        appIdElem.appendChild( t );
    }

    // Words.
    QListView* lView = m_widget->substLView;
    QListViewItem* item = lView->firstChild();
    while (item)
    {
        QDomElement wordTag = doc.createElement( "word" );
        root.appendChild( wordTag );
        QDomElement propTag = doc.createElement( "type" );
        wordTag.appendChild( propTag);
        QDomText t = doc.createTextNode( item->text(0) );
        propTag.appendChild( t );

        propTag = doc.createElement( "match" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( item->text(1) );
        propTag.appendChild( t );

        propTag = doc.createElement( "subst" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( item->text(2) );
        propTag.appendChild( t );

        item = item->nextSibling();
    }

    // Write it all out.
    QTextStream ts( &file );
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ts << doc.toString();
    // kdDebug() << "StringReplacerConf::save: writing out " << doc.toString() << endl;
    file.close();
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void StringReplacerConf::defaults(){
    // kdDebug() << "StringReplacerConf::defaults: Running" << endl;
    // Default language is English.
    m_languageCode = "en";
    QString language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCode);
    m_widget->languageLineEdit->setText(language);
    // Default name.
    m_widget->nameLineEdit->setText(i18n("String Replacer") + " (" + language + ")");
    // Add some default words.
    QListView* lView = m_widget->substLView;
    lView->clear();
    QListViewItem* item = new KListViewItem(lView, substitutionTypeToString(stWord), "KDE", "K D E");
    item = new KListViewItem(lView, item, substitutionTypeToString(stWord), "TTS", "T T S");
    item = new KListViewItem(lView, item, substitutionTypeToString(stWord), "KTTS", "K T T S");
    item = new KListViewItem(lView, item, substitutionTypeToString(stWord), "KTTSD", "K T T S D");
    item = new KListViewItem(lView, item, substitutionTypeToString(stWord), "kttsd", "K T T S D");
    // Default App ID is blank.
    m_widget->appIdLineEdit->setText( " " );
    enableDisableButtons();
    // kdDebug() << "StringReplacerConf::defaults: Exiting" << endl;
}

/**
 * Indicates whether the plugin supports multiple instances.  Return
 * False if only one instance of the plugin can be configured.
 * @return            True if multiple instances are possible.
 */
bool StringReplacerConf::supportsMultiInstance() { return true; }

/**
 * Returns the name of the plugin.  Displayed in Filters tab of KTTSMgr.
 * If there can be more than one instance of a filter, it should return
 * a unique name for each instance.  The name should be translated for
 * the user if possible.  If the plugin is not correctly configured,
 * return an empty string.
 * @return          Filter instance name.
 */
QString StringReplacerConf::userPlugInName()
{
    if ( m_widget->substLView->childCount() == 0 ) return QString::null;
    QString instName = m_widget->nameLineEdit->text();
    if ( instName.isEmpty() )
    {
        QString language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCode);
        instName = i18n("String Replacer") + " (" + language + ")";
    }
    return instName;
}

// Converts a Substitution Type to displayable string.
QString StringReplacerConf::substitutionTypeToString(const int substitutionType)
{
    switch (substitutionType)
    {
        case stWord:        return i18n("Word");
        case stRegExp:      return "RegExp";
    }
    return i18n("Error");
}

void StringReplacerConf::slotLanguageBrowseButton_clicked()
{
    // Create a  QHBox to host KListView.
    QHBox* hBox = new QHBox(m_widget, "SelectLanguage_hbox");
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    QString locale;
    QString languageCode;
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
        m_widget,
        "SelectLanguage_dlg",
        true,
        true);
    dlg->setMainWidget(hBox);
    dlg->setHelp("", "kttsd");
    dlg->setInitialSize(QSize(200, 500), false);
    int dlgResult = dlg->exec();
    languageCode = QString::null;
    if (langLView->currentItem()) languageCode = langLView->currentItem()->text(1);
    delete dlg;
    // TODO: Also delete KListView and QHBox?
    if (dlgResult != QDialog::Accepted) return;
    m_languageCode = languageCode;
    language = KGlobal::locale()->twoAlphaToLanguageName(languageCode);
    QString s1 = m_widget->languageLineEdit->text();
    m_widget->languageLineEdit->setText(language);
    // Replace language in the user's filter name.
    QString s2 = m_widget->nameLineEdit->text();
    m_widget->nameLineEdit->setText(s2.replace(s1, language));
    configChanged();
}

void StringReplacerConf::enableDisableButtons()
{
    bool enableBtn = (m_widget->substLView->selectedItem() != 0);
    m_widget->editButton->setEnabled(enableBtn);
    m_widget->removeButton->setEnabled(enableBtn);
}

void StringReplacerConf::slotAddButton_clicked()
{
    addOrEditSubstitution( true );
}

void StringReplacerConf::slotEditButton_clicked()
{
    addOrEditSubstitution( false );
}

// Displays the add/edit string replacement dialog.
void StringReplacerConf::addOrEditSubstitution(bool isAdd)
{
    QListViewItem* item = 0;
    if (isAdd)
        item = m_widget->substLView->lastChild();
    else
    {
        item = m_widget->substLView->selectedItem();
        if (!item) return;
    }
    // Create a QHBox to host widget.
    QHBox* hBox = new QHBox(m_widget, "AddOrEditSubstitution_hbox" );
    // Create widget.
    m_editWidget = new EditReplacementWidget( hBox, "AddOrEditSubstitution_widget" );
    // Set controls if editing existing.
    m_editWidget->matchButton->setEnabled( false );
    if (!isAdd)
    {
        if ( item->text(0) == "RegExp" )
        {
            m_editWidget->regexpRadioButton->setChecked( true );
            m_editWidget->matchButton->setEnabled( m_reEditorInstalled );
        }
        m_editWidget->matchLineEdit->setText( item->text(1) );
        m_editWidget->substLineEdit->setText( item->text(2) );
    }
    // The match box may not be blank.
    connect( m_editWidget->matchLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(slotMatchLineEdit_textChanged(const QString&)) );
    connect( m_editWidget->regexpRadioButton, SIGNAL(clicked()),
         this, SLOT(slotTypeButtonGroup_clicked()) );
    connect( m_editWidget->wordRadioButton, SIGNAL(clicked()),
         this, SLOT(slotTypeButtonGroup_clicked()) );
    connect( m_editWidget->matchButton, SIGNAL(clicked()),
         this, SLOT(slotMatchButton_clicked()) );
    // Display the box in a dialog.
    m_editDlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Edit String Replacement"),
        KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_widget,
        "AddOrEditSubstitution_dlg",
        true,
        true);
    // Disable OK button if match field blank.
    m_editDlg->setMainWidget( hBox );
    m_editDlg->setHelp( "", "kttsd" );
    m_editDlg->enableButton( KDialogBase::Ok, !m_editWidget->matchLineEdit->text().isEmpty() );
    int dlgResult = m_editDlg->exec();
    QString substType = i18n( "Word" );
    if ( m_editWidget->regexpRadioButton->isChecked() ) substType = "RegExp";
    QString match = m_editWidget->matchLineEdit->text();
    QString subst = m_editWidget->substLineEdit->text();
    delete m_editDlg;
    m_editDlg = 0;
    m_editWidget = 0;
    if (dlgResult != QDialog::Accepted) return;
    // TODO: Also delete hBox and w?
    if ( match.isEmpty() ) return;
    if ( isAdd )
    {
        if ( item )
            item = new KListViewItem( m_widget->substLView, item, substType, match, subst );
        else
            item = new KListViewItem( m_widget->substLView, substType, match, subst );
    }
    else
    {
        item->setText( 0, substType );
        item->setText( 1, match );
        item->setText( 2, subst );
    }
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotMatchLineEdit_textChanged(const QString& text)
{
    // Disable OK button if match field blank.
    if ( !m_editDlg ) return;
    m_editDlg->enableButton( KDialogBase::Ok, !text.isEmpty() );
}

void StringReplacerConf::slotRemoveButton_clicked()
{
    QListViewItem* item = m_widget->substLView->selectedItem();
    if (!item) return;
    delete item;
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotTypeButtonGroup_clicked()
{
    // Enable Regular Expression Editor button if editor is installed (requires kdeutils).
    if ( !m_editWidget ) return;
    m_editWidget->matchButton->setEnabled( m_editWidget->regexpRadioButton->isOn() &&  m_reEditorInstalled );
}

void StringReplacerConf::slotMatchButton_clicked()
{
    // Show Regular Expression Editor dialog if it is installed.
    if ( !m_editWidget ) return;
    if ( !m_editDlg ) return;
    if ( !m_reEditorInstalled ) return;
    QDialog *editorDialog = 
        KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor" );
    if ( editorDialog )
    {
        // kdeutils was installed, so the dialog was found.  Fetch the editor interface.
        KRegExpEditorInterface *reEditor =
            static_cast<KRegExpEditorInterface *>(editorDialog->qt_cast( "KRegExpEditorInterface" ) );
        Q_ASSERT( reEditor ); // This should not fail!// now use the editor.
        reEditor->setRegExp( m_editWidget->matchLineEdit->text() );
        int dlgResult = editorDialog->exec();
        if ( dlgResult == QDialog::Accepted )
        {
            QString re = reEditor->regExp();
            m_editWidget->matchLineEdit->setText( re );
            m_editDlg->enableButton( KDialogBase::Ok, !re.isEmpty() );
        }
        delete editorDialog;
    } else return;
}
