/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, rewind,
  advance, change Talker, etc. 
  -------------------
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright : (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------

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

#ifndef KTTSJOBMGR_H
#define KTTSJOBMGR_H

// Qt includes
#include <QWidget>

// KDE includes.

// KTTS includes.
#include "kspeechinterface.h"

class KAboutData;
class KPushButton;
class KttsJobMgrBrowserExtension;
class JobInfo;
class JobInfoListModel;

namespace Ui
{
    class kttsjobmgr;
}

class KttsJobMgr:
  public QWidget
{
    Q_OBJECT
public:
    explicit KttsJobMgr(QWidget *parent = 0);
    virtual ~KttsJobMgr();

    /** apply current settings, i.e. tell speech-dispatcher what to do */
    void save();
    /** get the current settings from speech-dispatcher */
    void load();
    
signals:
    void configChanged();

private slots:
    /**
    * Slots connected to buttons.
    */
    void slot_stop();
    void slot_cancel();
    void slot_pause();
    void slot_resume();
    void slot_speak_clipboard();
    void slot_speak_file();

private:
    /**
    * DBUS KSpeech Interface.
    */
    org::kde::KSpeech* m_kspeech;

    /**
    * Return the Talker ID corresponding to a Talker Code, retrieving from cached list if present.
    * @param talkerCode    Talker Code.
    * @return              Talker ID.
    */
    QString cachedTalkerCodeToTalkerID(const QString& talkerCode);

    /**
    * Job ListView.
    */
    Ui::kttsjobmgr * m_ui;

    /**
    * Cache mapping Talker Codes to Talker IDs.
    */
    QMap<QString,QString> m_talkerCodesToTalkerIDs;
};

#endif    // KTTSJOBMGR_H
