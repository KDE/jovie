/*************************************************** vim:set ts=4 sw=4 sts=4:
  This class holds KTTS data from config file.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
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

// ConfigData includes.
#include "configdata.h"

// Qt includes.
#include <QtXml/QDomDocument>
#include <QtCore/QFile>

// KDE includes.
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "talkercode.h"

ConfigData::ConfigData(KConfig* config) : m_config(config)
{
    readConfig();
}

ConfigData::~ConfigData() { delete m_config; }

KConfig* ConfigData::config() { return m_config; }

bool ConfigData::readConfig()
{
    // Load configuration

    // Set the group general for the configuration of KTTSD itself (no plug ins)
    KConfigGroup config(m_config, "General");

    // Load the configuration of the text interruption messages and sound
    textPreMsgEnabled = config.readEntry("TextPreMsgEnabled", false);
    textPreMsg = config.readEntry("TextPreMsg");

    textPreSndEnabled = config.readEntry("TextPreSndEnabled", false);
    textPreSnd = config.readEntry("TextPreSnd");

    textPostMsgEnabled = config.readEntry("TextPostMsgEnabled", false);
    textPostMsg = config.readEntry("TextPostMsg");

    textPostSndEnabled = config.readEntry("TextPostSndEnabled", false);
    textPostSnd = config.readEntry("TextPostSnd");
    keepAudio = config.readEntry("KeepAudio", false);
    keepAudioPath = config.readEntry("KeepAudioPath",
        KStandardDirs::locateLocal("data", "kttsd/audio/"));


    // KTTSMgr auto start and auto exit.
    autoStartManager = config.readEntry("AutoStartManager", false);
    autoExitManager = config.readEntry("AutoExitManager", false);

    // Default to Phonon (0).
    playerOption = config.readEntry("AudioOutputMethod", 0);

    // Map 50% to 100% onto 2.0 to 0.5.
    audioStretchFactor = 1.0/(float(config.readEntry("AudioStretchFactor", 100))/100.0);
    switch (playerOption)
    {
        case 0:
        case 1: break;
        case 2:
            KConfigGroup alsaConfig( m_config, "ALSAPlayer");
            sinkName = alsaConfig.readEntry("PcmName", "default");
            if ("custom" == sinkName)
                sinkName = alsaConfig.readEntry("CustomPcmName", "default");
            periodSize = alsaConfig.readEntry("PeriodSize", 128);
            periods = alsaConfig.readEntry("Periods", 8);
            playerDebugLevel = alsaConfig.readEntry("DebugLevel", 1);
            break;
    }

    return true;
}

