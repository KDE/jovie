/***************************************************** vim:set ts=4 sw=4 sts=4:
  Player Object for playing synthesized audio files.  Plays them
  synchronously.
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// Qt includes.
#include <qfile.h>

// KDE includes.
#include <kapplication.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kparts/componentfactory.h>
#include <ktrader.h>
#include <kdebug.h>

// KTTS includes.
#include "player.h"
#include "stretcher.h"
#include "pluginconf.h"

// TestPlayer includes.
#include "testplayer.h"

/**
 * Constructor.
 */
TestPlayer::TestPlayer(QObject *parent, const char *name,
    const int playerOption, const float audioStretchFactor, const QString &sinkName) :
    QObject(parent, name)
{
    m_playerOption = playerOption;
    m_audioStretchFactor = audioStretchFactor;
    m_stretcher = 0;
    m_player = 0;
    m_sinkName = sinkName;
}

/**
 * Destructor.
 */
TestPlayer::~TestPlayer()
{
    delete m_stretcher;
    delete m_player;
}

/**
 * Sets which audio player to use.
 *  0 = aRts
 *  1 = gstreamer
 */
void TestPlayer::setPlayerOption(const int playerOption) { m_playerOption = playerOption; }

/**
 * Sets the audio stretch factor (Speed adjustment).
 * 1.0 = normal
 * 0.5 = twice as fast
 * 2.0 = twice as slow
 */
void TestPlayer::setAudioStretchFactor(const float audioStretchFactor)
    { m_audioStretchFactor = audioStretchFactor; }

void TestPlayer::setSinkName(const QString &sinkName) { m_sinkName = sinkName; }

/**
 * Plays the specifified audio file and waits for completion.
 * The audio file speed is adjusted according to the stretch factor.
 * @param waveFile              Name of the audio file to play.
 */
void TestPlayer::play(const QString &waveFile)
{
    // kdDebug() << "TestPlayer::play: running" << endl;
    // Create a Stretcher object to adjust the audio Speed.
    QString playFile = waveFile;
    QString tmpFile;
    if (m_audioStretchFactor != 1.0)
    {
        tmpFile = makeSuggestedFilename();
        // kdDebug() << "TestPlayer::play: stretching file " << playFile << " by " << m_audioStretchFactor
        //     << " to file " << tmpFile << endl;
        m_stretcher = new Stretcher();
        if (m_stretcher->stretch(playFile, tmpFile, m_audioStretchFactor))
        {
            while (m_stretcher->getState() != Stretcher::ssFinished) qApp->processEvents();
            playFile = m_stretcher->getOutFilename();
        }
        delete m_stretcher;
        m_stretcher = 0;
    }

    // Create player object based on player option.
    // kdDebug() << "TestPlayer::play: creating Player object with playerOption " << m_playerOption << endl;
    m_player = createPlayerObject(m_playerOption);
    // If player object could not be created, avoid crash is the best we can do!
    if (!m_player) return;
    // kdDebug() << "TestPlayer::play: starting playback." << endl;
    m_player->startPlay(playFile);

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let take care of
    // cleaning up the play object.  However, because we've been called from DCOP,
    // this seems to be necessary.  The call to processEvents is problematic because
    // it can cause re-entrancy.
    while (m_player->playing()) qApp->processEvents();
    // kdDebug() << "TestPlayer::play: stopping playback." << endl;
    m_player->stop();
    delete m_player;
    m_player = 0;
    if (!tmpFile.isEmpty()) QFile::remove(tmpFile);
}

/**
 * Creates and returns a player object based on user option.
 */
Player* TestPlayer::createPlayerObject(int playerOption)
{
    Player* player = 0;
    QString plugInName;
    switch(playerOption)
    {
        case 1 :
        {
            plugInName = "KTTSD GStreamer Plugin";
            break;
        }
        default:
        {
            plugInName = "KTTSD Arts Plugin";
            break;
        }
    }
    KTrader::OfferList offers = KTrader::self()->query(
            "KTTSD/AudioPlugin", QString("Name == '%1'").arg(plugInName));

    if(offers.count() == 1)
    {
        kdDebug() << "TestPlayer::createPlayerObject: Loading " << offers[0]->library() << endl;
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if (factory)
            player = 
                KParts::ComponentFactory::createInstanceFromLibrary<Player>(
                    offers[0]->library().latin1(), this, offers[0]->library().latin1());
        else
            kdDebug() << "TestPlayer::createPlayerObject: Could not create factory." << endl;
    }
    if (player == 0)
        kdDebug() << "TestPlayer::createPlayerObject: Could not load " + plugInName +
            ".  Is KDEDIRS set correctly?" << endl;
    else
        // Must have GStreamer >= 0.8.7.
        if (playerOption == 1)
        {
            if (!player->requireVersion(0, 8, 7))
            {
                delete player;
                player = 0;
            }
            else
                player->setSinkName(m_sinkName);
        }
    return player;
}

/**
 * Constructs a temporary filename for plugins to use as a suggested filename
 * for synthesis to write to.
 * @return                        Full pathname of suggested file.
 */
QString TestPlayer::makeSuggestedFilename()
{
    KTempFile tempFile (locateLocal("tmp", "kttsmgr-"), ".wav");
    QString waveFile = tempFile.file()->name();
    tempFile.close();
    QFile::remove(waveFile);
    kdDebug() << "TestPlayer::makeSuggestedFilename: Suggesting filename: " << waveFile << endl;
    return PlugInConf::realFilePath(waveFile);
}

