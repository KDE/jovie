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

class QTreeView;
class KAboutData;
class KPushButton;
class KttsJobMgrBrowserExtension;
class KVBox;
class KTextEdit;
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

    /**
    * This signal is emitted each time the state of a job changes.
    * @param appId              The DBUS sender ID of the application that
    *                           submitted the job.
    * @param jobNum             Job Number.
    * @param state              Job state.  @see KSpeech::JobState.
    */
    Q_SCRIPTABLE void jobStateChanged(const QString &appId, int jobNum, int state);

    /**
    * This signal is emitted when a marker is processed.
    * Currently only emits mtSentenceBegin and mtSentenceEnd.
    * @param appId         The DBUS sender ID of the application that submitted the job.
    * @param jobNum        Job Number of the job emitting the marker.
    * @param markerType    The type of marker.
    *                      Currently either mtSentenceBegin or mtSentenceEnd.
    * @param markerData    Data for the marker.
    *                      Currently, this is the sequence number of the sentence
    *                      begun or ended.  Sequence numbers begin at 1.
    */
    Q_SCRIPTABLE void marker(const QString &appId, int jobNum, int markerType, const QString &markerData);

    /**
     * slot for when jobs are filtered in the daemon so we can show it in our ui
     */
    Q_SCRIPTABLE void slotJobFiltered(const QString&, const QString&);

private slots:
    /**
    * This slot is connected to the Job List View clicked signal.
    */
    void slot_jobListView_clicked();

    /**
    * Slots connected to buttons.
    */
    void slot_job_stop();
    void slot_job_pause();
    void slot_job_resume();
    void slot_job_restart();
    void slot_job_remove();
    void slot_job_move();
    void slot_job_change_talker();
    void slot_speak_clipboard();
    void slot_speak_file();
    void slot_refresh();
    void slot_job_prev_sen();
    void slot_job_next_sen();

private:
    /**
    * Get the Job Number of the currently-selected job in the Job List View.
    * @return               Job Number of currently-selected job.
    *                       0 if no currently-selected job.
    */
    int getCurrentJobNum();

    /**
    * Enables or disables all the job-related buttons.
    * @param enable        True to enable the job-related butons.  False to disable.
    */
    void enableJobActions(bool enable);

    /**
    * Retrieves JobInfo from KTTSD, creates and fills JobInfo object.
    * @param jobNum         Job Number.
    */
    JobInfo* retrieveJobInfo(int jobNum);

    /**
    * Refresh display of a single job in the JobListView.
    * @param jobNum         Job Number.
    */
    void refreshJob(int jobNum);

    /**
    * Fill the Job List.
    */
    void refreshJobList();

    /**
    * If nothing selected in Job List View and list not empty, select top item.
    * If nothing selected and list is empty, disable job buttons.
    */
    void autoSelectInJobListView();
    
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
    JobInfoListModel* m_jobListModel;
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
