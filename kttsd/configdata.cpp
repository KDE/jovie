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
    m_config->setGroup("General");

    // Load the configuration of the text interruption messages and sound
    textPreMsgEnabled = m_config->readEntry("TextPreMsgEnabled", false);
    textPreMsg = m_config->readEntry("TextPreMsg");

    textPreSndEnabled = m_config->readEntry("TextPreSndEnabled", false);
    textPreSnd = m_config->readEntry("TextPreSnd");

    textPostMsgEnabled = m_config->readEntry("TextPostMsgEnabled", false);
    textPostMsg = m_config->readEntry("TextPostMsg");

    textPostSndEnabled = m_config->readEntry("TextPostSndEnabled", false);
    textPostSnd = m_config->readEntry("TextPostSnd");
    keepAudio = m_config->readEntry("KeepAudio", false);
    keepAudioPath = m_config->readEntry("KeepAudioPath",
        KStandardDirs::locateLocal("data", "kttsd/audio/"));

    // Notification (KNotify).
    notify = m_config->readEntry("Notify", false);
    notifyExcludeEventsWithSound = m_config->readEntry("ExcludeEventsWithSound", true);
    loadNotifyEventsFromFile(KStandardDirs::locateLocal("config", "kttsd_notifyevents.xml"), true );

    // KTTSMgr auto start and auto exit.
    autoStartManager = m_config->readEntry("AutoStartManager", false);
    autoExitManager = m_config->readEntry("AutoExitManager", false);

    // Default to Phonon (0).
    playerOption = m_config->readEntry("AudioOutputMethod", 0);
    
    // Map 50% to 100% onto 2.0 to 0.5.
    audioStretchFactor = 1.0/(float(m_config->readEntry("AudioStretchFactor", 100))/100.0);
    switch (playerOption)
    {
        case 0:
        case 1: break;
        case 2:
            m_config->setGroup("ALSAPlayer");
            sinkName = m_config->readEntry("PcmName", "default");
            if ("custom" == sinkName)
                sinkName = m_config->readEntry("CustomPcmName", "default");
            periodSize = m_config->readEntry("PeriodSize", 128);
            periods = m_config->readEntry("Periods", 8);
            playerDebugLevel = m_config->readEntry("DebugLevel", 1);
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
