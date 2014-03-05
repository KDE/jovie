/***************************************************** vim:set ts=4 sw=4 sts=4:
  Convenience object for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see kspeech.h.
  -------------------
  Copyright 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2009 - 2010 by Jeremy Whiting <jpwhiting@kde.org>
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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kservicetypetrader.h>

class TalkerCodePrivate
{
public:
    TalkerCodePrivate(TalkerCode *parent)
        :q(parent)
    {
    }

    ~TalkerCodePrivate()
    {
    }
    
    QString name;           /* name="xxx"        */
    QString language;       /* lang="xx"         */
    int voiceType;          /* voiceType="xxx"   */
    int volume;             /* volume="xxx"      */
    int rate;               /* rate="xxx"        */
    int pitch;              /* pitch="xxx"       */
    QString voiceName;      /* voiceName="xxx"   */
    QString outputModule;   /* synthesizer="xxx" */

    TalkerCode *q;
};

/**
 * Constructor.
 */
TalkerCode::TalkerCode(const QString &code/*=QString()*/, bool normal /*=false*/)
:d(new TalkerCodePrivate(this))
{
    if (!code.isEmpty())
        parseTalkerCode(code);
    //if (normal)
    //    normalize();
}

/**
 * Copy Constructor.
 */
TalkerCode::TalkerCode(const TalkerCode& other)
:d(new TalkerCodePrivate(this))
{
    d->name = other.name();
    d->language = other.language();
    d->voiceType = other.voiceType();
    d->volume = other.volume();
    d->rate = other.rate();
    d->pitch = other.pitch();
    d->voiceName = other.voiceName();
    d->outputModule = other.outputModule();
}

/**
 * Destructor.
 */
TalkerCode::~TalkerCode()
{
    delete d;
}

TalkerCode &TalkerCode::operator=(const TalkerCode &other)
{
    d->name = other.name();
    d->language = other.language();
    d->voiceType = other.voiceType();
    d->volume = other.volume();
    d->rate = other.rate();
    d->pitch = other.pitch();
    d->voiceName = other.voiceName();
    d->outputModule = other.outputModule();
    return *this;
}

TalkerCode::TalkerCodeList TalkerCode::loadTalkerCodesFromConfig(KConfig* c)
{
    TalkerCodeList list;
    // Iterate through list of the TalkerCode IDs.
    KConfigGroup config(c, "General");
    QStringList talkerIDsList = config.readEntry("TalkerIDs", QStringList());
    // kDebug() << "TalkerCode::loadTalkerCodesFromConfig: talkerIDsList = " << talkerIDsList;
    if (!talkerIDsList.isEmpty())
    {
        QStringList::ConstIterator itEnd = talkerIDsList.constEnd();
        for (QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it)
        {
            QString talkerID = *it;
            kDebug() << "TalkerCode::loadTalkerCodesFromConfig: talkerID = " << talkerID;
            KConfigGroup talkGroup(c, "Talkers");
            QString talkerCode = talkGroup.readEntry(talkerID);
            TalkerCode tc(talkerCode, true);
            kDebug() << "TalkerCode::loadTalkerCodesFromConfig: talkerCode = " << talkerCode;
            list.append(tc);
        }
    }
    return list;
}

/**
 * Properties.
 */
QString TalkerCode::name() const
{
    return d->name;
}

QString TalkerCode::language() const
{
    return d->language;
}

int TalkerCode::voiceType() const
{
    return d->voiceType;
}

int TalkerCode::volume() const
{
    return d->volume;
}

int TalkerCode::rate() const
{
    return d->rate;
}

int TalkerCode::pitch() const
{
    return d->pitch;
}

QString TalkerCode::voiceName() const
{
    return d->voiceName;
}

QString TalkerCode::outputModule() const
{
    return d->outputModule;
}

void TalkerCode::setName(const QString &name)
{
    d->name = name;
}

void TalkerCode::setLanguage(const QString &language)
{
    d->language = language;
}

void TalkerCode::setVoiceType(int voiceType)
{
    d->voiceType = voiceType;
}

void TalkerCode::setVolume(int volume)
{
    d->volume = volume;
}

void TalkerCode::setRate(int rate)
{
    d->rate = rate;
}

void TalkerCode::setPitch(int pitch)
{
    d->pitch = pitch;
}

void TalkerCode::setVoiceName(const QString &voiceName)
{
    d->voiceName = voiceName;
}

void TalkerCode::setOutputModule(const QString &moduleName)
{
    d->outputModule = moduleName;
}

/**
 * The Talker Code returned in XML format.
 */
void TalkerCode::setTalkerCode(const QString& code)
{
    parseTalkerCode(code);
}

QString TalkerCode::getTalkerCode() const
{
    QString xml(QLatin1String("<voice name=\"%1\" lang=\"%2\" outputModule=\"%3\""
                              " voiceName=\"%4\" voiceType=\"%5\">"
                              "<prosody volume=\"%6\" rate=\"%7\" pitch=\"%8\" /></voice>"));
    QString code = xml.arg(d->name)
                   .arg(d->language)
                   .arg(d->outputModule)
                   .arg(d->voiceName)
                   .arg(d->voiceType)
                   .arg(d->volume)
                   .arg(d->rate)
                   .arg(d->pitch);
    return code;
}

/**
 * The Talker Code translated for display.
 */
QString TalkerCode::getTranslatedDescription() const
{
    QString code;
    if (!d->name.isEmpty())
    {
        code = d->name;
    }
    else
    {
        code = d->language;
        bool prefer;
        QString fullLangCode = d->language;
        if (!fullLangCode.isEmpty()) code = languageCodeToLanguage( fullLangCode );
        // TODO: The PlugInName is always English.  Need a way to convert this to a translated
        // name (possibly via DesktopEntryNameToName, but to do that, we need the desktopEntryName
        // from the config file).
        if (!d->outputModule.isEmpty()) code += QLatin1Char( ' ' ) + stripPrefer(d->outputModule, prefer);
        code += QLatin1Char(' ') + translatedVoiceType(d->voiceType);
        code += QString(QLatin1String(" volume: %1 rate: %2")).arg(d->volume).arg(d->rate);
        code = code.trimmed();
    }
    if (code.isEmpty())
        code = i18nc("Default language code", "default");
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
    if (language.left(1) == QLatin1String( "*" ))
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
    if (languageCode == QLatin1String( "other" ))
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
        language += QLatin1String( " (" ) + countryName + QLatin1Char( ')' );
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

    QDomElement voice = doc.firstChildElement(QLatin1String( "voice" ));
    if (!voice.isNull())
    {
        d->name = voice.attribute(QLatin1String( "name" ));
        d->language = voice.attribute(QLatin1String( "lang" ));
        d->outputModule = voice.attribute(QLatin1String( "outputModule" ));
        d->voiceName = voice.attribute(QLatin1String( "voiceName" ));
        bool result = false;
        d->voiceType = voice.attribute(QLatin1String( "voiceType" )).toInt(&result);
        if (!result)
            d->voiceType = 1;

        QDomElement prosody = voice.firstChildElement(QLatin1String( "prosody" ));
        if (!prosody.isNull())
        {
            bool result = false;
            d->volume = prosody.attribute(QLatin1String( "volume" )).toInt(&result);
            if (!result)
                d->volume = 0;
            d->rate = prosody.attribute(QLatin1String( "rate" )).toInt(&result);
            if (!result)
                d->rate = 0;
            d->pitch = prosody.attribute(QLatin1String( "pitch" )).toInt(&result);
            if (!result)
                d->pitch = 0;
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
        // kDebug() << "Comparing language code " << parsedTalkerCode.languageCode() << " to " << d->loadedPlugIns[ndx].parsedTalkerCode.languageCode();
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
    if ( code.left(1) == QLatin1String( "*" ) )
    {
        preferred = true;
        return code.mid(1);
    } else {
        preferred = false;
        return code;
    }
}

bool TalkerCode::operator==(TalkerCode &other) const
{
    return d->language == other.language() &&
           d->voiceType == other.voiceType() &&
           d->rate == other.rate() &&
           d->volume == other.volume() &&
           d->pitch == other.pitch() &&
           d->voiceName == other.voiceName() &&
           d->outputModule == other.outputModule();
}

bool TalkerCode::operator!=(TalkerCode &other) const
{
    return d->language != other.language() ||
           d->voiceType != other.voiceType() ||
           d->rate != other.rate() ||
           d->volume != other.volume() ||
           d->pitch != other.pitch() ||
           d->voiceName != other.voiceName() ||
           d->outputModule != other.outputModule();
}
