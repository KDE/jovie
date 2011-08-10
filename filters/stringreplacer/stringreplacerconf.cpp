/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Configuration class.
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

// TQt includes.
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqstring.h>
#include <tqhbox.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqdom.h>
#include <tqfile.h>
#include <tqradiobutton.h>

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
#include <kmessagebox.h>

// KTTS includes.
#include "filterconf.h"

// StringReplacer includes.
#include "stringreplacerconf.h"
#include "stringreplacerconf.moc"
#include "editreplacementwidget.h"

/**
* Constructor 
*/
StringReplacerConf::StringReplacerConf( TQWidget *parent, const char *name, const TQStringList& /*args*/) :
    KttsFilterConf(parent, name),
    m_editDlg(0),
    m_editWidget(0)
{
    // kdDebug() << "StringReplacerConf::StringReplacerConf: Running" << endl;

    // Create configuration widget.
    TQVBoxLayout *tqlayout = new TQVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "StringReplacerConfigWidgetLayout");
    tqlayout->tqsetAlignment (TQt::AlignTop);
    m_widget = new StringReplacerConfWidget(this, "StringReplacerConfigWidget");
    tqlayout->addWidget(m_widget);
    m_widget->substLView->setSortColumn(-1);

    connect(m_widget->nameLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->languageBrowseButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotLanguageBrowseButton_clicked()));
    connect(m_widget->addButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotAddButton_clicked()));
    connect(m_widget->upButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotUpButton_clicked()));
    connect(m_widget->downButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotDownButton_clicked()));
    connect(m_widget->editButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotEditButton_clicked()));
    connect(m_widget->removeButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotRemoveButton_clicked()));
    connect(m_widget->loadButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotLoadButton_clicked()));
    connect(m_widget->saveButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotSaveButton_clicked()));
    connect(m_widget->clearButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotClearButton_clicked()));
    connect(m_widget->substLView, TQT_SIGNAL(selectionChanged()),
        this, TQT_SLOT(enableDisableButtons()));
    connect(m_widget->appIdLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));

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
void StringReplacerConf::load(KConfig* config, const TQString& configGroup){
    // kdDebug() << "StringReplacerConf::load: Running" << endl;
    // See if this filter previously save its word list.
    config->setGroup( configGroup );
    TQString wordsFilename = config->readEntry( "WordListFile" );
    if ( !wordsFilename.isEmpty() )
    {
        TQString errMsg = loadFromFile( wordsFilename, true );
        if ( !errMsg.isEmpty() )
            kdDebug() << "StringReplacerConf::load: " << errMsg << endl;
        enableDisableButtons();
    }
}

// Loads word list and settings from a file.  Clearing configuration if clear is True.
TQString StringReplacerConf::loadFromFile( const TQString& filename, bool clear)
{
    // Open existing word list.
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
    if ( clear ) m_widget->substLView->clear();

    // Name setting.
    TQDomNodeList nameList = doc.elementsByTagName( "name" );
    TQDomNode nameNode = nameList.item( 0 );
    m_widget->nameLineEdit->setText( nameNode.toElement().text() );
    // kdDebug() << "StringReplacerConf::load: name = " << nameNode.toElement().text() << endl;

    // Language Codes setting.  List may be single element of comma-separated values,
    // or multiple elements.
    TQString languageCodes;
    TQDomNodeList languageList = doc.elementsByTagName( "language-code" );
    for ( uint ndx=0; ndx < languageList.count(); ++ndx )
    {
        TQDomNode languageNode = languageList.item( ndx );
        if (!languageCodes.isEmpty()) languageCodes += ",";
        languageCodes += languageNode.toElement().text();
    }
    if ( clear )
        m_languageCodeList = TQStringList::split(',', languageCodes, false);
    else
        m_languageCodeList += TQStringList::split(',', languageCodes, false);
    TQString language;
    m_languageCodeList.sort();
    // Eliminate dups.
    for ( int ndx = m_languageCodeList.count() - 2; ndx >= 0; --ndx )
    {
        if ( m_languageCodeList[ndx] == m_languageCodeList[ndx+1] )
            m_languageCodeList.remove(m_languageCodeList.at(ndx+1));
    }
    for ( uint ndx=0; ndx < m_languageCodeList.count(); ++ndx )
    {
        if (!language.isEmpty()) language += ",";
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    m_widget->languageLineEdit->setText(language);

    // AppId.  Apply this filter only if DCOP appId of application that queued
    // the text contains this string.  List may be single element of comma-separated values,
    // or multiple elements.
    TQDomNodeList appIdList = doc.elementsByTagName( "appid" );
    TQString appIds;
    for ( uint ndx=0; ndx < appIdList.count(); ++ndx )
    {
        TQDomNode appIdNode = appIdList.item( ndx );
        if (!appIds.isEmpty()) appIds += ",";
        appIds += appIdNode.toElement().text();
    }
    if ( !clear ) appIds = m_widget->appIdLineEdit->text() + appIds;
    m_widget->appIdLineEdit->setText( appIds );

    // Word list.
    TQListViewItem* item = 0;
    if ( !clear ) item = m_widget->substLView->lastChild();
    TQDomNodeList wordList = doc.elementsByTagName("word");
    const int wordListCount = wordList.count();
    for (int wordIndex = 0; wordIndex < wordListCount; ++wordIndex)
    {
        // kdDebug() << "StringReplacerConf::load: start parsing of word " << wordIndex << endl;
        TQDomNode wordNode = wordList.item(wordIndex);
        TQDomNodeList propList = wordNode.childNodes();
        TQString wordType;
        TQString matchCase = "No"; // Default for old (v<=3.5.3) config files with no <case/>.
        TQString match;
        TQString subst;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex)
        {
            TQDomNode propNode = propList.item(propIndex);
            TQDomElement prop = propNode.toElement();
            if (prop.tagName() == "type") wordType = prop.text();
            if (prop.tagName() == "case") matchCase = prop.text();
            if (prop.tagName() == "match") match = prop.text();
            if (prop.tagName() == "subst") subst = prop.text();
        }
        TQString wordTypeStr = 
            (wordType=="RegExp"?i18n("Abbreviation for 'Regular Expression'", "RegExp"):i18n("Word"));
        TQString matchCaseStr = 
            (matchCase=="Yes"?i18n("Yes"):i18n("No"));  
        if (!item)
            item = new KListViewItem(m_widget->substLView, wordTypeStr, matchCaseStr, match, subst);
        else
            item = new KListViewItem(m_widget->substLView, item, wordTypeStr, matchCaseStr, match, subst);
    }

    return TQString();
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
void StringReplacerConf::save(KConfig* config, const TQString& configGroup){
    // kdDebug() << "StringReplacerConf::save: Running" << endl;
    TQString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", true );
    if ( wordsFilename.isEmpty() )
    {
        kdDebug() << "StringReplacerConf::save: no save location" << endl;
        return;
    }
    wordsFilename += configGroup;
    TQString errMsg = saveToFile( wordsFilename );
    if ( errMsg.isEmpty() )
    {
        config->setGroup( configGroup );
        config->writeEntry( "WordListFile", realFilePath(wordsFilename) );
    }
    else
        kdDebug() << "StringReplacerConf::save: " << errMsg << endl;
}

// Saves word list and settings to a file.
TQString StringReplacerConf::saveToFile(const TQString& filename)
{
    // kdDebug() << "StringReplacerConf::saveToFile: saving to file " << wordsFilename << endl;

    TQFile file( filename );
    if ( !file.open( IO_WriteOnly ) )
        return i18n("Unable to open file ") + filename;

    // TQDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    TQDomDocument doc( "" );

    TQDomElement root = doc.createElement( "wordlist" );
    doc.appendChild( root );

    // Name.
    TQDomElement name = doc.createElement( "name" );
    root.appendChild( name );
    TQDomText t = doc.createTextNode( m_widget->nameLineEdit->text() );
    name.appendChild( t );

    // Language code.
    for ( uint ndx=0; ndx < m_languageCodeList.count(); ++ndx )
    {
        TQDomElement languageCode = doc.createElement( "language-code" );
        root.appendChild( languageCode );
        t = doc.createTextNode( m_languageCodeList[ndx] );
        languageCode.appendChild( t );
    }

    // Application ID
    TQString appId = m_widget->appIdLineEdit->text().replace(" ", "");
    if ( !appId.isEmpty() )
    {
        TQStringList appIdList = TQStringList::split(",", appId);
        for ( uint ndx=0; ndx < appIdList.count(); ++ndx )
        {
            TQDomElement appIdElem = doc.createElement( "appid" );
            root.appendChild( appIdElem );
            t = doc.createTextNode( appIdList[ndx] );
            appIdElem.appendChild( t );
        }
    }

    // Words.
    TQListView* lView = m_widget->substLView;
    TQListViewItem* item = lView->firstChild();
    while (item)
    {
        TQDomElement wordTag = doc.createElement( "word" );
        root.appendChild( wordTag );
        TQDomElement propTag = doc.createElement( "type" );
        wordTag.appendChild( propTag);
        TQDomText t = doc.createTextNode( item->text(0)==i18n("Word")?"Word":"RegExp" );
        propTag.appendChild( t );

        propTag = doc.createElement( "case" );
        wordTag.appendChild( propTag);
        t = doc.createTextNode( item->text(1)==i18n("Yes")?"Yes":"No" );
        propTag.appendChild( t );      

        propTag = doc.createElement( "match" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( item->text(2) );
        propTag.appendChild( t );

        propTag = doc.createElement( "subst" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( item->text(3) );
        propTag.appendChild( t );

        item = item->nextSibling();
    }

    // Write it all out.
    TQTextStream ts( &file );
    ts.setEncoding( TQTextStream::UnicodeUTF8 );
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ts << doc.toString();
    // kdDebug() << "StringReplacerConf::saveToFile: writing out " << doc.toString() << endl;
    file.close();

    return TQString();
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
    // Default language is none.
    m_languageCodeList.clear();
    m_widget->languageLineEdit->setText( "" );
    // Default name.
    m_widget->nameLineEdit->setText( i18n("String Replacer") );
    m_widget->substLView->clear();
    // Default App ID is blank.
    m_widget->appIdLineEdit->setText( "" );
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
TQString StringReplacerConf::userPlugInName()
{
    if ( m_widget->substLView->childCount() == 0 ) return TQString();
    TQString instName = m_widget->nameLineEdit->text();
    if ( instName.isEmpty() )
    {
        TQString language;
        if (m_languageCodeList.count() == 1)
            language = KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[0]);
        if (m_languageCodeList.count() > 1)
            language = i18n("Multiple Languages");
        if (!language.isEmpty())
            instName = i18n("String Replacer") + " (" + language + ")";
    }
    return instName;
}

void StringReplacerConf::slotLanguageBrowseButton_clicked()
{
    // Create a  TQHBox to host KListView.
    TQHBox* hBox = new TQHBox(m_widget, "SelectLanguage_hbox");
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    langLView->setSelectionMode(TQListView::Extended);
    TQStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    TQString locale;
    TQString languageCode;
    TQString countryCode;
    TQString charSet;
    TQString language;
    // Blank line so user can select no language.
    TQListViewItem* item = new KListViewItem(langLView, "", "");
    if (m_languageCodeList.isEmpty()) item->setSelected(true);
    const int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ++ndx)
    {
        locale = allLocales[ndx];
        KGlobal::locale()->splitLocale(locale, languageCode, countryCode, charSet);
        language = KGlobal::locale()->twoAlphaToLanguageName(languageCode);
        if (!countryCode.isEmpty()) language +=
            " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode)+")";
        item = new KListViewItem(langLView, language, locale);
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
    dlg->setInitialSize(TQSize(300, 500), false);
    int dlgResult = dlg->exec();
    languageCode = TQString();
    if (dlgResult == TQDialog::Accepted)
    {
        m_languageCodeList.clear();
        TQListViewItem* item = langLView->firstChild();
        while (item)
        {
            if (item->isSelected()) m_languageCodeList += item->text(1);
            item = item->nextSibling();
        }
    }
    delete dlg;
    // TODO: Also delete KListView and TQHBox?
    if (dlgResult != TQDialog::Accepted) return;
    language = "";
    for ( uint ndx=0; ndx < m_languageCodeList.count(); ++ndx)
    {
        if (!language.isEmpty()) language += ",";
        language += KGlobal::locale()->twoAlphaToLanguageName(m_languageCodeList[ndx]);
    }
    TQString s1 = m_widget->languageLineEdit->text();
    m_widget->languageLineEdit->setText(language);
    // Replace language in the user's filter name.
    TQString s2 = m_widget->nameLineEdit->text();
    if (m_languageCodeList.count() > 1) language = i18n("Multiple Languages");
    if ( !s1.isEmpty() )
    {
        s2.replace( s1, language );
        s2.replace( i18n("Multiple Languages"), language );
    }
    s2.replace(" ()", "");
    if ( !s2.contains("(") && !language.isEmpty() ) s2 += " (" + language + ")";
    m_widget->nameLineEdit->setText(s2);
    configChanged();
}

void StringReplacerConf::enableDisableButtons()
{
    bool enableBtn = (m_widget->substLView->selectedItem() != 0);
    if (enableBtn)
    {
        m_widget->upButton->setEnabled(
            m_widget->substLView->selectedItem()->itemAbove() != 0);
        m_widget->downButton->setEnabled(
            m_widget->substLView->selectedItem()->itemBelow() != 0);
    } else {
        m_widget->upButton->setEnabled(false);
        m_widget->downButton->setEnabled(false);
    }
    m_widget->editButton->setEnabled(enableBtn);
    m_widget->removeButton->setEnabled(enableBtn);
    m_widget->clearButton->setEnabled(m_widget->substLView->firstChild());
    m_widget->saveButton->setEnabled(m_widget->substLView->firstChild());
}

void StringReplacerConf::slotUpButton_clicked()
{
    TQListViewItem* item = m_widget->substLView->selectedItem();
    if (!item) return;
    TQListViewItem* prevItem = item->itemAbove();
    if (!prevItem) return;
    prevItem->moveItem(item);
    m_widget->substLView->setSelected(item, true);
    m_widget->substLView->ensureItemVisible(item);
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotDownButton_clicked()
{
    TQListViewItem* item = m_widget->substLView->selectedItem();
    if (!item) return;
    TQListViewItem* nextItem = item->itemBelow();
    if (!nextItem) return;
    item->moveItem(nextItem);
    m_widget->substLView->setSelected(item, true);
    m_widget->substLView->ensureItemVisible(item);
    enableDisableButtons();
    configChanged();
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
    TQListViewItem* item = 0;
    if (isAdd)
        item = m_widget->substLView->lastChild();
    else
    {
        item = m_widget->substLView->selectedItem();
        if (!item) return;
    }
    // Create a TQHBox to host widget.
    TQHBox* hBox = new TQHBox(m_widget, "AddOrEditSubstitution_hbox" );
    // Create widget.
    m_editWidget = new EditReplacementWidget( hBox, "AddOrEditSubstitution_widget" );
    // Set controls if editing existing.
    m_editWidget->matchButton->setEnabled( false );
    if (!isAdd)
    {
        if ( item->text(0) == i18n("Abbreviation for 'Regular Expression'", "RegExp") )
        {
            m_editWidget->regexpRadioButton->setChecked( true );
            m_editWidget->matchButton->setEnabled( m_reEditorInstalled );
        }
        m_editWidget->caseCheckBox->setChecked( (item->text(1))==i18n("Yes") );
        m_editWidget->matchLineEdit->setText( item->text(2) );
        m_editWidget->substLineEdit->setText( item->text(3) );
    }
    // The match box may not be blank.
    connect( m_editWidget->matchLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
         this, TQT_SLOT(slotMatchLineEdit_textChanged(const TQString&)) );
    connect( m_editWidget->regexpRadioButton, TQT_SIGNAL(clicked()),
         this, TQT_SLOT(slotTypeButtonGroup_clicked()) );
    connect( m_editWidget->wordRadioButton, TQT_SIGNAL(clicked()),
         this, TQT_SLOT(slotTypeButtonGroup_clicked()) );
    connect( m_editWidget->matchButton, TQT_SIGNAL(clicked()),
         this, TQT_SLOT(slotMatchButton_clicked()) );
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
    TQString substType = i18n( "Word" );
    if ( m_editWidget->regexpRadioButton->isChecked() )
    	substType = i18n("Abbreviation for 'Regular Expression'", "RegExp");
    TQString matchCase = i18n("No");
    if ( m_editWidget->caseCheckBox->isChecked() ) matchCase = i18n("Yes");
    TQString match = m_editWidget->matchLineEdit->text();
    TQString subst = m_editWidget->substLineEdit->text();
    delete m_editDlg;
    m_editDlg = 0;
    m_editWidget = 0;
    if (dlgResult != TQDialog::Accepted) return;
    // TODO: Also delete hBox and w?
    if ( match.isEmpty() ) return;
    if ( isAdd )
    {
        if ( item )
            item = new KListViewItem( m_widget->substLView, item, substType, matchCase, match, subst );
        else
            item = new KListViewItem( m_widget->substLView, substType, matchCase, match, subst );
        m_widget->substLView->setSelected( item, true );
    }
    else
    {
        item->setText( 0, substType );
        item->setText( 1, matchCase );
        item->setText( 2, match );
        item->setText( 3, subst );
    }
    m_widget->substLView->ensureItemVisible( item );
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotMatchLineEdit_textChanged(const TQString& text)
{
    // Disable OK button if match field blank.
    if ( !m_editDlg ) return;
    m_editDlg->enableButton( KDialogBase::Ok, !text.isEmpty() );
}

void StringReplacerConf::slotRemoveButton_clicked()
{
    TQListViewItem* item = m_widget->substLView->selectedItem();
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
    TQDialog *editorDialog = 
        KParts::ComponentFactory::createInstanceFromQuery<TQDialog>( "KRegExpEditor/KRegExpEditor" );
    if ( editorDialog )
    {
        // kdeutils was installed, so the dialog was found.  Fetch the editor interface.
        KRegExpEditorInterface *reEditor =
            static_cast<KRegExpEditorInterface *>(editorDialog->qt_cast( "KRegExpEditorInterface" ) );
        Q_ASSERT( reEditor ); // This should not fail!// now use the editor.
        reEditor->setRegExp( m_editWidget->matchLineEdit->text() );
        int dlgResult = editorDialog->exec();
        if ( dlgResult == TQDialog::Accepted )
        {
            TQString re = reEditor->regExp();
            m_editWidget->matchLineEdit->setText( re );
            m_editDlg->enableButton( KDialogBase::Ok, !re.isEmpty() );
        }
        delete editorDialog;
    } else return;
}

void StringReplacerConf::slotLoadButton_clicked()
{
    // TQString dataDir = KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/stringreplacer/";
    TQString dataDir = KGlobal::dirs()->findAllResources("data", "kttsd/stringreplacer/").last();
    TQString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*.xml|String Replacer Word List (*.xml)",
        m_widget,
        "stringreplacer_loadfile");
    if ( filename.isEmpty() ) return;
    TQString errMsg = loadFromFile( filename, false );
    enableDisableButtons();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_widget, errMsg, i18n("Error Opening File") );
    else
        configChanged();
}

void StringReplacerConf::slotSaveButton_clicked()
{
    TQString filename = KFileDialog::getSaveFileName(
            KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", false ),
           "*.xml|String Replacer Word List (*.xml)",
            m_widget,
            "stringreplacer_savefile");
    if ( filename.isEmpty() ) return;
    TQString errMsg = saveToFile( filename );
    enableDisableButtons();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( m_widget, errMsg, i18n("Error Opening File") );
}

void StringReplacerConf::slotClearButton_clicked()
{
    m_widget->substLView->clear();
    enableDisableButtons();
}
