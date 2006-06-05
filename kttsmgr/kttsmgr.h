/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSMgr System Tray Application
  -------------------------------
  Copyright:
  (C) 2004-2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>

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

#ifndef KTTSMGR_H
#define KTTSMGR_H

// KDE includes.
#include <ksystemtray.h>

// KTTS includes.
#include "kspeech_stub.h"
#include "kspeechsink.h"

class QEvent;
class QMouseEvent;
class QAction;

class KttsMgrTray: public KSystemTray, public KSpeech_stub, virtual public KSpeechSink
{
    Q_OBJECT

    public:
        KttsMgrTray(QWidget *parent=0);
        ~KttsMgrTray();

        void setExitWhenFinishedSpeaking();
        QString getStatus();

    protected:
        ASYNC textFinished(const QByteArray& appId, uint jobNum);
        bool event(QEvent *event);
        void mousePressEvent(QMouseEvent* ev);
        virtual void contextMenuAboutToShow(KMenu* menu);

    private slots:

        void speakClipboardSelected();
        void stopSelected();
        void pauseSelected();
        void resumeSelected();
        void repeatSelected();
        void configureSelected();
        void aboutSelected();
        void helpSelected();
        void quitSelected();

    private:
        /**
         * Convert a KTTSD job state integer into a display string.
         * @param state          KTTSD job state
         * @return               Display string for the state.
         */
        QString stateToStr(int state);
        void exitWhenFinishedSpeaking();
        
        bool isKttsdRunning();
        QAction* actStop;
        QAction* actPause;
        QAction* actResume;
        QAction* actRepeat;
        QAction* actSpeakClipboard;
        QAction* actConfigure;
};

#endif    // KTTSMGR_H
