/***************************************************** vim:set ts=4 sw=4 sts=4:
  Object containing a Talker Code and providing convenience
  functions for manipulating Talker Codes.
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

#ifndef _TALKERCODE_H_
#define _TALKERCODE_H_

// Qt includes.
#include <qstring.h>
#include <kdemacros.h>
#include "kdeexportfix.h"

class KDE_EXPORT TalkerCode
{
    public:
        /**
         * Constructor.
         */
        TalkerCode(const QString &code=QString::null, bool normal=false);
        /**
         * Copy Constructor.
         */
        TalkerCode(TalkerCode* talker, bool normal=false);

        /**
         * Destructor.
         */
        ~TalkerCode();

        /**
         * Properties.
         */
        QString languageCode() const;       /* lang="xx" */
        QString countryCode() const;        /* lang="yy_xx */
        QString voice() const;              /* name="xxx" */
        QString gender() const;             /* gender="xxx" */
        QString volume() const;             /* volume="xxx" */
        QString rate() const;               /* rate="xxx" */
        QString plugInName() const;         /* synthesizer="xxx" */

        /**
         * Returns the language code plus country code (if any).
         */
        QString fullLanguageCode();

        void setLanguageCode(const QString &languageCode);
        void setCountryCode(const QString &countryCode);
        void setVoice(const QString &voice);
        void setGender(const QString &gender);
        void setVolume(const QString &volume);
        void setRate(const QString &rate);
        void setPlugInName(const QString plugInName);

        /**
         * Sets the language code and country code (if given).
         */
        void setFullLanguageCode(const QString &fullLanguageCode);

        /**
         * The Talker Code returned in XML format.
         */
        QString getTalkerCode();

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
        static QString normalizeTalkerCode(const QString &talkerCode, QString &fullLanguageCode);

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
        static QString translatedGender(const QString &gender);
        static QString translatedVolume(const QString &volume);
        static QString translatedRate(const QString &rate);
        static QString untranslatedGender(const QString &gender);
        static QString untranslatedVolume(const QString &volume);
        static QString untranslatedRate(const QString &rate);

    private:
        /**
         * Given a talker code, parses out the attributes.
         * @param talkerCode       The talker code.
         */
        void parseTalkerCode(const QString &talkerCode);

        QString m_languageCode;       /* lang="xx" */
        QString m_countryCode;        /* lang="yy_xx */
        QString m_voice;              /* name="xxx" */
        QString m_gender;             /* gender="xxx" */
        QString m_volume;             /* volume="xxx" */
        QString m_rate;               /* rate="xxx" */
        QString m_plugInName;         /* synthesizer="xxx" */
};

#endif      // _TALKERCODE_H_
