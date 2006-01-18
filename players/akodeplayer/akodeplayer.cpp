/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include <kdebug.h>

#include <qfile.h>

#include "akode/player.h"
#include "akode/decoder.h"

#include "akodeplayer.h"

using namespace aKode;

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

aKodePlayer::aKodePlayer(QObject* parent, const char* name, const QStringList& args) :
    Player(parent, name, args),
    m_player(0)
{}

aKodePlayer::~aKodePlayer()
{
    delete m_player;
}

//void aKodePlayer::play(const FileHandle &file)
void aKodePlayer::startPlay(const QString &file)
{
    kdDebug() << k_funcinfo << endl;

    if (file.isNull()) { // null FileHandle file means unpause
        if (paused())
            // m_player->resume();
            m_player->play();
        else
            stop();
        return;
    }

    // QString filename = file.absFilePath();

    kdDebug() << "Opening: " << file << endl;

    if (m_player)
        m_player->stop();
    else {
        m_player = new aKode::Player();
        if (!m_player->open(m_sinkName.ascii())) {
            kdDebug() << k_funcinfo << "Unable to open aKode " << m_sinkName << " sink. "
                << "Falling back to auto." << endl;
            m_player->open("auto");
        }
    }

    if (m_player->load(QFile::encodeName(file)))
        m_player->play();

}

void aKodePlayer::pause()
{
    if (m_player)
        m_player->pause();
}

void aKodePlayer::stop()
{
    if (m_player) {
        m_player->stop();
        m_player->unload();
    }
}

void aKodePlayer::setVolume(float volume)
{
    if (m_player)
        m_player->setVolume(volume);
}

float aKodePlayer::volume() const
{
    if (m_player)
        return m_player->volume();
    // 1.0 is full volume
    return 1.0;
}

/////////////////////////////////////////////////////////////////////////////////
// m_player status functions
/////////////////////////////////////////////////////////////////////////////////

bool aKodePlayer::playing() const
{
    if (m_player && m_player->decoder())
        return !m_player->decoder()->eof();
    else
        return false;
}

bool aKodePlayer::paused() const
{
    return m_player && (m_player->state() == aKode::Player::Paused);
}

int aKodePlayer::totalTime() const
{
    if (m_player) {
        Decoder *d = m_player->decoder();
        if (d)
            return d->length() / 1000;
    }
    return -1;
}

int aKodePlayer::currentTime() const
{
    if (m_player) {
        Decoder *d = m_player->decoder();
        if (d)
            return d->position() / 1000;
    }
    return -1;
}

int aKodePlayer::position() const
{
    if (m_player) {
        Decoder *d = m_player->decoder();
        if (d && d->length())
            return (d->position()*1000)/(d->length());
        else
            return -1;
    }
    else
        return -1;
}

/////////////////////////////////////////////////////////////////////////////////
// m_player seek functions
/////////////////////////////////////////////////////////////////////////////////

void aKodePlayer::seek(int seekTime)
{
    // seek time in seconds?
    if (m_player)
        m_player->decoder()->seek(seekTime*1000);
}

void aKodePlayer::seekPosition(int position)
{
    // position unit is 1/1000th
    if (m_player)
        m_player->decoder()->seek((position * m_player->decoder()->length())/1000);
}

QStringList aKodePlayer::getPluginList( const QCString& /*classname*/ )
{
    return QStringList::split("|", "auto|polyp|alsa|jack|oss");
}

void aKodePlayer::setSinkName(const QString& sinkName) { m_sinkName = sinkName; }

#include "akodeplayer.moc"
