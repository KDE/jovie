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

// Qt includes.
#include <QDomDocument>
#include <QFile>

// KDE includes.
#include <kconfig.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "talkercode.h"
#include "notify.h"

// ConfigData includes.
#include "configdata.h"

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

    // Notification (KNotify).
    notify = config.readEntry("Notify", false);
    notifyExcludeEventsWithSound = config.readEntry("ExcludeEventsWithSound", true);
    loadNotifyEventsFromFile(KStandardDirs::locateLocal("config", "kttsd_notifyevents.xml"), true );

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

void ConfigData::loadNotifyEventsFromFile( const QString& filename, bool clear)
{
    // Open existing event list.
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        kDebug() << "SpeechData::loadNotifyEventsFromFile: Unable to open file " << filename << endl;
        return;
    }
    // QDomDocument doc( "http://www.kde.org/share/apps/kttsd/stringreplacer/wordlist.dtd []" );
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        kDebug() << "SpeechData::loadNotifyEventsFromFile: File not in proper XML format. " << filename << endl;
    }
    // kDebug() << "StringReplacerConf::load: document successfully parsed." << endl;
    file.close();

    if ( clear ) {
        notifyDefaultPresent = NotifyPresent::Passive;
        notifyDefaultOptions.action = NotifyAction::SpeakMsg;
        notifyDefaultOptions.talker.clear();
        notifyDefaultOptions.customMsg.clear();
        notifyAppMap.clear();
    }

    // Event list.
    QDomNodeList eventList = doc.elementsByTagName("notifyEvent");
    const int eventListCount = eventList.count();
    for (int eventIndex = 0; eventIndex < eventListCount; ++eventIndex)
    {
        QDomNode eventNode = eventList.item(eventIndex);
        QDomNodeList propList = eventNode.childNodes();
        QString eventSrc;
        QString event;
        QString actionName;
        QString message;
        TalkerCode talkerCode;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex) {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "eventSrc") eventSrc = prop.text();
            if (prop.tagName() == "event") event = prop.text();
            if (prop.tagName() == "action") actionName = prop.text();
            if (prop.tagName() == "message") message = prop.text();
            if (prop.tagName() == "talker") talkerCode = TalkerCode(prop.text(), false);
        }
        NotifyOptions notifyOptions;
        notifyOptions.action = NotifyAction::action( actionName );
        notifyOptions.talker = talkerCode.getTalkerCode();
        notifyOptions.customMsg = message;
        if ( eventSrc != "default" ){
            notifyOptions.eventName = NotifyEvent::getEventName( eventSrc, event );
            NotifyEventMap notifyEventMap = notifyAppMap[ eventSrc ];
            notifyEventMap[ event ] = notifyOptions;
            notifyAppMap[ eventSrc ] = notifyEventMap;
        } else {
            notifyOptions.eventName.clear();
            notifyDefaultPresent = NotifyPresent::present( event );
            notifyDefaultOptions = notifyOptions;
        }
    }
}
