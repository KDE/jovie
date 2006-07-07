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

#ifndef _CONFIGDATA_H_
#define _CONFIGDATA_H_

// Qt includes.
#include <QString>
#include <QMap>

class KConfig;

/**
 * Struct used to keep notification options.
 */
struct NotifyOptions {
    QString eventName;
    int action;
    QString talker;
    QString customMsg;
};

/**
 * A list of notification options for a single app, indexed by event.
 */
typedef QMap<QString, NotifyOptions> NotifyEventMap;

/**
 * A list of notification event maps for all apps, indexed by app.
 */
typedef QMap<QString, NotifyEventMap> NotifyAppMap;

class ConfigData
{
public:
    ConfigData(KConfig* config);
    ~ConfigData();
    
    KConfig* config();
    
    /**
    * Text pre message
    */
    QString textPreMsg;

    /**
    * Text pre message enabled ?
    */
    bool textPreMsgEnabled;

    /**
    * Text pre sound
    */
    QString textPreSnd;

    /**
    * Text pre sound enabled ?
    */
    bool textPreSndEnabled;

    /**
    * Text post message
    */
    QString textPostMsg;

    /**
    * Text post message enabled ?
    */
    bool textPostMsgEnabled;

    /**
    * Text post sound
    */
    QString textPostSnd;

    /**
    * Text post sound enabled ?
    */
    bool textPostSndEnabled;

    /**
    * Paragraph pre message
    */
    QString parPreMsg;

    /**
    * Paragraph pre message enabled ?
    */
    bool parPreMsgEnabled;

    /**
    * Paragraph pre sound
    */
    QString parPreSnd;

    /**
    * Paragraph pre sound enabled ?
    */
    bool parPreSndEnabled;

    /**
    * Paragraph post message
    */
    QString parPostMsg;

    /**
    * Paragraph post message enabled ?
    */
    bool parPostMsgEnabled;

    /**
    * Paragraph post sound
    */
    QString parPostSnd;

    /**
    * Paragraph post sound enabled ?
    */
    bool parPostSndEnabled;

    /**
    * Keep audio files.  Do not delete generated tmp wav files.
    */
    bool keepAudio;
    QString keepAudioPath;

    /**
    * Notification settings.
    */
    bool notify;
    bool notifyExcludeEventsWithSound;
    NotifyAppMap notifyAppMap;
    int notifyDefaultPresent;
    NotifyOptions notifyDefaultOptions;

    /**
    * Automatically start KTTSMgr whenever speaking.
    */
    bool autoStartManager;

    /**
    * Automatically exit auto-started KTTSMgr when speaking finishes.
    */
    bool autoExitManager;
    
    /**
    * Which audio player to use.
    *  0 = aRts
    *  1 = gstreamer
    *  2 = ALSA
    */
    int playerOption;

    /**
    * Audio stretch factor (Speed).
    */
    float audioStretchFactor;

    /**
    * GStreamer sink name to use, or ALSA PCM device name.
    */
    QString sinkName;

    /**
    * Some parameters used by ALSA plugin.
    * Size of buffer interrupt period (in frames)
    * Number of periods in buffer.
    */
    uint periodSize;
    uint periods;

    /**
    * Debug level in players.
    */
    uint playerDebugLevel;
        
private:
    /**
    * Read the configuration
    */
    bool readConfig();
    
    /**
    * Read application notification options from file.
    */
    void loadNotifyEventsFromFile( const QString& filename, bool clear);
    
    KConfig* m_config;
};

#endif
