/***************************************************** vim:set ts=4 sw=4 sts=4:
  Convenience object for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see kspeech.h. 
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

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <ktrader.h>
#include <kdebug.h>

// TalkerCode includes.
#include "talkercode.h"

/**
 * Constructor.
 */
TalkerCode::TalkerCode(const TQString &code/*=TQString::null*/, bool normal /*=false*/)
{
    if (!code.isEmpty())
        parseTalkerCode(code);
    if (normal) normalize();
}

/**
 * Copy Constructor.
 */
TalkerCode::TalkerCode(TalkerCode* talker, bool normal /*=false*/)
{
    m_languageCode = talker->languageCode();
    m_countryCode = talker->countryCode();
    m_voice = talker->voice();
    m_gender = talker->gender();
    m_volume = talker->volume();
    m_rate = talker->rate();
    m_plugInName = talker->plugInName();
    if (normal) normalize();
}

/**
 * Destructor.
 */
TalkerCode::~TalkerCode() { }

/**
 * Properties.
 */
TQString TalkerCode::languageCode() const { return m_languageCode; }
TQString TalkerCode::countryCode() const { return m_countryCode; }
TQString TalkerCode::voice() const { return m_voice; }
TQString TalkerCode::gender() const { return m_gender; }
TQString TalkerCode::volume() const { return m_volume; }
TQString TalkerCode::rate() const { return m_rate; }
TQString TalkerCode::plugInName() const { return m_plugInName; }

void TalkerCode::setLanguageCode(const TQString &languageCode) { m_languageCode = languageCode; }
void TalkerCode::setCountryCode(const TQString &countryCode) { m_countryCode = countryCode; }
void TalkerCode::setVoice(const TQString &voice) { m_voice = voice; }
void TalkerCode::setGender(const TQString &gender) { m_gender = gender; }
void TalkerCode::setVolume(const TQString &volume) { m_volume = volume; }
void TalkerCode::setRate(const TQString &rate) { m_rate = rate; }
void TalkerCode::setPlugInName(const TQString plugInName) { m_plugInName = plugInName; }

/**
 * Sets the language code and country code (if given).
 */
void TalkerCode::setFullLanguageCode(const TQString &fullLanguageCode)
{
    splitFullLanguageCode(fullLanguageCode, m_languageCode, m_countryCode);
}

/**
 * Returns the language code plus country code (if any).
 */
TQString TalkerCode::fullLanguageCode() const
{
    if (!m_countryCode.isEmpty())
        return m_languageCode + "_" + m_countryCode;
    else
        return m_languageCode;
}

/**
 * The Talker Code returned in XML format.
 */
TQString TalkerCode::getTalkerCode() const
{
    TQString code;
    TQString languageCode = m_languageCode;
    if (!m_countryCode.isEmpty()) languageCode += "_" + m_countryCode;
    if (!languageCode.isEmpty()) code = "lang=\"" + languageCode + "\" ";
    if (!m_voice.isEmpty()) code += "name=\"" + m_voice + "\" ";
    if (!m_gender.isEmpty()) code += "gender=\"" + m_gender + "\" ";
    if (!code.isEmpty()) code = "<voice " + code + "/>";
    TQString prosody;
    if (!m_volume.isEmpty()) prosody = "volume=\"" + m_volume + "\" ";
    if (!m_rate.isEmpty()) prosody += "rate=\"" + m_rate + "\" ";
    if (!prosody.isEmpty()) code += "<prosody " + prosody + "/>";
    if (!m_plugInName.isEmpty()) code += "<kttsd synthesizer=\"" + m_plugInName + "\" />";
    return code;
}

/**
 * The Talker Code translated for display.
 */
TQString TalkerCode::getTranslatedDescription() const
{
    TQString code;
    bool prefer;
    TQString fullLangCode = fullLanguageCode();
    if (!fullLangCode.isEmpty()) code = languageCodeToLanguage( fullLangCode );
    // TODO: The PlugInName is always English.  Need a way to convert this to a translated
    // name (possibly via DesktopEntryNameToName, but to do that, we need the desktopEntryName
    // from the config file).
    if (!m_plugInName.isEmpty()) code += " " + stripPrefer(m_plugInName, prefer);
    if (!m_voice.isEmpty()) code += " " + stripPrefer(m_voice, prefer);
    if (!m_gender.isEmpty()) code += " " + translatedGender(stripPrefer(m_gender, prefer));
    if (!m_volume.isEmpty()) code += " " + translatedVolume(stripPrefer(m_volume, prefer));
    if (!m_rate.isEmpty()) code += " " + translatedRate(stripPrefer(m_rate, prefer));
    code = code.stripWhiteSpace();
    if (code.isEmpty()) code = i18n("default");
    return code;
}

/**
 * Normalizes the Talker Code by filling in defaults.
 */
void TalkerCode::normalize()
{
    if (m_voice.isEmpty()) m_voice = "fixed";
    if (m_gender.isEmpty()) m_gender = "neutral";
    if (m_volume.isEmpty()) m_volume = "medium";
    if (m_rate.isEmpty()) m_rate = "medium";
}

/**
 * Given a talker code, normalizes it into a standard form and also returns
 * the language code.
 * @param talkerCode            Unnormalized talker code.
 * @return fullLanguageCode     Language code from the talker code (including country code if any).
 * @return                      Normalized talker code.
 */
/*static*/ TQString TalkerCode::normalizeTalkerCode(const TQString &talkerCode, TQString &fullLanguageCode)
{
    TalkerCode tmpTalkerCode(talkerCode);
    tmpTalkerCode.normalize();
    fullLanguageCode = tmpTalkerCode.fullLanguageCode();
    return tmpTalkerCode.getTalkerCode();
}

/**
 * Given a language code that might contain a country code, splits the code into
 * the two letter language code and country code.
 * @param fullLanguageCode     Language code to be split.
 * @return languageCode        Just the language part of the code.
 * @return countryCode         The country code part (if any).
 *
 * If the input code begins with an asterisk, it is ignored and removed from the returned
 * languageCode.
 */
/*static*/ void TalkerCode::splitFullLanguageCode(const TQString &lang, TQString &languageCode, TQString &countryCode)
{
    TQString language = lang;
    if (language.left(1) == "*") language = language.mid(1);
    TQString charSet;
    KGlobal::locale()->splitLocale(language, languageCode, countryCode, charSet);
}

/**
 * Given a full language code and plugin name, returns a normalized default talker code.
 * @param fullLanguageCode      Language code.
 * @param plugInName            Name of the Synthesizer plugin.
 * @return                      Full normalized talker code.
 *
 * Example returned from defaultTalkerCode("en", "Festival")
 *   <voice lang="en" name="fixed" gender="neutral"/>
 *   <prosody volume="medium" rate="medium"/>
 *   <kttsd synthesizer="Festival" />
 */
/*static*/ TQString TalkerCode::defaultTalkerCode(const TQString &fullLanguageCode, const TQString &plugInName)
{
    TalkerCode tmpTalkerCode;
    tmpTalkerCode.setFullLanguageCode(fullLanguageCode);
    tmpTalkerCode.setPlugInName(plugInName);
    tmpTalkerCode.normalize();
    return tmpTalkerCode.getTalkerCode();
}

/**
 * Converts a language code plus optional country code to language description.
 */
/*static*/ TQString TalkerCode::languageCodeToLanguage(const TQString &languageCode)
{
    TQString twoAlpha;
    TQString countryCode;
    TQString language;
    if (languageCode == "other")
        language = i18n("Other");
    else
    {
        splitFullLanguageCode(languageCode, twoAlpha, countryCode);
        language = KGlobal::locale()->twoAlphaToLanguageName(twoAlpha);
    }
    if (!countryCode.isEmpty())
    {
        TQString countryName = KGlobal::locale()->twoAlphaToCountryName(countryCode);
        // Some abbreviations to save screen space.
        if (countryName == i18n("full country name", "United States of America"))
            countryName = i18n("abbreviated country name", "USA");
        if (countryName == i18n("full country name", "United Kingdom"))
            countryName = i18n("abbreviated country name", "UK");
        language += " (" + countryName + ")";
    }
    return language;
}

/**
 * These functions return translated Talker Code attributes.
 */
/*static*/ TQString TalkerCode::translatedGender(const TQString &gender)
{
    if (gender == "male")
        return i18n("male");
    else if (gender == "female")
        return i18n("female");
    else if (gender == "neutral")
        return i18n("neutral gender", "neutral");
    else return gender;
}
/*static*/ TQString TalkerCode::untranslatedGender(const TQString &gender)
{
    if (gender == i18n("male"))
        return "male";
    else if (gender == i18n("female"))
        return "female";
    else if (gender == i18n("neutral gender", "neutral"))
        return "neutral";
    else return gender;
}
/*static*/ TQString TalkerCode::translatedVolume(const TQString &volume)
{
    if (volume == "medium")
        return i18n("medium sound", "medium");
    else if (volume == "loud")
        return i18n("loud sound", "loud");
    else if (volume == "soft")
        return i18n("soft sound", "soft");
    else return volume;
}
/*static*/ TQString TalkerCode::untranslatedVolume(const TQString &volume)
{
    if (volume == i18n("medium sound", "medium"))
        return "medium";
    else if (volume == i18n("loud sound", "loud"))
        return "loud";
    else if (volume == i18n("soft sound", "soft"))
        return "soft";
    else return volume;
}
/*static*/ TQString TalkerCode::translatedRate(const TQString &rate)
{
    if (rate == "medium")
        return i18n("medium speed", "medium");
    else if (rate == "fast")
        return i18n("fast speed", "fast");
    else if (rate == "slow")
        return i18n("slow speed", "slow");
    else return rate;
}
/*static*/ TQString TalkerCode::untranslatedRate(const TQString &rate)
{
    if (rate == i18n("medium speed", "medium"))
        return "medium";
    else if (rate == i18n("fast speed", "fast"))
        return "fast";
    else if (rate == i18n("slow speed", "slow"))
        return "slow";
    else return rate;
}

/**
 * Given a talker code, parses out the attributes.
 * @param talkerCode       The talker code.
 */
void TalkerCode::parseTalkerCode(const TQString &talkerCode)
{
    TQString fullLanguageCode;
    if (talkerCode.contains("\""))
    {
        fullLanguageCode = talkerCode.section("lang=", 1, 1);
        fullLanguageCode = fullLanguageCode.section('"', 1, 1);
    }
    else
        fullLanguageCode = talkerCode;
    TQString languageCode;
    TQString countryCode;
    splitFullLanguageCode(fullLanguageCode, languageCode, countryCode);
    m_languageCode = languageCode;
    if (fullLanguageCode.left(1) == "*") countryCode = "*" + countryCode;
    m_countryCode = countryCode;
    m_voice = talkerCode.section("name=", 1, 1);
    m_voice = m_voice.section('"', 1, 1);
    m_gender = talkerCode.section("gender=", 1, 1);
    m_gender = m_gender.section('"', 1, 1);
    m_volume = talkerCode.section("volume=", 1, 1);
    m_volume = m_volume.section('"', 1, 1);
    m_rate = talkerCode.section("rate=", 1, 1);
    m_rate = m_rate.section('"', 1, 1);
    m_plugInName = talkerCode.section("synthesizer=", 1, 1);
    m_plugInName = m_plugInName.section('"', 1, 1);
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
    const TQString& talker,
    bool assumeDefaultLang)
{
    // kdDebug() << "TalkerCode::findClosestMatchingTalker: matching on talker code " << talker << endl;
    // If nothing to match on, winner is top in the list.
    if (talker.isEmpty()) return 0;
    // Parse the given talker.
    TalkerCode parsedTalkerCode(talker);
    // If no language code specified, use the language code of the default talker.
    if (assumeDefaultLang)
    {
        if (parsedTalkerCode.languageCode().isEmpty()) parsedTalkerCode.setLanguageCode(
            talkers[0].languageCode());
    }
    // The talker that matches on the most priority attributes wins.
    int talkersCount = int(talkers.count());
    TQMemArray<int> priorityMatch(talkersCount);
    for (int ndx = 0; ndx < talkersCount; ++ndx)
    {
        priorityMatch[ndx] = 0;
        // kdDebug() << "Comparing language code " << parsedTalkerCode.languageCode() << " to " << m_loadedPlugIns[ndx].parsedTalkerCode.languageCode() << endl;
        if (parsedTalkerCode.languageCode() == talkers[ndx].languageCode())
        {
            ++priorityMatch[ndx];
            // kdDebug() << "TalkerCode::findClosestMatchingTalker: Match on language " << parsedTalkerCode.languageCode() << endl;
        }
        if (parsedTalkerCode.countryCode().left(1) == "*")
            if (parsedTalkerCode.countryCode().mid(1) ==
                talkers[ndx].countryCode())
                ++priorityMatch[ndx];
        if (parsedTalkerCode.voice().left(1) == "*")
            if (parsedTalkerCode.voice().mid(1) == talkers[ndx].voice())
                ++priorityMatch[ndx];
        if (parsedTalkerCode.gender().left(1) == "*")
            if (parsedTalkerCode.gender().mid(1) == talkers[ndx].gender())
                ++priorityMatch[ndx];
        if (parsedTalkerCode.volume().left(1) == "*")
            if (parsedTalkerCode.volume().mid(1) == talkers[ndx].volume())
                ++priorityMatch[ndx];
        if (parsedTalkerCode.rate().left(1) == "*")
            if (parsedTalkerCode.rate().mid(1) == talkers[ndx].rate())
                ++priorityMatch[ndx];
        if (parsedTalkerCode.plugInName().left(1) == "*")
            if (parsedTalkerCode.plugInName().mid(1) ==
                talkers[ndx].plugInName())
                ++priorityMatch[ndx];
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
    // kdDebug() << "Priority phase: winnerCount = " << winnerCount 
    //     << " winner = " << winner
    //     << " maxPriority = " << maxPriority << endl;
    // If a tie, the one that matches on the most priority and preferred attributes wins.
    // If there is still a tie, the one nearest the top of the kttsmgr display
    // (first configured) will be chosen.
    if (winnerCount > 1)
    {
        TQMemArray<int> preferredMatch(talkersCount);
        for (int ndx = 0; ndx < talkersCount; ++ndx)
        {
            preferredMatch[ndx] = 0;
            if (priorityMatch[ndx] == maxPriority)
            {
                if (parsedTalkerCode.countryCode().left(1) != "*")
                    if (!talkers[ndx].countryCode().isEmpty())
                        if (parsedTalkerCode.countryCode() == talkers[ndx].countryCode())
                            ++preferredMatch[ndx];
                if (parsedTalkerCode.voice().left(1) != "*")
                    if (parsedTalkerCode.voice() == talkers[ndx].voice())
                        ++preferredMatch[ndx];
                if (parsedTalkerCode.gender().left(1) != "*")
                    if (parsedTalkerCode.gender() == talkers[ndx].gender())
                        ++preferredMatch[ndx];
                if (parsedTalkerCode.volume().left(1) != "*")
                    if (parsedTalkerCode.volume() == talkers[ndx].volume())
                        ++preferredMatch[ndx];
                if (parsedTalkerCode.rate().left(1) != "*")
                    if (parsedTalkerCode.rate() == talkers[ndx].rate())
                        ++preferredMatch[ndx];
                if (parsedTalkerCode.plugInName().left(1) != "*")
                    if (parsedTalkerCode.plugInName() ==
                        talkers[ndx].plugInName())
                        ++preferredMatch[ndx];
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
        // kdDebug() << "Preferred phase: winnerCount = " << winnerCount 
        //     << " winner = " << winner
        //     << " maxPreferred = " << maxPreferred << endl;
    }
    // If no winner found, use the first talker.
    if (winner < 0) winner = 0;
    // kdDebug() << "TalkerCode::findClosestMatchingTalker: returning winner = " << winner << endl;
    return winner;
}

/*static*/ TQString TalkerCode::stripPrefer( const TQString& code, bool& preferred)
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

/**
* Uses KTrader to convert a translated Synth Plugin Name to DesktopEntryName.
* @param name                   The translated plugin name.  From Name= line in .desktop file.
* @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
*                               TQString::null if not found.
*/
/*static*/ TQString TalkerCode::TalkerNameToDesktopEntryName(const TQString& name)
{
    if (name.isEmpty()) return TQString::null;
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");
    for (uint ndx = 0; ndx < offers.count(); ++ndx)
        if (offers[ndx]->name() == name) return offers[ndx]->desktopEntryName();
    return TQString::null;
}

/**
* Uses KTrader to convert a DesktopEntryName into a translated Synth Plugin Name.
* @param desktopEntryName       The DesktopEntryName.
* @return                       The translated Name of the plugin, from Name= line in .desktop file.
*/
/*static*/ TQString TalkerCode::TalkerDesktopEntryNameToName(const TQString& desktopEntryName)
{
    if (desktopEntryName.isEmpty()) return TQString::null;
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin",
    TQString("DesktopEntryName == '%1'").arg(desktopEntryName));

    if (offers.count() == 1)
        return offers[0]->name();
    else
        return TQString::null;
}

