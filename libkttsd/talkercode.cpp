/***************************************************** vim:set ts=4 sw=4 sts=4:
  Convenience object for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see kspeech.h. 
  -------------------
  Copyright: (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright: (C) 2009 - 2010 by Jeremy Whiting <jpwhiting@kde.org>
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

// TalkerCode includes.
#include "talkercode.h"

// Qt includes.
#include <QtCore/QVector>
#include <QtXml/QDomDocument>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kservicetypetrader.h>

/**
 * Constructor.
 */
TalkerCode::TalkerCode(const QString &code/*=QString()*/, bool normal /*=false*/)
{
    if (!code.isEmpty())
        parseTalkerCode(code);
    //if (normal)
    //    normalize();
}

/**
 * Copy Constructor.
 */
TalkerCode::TalkerCode(TalkerCode* talker, bool normal /*=false*/)
{
    m_name = talker->name();
    m_language = talker->language();
    m_voiceType = talker->voiceType();
    m_volume = talker->volume();
    m_rate = talker->rate();
    m_pitch = talker->pitch();
    m_outputModule = talker->outputModule();
    //if (normal)
    //    normalize();
}

/**
 * Destructor.
 */
TalkerCode::~TalkerCode() { }

/**
 * Properties.
 */
QString TalkerCode::name() const { return m_name; }
QString TalkerCode::language() const { return m_language; }
int TalkerCode::voiceType() const { return m_voiceType; }
int TalkerCode::volume() const { return m_volume; }
int TalkerCode::rate() const { return m_rate; }
int TalkerCode::pitch() const { return m_pitch; }
QString TalkerCode::outputModule() const { return m_outputModule; }

void TalkerCode::setName(const QString &name) { m_name = name; }
void TalkerCode::setLanguage(const QString &language) { m_language = language; }
void TalkerCode::setVoiceType(int voiceType) { m_voiceType = voiceType; }
void TalkerCode::setVolume(int volume) { m_volume = volume; }
void TalkerCode::setRate(int rate) { m_rate = rate; }
void TalkerCode::setPitch(int pitch) { m_pitch = pitch; }
void TalkerCode::setOutputModule(const QString &moduleName) { m_outputModule = moduleName; }

/**
 * The Talker Code returned in XML format.
 */
void TalkerCode::setTalkerCode(const QString& code)
{
    parseTalkerCode(code);
}

QString TalkerCode::getTalkerCode() const
{
    QString code = QString("<voice name=\"%1\" lang=\"%2\" outputModule=\"%3\" voiceType=\"%4\">").arg(m_name).arg(m_language).arg(m_outputModule).arg(m_voiceType);
    code += QString("<prosody volume=\"%1\" rate=\"%2\" pitch=\"%3\" /></voice>").arg(m_volume).arg(m_rate).arg(m_pitch);
    return code;
}

/**
 * The Talker Code translated for display.
 */
QString TalkerCode::getTranslatedDescription() const
{
    QString code = m_language;
    bool prefer;
    //QString fullLangCode = m_language;
    //if (!fullLangCode.isEmpty()) code = languageCodeToLanguage( fullLangCode );
    // TODO: The PlugInName is always English.  Need a way to convert this to a translated
    // name (possibly via DesktopEntryNameToName, but to do that, we need the desktopEntryName
    // from the config file).
    if (!m_outputModule.isEmpty()) code += ' ' + stripPrefer(m_outputModule, prefer);
    //if (!m_voiceType.isEmpty()) code += ' ' + stripPrefer(m_voiceType, prefer);
    //if (!m_volume.isEmpty()) code += ' ' + translatedVolume(stripPrefer(m_volume, prefer));
    //if (!m_rate.isEmpty()) code += ' ' + translatedRate(stripPrefer(m_rate, prefer));
    code = code.trimmed();
    if (code.isEmpty()) code = i18nc("Default language code", "default");
    return code;
}

QString TalkerCode::translatedVoiceType(int voiceType)
{
    switch (voiceType)
    {
        case 1: return i18nc("The name of the first Male voice","Male 1"); break;
        case 2: return i18n("Male 2"); break;
        case 3: return i18n("Male 3"); break;
        case 4: return i18nc("The name of the first Female voice", "Female 1"); break;
        case 5: return i18n("Female 2"); break;
        case 6: return i18n("Female 3"); break;
        case 7: return i18nc("The name of the male child voice", "Boy"); break;
        case 8: return i18nc("The name of the female child voice", "Girl"); break;
    }
    return i18nc("Somehow user has gotten a voice type that is not valid, i.e. not Male1, Male2, etc.","Invalid voice type");
}

/*static*/ void TalkerCode::splitFullLanguageCode(const QString &lang, QString &languageCode, QString &countryCode)
{
    QString language = lang;
    if (language.left(1) == "*")
        language = language.mid(1);
    QString modifier;
    QString charSet;
    KGlobal::locale()->splitLocale(language, languageCode, countryCode, modifier, charSet);
}

/*static*/ QString TalkerCode::defaultTalkerCode(const QString &fullLanguageCode, const QString &moduleName)
{
    TalkerCode tmpTalkerCode;
    //tmpTalkerCode.setFullLanguageCode(fullLanguageCode);
    tmpTalkerCode.setOutputModule(moduleName);
    //tmpTalkerCode.normalize();
    return tmpTalkerCode.getTalkerCode();
}

/*static*/ QString TalkerCode::languageCodeToLanguage(const QString &languageCode)
{
    QString langAlpha;
    QString countryCode;
    QString language;
    if (languageCode == "other")
        language = i18nc("Other language", "Other");
    else
    {
        splitFullLanguageCode(languageCode, langAlpha, countryCode);
        language = KGlobal::locale()->languageCodeToName(langAlpha);
    }
    if (!countryCode.isEmpty())
    {
        QString countryName = KGlobal::locale()->countryCodeToName(countryCode);
        // Some abbreviations to save screen space.
        if (countryName == i18nc("full country name", "United States of America"))
            countryName = i18nc("abbreviated country name", "USA");
        if (countryName == i18nc("full country name", "United Kingdom"))
            countryName = i18nc("abbreviated country name", "UK");
        language += " (" + countryName + ')';
    }
    return language;
}

/**
 * Given a talker code, parses out the attributes.
 * @param talkerCode       The talker code.
 */
void TalkerCode::parseTalkerCode(const QString &talkerCode)
{
    QDomDocument doc;
    doc.setContent(talkerCode);
    
    QDomElement voice = doc.firstChildElement("voice");
    if (!voice.isNull())
    {
        m_name = voice.attribute("name");
        m_language = voice.attribute("lang");
        m_outputModule = voice.attribute("outputModule");
        bool result = false;
        m_voiceType = voice.attribute("voiceType").toInt(&result);
        if (!result)
            m_voiceType = 1;

        QDomElement prosody = voice.firstChildElement("prosody");
        if (!prosody.isNull())
        {
            bool result = false;
            m_volume = prosody.attribute("volume").toInt(&result);
            if (!result)
                m_volume = 0;
            m_rate = prosody.attribute("rate").toInt(&result);
            if (!result)
                m_rate = 0;
            m_pitch = prosody.attribute("pitch").toInt(&result);
            if (!result)
                m_pitch = 0;
        }
        else
        {
            kDebug() << "got a voice with no prosody tag";
        }
    }
    else
    {
        kDebug() << "got a voice with no voice tag";
    }
}

/**
 * Given a list of parsed talker codes and a desired talker code, finds the closest
 * matching talker in the list.
 * @param talkers                       The list of parsed talker codes.
 * @param talker                        The desired talker code.
 * @param assumeDefaultLang             If true, and desired talker code lacks a language code,
 *                                      the default language is assumed.
 * @return                              Index into talkers of the closest matching talker.
 */
/*static*/ int TalkerCode::findClosestMatchingTalker(
    const TalkerCodeList& talkers,
    const QString& talker,
    bool assumeDefaultLang)
{
    // kDebug() << "TalkerCode::findClosestMatchingTalker: matching on talker code " << talker;
    // If nothing to match on, winner is top in the list.
    if (talker.isEmpty()) return 0;
    // Parse the given talker.
    TalkerCode parsedTalkerCode(talker);
    // If no language code specified, use the language code of the default talker.
    if (assumeDefaultLang)
    {
        if (parsedTalkerCode.language().isEmpty()) parsedTalkerCode.setLanguage(
            talkers[0].language());
    }
    // The talker that matches on the most priority attributes wins.
    int talkersCount = int(talkers.count());
    QVector<int> priorityMatch(talkersCount);
    for (int ndx = 0; ndx < talkersCount; ++ndx)
    {
        priorityMatch[ndx] = 0;
        // kDebug() << "Comparing language code " << parsedTalkerCode.languageCode() << " to " << m_loadedPlugIns[ndx].parsedTalkerCode.languageCode();
    }
    // Determine the maximum number of priority attributes that were matched.
    int maxPriority = -1;
    for (int ndx = 0; ndx < talkersCount; ++ndx)
    {
        if (priorityMatch[ndx] > maxPriority) maxPriority = priorityMatch[ndx];
    }
    // Find the talker(s) that matched on most priority attributes.
    int winnerCount = 0;
    int winner = -1;
    for (int ndx = 0; ndx < talkersCount; ++ndx)
    {
        if (priorityMatch[ndx] == maxPriority)
        {
            ++winnerCount;
            winner = ndx;
        }
    }
    // kDebug() << "Priority phase: winnerCount = " << winnerCount 
    //     << " winner = " << winner
    //     << " maxPriority = " << maxPriority << endl;
    // If a tie, the one that matches on the most priority and preferred attributes wins.
    // If there is still a tie, the one nearest the top of the kttsmgr display
    // (first configured) will be chosen.
    if (winnerCount > 1)
    {
        QVector<int> preferredMatch(talkersCount);
        for (int ndx = 0; ndx < talkersCount; ++ndx)
        {
            preferredMatch[ndx] = 0;
            if (priorityMatch[ndx] == maxPriority)
            {
            }
        }
        // Determine the maximum number of preferred attributes that were matched.
        int maxPreferred = -1;
        for (int ndx = 0; ndx < talkersCount; ++ndx)
        {
            if (preferredMatch[ndx] > maxPreferred) maxPreferred = preferredMatch[ndx];
        }
        winner = -1;
        winnerCount = 0;
        // Find the talker that matched on most priority and preferred attributes.
        // Work bottom to top so topmost wins in a tie.
        for (int ndx = talkersCount-1; ndx >= 0; --ndx)
        {
            if (priorityMatch[ndx] == maxPriority)
            {
                if (preferredMatch[ndx] == maxPreferred)
                {
                    ++winnerCount;
                    winner = ndx;
                }
            }
        }
        // kDebug() << "Preferred phase: winnerCount = " << winnerCount 
        //     << " winner = " << winner
        //     << " maxPreferred = " << maxPreferred << endl;
    }
    // If no winner found, use the first talker.
    if (winner < 0) winner = 0;
    // kDebug() << "TalkerCode::findClosestMatchingTalker: returning winner = " << winner;
    return winner;
}

/*static*/ QString TalkerCode::stripPrefer( const QString& code, bool& preferred)
{
    if ( code.left(1) == "*" )
    {
        preferred = true;
        return code.mid(1);
    } else {
        preferred = false;
        return code;
    }
}
