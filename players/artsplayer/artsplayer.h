/***************************************************************************
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org

    copyright            : (C) 2003 by Matthias Kretz
    email                : kretz@kde.org

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

#ifndef ARTSPLAYER_H
#define ARTSPLAYER_H

// Qt includes.
#include <QObject>
#include <QString>
#include <QByteArray>

// KDE includes.
#include <config.h>
#include <kdemacros.h>
#include <kurl.h>
#include <artsflow.h>

// ArtsPlayer includes.
#include "player.h"

class KArtsDispatcher;
class KArtsServer;
class KAudioManagerPlay;

namespace KDE {
    class PlayObjectFactory;
    class PlayObject;
}

class KDE_EXPORT ArtsPlayer : public Player
{
    Q_OBJECT

public:
    ArtsPlayer(QObject* parent = 0, const char* name = 0, const QStringList& args=QStringList());
    ~ArtsPlayer();

//    virtual void play(const FileHandle &file = FileHandle::null());
    virtual void startPlay(const QString& file);
    virtual void pause();
    virtual void stop();

    virtual void setVolume(float volume = 1.0);
    virtual float volume() const;

    virtual bool playing() const;
    virtual bool paused() const;

    virtual int totalTime() const;
    virtual int currentTime() const;
    virtual int position() const; // in this case not really the percent

    virtual void seek(int seekTime);
    virtual void seekPosition(int position);

private slots:
    void setupArtsObjects();
    void playObjectCreated();

private:
    void setupPlayer();
    void setupVolumeControl();
    bool serverRunning() const;

    KArtsDispatcher *m_dispatcher;
    KArtsServer *m_server;
    KDE::PlayObjectFactory *m_factory;
    KDE::PlayObject *m_playobject;
    KAudioManagerPlay *m_amanPlay;

    // This is a pretty heavy module for the needs that JuK has, it would probably
    // be good to use two Synth_MUL instead or the one from Noatun.

    Arts::StereoVolumeControl m_volumeControl;

    KUrl m_currentURL;
    float m_currentVolume;
};

#endif

// vim: sw=4 ts=8 et
