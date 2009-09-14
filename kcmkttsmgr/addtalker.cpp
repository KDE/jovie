/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language and synthesizer.
  Uses addtalkerwidget.ui.
  -------------------
  Copyright: (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright: (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

// KTTS includes.
#include "addtalker.h"

// Qt includes.
#include <QtGui/QDialog>

// KDE includes.
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kspeechinterface.h"

#include "ui_addtalkerwidget.h"

AddTalker::AddTalker(QWidget* parent)
    : KDialog(parent)
{
    this->setCaption(i18n("Add Talker"));
    this->setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    this->setDefaultButton(KDialog::Cancel);
    this->enableButtonOk(false);
    this->setHelp("select-plugin", "kttsd");

    QWidget * widget = new QWidget(this);
    mUi = new Ui::AddTalkerWidget;
    mUi->setupUi(widget);
    connect(mUi->AvailableTalkersTable, SIGNAL(itemSelectionChanged()), this, SLOT(slot_tableSelectionChanged()));
    this->setMainWidget(widget);

    org::kde::KSpeech* kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());

    m_outputModules = kspeech->outputModules();

    mUi->AvailableTalkersTable->setSortingEnabled(false);

    foreach (const QString module, m_outputModules)
    {
        QStringList languages = kspeech->languagesByModule(module);
        
        foreach (const QString language, languages)
        {
            int rowcount = mUi->AvailableTalkersTable->rowCount();
            mUi->AvailableTalkersTable->setRowCount(rowcount + 1);
            
            // set the synthesizer item
            QTableWidgetItem * item = new QTableWidgetItem(module);
            mUi->AvailableTalkersTable->setItem(rowcount, 1, item);

            QString langName = languageCodeToLanguage(language);
            // set the language name item
            item = new QTableWidgetItem(langName.isEmpty() ? language : langName);
            item->setToolTip(language);
            mUi->AvailableTalkersTable->setItem(rowcount, 0, item);
        }
    }
    mUi->AvailableTalkersTable->setSortingEnabled(true);
    // Default to user's desktop language.
    QString languageCode = KGlobal::locale()->defaultLanguage();
    // If there is not a synth that supports the locale, try stripping country code.
    //if (!m_langToSynthMap.contains(languageCode))
    //{
    //    QString countryCode;
    //    QString modifier;
    //    QString charSet;
    //    QString langAlpha;
    //    KGlobal::locale()->splitLocale(languageCode, langAlpha, countryCode, modifier, charSet);
    //    languageCode = langAlpha;
    //}
    //// If there is still not a synth that supports the language code, default to "other".
    //if (!m_langToSynthMap.contains(languageCode)) languageCode = "other";

    //// Select the language in the language combobox.
    //QString language = languageCodeToLanguage(languageCode);
    //languageSelection->setCurrentItem(language, false);
}

AddTalker::~AddTalker()
{
}

void AddTalker::slot_tableSelectionChanged()
{
    this->enableButtonOk(true);
}

/**
* Returns user's chosen language code.
*/
QString AddTalker::getLanguageCode() const
{
    //return m_languageToLanguageCodeMap[languageSelection->currentText()];
    
    return mUi->AvailableTalkersTable->item(mUi->AvailableTalkersTable->currentRow(), 0)->toolTip();
}

/**
* Returns user's chosen synthesizer.
*/
QString AddTalker::getSynthesizer() const 
{ 
	//return synthesizerSelection->currentText(); 
    return mUi->AvailableTalkersTable->item(mUi->AvailableTalkersTable->currentRow(), 2)->text();
}

// Set the synthesizer-to-languages map.
// @param synthToLang        QMap of supported language codes indexed by synthesizer.
//void AddTalker::setSynthToLangMap(SynthToLangMap synthToLangMap)
//{
//    m_synthToLangMap = synthToLangMap;
//    // "Invert" the map, i.e., map language codes to synthesizers.
//    QStringList synthList = m_synthToLangMap.keys();
//    const int synthListCount = synthList.count();
//    for (int synthNdx=0; synthNdx < synthListCount; ++synthNdx)
//    {
//        QString synth = synthList[synthNdx];
//        QStringList languageCodeList = m_synthToLangMap[synth];
//        const int languageCodeListCount = languageCodeList.count();
//        for (int langNdx=0; langNdx < languageCodeListCount; ++langNdx)
//        {
//            QString languageCode = languageCodeList[langNdx];
//            QStringList synthesizerList = m_langToSynthMap[languageCode];
//            synthesizerList.append(synth);
//            m_langToSynthMap[languageCode] = synthesizerList;
//        }
//    }
//    // Fill language to language code map.
//    QStringList languageCodeList = m_langToSynthMap.keys();
//    const int languageCodeListCount = languageCodeList.count();
//    for (int ndx = 0; ndx < languageCodeListCount; ++ndx)
//    {
//        QString languageCode = languageCodeList[ndx];
//        QString language = languageCodeToLanguage(languageCode);
//        m_languageToLanguageCodeMap[language] = languageCode;
//    }
//}

// Converts a language code plus optional country code to language description.
QString AddTalker::languageCodeToLanguage(const QString &languageCode)
{
    QString langAlpha;
    QString countryCode;
    QString modifier;
    QString charSet;
    QString language;
    if (languageCode == "other")
        language = i18nc("Other language", "Other");
    else
    {
        KGlobal::locale()->splitLocale(languageCode, langAlpha, countryCode, modifier, charSet);
        language = KGlobal::locale()->languageCodeToName(langAlpha);
    }
    if (!countryCode.isEmpty())
        language += " (" + KGlobal::locale()->countryCodeToName(countryCode) + ')';
    return language;
}

// Based on user's radio button selection, filters choices for language or synthesizer
// comboboxes based on what is selected in the other combobox.
//void AddTalker::applyFilter()
//{
//    if (languageRadioButton->isChecked())
//    {
//        // Get current language.
//        QString language = languageSelection->currentText();
//        // Fill language combobox will all possible languages.
//        languageSelection->clear();
//        QStringList languageCodeList = m_langToSynthMap.keys();
//        const int languageCodeListCount = languageCodeList.count();
//        QStringList languageList;
//        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
//        {
//            languageList.append(languageCodeToLanguage(languageCodeList[ndx]));
//        }
//        languageList.sort();
//        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
//        {
//            languageSelection->addItem(languageList[ndx]);
//        }
//        // Re-select user's selection.
//        languageSelection->setCurrentItem(language, false);
//        // Get current language selection.
//        language = languageSelection->currentText();
//        // Map current language to language code.
//        QString languageCode = m_languageToLanguageCodeMap[language];
//        // Get list of synths that support this language code.
//        QStringList synthList = m_langToSynthMap[languageCode];
//        // Get current user's synth selection.
//        QString synth = synthesizerSelection->currentText();
//        // Fill synthesizer combobox.
//        synthesizerSelection->clear();
//        synthList.sort();
//        const int synthListCount = synthList.count();
//        for (int ndx=0; ndx < synthListCount; ++ndx)
//        {
//            synthesizerSelection->addItem(synthList[ndx]);
//        }
//        // Re-select user's selection.
//        synthesizerSelection->setCurrentItem(synth, false);
//    }
//    else
//    {
//        // Get current synth selection.
//        QString synth = synthesizerSelection->currentText();
//        // Fill synthesizer combobox with all possible synths.
//        synthesizerSelection->clear();
//        QStringList synthList = m_synthToLangMap.keys();
//        synthList.sort();
//        const int synthListCount = synthList.count();
//        for (int ndx=0; ndx < synthListCount; ++ndx)
//        {
//            synthesizerSelection->addItem(synthList[ndx]);
//        }
//        // Re-select user's synthesizer.
//        synthesizerSelection->setCurrentItem(synth, false);
//        // Get current synth selection.
//        synth = synthesizerSelection->currentText();
//        // Get list of supported language codes.
//        QStringList languageCodeList = m_synthToLangMap[synth];
//        // Get current user's language selection.
//        QString language = languageSelection->currentText();
//        // Fill language combobox with language descriptions.
//        languageSelection->clear();
//        const int languageCodeListCount = languageCodeList.count();
//        QStringList languageList;
//        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
//        {
//            languageList.append(languageCodeToLanguage(languageCodeList[ndx]));
//        }
//        languageList.sort();
//        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
//        {
//            languageSelection->addItem(languageList[ndx]);
//        }
//        // Re-select user's language selection.
//        languageSelection->setCurrentItem(language, false);
//    }
//}

#include "addtalker.moc"

