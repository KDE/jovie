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

#ifndef _TESTPLAYER_H_
#define _TESTPLAYER_H_

class Player;
class Stretcher;

// TODO: Make this work asynchronously.

class TestPlayer : public QObject{
    public:
        /**
         * Constructor.
         * @param playerOption
         * @param audioStretchFactor
         */
        TestPlayer(QObject *parent = 0, const char *name = 0,
            const int playerOption = 0, const float audioStretchFactor = 1.0,
            const QString &sinkName = QString::null);

        /**
         * Destructor.
         */
        ~TestPlayer();

        /**
         * Sets which audio player to use.
         *  0 = aRts
         *  1 = gstreamer
         */
        void setPlayerOption(const int playerOption);

        /**
         * Sets the audio stretch factor (Speed adjustment).
         * 1.0 = normal
         * 0.5 = twice as fast
         * 2.0 = twice as slow
         */
        void setAudioStretchFactor(const float audioStretchFactor);

        /**
         * Plays the specifified audio file and waits for completion.
         * The audio file speed is adjusted according to the stretch factor.
         * @param waveFile              Name of the audio file to play.
         */
        void play(const QString &waveFile);

        /**
        * Sets the GStreamer Sink Name.  Examples: "alsasink", "osssink", "nassink".
        */
        void setSinkName(const QString &sinkName);

    private:
        /**
         * Creates and returns a player object based on user option.
         */
        Player* createPlayerObject();

        /**
         * Constructs a temporary filename for plugins to use as a suggested filename
         * for synthesis to write to.
         * @return                        Full pathname of suggested file.
         */
        QString makeSuggestedFilename();

         /**
         * Which audio player to use.
         *  0 = aRts
         *  1 = gstreamer
         */
        int m_playerOption;

        /**
         * Audio stretch factor (Speed).
         */
        float m_audioStretchFactor;

        /**
        * GStreamer sink name.
        */
        QString m_sinkName;

        /**
         * Stretcher object.
         */
        Stretcher* m_stretcher;

        /**
         * Player object.
         */
        Player* m_player;
};

#endif      // _TESTPLAYER_H_
