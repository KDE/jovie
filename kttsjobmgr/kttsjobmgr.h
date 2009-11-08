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

#ifndef KTTSJOBMGRPART_H
#define KTTSJOBMGRPART_H

// Qt includes


// KDE includes.
#include <kparts/browserextension.h>

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

class KttsJobMgrPart:
    public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    KttsJobMgrPart(QWidget *parentWidget, QObject *parent, const QStringList& args=QStringList());
    virtual ~KttsJobMgrPart();
    static KAboutData* createAboutData();

protected:
    virtual bool openFile();
    virtual bool closeUrl();

    /** Slots connected to DBUS Signals emitted by KTTSD. */
protected Q_SLOTS:
    /**
    * This signal is emitted when KTTSD starts or restarts after a call to reinit.
    */
    Q_SCRIPTABLE void kttsdStarted();

private slots:
    /**
    * Slots connected to buttons.
    */
    void slot_stop();
    void slot_cancel();
    void slot_pause();
    void slot_resume();
    void slot_job_change_talker();
    void slot_speak_clipboard();
    void slot_speak_file();

    /**
    * Slots for comboboxes.
    */
    
    void slot_moduleChanged(const QString & module);
    void slot_languageChanged(const QString & language);
    void slot_voiceChanged(int voice);

    /**
    * Slots connected to sliders.
    */
    void slot_speedSliderChanged(int);
    void slot_pitchSliderChanged(int);
    void slot_volumeSliderChanged(int);
    
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
    KttsJobMgrBrowserExtension *m_extension;
    Ui::kttsjobmgr * m_ui;
    QList<KPushButton*> m_jobButtons;

    /**
    * This flag is set to True whenever we want to select the next job that
    * is announced in a textSet signal.
    */
    bool m_selectOnTextSet;

    /**
    * Cache mapping Talker Codes to Talker IDs.
    */
    QMap<QString,QString> m_talkerCodesToTalkerIDs;
};

class KttsJobMgrBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
    friend class KttsJobMgrPart;
public:
    KttsJobMgrBrowserExtension(KttsJobMgrPart *parent);
    virtual ~KttsJobMgrBrowserExtension();
};

#endif    // KTTSJOBMGRPART_H
