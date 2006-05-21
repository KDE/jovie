/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language and synthesizer
  (button).  Uses addtalkerwidget.ui.
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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
#include <QRadioButton>
#include <QDialog>

// KDE includes.
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// KTTS includes.
#include "addtalker.h"

AddTalker::AddTalker(SynthToLangMap synthToLangMap, QWidget* parent, const char* name, Qt::WFlags fl)
    : QDialog(parent)
{
    Q_UNUSED(name);
    Q_UNUSED(fl);

    setupUi(this);

    // Build maps.
    setSynthToLangMap(synthToLangMap);

    // Fill comboboxes.
    applyFilter();

    // Default to user's desktop language.
    QString languageCode = KGlobal::locale()->defaultLanguage();
    // If there is not a synth that supports the locale, try stripping country code.
    if (!m_langToSynthMap.contains(languageCode))
    {
        QString countryCode;
        QString charSet;
        QString twoAlpha;
        KGlobal::locale()->splitLocale(languageCode, twoAlpha, countryCode, charSet);
        languageCode = twoAlpha;
    }
    // If there is still not a synth that supports the language code, default to "other".
    if (!m_langToSynthMap.contains(languageCode)) languageCode = "other";

    // Select the language in the language combobox.
    QString language = languageCodeToLanguage(languageCode);
    languageSelection->setCurrentItem(language, false);

    // Filter comboboxes.
    applyFilter();

    // Connect widgets to slots.
    connect(languageRadioButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(synthesizerRadioButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(languageSelection, SIGNAL(activated(int)), this, SLOT(applyFilter()));
    connect(synthesizerSelection, SIGNAL(activated(int)), this, SLOT(applyFilter()));
}

AddTalker::~AddTalker()
{
}

/**
* Returns user's chosen language code.
*/
QString AddTalker::getLanguageCode()
{
     return m_languageToLanguageCodeMap[languageSelection->currentText()];
}

/**
* Returns user's chosen synthesizer.
*/
QString AddTalker::getSynthesizer() { return synthesizerSelection->currentText(); }

// Set the synthesizer-to-languages map.
// @param synthToLang        QMap of supported language codes indexed by synthesizer.
void AddTalker::setSynthToLangMap(SynthToLangMap synthToLangMap)
{
    m_synthToLangMap = synthToLangMap;
    // "Invert" the map, i.e., map language codes to synthesizers.
    QStringList synthList = m_synthToLangMap.keys();
    const int synthListCount = synthList.count();
    for (int synthNdx=0; synthNdx < synthListCount; ++synthNdx)
    {
        QString synth = synthList[synthNdx];
        QStringList languageCodeList = m_synthToLangMap[synth];
        const int languageCodeListCount = languageCodeList.count();
        for (int langNdx=0; langNdx < languageCodeListCount; ++langNdx)
        {
            QString languageCode = languageCodeList[langNdx];
            QStringList synthesizerList = m_langToSynthMap[languageCode];
            synthesizerList.append(synth);
            m_langToSynthMap[languageCode] = synthesizerList;
        }
    }
    // Fill language to language code map.
    QStringList languageCodeList = m_langToSynthMap.keys();
    const int languageCodeListCount = languageCodeList.count();
    for (int ndx = 0; ndx < languageCodeListCount; ++ndx)
    {
        QString languageCode = languageCodeList[ndx];
        QString language = languageCodeToLanguage(languageCode);
        m_languageToLanguageCodeMap[language] = languageCode;
    }
}

// Converts a language code plus optional country code to language description.
QString AddTalker::languageCodeToLanguage(const QString &languageCode)
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

// Based on user's radio button selection, filters choices for language or synthesizer
// comboboxes based on what is selected in the other combobox.
void AddTalker::applyFilter()
{
    if (languageRadioButton->isChecked())
    {
        // Get current language.
        QString language = languageSelection->currentText();
        // Fill language combobox will all possible languages.
        languageSelection->clear();
        QStringList languageCodeList = m_langToSynthMap.keys();
        const int languageCodeListCount = languageCodeList.count();
        QStringList languageList;
        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
        {
            languageList.append(languageCodeToLanguage(languageCodeList[ndx]));
        }
        languageList.sort();
        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
        {
            languageSelection->addItem(languageList[ndx]);
        }
        // Re-select user's selection.
        languageSelection->setCurrentItem(language, false);
        // Get current language selection.
        language = languageSelection->currentText();
        // Map current language to language code.
        QString languageCode = m_languageToLanguageCodeMap[language];
        // Get list of synths that support this language code.
        QStringList synthList = m_langToSynthMap[languageCode];
        // Get current user's synth selection.
        QString synth = synthesizerSelection->currentText();
        // Fill synthesizer combobox.
        synthesizerSelection->clear();
        synthList.sort();
        const int synthListCount = synthList.count();
        for (int ndx=0; ndx < synthListCount; ++ndx)
        {
            synthesizerSelection->addItem(synthList[ndx]);
        }
        // Re-select user's selection.
        synthesizerSelection->setCurrentItem(synth, false);
    }
    else
    {
        // Get current synth selection.
        QString synth = synthesizerSelection->currentText();
        // Fill synthesizer combobox with all possible synths.
        synthesizerSelection->clear();
        QStringList synthList = m_synthToLangMap.keys();
        synthList.sort();
        const int synthListCount = synthList.count();
        for (int ndx=0; ndx < synthListCount; ++ndx)
        {
            synthesizerSelection->addItem(synthList[ndx]);
        }
        // Re-select user's synthesizer.
        synthesizerSelection->setCurrentItem(synth, false);
        // Get current synth selection.
        synth = synthesizerSelection->currentText();
        // Get list of supported language codes.
        QStringList languageCodeList = m_synthToLangMap[synth];
        // Get current user's language selection.
        QString language = languageSelection->currentText();
        // Fill language combobox with language descriptions.
        languageSelection->clear();
        const int languageCodeListCount = languageCodeList.count();
        QStringList languageList;
        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
        {
            languageList.append(languageCodeToLanguage(languageCodeList[ndx]));
        }
        languageList.sort();
        for (int ndx=0; ndx < languageCodeListCount; ++ndx)
        {
            languageSelection->addItem(languageList[ndx]);
        }
        // Re-select user's language selection.
        languageSelection->setCurrentItem(language, false);
    }
}

#include "addtalker.moc"

