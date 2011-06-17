/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTS Manager Program
  --------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

// TQt includes.
#include <tqevent.h>
#include <tqtooltip.h>

// KDE includes.
#include <ksystemtray.h>

// KTTS includes.
#include "kspeech_stub.h"
#include "kspeechsink.h"

class KttsToolTip: public TQToolTip
{
    public:
        KttsToolTip ( TQWidget* tqparent );

    protected:
        virtual void maybeTip ( const TQPoint & p );
};

class KttsMgrTray: public KSystemTray, public KSpeech_stub, virtual public KSpeechSink
{
    Q_OBJECT
  TQ_OBJECT

    public:
        KttsMgrTray(TQWidget *tqparent=0);
        ~KttsMgrTray();

        void setExitWhenFinishedSpeaking();
        TQString gettqStatus();

    protected:
        // ASYNC textStarted(const TQCString& appId, uint jobNum);
        ASYNC textFinished(const TQCString& appId, uint jobNum);
        virtual bool eventFilter( TQObject* o, TQEvent* e );

    private slots:

        void speakClipboardSelected();
        void holdSelected();
        void resumeSelected();
        void aboutSelected();
        void helpSelected();
        void quitSelected();

    private:
        /**
         * Convert a KTTSD job state integer into a display string.
         * @param state          KTTSD job state
         * @return               Display string for the state.
         */
        TQString stateToStr(int state);

        bool isKttsdRunning();
        void exitWhenFinishedSpeaking();
        KttsToolTip* m_toolTip;
};

#endif    // KTTSMGR_H
