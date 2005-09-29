/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Epos plug in
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

#ifndef _EPOSCONF_H_
#define _EPOSCONF_H_

// Qt includes.
#include <qstring.h>

// KDE includes.
#include <kconfig.h>
#include <kdebug.h>
#include <kprogress.h>

// KTTS includes.
#include <pluginconf.h>

// Epos plugin includes.
#include "eposconfwidget.h"

class EposProc;
class KProgressDialog;

class EposConf : public PlugInConf {
    Q_OBJECT 

    public:
        /** Constructor */
        EposConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~EposConf();

        /** This method is invoked whenever the module should read its 
        * configuration (most of the times from a config file) and update the 
        * user interface. This happens when the user clicks the "Reset" button in 
        * the control center, to undo all of his changes and restore the currently 
        * valid settings. NOTE that this is not called after the modules is loaded,
        * so you probably want to call this method in the constructor.
        */
        void load(KConfig *config, const QString &configGroup);

        /** This function gets called when the user wants to save the settings in 
        * the user interface, updating the config files or wherever the 
        * configuration is stored. The method is called when the user clicks "Apply" 
        * or "Ok". 
        */
        void save(KConfig *config, const QString &configGroup);

        /** This function is called to set the settings in the module to sensible
        * default values. It gets called when hitting the "Default" button. The 
        * default values should probably be the same as the ones the application 
        * uses when started without a config file. 
        */
        void defaults();

        /**
        * This function informs the plugin of the desired language to be spoken
        * by the plugin.  The plugin should attempt to adapt itself to the
        * specified language code, choosing sensible defaults if necessary.
        * If the passed-in code is QString::null, no specific language has
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
        void setDesiredLanguage(const QString &lang);

        /**
        * Return fully-specified talker code for the configured plugin.  This code
        * uniquely identifies the configured instance of the plugin and distinquishes
        * one instance from another.  If the plugin has not been fully configured,
        * i.e., cannot yet synthesize, return QString::null.
        * @return            Fully-specified talker code.
        */
        QString getTalkerCode();

    private slots:
        void configChanged(){
            kdDebug() << "EposConf::configChanged: Running" << endl;
            emit changed(true);
        };
        void slotEposTest_clicked();
        void slotSynthFinished();
        void slotSynthStopped();
        void timeBox_valueChanged(int percentValue);
        void frequencyBox_valueChanged(int percentValue);
        void timeSlider_valueChanged(int sliderValue);
        void frequencySlider_valueChanged(int sliderValue);

    private:
        /**
        * Converts a language code into the language setting passed to Epos synth.
        */
        QString languageCodeToEposLanguage(const QString &languageCode);

        int percentToSlider(int percentValue);
        int sliderToPercent(int sliderValue);

        // Language code.
        QString m_languageCode;

        // Configuration widget.
        EposConfWidget* m_widget;

        // Epos synthesizer.
        EposProc* m_eposProc;
        // Synthesized wave file name.
        QString m_waveFile;
        // Progress dialog.
        KProgressDialog* m_progressDlg;
        // List of displayed codec names.
        QStringList m_codecList;
};
#endif // _EPOSCONF_H_
