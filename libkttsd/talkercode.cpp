/***************************************************** vim:set ts=4 sw=4 sts=4:
  Convenience object for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see speech.h. 
  -------------------
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// KDE includes.
#include <kglobal.h>
#include <klocale.h>

// TalkerCode includes.
#include "talkercode.h"

/**
 * Constructor.
 */
TalkerCode::TalkerCode(const QString &code/*=QString::null*/, bool normal /*=false*/)
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
QString TalkerCode::languageCode() { return m_languageCode; }
QString TalkerCode::countryCode() { return m_countryCode; }
QString TalkerCode::voice() { return m_voice; }
QString TalkerCode::gender() { return m_gender; }
QString TalkerCode::volume() { return m_volume; }
QString TalkerCode::rate() { return m_rate; }
QString TalkerCode::plugInName() { return m_plugInName; }

void TalkerCode::setLanguageCode(const QString &languageCode) { m_languageCode = languageCode; }
void TalkerCode::setCountryCode(const QString &countryCode) { m_countryCode = countryCode; }
void TalkerCode::setVoice(const QString &voice) { m_voice = voice; }
void TalkerCode::setGender(const QString &gender) { m_gender = gender; }
void TalkerCode::setVolume(const QString &volume) { m_volume = volume; }
void TalkerCode::setRate(const QString &rate) { m_rate = rate; }
void TalkerCode::setPlugInName(const QString plugInName) { m_plugInName = plugInName; }

/**
 * Sets the language code and country code (if given).
 */
void TalkerCode::setFullLanguageCode(const QString &fullLanguageCode)
{
    splitFullLanguageCode(fullLanguageCode, m_languageCode, m_countryCode);
}

/**
 * Returns the language code plus country code (if any).
 */
QString TalkerCode::fullLanguageCode()
{
    if (!m_countryCode.isEmpty())
        return m_languageCode + "_" + m_countryCode;
    else
        return m_languageCode;
}

/**
 * The Talker Code returned in XML format.
 */
QString TalkerCode::getTalkerCode()
{
    QString languageCode = m_languageCode;
    if (!m_countryCode.isEmpty()) languageCode += "_" + m_countryCode;
    QString code = "<voice lang=\"" + languageCode + "\" ";
    if (!m_voice.isEmpty()) code += "name=\"" + m_voice + "\" ";
    if (!m_gender.isEmpty()) code += "gender=\"" + m_gender + "\" ";
    code += "/>";
    QString prosody;
    if (!m_volume.isEmpty()) prosody = "volume=\"" + m_volume + "\" ";
    if (!m_rate.isEmpty()) prosody += "rate=\"" + m_rate + "\" ";
    if (!prosody.isEmpty()) code += "<prosody " + prosody + "/>";
    if (!m_plugInName.isEmpty()) code += "<kttsd synthesizer=\"" + m_plugInName + "\" />";
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
/*static*/ QString TalkerCode::normalizeTalkerCode(const QString &talkerCode, QString &fullLanguageCode)
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
/*static*/ void TalkerCode::splitFullLanguageCode(const QString &lang, QString &languageCode, QString &countryCode)
{
    QString language = lang;
    if (language.left(1) == "*") language = language.mid(1);
    QString charSet;
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
/*static*/ QString TalkerCode::defaultTalkerCode(const QString &fullLanguageCode, const QString &plugInName)
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
/*static*/ QString TalkerCode::languageCodeToLanguage(const QString &languageCode)
{
    QString twoAlpha;
    QString countryCode;
    QString language;
    if (languageCode == "other")
        language = i18n("Other");
    else
    {
        splitFullLanguageCode(languageCode, twoAlpha, countryCode);
        language = KGlobal::locale()->twoAlphaToLanguageName(twoAlpha);
    }
    if (!countryCode.isEmpty())
        language += " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode) + ")";
    return language;
}

/**
 * These functions return translated Talker Code attributes.
 */
/*static*/ QString TalkerCode::translatedGender(const QString &gender)
{
    if (gender == "male")
        return i18n("male");
    else if (gender == "female")
        return i18n("female");
    else if (gender == "neutral")
        return i18n("neutral gender", "neutral");
    else return gender;
}
/*static*/ QString TalkerCode::translatedVolume(const QString &volume)
{
    if (volume == "medium")
        return i18n("medium sound", "medium");
    else if (volume == "loud")
        return i18n("loud sound", "loud");
    else if (volume == "soft")
        return i18n("soft sound", "soft");
    else return volume;
}
/*static*/ QString TalkerCode::translatedRate(const QString &rate)
{
    if (rate == "medium")
        return i18n("medium speed", "medium");
    else if (rate == "fast")
        return i18n("fast speed", "fast");
    else if (rate == "slow")
        return i18n("slow speed", "slow");
    else return rate;
}

/**
 * Given a talker code, parses out the attributes.
 * @param talkerCode       The talker code.
 */
void TalkerCode::parseTalkerCode(const QString &talkerCode)
{
    QString fullLanguageCode;
    if (talkerCode.contains("\""))
    {
        fullLanguageCode = talkerCode.section("lang=", 1, 1);
        fullLanguageCode = fullLanguageCode.section('"', 1, 1);
    }
    else
        fullLanguageCode = talkerCode;
    QString languageCode;
    QString countryCode;
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

