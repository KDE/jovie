/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Festival (Interactive) plug in
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

#ifndef _FESTIVALINTCONF_H_
#define _FESTIVALINTCONF_H_

// Qt includes.
#include <tqstringlist.h>
#include <tqvaluelist.h>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>

// KTTS includes.
#include "pluginconf.h"

// FestivalInt includes.
#include "festivalintconfwidget.h"
#include "festivalintproc.h"

class KProgressDialog;
class QDomNode;

typedef struct voiceStruct{
    TQString code;               // Code as sent to Festival
    TQString name;               // Name as displayed and returned in Talker Code.
    TQString languageCode;       // Language code (en, es, etc)
    TQString codecName;          // Character encoding codec name (eg. ISO 8859-1)
    TQString gender;             // male, female, or neutral
    bool preload;               // Start Festival and load this language when KTTSD is started.
    bool volumeAdjustable;      // True if the voice supports volume adjustments.
    bool rateAdjustable;        // True if the voice supports rate adjustments.
    bool pitchAdjustable;       // True if the voice supports pitch adjustments.
} voice;

class FestivalIntConf : public PlugInConf {
    Q_OBJECT 

    public:
        /** Constructor */
        FestivalIntConf( TQWidget* parent = 0, const char* name = 0, const TQStringList &args = TQStringList());

        /** Destructor */
        ~FestivalIntConf();

        /** This method is invoked whenever the module should read its 
        *  configuration (most of the times from a config file) and update the 
        *  user interface. This happens when the user clicks the "Reset" button in 
        *  the control center, to undo all of his changes and restore the currently 
        *  valid settings. NOTE that this is not called after the modules is loaded,
        *  so you probably want to call this method in the constructor.
        */
        void load(KConfig *config, const TQString &configGroup);

        /** This function gets called when the user wants to save the settings in 
        *  the user interface, updating the config files or wherever the 
        *  configuration is stored. The method is called when the user clicks "Apply" 
        *  or "Ok". 
        */
        void save(KConfig *config, const TQString &configGroup);

        /** This function is called to set the settings in the module to sensible
        *  default values. It gets called when hitting the "Default" button. The 
        *  default values should probably be the same as the ones the application 
        *  uses when started without a config file. */
        void defaults();

        /**
        * This function informs the plugin of the desired language to be spoken
        * by the plugin.  The plugin should attempt to adapt itself to the
        * specified language code, choosing sensible defaults if necessary.
        * If the passed-in code is TQString::null, no specific language has
        * been chosen.
        * @param lang        The desired language code or Null if none.
        *
        * If the plugin is unable to support the desired language, that is OK.
        * Language codes are given by ISO 639-1 and are in lowercase.
        * The code may also include an ISO 3166 country code in uppercase
        * separated from the language code by underscore (_).  For
        * example, en_GB.  If your plugin supports the given language, but
        * not the given country, treat it as though the country
        * code were not specified, i.e., adapt to the given language.
        */
        void setDesiredLanguage(const TQString &lang);

        /**
        * Return fully-specified talker code for the configured plugin.  This code
        * uniquely identifies the configured instance of the plugin and distinquishes
        * one instance from another.  If the plugin has not been fully configured,
        * i.e., cannot yet synthesize, return TQString::null.
        * @return            Fully-specified talker code.
        */
        TQString getTalkerCode();

    private slots:
        /** Scan for the different voices in festivalPath/lib */
        void scanVoices();
        void configChanged(){
            // kdDebug() << "FestivalIntConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotTest_clicked();
        void slotSynthFinished();
        void slotSynthStopped();
        void volumeBox_valueChanged(int percentValue);
        void timeBox_valueChanged(int percentValue);
        void frequencyBox_valueChanged(int percentValue);
        void volumeSlider_valueChanged(int sliderValue);
        void timeSlider_valueChanged(int sliderValue);
        void frequencySlider_valueChanged(int sliderValue);
        void slotFestivalPath_textChanged();
        void slotSelectVoiceCombo_activated();
        void slotQueryVoicesFinished(const TQStringList &voiceCodes);

   private:
        int percentToSlider(int percentValue);
        int sliderToPercent(int sliderValue);

        /**
        * Given an XML node and child element name, returns the string value from the child element.
        * If no such child element, returns def.
        */
        TQString readXmlString(TQDomNode &node, const TQString &elementName, const TQString &def);

        /**
        * Given an XML node and child element name, returns the boolean value from the child element.
        * If no such child element, returns def.
        */
        bool readXmlBool(TQDomNode &node, const TQString &elementName, bool def);

        /**
        * Given a voice code, returns index into m_voiceList array (and voiceCombo box).
        * -1 if not found.
        */
        int voiceCodeToListIndex(const TQString& voiceCode) const;

        /**
        * Chooses a default voice given scanned list of voices in m_voiceList and current
        * language and country code, and updates controls.
        * @param currentVoiceIndex      This voice is preferred if it matches.
        */
        void setDefaultVoice(int currentVoiceIndex);

        // Configuration Widget.
        FestivalIntConfWidget* m_widget;

        // Language code.
        TQString m_languageCode;
        // Language country code (if any).
        TQString m_countryCode;
        // List of voices */
        TQValueList<voice> m_voiceList;
        // Festival synthesizer.
        FestivalIntProc* m_festProc;
        // Synthesized wave file name.
        TQString m_waveFile;
        // Progress dialog.
        KProgressDialog* m_progressDlg;
        // List of voice codes supported by Festival.
        TQStringList m_supportedVoiceCodes;
        // List of displayed codec names.
        TQStringList m_codecList;
        // Whether Festival supports SSML or not.
        FestivalIntProc::SupportsSSML m_supportsSSML;
};
#endif // _FESTIVALINTCONF_H_
