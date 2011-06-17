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

#ifndef ADDTALKER_H
#define ADDTALKER_H

// TQt includes.
#include <tqmap.h>

#include "addtalkerwidget.h"

typedef TQMap<TQString,TQStringList> SynthToLangMap;
typedef TQMap<TQString,TQStringList> LangToSynthMap;

class AddTalker : public AddTalkerWidget
{
    Q_OBJECT
  TQ_OBJECT

public:
    /**
    * Constructor.
    * @param synthToLangMap     TQMap of supported language codes indexed by synthesizer.
    * @param tqparent             Inherited KDialog parameter.
    * @param name               Inherited KDialog parameter.
    */
    AddTalker(SynthToLangMap synthToLangMap, TQWidget* tqparent = 0, const char* name = 0, WFlags fl = 0 );

    /**
    * Destructor.
    */
    ~AddTalker();

    /**
    * Returns user's chosen language code.
    */
    TQString getLanguageCode();

    /**
    * Returns user's chosen synthesizer.
    */
    TQString getSynthesizer();


private:
    /**
    * Set the synthesizer-to-languages map.
    * @param synthToLang        TQMap of supported language codes indexed by synthesizer.
    */
    void setSynthToLangMap(SynthToLangMap synthToLangMap);

    // Converts a language code plus optional country code to language description.
    TQString languageCodeToLanguage(const TQString &languageCode);

    // TQMap of language descriptions to language codes.
    TQMap<TQString,TQString> m_languageToLanguageCodeMap;
    // TQMap of supported languages indexed by synthesizer.
    SynthToLangMap m_synthToLangMap;
    // TQMap of synthesizers indexed by language code they support.
    LangToSynthMap m_langToSynthMap;

private slots:
    // Based on user's radio button selection, filters choices for language or synthesizer
    // comboboxes based on what is selected in the other combobox.
    void applyFilter();
};

#endif

