/***************************************************** vim:set ts=4 sw=4 sts=4:
  JovieTrayIcon tray icon for jovie text to speech service
  -------------------------------
  Copyright 2004-2006 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2010 by Jeremy Whiting <jpwhiting@kde.org>

  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Jeremy Whiting <jpwhiting@kde.org>

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

#ifndef _JOVIETRAYICON_H
#define _JOVIETRAYICON_H

// KDE includes.
#include <kmenu.h>
#include <kstatusnotifieritem.h>

class QEvent;
class QAction;

class JovieTrayIcon: public KStatusNotifierItem
{
    Q_OBJECT

    public:
        JovieTrayIcon(QWidget *parent=0);
        ~JovieTrayIcon();

    protected Q_SLOTS:
        void slotActivateRequested(bool active, const QPoint &pos);
        virtual void contextMenuAboutToShow();

    private slots:

        void speakClipboardSelected();
        void stopSelected();
        void pauseSelected();
        void resumeSelected();
        void repeatSelected();
        void configureSelected();
        void aboutSelected();
        void helpSelected();

    private:
        void setupIcons();

        QAction* actStop;
        QAction* actPause;
        QAction* actResume;
        QAction* actRepeat;
        QAction* actSpeakClipboard;
        QAction* actConfigure;
};

#endif    // _JOVIETRAYICON_H
