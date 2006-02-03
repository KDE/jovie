/***************************************************************************
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org

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

#ifndef PLAYER_H
#define PLAYER_H

// Qt includes.
#include <QObject>
#include <QStringList>
#include <QByteArray>

// KDE includes.
#include <kdemacros.h>

// #include "filehandle.h"

class KDE_EXPORT Player : virtual public QObject
{
    Q_OBJECT

public:
    virtual ~Player() {}

//    virtual void play(const FileHandle &file = FileHandle::null()) = 0;
    virtual void startPlay(const QString& file) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual void setVolume(float volume = 1.0) = 0;
    virtual float volume() const = 0;

    virtual bool playing() const = 0;
    virtual bool paused() const = 0;

    virtual int totalTime() const = 0;
    virtual int currentTime() const = 0;
    virtual int position() const = 0; // in this case not really the percent

    virtual void seek(int seekTime) = 0;
    virtual void seekPosition(int position) = 0;

    virtual QStringList getPluginList( const QByteArray& classname ) {
        Q_UNUSED(classname);
        return QStringList();
    }
    virtual void setSinkName(const QString &sinkName) { Q_UNUSED(sinkName); }
    virtual bool requireVersion(uint major, uint minor, uint micro) {
        Q_UNUSED(major);
        Q_UNUSED(minor);
        Q_UNUSED(micro);
        return true;
    }
    virtual void setDebugLevel(uint level) { Q_UNUSED(level); }
    virtual void setPeriodSize(uint periodSize) { Q_UNUSED(periodSize); }
    virtual void setPeriods(uint periods) {Q_UNUSED(periods); }

protected:
    Player(QObject* parent = 0, const char* name = 0, const QStringList& args=QStringList() ) :
        QObject(parent, name) { if (args.isEmpty()); } // TODO: Avoid compiler WARNING.  Better way?

};

#endif
