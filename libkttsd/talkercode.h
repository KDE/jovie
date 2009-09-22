/***************************************************** vim:set ts=4 sw=4 sts=4:
  Object containing a Talker Code and providing convenience
  functions for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see speech.h. 
  -------------------
  Copyright: (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef TALKERCODE_H
#define TALKERCODE_H

// Qt includes.
#include <QtCore/QList>
#include <QtCore/QString>

// KDE includes.
#include <kdemacros.h>

class KDE_EXPORT TalkerCode
{
    public:
        /**
         * Constructor.
         */
        explicit TalkerCode(const QString &code=QString(), bool normal=false);
        /**
         * Copy Constructor.
         */
        explicit TalkerCode(TalkerCode* talker, bool normal=false);

        /**
         * Destructor.
         */
        ~TalkerCode();

        typedef QList<TalkerCode> TalkerCodeList;

        /**
         * Properties.
         */
        QString name() const;           /* name         */
        QString language() const;       /* lang="xx"    */
        int voiceType() const;          /* voiceType="xxx" equivalent to SPDVoiceType enumeration */
        int volume() const;             /* volume="xxx" */
        int rate() const;               /* rate="xxx"   */
        int pitch() const;              /* pitch="xxx"  */
        QString outputModule() const;         /* synthesizer="xxx" */

        /**
         * Returns the language code plus country code (if any).
         */
        QString fullLanguageCode() const;

        void setName(const QString& name);
        void setLanguage(const QString &language);
        void setVoiceType(int voiceType);
        void setVolume(int volume);
        void setRate(int rate);
        void setPitch(int pitch);
        void setOutputModule(const QString &moduleName);

        /**
         * Sets the language code and country code (if given).
         */
        void setFullLanguageCode(const QString &fullLanguageCode);

        /**
         * The Talker Code returned in XML format.
         */
        void setTalkerCode(const QString& code);
        QString getTalkerCode() const;

        /**
         * The Talker Code translated for display.
         */
        QString getTranslatedDescription() const;

        /**
         * Normalizes the Talker Code by filling in defaults.
         */
        //void normalize();

        /**
         * Given a talker code, normalizes it into a standard form and also returns
         * the full language code.
         * @param talkerCode         Unnormalized talker code.
         * @return fullLanguageCode  Language code from the talker code (including country code if any).
         * @return                   Normalized talker code.
         */
        //static QString normalizeTalkerCode(const QString &talkerCode, QString &fullLanguageCode);

        /**
         * Given a language code that might contain a country code, splits the code into
         * the two letter language code and country code.
         * @param fullLanguageCode   Language code to be split.
         * @return languageCode      Just the language part of the code.
         * @return countryCode       The country code part (if any).
         *
         * If the input code begins with an asterisk, it is ignored and removed from the returned
         * languageCode.
         */
        static void splitFullLanguageCode(const QString &lang, QString &languageCode, QString &countryCode);

        /**
         * Given a language code and plugin name, returns a normalized default talker code.
         * @param fullLanguageCode      Language code.
         * @param plugInName            Name of the Synthesizer plugin.
         * @return                      Full normalized talker code.
         *
         * Example returned from defaultTalkerCode("en", "Festival")
         *   <voice lang="en" name="fixed" gender="neutral"/>
         *   <prosody volume="medium" rate="medium"/>
         *   <kttsd synthesizer="Festival" />
         */
        static QString defaultTalkerCode(const QString &fullLanguageCode, const QString &plugInName);

        /**
         * Converts a language code plus optional country code to language description.
         */
        static QString languageCodeToLanguage(const QString &languageCode);

        /**
         * These functions return translated Talker Code attributes.
         */
        static QString translatedVoiceType(int voiceType);
        //static QString translatedVolume(const QString &volume);
        //static QString translatedRate(const QString &rate);
        //static QString untranslatedVolume(const QString &volume);
        //static QString untranslatedRate(const QString &rate);

        /**
         * Given a list of parsed talker codes and a desired talker code, finds the closest
         * matching talker in the list.
         * @param talkers                       The list of parsed talker codes.
         * @param talker                        The desired talker code.
         * @param assumeDefaultLang             If true, and desired talker code lacks a language code,
         *                                      the default language is assumed.
         * @return                              Index into talkers of the closest matching talker.
         */
        static int findClosestMatchingTalker(
            const TalkerCodeList& talkers,
            const QString& talker,
            bool assumeDefaultLang = true);

        /**
         * Strips leading * from a code.
         */
        static QString stripPrefer( const QString& code, bool& preferred);

        /**
        * Uses KTrader to convert a translated Synth Plugin Name to DesktopEntryName.
        * @param name                   The translated plugin name.  From Name= line in .desktop file.
        * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
        *                               QString() if not found.
        */
        //static QString TalkerNameToDesktopEntryName(const QString& name);

        /**
        * Uses KTrader to convert a DesktopEntryName into a translated Synth Plugin Name.
        * @param desktopEntryName       The DesktopEntryName.
        * @return                       The translated Name of the plugin, from Name= line in .desktop file.
        */
        //static QString TalkerDesktopEntryNameToName(const QString& desktopEntryName);

    private:
        /**
         * Given a talker code, parses out the attributes.
         * @param talkerCode       The talker code.
         */
        void parseTalkerCode(const QString &talkerCode);

        QString m_name;           /* name="xxx"        */
        QString m_language;       /* lang="xx"         */
        int m_voiceType;          /* voiceType="xxx"   */
        int m_volume;             /* volume="xxx"      */
        int m_rate;               /* rate="xxx"        */
        int m_pitch;              /* pitch="xxx"       */
        QString m_outputModule;   /* synthesizer="xxx" */
};

#endif      // TALKERCODE_H
