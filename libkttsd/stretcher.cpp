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

// KDE includes.
#include <kprocess.h>
#include <kdebug.h>

// Stretcher includes.
#include "stretcher.h"
#include "stretcher.moc"

/**
 * Constructor.
 */
Stretcher::Stretcher(QObject *parent, const char *name) :
    QObject(parent)
{
    Q_UNUSED(name);
    m_state = 0;
    m_stretchProc = 0;
}

/**
 * Destructor.
 */
Stretcher::~Stretcher()
{
    delete m_stretchProc;
}

/**
 * Stretch the given input file to an output file.
 * @param inFilename        Name of input audio file.
 * @param outFilename       Name of output audio file.
 * @param stretchFactor     Amount to stretch.  2.0 is twice as slow.  0.5 is twice as fast.
 * @return                  False if an error occurs.
 */
bool Stretcher::stretch(const QString &inFilename, const QString &outFilename, float stretchFactor)
{
    if (m_stretchProc) return false;
    m_outFilename = outFilename;
    m_stretchProc = new KProcess;
    QString stretchStr = QString("%1").arg(stretchFactor, 0, 'f', 3);
    *m_stretchProc << "sox" << inFilename << outFilename << "stretch" << stretchStr;
    connect(m_stretchProc, SIGNAL(processExited(KProcess*)),
        this, SLOT(slotProcessExited(KProcess*)));
    if (!m_stretchProc->start(KProcess::NotifyOnExit, KProcess::NoCommunication))
    {
        kDebug() << "Stretcher::stretch: Error starting audio stretcher process.  Is sox installed?" << endl;
        return false;
    }
    m_state = ssStretching;
    return true;
}

void Stretcher::slotProcessExited(KProcess*)
{
    m_stretchProc->deleteLater();
    m_stretchProc = 0;
    m_state = ssFinished;
    emit stretchFinished();
}

/**
 * Returns the state of the Stretcher.
 */
int Stretcher::getState() { return m_state; }

/**
 * Returns the output filename (as given in call to stretch).
 */
QString Stretcher::getOutFilename() { return m_outFilename; }

/**
 * Acknowledges the finished stretching.
 */
void Stretcher::ackFinished() { m_state = ssIdle; }

