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

// StringReplacer includes.
#include "stringreplacerconf.h"
#include "stringreplacerconf.moc"

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtXml/QDomDocument>
#include <QtGui/QRadioButton>
#include <QtCore/QTextStream>
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>

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
#include <kmessagebox.h>
#include <khbox.h>
#include <kservicetypetrader.h>

// KTTS includes.
#include "selectlanguagedlg.h"
#include "filterconf.h"

StringReplacerConf::StringReplacerConf( QWidget *parent, const QVariantList& args ) :
    KttsFilterConf(parent, args),
    m_editDlg(0),
    m_editWidget(0)
{
    Q_UNUSED(args);
    // kDebug() << "StringReplacerConf::StringReplacerConf: Running";

    // Create configuration widget.
    setupUi(this);

    substLView->setSortingEnabled(false);
    substLView->verticalHeader()->hide();
    substLView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(nameLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(languageBrowseButton, SIGNAL(clicked()),
        this, SLOT(slotLanguageBrowseButton_clicked()));
    connect(addButton, SIGNAL(clicked()),
        this, SLOT(slotAddButton_clicked()));
    connect(upButton, SIGNAL(clicked()),
        this, SLOT(slotUpButton_clicked()));
    connect(downButton, SIGNAL(clicked()),
        this, SLOT(slotDownButton_clicked()));
    connect(editButton, SIGNAL(clicked()),
        this, SLOT(slotEditButton_clicked()));
    connect(removeButton, SIGNAL(clicked()),
        this, SLOT(slotRemoveButton_clicked()));
    connect(loadButton, SIGNAL(clicked()),
        this, SLOT(slotLoadButton_clicked()));
    connect(saveButton, SIGNAL(clicked()),
        this, SLOT(slotSaveButton_clicked()));
    connect(clearButton, SIGNAL(clicked()),
        this, SLOT(slotClearButton_clicked()));
    connect(substLView, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),
        this, SLOT(enableDisableButtons()));
    connect(appIdLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KServiceTypeTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();

    // Set up defaults.
    defaults();
}

StringReplacerConf::~StringReplacerConf(){
    // kDebug() << "StringReplacerConf::~StringReplacerConf: Running";
}

void StringReplacerConf::load(KConfig* c, const QString& configGroup){
    // kDebug() << "StringReplacerConf::load: Running";
    // See if this filter previously save its word list.
    KConfigGroup config( c, configGroup );
    QString wordsFilename = config.readEntry( "WordListFile" );
    if ( !wordsFilename.isEmpty() )
    {
        QString errMsg = loadFromFile( wordsFilename, true );
        if ( !errMsg.isEmpty() )
            kDebug() << "StringReplacerConf::load: " << errMsg;
        enableDisableButtons();
    }
}

// Loads word list and settings from a file.  Clearing configuration if clear is True.
QString StringReplacerConf::loadFromFile( const QString& filename, bool clear)
{
    // Open existing word list.
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        return i18n("Unable to open file.") + filename;
    }
    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        return i18n("File not in proper XML format.");
    }
    // kDebug() << "StringReplacerConf::load: document successfully parsed.";
    file.close();

    // Clear list view.
    if ( clear ) substLView->setRowCount(0);

    // Name setting.
    QDomNodeList nameList = doc.elementsByTagName( "name" );
    QDomNode nameNode = nameList.item( 0 );
    nameLineEdit->setText( nameNode.toElement().text() );
    // kDebug() << "StringReplacerConf::load: name = " << nameNode.toElement().text();

    // Language Codes setting.  List may be single element of comma-separated values,
    // or multiple elements.
    QString languageCodes;
    QDomNodeList languageList = doc.elementsByTagName( "language-code" );
    for ( int ndx=0; ndx < languageList.count(); ++ndx )
    {
        QDomNode languageNode = languageList.item( ndx );
        if (!languageCodes.isEmpty()) languageCodes += ',';
        languageCodes += languageNode.toElement().text();
    }
    if ( clear )
        m_languageCodeList = languageCodes.split(',', QString::SkipEmptyParts);
    else
        m_languageCodeList += languageCodes.split(',', QString::SkipEmptyParts);
    QString language;
    m_languageCodeList.sort();
    // Eliminate dups.
    {
        int ndx = m_languageCodeList.count() - 2;
        while ( ndx >= 0 ) {
            if ( m_languageCodeList[ndx] == m_languageCodeList[ndx+1] )
                m_languageCodeList.removeAt(ndx+1);
            ndx--;
        }
    }
    for ( int ndx=0; ndx < m_languageCodeList.count(); ++ndx )
    {
        if (!language.isEmpty()) language += ',';
        language += KGlobal::locale()->languageCodeToName(m_languageCodeList[ndx]);
    }
    languageLineEdit->setText(language);

    // AppId.  Apply this filter only if DCOP appId of application that queued
    // the text contains this string.  List may be single element of comma-separated values,
    // or multiple elements.
    QDomNodeList appIdList = doc.elementsByTagName( "appid" );
    QString appIds;
    for ( int ndx=0; ndx < appIdList.count(); ++ndx )
    {
        QDomNode appIdNode = appIdList.item( ndx );
        if (!appIds.isEmpty()) appIds += ',';
        appIds += appIdNode.toElement().text();
    }
    if ( !clear ) appIds = appIdLineEdit->text() + appIds;
    appIdLineEdit->setText( appIds );

    // Word list.
    QDomNodeList wordList = doc.elementsByTagName( "word" );
    const int wordListCount = wordList.count();
    for ( int wordIndex = 0; wordIndex < wordListCount; ++wordIndex )
    {
        // kDebug() << "StringReplacerConf::load: start parsing of word " << wordIndex;
        QDomNode wordNode = wordList.item(wordIndex);
        QDomNodeList propList = wordNode.childNodes();
        QString wordType;
        QString matchCase = "No";
        QString match;
        QString subst;
        const int propListCount = propList.count();
        for ( int propIndex = 0; propIndex < propListCount; ++propIndex )
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "type") wordType = prop.text();
            if (prop.tagName() == "case") matchCase = prop.text();
            if (prop.tagName() == "match") match = prop.text();
            if (prop.tagName() == "subst") subst = prop.text();
        }
        QString wordTypeStr = 
            (wordType=="RegExp"?i18nc("Abbreviation for 'Regular Expression'", "RegExp"):i18n("Word"));
        int tableRow = substLView->rowCount();
        QString matchCaseStr = 
            (matchCase=="Yes"?i18nc("Yes or no", "Yes"):i18nc("Yes or no", "No"));  
        substLView->setRowCount( tableRow + 1 );
        substLView->setItem( tableRow, 0, new QTableWidgetItem( wordTypeStr ) );
        substLView->setItem( tableRow, 1, new QTableWidgetItem( matchCaseStr ) );
        substLView->setItem( tableRow, 2, new QTableWidgetItem( match ) );
        substLView->setItem( tableRow, 3, new QTableWidgetItem( subst ) );
    }

    return QString();
}

void StringReplacerConf::save(KConfig* c, const QString& configGroup){
    // kDebug() << "StringReplacerConf::save: Running";
    QString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", true );
    if ( wordsFilename.isEmpty() )
    {
        kDebug() << "StringReplacerConf::save: no save location";
        return;
    }
    wordsFilename += configGroup;
    QString errMsg = saveToFile( wordsFilename );
    if ( errMsg.isEmpty() )
    {
        KConfigGroup config( c, configGroup );
        config.writeEntry( "WordListFile", realFilePath(wordsFilename) );
    }
    else
        kDebug() << "StringReplacerConf::save: " << errMsg;
}

// Saves word list and settings to a file.
QString StringReplacerConf::saveToFile(const QString& filename)
{
    // kDebug() << "StringReplacerConf::saveToFile: saving to file " << wordsFilename;

    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly ) )
        return i18n("Unable to open file ") + filename;

    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );

    QDomElement root = doc.createElement( "wordlist" );
    doc.appendChild( root );

    // Name.
    QDomElement name = doc.createElement( "name" );
    root.appendChild( name );
    QDomText t = doc.createTextNode( nameLineEdit->text() );
    name.appendChild( t );

    // Language code.
    for ( int ndx=0; ndx < m_languageCodeList.count(); ++ndx )
    {
        QDomElement languageCode = doc.createElement( "language-code" );
        root.appendChild( languageCode );
        t = doc.createTextNode( m_languageCodeList[ndx] );
        languageCode.appendChild( t );
    }

    // Application ID
    QString appId = appIdLineEdit->text().remove(' ');
    if ( !appId.isEmpty() )
    {
        QStringList appIdList = appId.split( ',', QString::SkipEmptyParts );
        for ( int ndx=0; ndx < appIdList.count(); ++ndx )
        {
            QDomElement appIdElem = doc.createElement( "appid" );
            root.appendChild( appIdElem );
            t = doc.createTextNode( appIdList[ndx] );
            appIdElem.appendChild( t );
        }
    }

    // Words.
    for ( int row = 0; row < substLView->rowCount(); ++row )
    {
        QDomElement wordTag = doc.createElement( "word" );
        root.appendChild( wordTag );
        QDomElement propTag = doc.createElement( "type" );
        wordTag.appendChild( propTag);
        QDomText t = doc.createTextNode( 
            substLView->item(row, 0)->text()==i18n("Word")?"Word":"RegExp" );
        propTag.appendChild( t );

        propTag = doc.createElement( "case" );
        wordTag.appendChild( propTag);
        t = doc.createTextNode(
            substLView->item(row, 2)->text()==i18nc("Yes or no", "Yes")?"Yes":"No" );
        propTag.appendChild( t );      

        propTag = doc.createElement( "match" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( substLView->item(row, 2)->text() );
        propTag.appendChild( t );

        propTag = doc.createElement( "subst" );
        wordTag.appendChild( propTag);
        t = doc.createCDATASection( substLView->item(row, 3)->text() );
        propTag.appendChild( t );
    }

    // Write it all out.
    QTextStream ts( &file );
    ts.setCodec( "UTF-8" );
    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ts << doc.toString();
    // kDebug() << "StringReplacerConf::saveToFile: writing out " << doc.toString();
    file.close();

    return QString();
}

void StringReplacerConf::defaults(){
    // kDebug() << "StringReplacerConf::defaults: Running";
    // Default language is none.
    m_languageCodeList.clear();
    languageLineEdit->setText( "" );
    // Default name.
    nameLineEdit->setText( i18n("String Replacer") );
    substLView->setRowCount(0);
    // Default App ID is blank.
    appIdLineEdit->setText( "" );
    enableDisableButtons();
    // kDebug() << "StringReplacerConf::defaults: Exiting";
}

bool StringReplacerConf::supportsMultiInstance() { return true; }

QString StringReplacerConf::userPlugInName()
{
    if ( substLView->rowCount() == 0 ) return QString();
    QString instName = nameLineEdit->text();
    if ( instName.isEmpty() )
    {
        QString language;
        if (m_languageCodeList.count() == 1)
            language = KGlobal::locale()->languageCodeToName(m_languageCodeList[0]);
        if (m_languageCodeList.count() > 1)
            language = i18n("Multiple Languages");
        if (!language.isEmpty())
            instName = i18n("String Replacer") + " (" + language + ')';
    }
    return instName;
}

// Converts a Substitution Type to displayable string.
QString StringReplacerConf::substitutionTypeToString(const int substitutionType)
{
    switch (substitutionType)
    {
        case stWord:        return i18n("Word");
        case stRegExp:      return i18nc("Abbreviation for 'Regular Expresion'", "RegExp");
    }
    return i18n("Error");
}

void StringReplacerConf::slotLanguageBrowseButton_clicked()
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
        language += KGlobal::locale()->languageCodeToName(m_languageCodeList[ndx]);
    }
    QString s1 = languageLineEdit->text();
    languageLineEdit->setText(language);
    // Replace language in the user's filter name.
    QString s2 = nameLineEdit->text();
    if (m_languageCodeList.count() > 1) language = i18n("Multiple Languages");
    if ( !s1.isEmpty() )
    {
        s2.replace( s1, language );
        s2.replace( i18n("Multiple Languages"), language );
    }
    s2.remove(" ()");
    if ( !s2.contains("(") && !language.isEmpty() ) s2 += " (" + language + ')';
    nameLineEdit->setText(s2);
    configChanged();
}

void StringReplacerConf::enableDisableButtons()
{
    int row = substLView->currentRow();
    bool enableBtn = (row >= 0 && row < substLView->rowCount());
    if (enableBtn)
    {
        upButton->setEnabled(row > 0);
        downButton->setEnabled(row < (substLView->rowCount() - 1));
    } else {
        upButton->setEnabled(false);
        downButton->setEnabled(false);
    }
    editButton->setEnabled(enableBtn);
    removeButton->setEnabled(enableBtn);
    clearButton->setEnabled(substLView->rowCount() > 0);
    saveButton->setEnabled(substLView->rowCount() > 0);
}

void StringReplacerConf::slotUpButton_clicked()
{
    int row = substLView->currentRow();
    if (row < 1 || row >= substLView->rowCount()) return;

    QTableWidgetItem *itemAbove = substLView->item(row - 1, 0);
    QTableWidgetItem *item = substLView->item(row, 0);
    QString t = itemAbove->text();
    itemAbove->setText(item->text());
    item->setText(t);

    itemAbove = substLView->item(row - 1, 1);
    item = substLView->item(row, 1);
    t = itemAbove->text();
    itemAbove->setText(item->text());
    item->setText(t);

    itemAbove = substLView->item(row - 1, 2);
    item = substLView->item(row, 2);
    t = itemAbove->text();
    itemAbove->setText(item->text());
    item->setText(t);

    itemAbove = substLView->item(row - 1, 3);
    item = substLView->item(row, 3);
    t = itemAbove->text();
    itemAbove->setText(item->text());
    item->setText(t);

    substLView->setCurrentItem(substLView->item(row - 1, substLView->currentColumn()));
    // TODO: Is this needed? substLView->scrollTo(substLView->indexFromItem(itemAbove));
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotDownButton_clicked()
{
    int row = substLView->currentRow();
    if (row < 0 || row >= substLView->rowCount() - 1) return;

    QTableWidgetItem *itemBelow = substLView->item(row + 1, 0);
    QTableWidgetItem *item = substLView->item(row, 0);
    QString t = itemBelow->text();
    itemBelow->setText(item->text());
    item->setText(t);

    itemBelow = substLView->item(row + 1, 1);
    item = substLView->item(row, 1);
    t = itemBelow->text();
    itemBelow->setText(item->text());
    item->setText(t);

    itemBelow = substLView->item(row + 1, 2);
    item = substLView->item(row, 2);
    t = itemBelow->text();
    itemBelow->setText(item->text());
    item->setText(t);

    itemBelow = substLView->item(row + 1, 3);
    item = substLView->item(row, 3);
    t = itemBelow->text();
    itemBelow->setText(item->text());
    item->setText(t);

    substLView->setCurrentItem(substLView->item(row + 1, substLView->currentColumn()));
    // TODO: Is this needed? substLView->scrollTo(substLView->indexFromItem(itemBelow));
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
    int row;
    if (isAdd)
        row = substLView->rowCount() - 1;
    else
        row = substLView->currentRow();
    // Create widget.
    QWidget *w = new QWidget();
    m_editWidget = new Ui::EditReplacementWidget();
    m_editWidget->setupUi( w );
    // Set controls if editing existing.
    m_editWidget->matchButton->setEnabled( false );
    if (!isAdd)
    {
        if ( substLView->item(row, 0)->text() != i18n("Word") )
        {
            m_editWidget->regexpRadioButton->setChecked( true );
            m_editWidget->matchButton->setEnabled( m_reEditorInstalled );
        }
        if ( substLView->item(row, 1)->text() == i18nc("Yes or no", "Yes") )
            m_editWidget->matchCaseCheckBox->setChecked( true );
        m_editWidget->matchLineEdit->setText( substLView->item(row, 2)->text() );
        m_editWidget->substLineEdit->setText( substLView->item(row, 3)->text() );
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
    m_editDlg = new KDialog(this);
    m_editDlg->setCaption(i18n("Edit String Replacement"));
    m_editDlg->setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    // TODO: KDialog bug workaround.  Remove when no longer needed.
    m_editDlg->setDefaultButton(KDialog::Cancel);
    // Disable OK button if match field blank.
    m_editDlg->setMainWidget( w );
    m_editDlg->setHelp( "", "jovie" );
    m_editDlg->enableButton( KDialog::Ok, !m_editWidget->matchLineEdit->text().isEmpty() );
    int dlgResult = m_editDlg->exec();
    QString substType = i18n( "Word" );
    if ( m_editWidget->regexpRadioButton->isChecked() ) 
        substType = i18nc("Abbreviation for 'Regular Expression'", "RegExp");
    QString matchCase = m_editWidget->matchCaseCheckBox->isChecked()?i18nc("Yes or no", "Yes"):i18nc("Yes or no", "No");
    QString match = m_editWidget->matchLineEdit->text();
    QString subst = m_editWidget->substLineEdit->text();
    delete m_editDlg;
    m_editDlg = 0;
    m_editWidget = 0;
    if (dlgResult != QDialog::Accepted) return;
    // TODO: Also delete QTableWidget and w?
    if ( match.isEmpty() ) return;
    if ( isAdd )
    {
        row = substLView->rowCount();
        substLView->setRowCount(row + 1);
        substLView->setCurrentItem(substLView->item(row, 0));
        for (int i = 0; i <= 4; ++i)
            substLView->setItem(row, i, new QTableWidgetItem(""));
    }
    substLView->item(row, 0)->setText(substType);
    substLView->item(row, 1)->setText(matchCase);
    substLView->item(row, 2)->setText(match);
    substLView->item(row, 3)->setText(subst);
    // TODO: Is this needed? substLView->scrollTo(substLView->indexFromItem(substLView->item(row,0)));
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotMatchLineEdit_textChanged(const QString& text)
{
    // Disable OK button if match field blank.
    if ( !m_editDlg ) return;
    m_editDlg->enableButton( KDialog::Ok, !text.isEmpty() );
}

void StringReplacerConf::slotRemoveButton_clicked()
{
    int row = substLView->currentRow();
    if (row <= 0 || row >= substLView->rowCount()) return;
    delete substLView->takeItem(row, 0);
    delete substLView->takeItem(row, 1);
    delete substLView->takeItem(row, 2);
    delete substLView->takeItem(row, 3);
    substLView->removeRow(row);
    enableDisableButtons();
    configChanged();
}

void StringReplacerConf::slotTypeButtonGroup_clicked()
{
    // Enable Regular Expression Editor button if editor is installed (requires kdeutils).
    if ( !m_editWidget ) return;
    m_editWidget->matchButton->setEnabled( m_editWidget->regexpRadioButton->isChecked() &&  m_reEditorInstalled );
}

void StringReplacerConf::slotMatchButton_clicked()
{
    // Show Regular Expression Editor dialog if it is installed.
    if ( !m_editWidget ) return;
    if ( !m_editDlg ) return;
    if ( !m_reEditorInstalled ) return;
    KDialog *editorDialog = 
        KServiceTypeTrader::createInstanceFromQuery<KDialog>( "KRegExpEditor/KRegExpEditor" );
    if ( editorDialog )
    {
        // kdeutils was installed, so the dialog was found.  Fetch the editor interface.

        KRegExpEditorInterface *reEditor = qobject_cast<KRegExpEditorInterface*>( editorDialog );
        Q_ASSERT( reEditor ); // This should not fail!// now use the editor.
        reEditor->setRegExp( m_editWidget->matchLineEdit->text() );
        int dlgResult = editorDialog->exec();
        if ( dlgResult == QDialog::Accepted )
        {
            QString re = reEditor->regExp();
            m_editWidget->matchLineEdit->setText( re );
            m_editDlg->enableButton( KDialog::Ok, !re.isEmpty() );
        }
        delete editorDialog;
    } else return;
}

void StringReplacerConf::slotLoadButton_clicked()
{
    QStringList dataDirs = KGlobal::dirs()->findAllResources("data", "kttsd/stringreplacer/");
    QString dataDir;
    if (!dataDirs.isEmpty()) dataDir = dataDirs.last();
    QString filename = KFileDialog::getOpenFileName(
        dataDir,
        "*.xml|String Replacer Word List (*.xml)",
        this,
        "stringreplacer_loadfile");
    if ( filename.isEmpty() ) return;
    QString errMsg = loadFromFile( filename, false );
    enableDisableButtons();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( this, errMsg, i18n("Error Opening File") );
    else
        configChanged();
}

void StringReplacerConf::slotSaveButton_clicked()
{
    QString filename = KFileDialog::getSaveFileName(
            KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", false ),
           "*.xml|String Replacer Word List (*.xml)",
            this,
            "stringreplacer_savefile");
    if ( filename.isEmpty() ) return;
    QString errMsg = saveToFile( filename );
    enableDisableButtons();
    if ( !errMsg.isEmpty() )
        KMessageBox::sorry( this, errMsg, i18n("Error Opening File") );
}

void StringReplacerConf::slotClearButton_clicked()
{
    substLView->setRowCount(0);
    enableDisableButtons();
}

