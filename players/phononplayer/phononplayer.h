/***************************************************** vim:set ts=4 sw=4 sts=4:
  Phonon player plugin for KTTS.
  ------------------------------
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

#ifndef PHONONPLAYER_H
#define PHONONPLAYER_H

// Qt includes.
#include <QObject>
#include <QStringList>

// KDE includes.
#include <kdemacros.h>

// KTTS includes.
#include "player.h"

namespace Phonon {
class SimplePlayer;
}

class KDE_EXPORT PhononPlayer : virtual public Player
{
    Q_OBJECT

public:
    PhononPlayer(QObject* parent = 0, const QStringList& args=QStringList() );
    virtual ~PhononPlayer();

    virtual void startPlay(const QString& file);
    virtual void pause();
    virtual void stop();

    virtual void setVolume(float volume = 1.0);
    virtual float volume() const;

    virtual bool playing() const;
    virtual bool paused() const;

    virtual int totalTime() const;
    virtual int currentTime() const;
    virtual int position() const;

    virtual void seek(int seekTime);
    virtual void seekPosition(int position);

protected:
    Phonon::SimplePlayer* m_simplePlayer;
};

#endif      // PHONONPLAYER_H
