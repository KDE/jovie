/***************************************************** vim:set ts=4 sw=4 sts=4:
  Player Object for playing synthesized audio files.  Plays them
  synchronously.
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef _TESTPLAYER_H_
#define _TESTPLAYER_H_

#include <kdemacros.h>
#include "kdeexportfix.h"

class Player;
class Stretcher;

// TODO: Make this work asynchronously.

class KDE_EXPORT TestPlayer : public TQObject{
    public:
        /**
         * Constructor.
         * @param playerOption
         * @param audioStretchFactor
         */
        TestPlayer(TQObject *tqparent = 0, const char *name = 0,
            const int playerOption = 0, const float audioStretchFactor = 1.0,
            const TQString &sinkName = TQString());

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
        void play(const TQString &waveFile);

        /**
        * Sets the GStreamer Sink Name.  Examples: "alsasink", "osssink", "nassink".
        */
        void setSinkName(const TQString &sinkName);

        /**
         * Creates and returns a player object based on user option.
         */
        Player* createPlayerObject(int playerOption);

    private:

        /**
         * Constructs a temporary filename for plugins to use as a suggested filename
         * for synthesis to write to.
         * @return                        Full pathname of suggested file.
         */
        TQString makeSuggestedFilename();

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
        TQString m_sinkName;

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
