/***************************************************** vim:set ts=4 sw=4 sts=4:
  Object containing a Talker Code and providing convenience
  functions for manipulating Talker Codes.
  For an explanation of what a Talker Code is, see speech.h. 
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

#ifndef _TALKERCODE_H_
#define _TALKERCODE_H_

// Qt includes.
#include <tqstring.h>
#include <kdemacros.h>
#include "kdeexportfix.h"
#include <tqvaluelist.h>

class KDE_EXPORT TalkerCode
{
    public:
        /**
         * Constructor.
         */
        TalkerCode(const TQString &code=TQString::null, bool normal=false);
        /**
         * Copy Constructor.
         */
        TalkerCode(TalkerCode* talker, bool normal=false);

        /**
         * Destructor.
         */
        ~TalkerCode();

        typedef TQValueList<TalkerCode> TalkerCodeList;

        /**
         * Properties.
         */
        TQString languageCode() const;       /* lang="xx" */
        TQString countryCode() const;        /* lang="yy_xx */
        TQString voice() const;              /* name="xxx" */
        TQString gender() const;             /* gender="xxx" */
        TQString volume() const;             /* volume="xxx" */
        TQString rate() const;               /* rate="xxx" */
        TQString plugInName() const;         /* synthesizer="xxx" */

        /**
         * Returns the language code plus country code (if any).
         */
        TQString fullLanguageCode() const;

        void setLanguageCode(const TQString &languageCode);
        void setCountryCode(const TQString &countryCode);
        void setVoice(const TQString &voice);
        void setGender(const TQString &gender);
        void setVolume(const TQString &volume);
        void setRate(const TQString &rate);
        void setPlugInName(const TQString plugInName);

        /**
         * Sets the language code and country code (if given).
         */
        void setFullLanguageCode(const TQString &fullLanguageCode);

        /**
         * The Talker Code returned in XML format.
         */
        TQString getTalkerCode() const;

        /**
         * The Talker Code translated for display.
         */
        TQString getTranslatedDescription() const;

        /**
         * Normalizes the Talker Code by filling in defaults.
         */
        void normalize();

        /**
         * Given a talker code, normalizes it into a standard form and also returns
         * the full language code.
         * @param talkerCode         Unnormalized talker code.
         * @return fullLanguageCode  Language code from the talker code (including country code if any).
         * @return                   Normalized talker code.
         */
        static TQString normalizeTalkerCode(const TQString &talkerCode, TQString &fullLanguageCode);

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
        static void splitFullLanguageCode(const TQString &lang, TQString &languageCode, TQString &countryCode);

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
        static TQString defaultTalkerCode(const TQString &fullLanguageCode, const TQString &plugInName);

        /**
         * Converts a language code plus optional country code to language description.
         */
        static TQString languageCodeToLanguage(const TQString &languageCode);

        /**
         * These functions return translated Talker Code attributes.
         */
        static TQString translatedGender(const TQString &gender);
        static TQString translatedVolume(const TQString &volume);
        static TQString translatedRate(const TQString &rate);
        static TQString untranslatedGender(const TQString &gender);
        static TQString untranslatedVolume(const TQString &volume);
        static TQString untranslatedRate(const TQString &rate);

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
            const TQString& talker,
            bool assumeDefaultLang = true);

        /**
         * Strips leading * from a code.
         */
        static TQString stripPrefer( const TQString& code, bool& preferred);

        /**
        * Uses KTrader to convert a translated Synth Plugin Name to DesktopEntryName.
        * @param name                   The translated plugin name.  From Name= line in .desktop file.
        * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
        *                               TQString::null if not found.
        */
        static TQString TalkerNameToDesktopEntryName(const TQString& name);

        /**
        * Uses KTrader to convert a DesktopEntryName into a translated Synth Plugin Name.
        * @param desktopEntryName       The DesktopEntryName.
        * @return                       The translated Name of the plugin, from Name= line in .desktop file.
        */
        static TQString TalkerDesktopEntryNameToName(const TQString& desktopEntryName);

    private:
        /**
         * Given a talker code, parses out the attributes.
         * @param talkerCode       The talker code.
         */
        void parseTalkerCode(const TQString &talkerCode);

        TQString m_languageCode;       /* lang="xx" */
        TQString m_countryCode;        /* lang="yy_xx */
        TQString m_voice;              /* name="xxx" */
        TQString m_gender;             /* gender="xxx" */
        TQString m_volume;             /* volume="xxx" */
        TQString m_rate;               /* rate="xxx" */
        TQString m_plugInName;         /* synthesizer="xxx" */
};

#endif      // _TALKERCODE_H_
