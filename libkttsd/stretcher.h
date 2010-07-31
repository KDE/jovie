/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
     Speeds up or slows down an audio file by stretching the audio stream.
     Uses the sox program to do the stretching.

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

#ifndef _STRETCHER_H_
#define _STRETCHER_H_

#include <kdemacros.h>
#include "kdeexportfix.h"

class KProcess;

class KDE_EXPORT Stretcher : public QObject{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        Stretcher(TQObject *parent = 0, const char *name = 0);

        /**
         * Destructor.
         */
        ~Stretcher();

        enum StretcherState {
            ssIdle = 0,             // Not doing anything.  Ready to stretch.
            ssStretching = 1,       // Stretching.
            ssFinished = 2          // Stretching finished.
        };

        /**
         * Stretch the given input file to an output file.
         * @param inFilename        Name of input audio file.
         * @param outFilename       Name of output audio file.
         * @param stretchFactor     Amount to stretch.  2.0 is twice as slow.  0.5 is twice as fast.
         */
        bool stretch(const TQString &inFilename, const TQString &outFilename, float stretchFactor);

        /**
        * Returns the state of the Stretcher.
        */
        int getState();

        /**
         * Returns the output filename (as given in call to stretch).
         */
        TQString getOutFilename();

        /**
        * Acknowledges the finished stretching.
        */
        void ackFinished();

    signals:
        /**
         * Emitted whenever stretching is completed.
         */
        void stretchFinished();

    private slots:
        void slotProcessExited(KProcess* proc);

    private:
        // Stretcher state.
        int m_state;

        // Sox process.
        KProcess* m_stretchProc;

        // Output file name.
        TQString m_outFilename;
};

#endif      // _STRETCHER_H_
