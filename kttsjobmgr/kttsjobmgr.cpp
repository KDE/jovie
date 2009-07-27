/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, cancel, pause,
  resume, change Talker, etc.
  -------------------
  Copyright : (C) 2004,2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

// KttsJobMgr includes.
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"
#include "ui_kttsjobmgr.h"

// QT includes.
#include <QtCore/QObject>
#include <QtGui/QLabel>
#include <QtGui/QSplitter>
#include <QtGui/QClipboard>
#include <QtGui/QPushButton>
#include <QtCore/QList>
#include <QtGui/QTreeView>
#include <QtCore/QMimeData>
#include <QtDBus/QtDBus>

// KDE includes.
#include <kcomponentdata.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kinputdialog.h>
#include <ktextedit.h>
#include <kvbox.h>
#include <kdemacros.h>
#include <kparts/genericfactory.h>
#include <kspeech.h>

// KTTS includes.
#include "talkercode.h"
#include "selecttalkerdlg.h"
#include "jobinfolistmodel.h"

typedef KParts::GenericFactory<KttsJobMgrPart> KttsJobMgrPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkttsjobmgrpart, KttsJobMgrPartFactory )

KAboutData* KttsJobMgrPart::createAboutData()
{
    KAboutData *about = new KAboutData("kttsjobmgr", 0, ki18n("KttsJobMgr"), "1.99");
    return about;
}

KttsJobMgrPart::KttsJobMgrPart(QWidget *parentWidget, QObject *parent, const QStringList& args) :
    KParts::ReadOnlyPart(parent)
{
    m_ui = new Ui::kttsjobmgr;
    QWidget * widget = new QWidget(parentWidget);
    m_ui->setupUi(widget);

//DBusAbstractInterfacePrivate
    Q_UNUSED(args);
    m_kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
    m_kspeech->setParent(this);

    // Establish ourself as a System Manager.
    m_kspeech->setApplicationName("KCMKttsMgr");
    m_kspeech->setIsSystemManager(true);

    // Initialize some variables.
    m_selectOnTextSet = false;

    // All the ktts components use the same catalog.
    KGlobal::locale()->insertCatalog("kttsd");

    m_jobListModel = new JobInfoListModel();
    m_ui->m_jobTableView->setModel(m_jobListModel);

    connect (m_ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_speedSliderChanged(int)));
    connect (m_ui->pitchSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_pitchSliderChanged(int)));
    connect (m_ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_volumeSliderChanged(int)));

    m_ui->stopButton->setIcon(KIcon("media-playback-stop"));
    connect (m_ui->stopButton, SIGNAL(clicked()), this, SLOT(slot_stop()));
    m_ui->cancelButton->setIcon(KIcon("edit-clear"));
    connect (m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(slot_cancel()));
    m_ui->pauseButton->setIcon(KIcon("media-playback-pause"));
    connect (m_ui->pauseButton, SIGNAL(clicked()), this, SLOT(slot_pause()));
    m_ui->resumeButton->setIcon(KIcon("media-playback-start"));
    connect (m_ui->resumeButton, SIGNAL(clicked()), this, SLOT(slot_resume()));

    m_ui->speak_clipboard->setIcon(KIcon("klipper"));
    connect (m_ui->speak_clipboard, SIGNAL(clicked()), this, SLOT(slot_speak_clipboard()));
    m_ui->speak_file->setIcon(KIcon("document-open"));
    connect (m_ui->speak_file, SIGNAL(clicked()), this, SLOT(slot_speak_file()));
    m_ui->job_changetalker->setIcon(KIcon("translate"));
    connect (m_ui->job_changetalker, SIGNAL(clicked()), this, SLOT(slot_job_change_talker()));

    // Set the main widget for the part.
    setWidget(widget);

    connect(m_ui->m_jobTableView, SIGNAL(clicked(const QModelIndex&)),
        this, SLOT(slot_jobListView_clicked()));

    // Fill the Job List.
    refreshJobList();
    
    // Select first item (if any).
    autoSelectInJobListView();

    // Connect DBUS Signals emitted by KTTSD to our own slots.
    connect(m_kspeech, SIGNAL(kttsdStarted()),
        this, SLOT(kttsdStarted()));
    connect(m_kspeech, SIGNAL(jobStateChanged(const QString&, int, int)),
        this, SLOT(jobStateChanged(const QString&, int, int)));
    connect(m_kspeech, SIGNAL(marker(const QString&, int, int, const QString&)),
        this, SLOT(marker(const QString&, int, int, const QString&)));
    connect(m_kspeech, SIGNAL(newJobFiltered(const QString&, const QString&)),
        this, SLOT(slotJobFiltered(const QString&, const QString&)));

    m_extension = new KttsJobMgrBrowserExtension(this);
}

KttsJobMgrPart::~KttsJobMgrPart()
{
    KGlobal::locale()->removeCatalog("kttsd");
    closeUrl();
    delete m_ui;
}

bool KttsJobMgrPart::openFile()
{
    return true;
}

bool KttsJobMgrPart::closeUrl()
{
    return true;
}

/**
* This slot is connected to the Job List View clicked signal.
*/
void KttsJobMgrPart::slot_jobListView_clicked()
{
}

/**
* Slots connected to buttons.
*/
void KttsJobMgrPart::slot_stop()
{
    m_kspeech->stop();
}

void KttsJobMgrPart::slot_cancel()
{
    m_kspeech->cancel();
}

void KttsJobMgrPart::slot_pause()
{
    m_kspeech->pause();
}

void KttsJobMgrPart::slot_resume()
{
    m_kspeech->resume();
}

void KttsJobMgrPart::slot_speedSliderChanged(int speed)
{
    kDebug() << "telling kspeech to set speed to " << speed;
    m_kspeech->setSpeed(speed);
}

void KttsJobMgrPart::slot_pitchSliderChanged(int pitch)
{
    kDebug() << "telling kspeech to set pitch to " << pitch;
    m_kspeech->setPitch(pitch);
}

void KttsJobMgrPart::slot_volumeSliderChanged(int volume)
{
    kDebug() << "telling kspeech to set volume to " << volume;
    m_kspeech->setVolume(volume);
}

void KttsJobMgrPart::slot_job_change_talker()
{
    QModelIndex index = m_ui->m_jobTableView->currentIndex();
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        QString talkerID = job.talkerID;
        QStringList talkerIDs = m_talkerCodesToTalkerIDs.values();
        int ndx = talkerIDs.indexOf(talkerID);
        QString talkerCode;
        if (ndx >= 0)
            talkerCode = m_talkerCodesToTalkerIDs.keys()[ndx];
        QPointer<SelectTalkerDlg> dlg = new SelectTalkerDlg(widget(), "selecttalkerdialog", i18n("Select Talker"), talkerCode, true);
        int dlgResult = dlg->exec();
        if (dlgResult != KDialog::Accepted)
            return;
        talkerCode = dlg->getSelectedTalkerCode();
        int jobNum = job.jobNum;
        m_kspeech->changeJobTalker(jobNum, talkerCode);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_speak_clipboard()
{
    // kDebug() << "KttsJobMgrPart::slot_speak_clipboard: running";

    // Get the clipboard object.
    QClipboard *cb = QApplication::clipboard();

    // Copy text from the clipboard.
    QString text;
    KSpeech::SayOptions sayOptions = KSpeech::soNone;
    const QMimeData* data = cb->mimeData();
    if (data)
    {
        if (data->hasFormat("text/html"))
        {
            // if (m_kspeech->supportsMarkup(NULL, KSpeech::mtHtml))
                text = data->html();
                sayOptions = KSpeech::soHtml;
        }
        if (data->hasFormat("text/ssml"))
        {
            // if (m_kspeech->supportsMarkup(NULL, KSpeech::mtSsml))
            {
                QByteArray d = data->data("text/ssml");
                text = QString(d);
                sayOptions = KSpeech::soSsml;
            }
        }
    }
    if (text.isEmpty()) {
        text = cb->text();
        sayOptions = KSpeech::soPlainText;
    }

    // Speak it.
    if ( !text.isEmpty() )
    {
        m_kspeech->say(text, sayOptions);
        // int jobNum = m_kspeech->say(text, sayOptions);
        // kDebug() << "KttsJobMgrPart::slot_speak_clipboard: started jobNum " << jobNum;
        // Set flag so that the job we just created will be selected when textSet signal is received.
        m_selectOnTextSet = true;
    }
}

void KttsJobMgrPart::slot_speak_file()
{
    KEncodingFileDialog dlg;
    KEncodingFileDialog::Result result = dlg.getOpenFileNameAndEncoding();
    if (result.fileNames.count() == 1)
    {
        // kDebug() << "KttsJobMgr::slot_speak_file: calling setFile with filename " <<
        //     result.fileNames[0] << " and encoding " << result.encoding << endl;
        m_kspeech->sayFile(result.fileNames[0], result.encoding);
    }
}

//void KttsJobMgrPart::slot_refresh()
//{
//    // Clear TalkerID cache.
//    m_talkerCodesToTalkerIDs.clear();
//    // Get current job number.
//    int jobNum = getCurrentJobNum();
//    refreshJobList();
//    // Select the previously-selected job.
//    if (jobNum)
//    {
//        QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
//        if (index.isValid())
//        {
//            m_ui->m_jobTableView->setCurrentIndex(index);
//            slot_jobListView_clicked();
//        }
//    }
//}


/**
* Get the Job Number of the currently-selected job in the Job List View.
* @return               Job Number of currently-selected job.
*                       0 if no currently-selected job.
*/
int KttsJobMgrPart::getCurrentJobNum()
{
    int jobNum = 0;
    QModelIndex index = m_ui->m_jobTableView->currentIndex();
    if (index.isValid())
        jobNum = m_jobListModel->getRow(index.row()).jobNum;
    return jobNum;
}

/**
* Retrieves JobInfo from KTTSD, creates and fills JobInfo object.
* @param jobNum         Job Number.
*/
JobInfo* KttsJobMgrPart::retrieveJobInfo(int jobNum)
{
    QByteArray jobInfo = m_kspeech->getJobInfo(jobNum);
    if (jobInfo != QByteArray()) {
        JobInfo* job = new JobInfo();
        QDataStream stream(&jobInfo, QIODevice::ReadOnly);
        qint32 priority;
        qint32 state;
        QString talkerCode;
        qint32 sentenceNum;
        qint32 sentenceCount;
        stream >> priority;
        stream >> state;
        stream >> job->appId;
        stream >> talkerCode;
        stream >> sentenceNum;
        stream >> sentenceCount;
        stream >> job->applicationName;
        job->jobNum = jobNum;
        job->priority = priority;
        job->state = state;
        job->sentenceNum = sentenceNum;
        job->sentenceCount = sentenceCount;
        job->talkerID = cachedTalkerCodeToTalkerID(talkerCode);
        return job;
    };
    return NULL;
}

/**
* Refresh display of a single job in the JobListView.
* @param jobNum         Job Number.
*/
void KttsJobMgrPart::refreshJob(int jobNum)
{
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid()) {
        JobInfo* job = retrieveJobInfo(jobNum);
        if (job)
            m_jobListModel->updateRow(index.row(), *job);
        else
            m_jobListModel->removeRow(index.row());
    }
}

/**
* Fill the Job List View.
*/
void KttsJobMgrPart::refreshJobList()
{
    // kDebug() << "KttsJobMgrPart::refreshJobList: Running";
    m_jobListModel->clear();
    JobInfoList jobInfoList;
    QStringList jobNums = m_kspeech->getJobNumbers(KSpeech::jpAll);
    for (int ndx = 0; ndx < jobNums.count(); ++ndx)
    {
        QString jobNumStr = jobNums[ndx];
        kDebug() << "jobNumStr = " << jobNumStr;
        int jobNum = jobNumStr.toInt(0, 10);
        kDebug() << "jobNum = " << jobNum;
        JobInfo* job = retrieveJobInfo(jobNum);
        if (job)
            jobInfoList.append(*job);
    }
    m_jobListModel->setDatastore(jobInfoList);
}

/**
* If nothing selected in Job List View and list not empty, select top item.
* If nothing selected and list is empty, disable job buttons.
*/
void KttsJobMgrPart::autoSelectInJobListView()
{
    // If something selected, nothing to do.
    if (m_ui->m_jobTableView->currentIndex().isValid()) return;
    // If empty, disable job buttons.
    
    //if (m_jobListModel->rowCount() == 0)
    //    enableJobActions(false);
    //else
    //{
        // Select first item.
        m_ui->m_jobTableView->setCurrentIndex(m_jobListModel->index(0, 0));
        slot_jobListView_clicked();
    //}
}

/**
* Return the Talker ID corresponding to a Talker Code, retrieving from cached list if present.
* @param talkerCode    Talker Code.
* @return              Talker ID.
*/
QString KttsJobMgrPart::cachedTalkerCodeToTalkerID(const QString& talkerCode)
{
    // If in the cache, return that.
    if (m_talkerCodesToTalkerIDs.contains(talkerCode))
        return m_talkerCodesToTalkerIDs[talkerCode];
    else
    {
        // Otherwise, retrieve Talker ID from KTTSD and cache it.
        QString talkerID = m_kspeech->talkerToTalkerId(talkerCode);
        m_talkerCodesToTalkerIDs[talkerCode] = talkerID;
        // kDebug() << "KttsJobMgrPart::cachedTalkerCodeToTalkerID: talkerCode = " << talkerCode << " talkerID = " << talkerID;
        return talkerID;
    }
}

/** Slots connected to DBUS Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
Q_SCRIPTABLE void KttsJobMgrPart::kttsdStarted()
{
    refreshJobList();
}


/**
* This signal is emitted each time the state of a job changes.
* @param appId              The DBUS sender ID of the application that
*                           submitted the job.
* @param jobNum             Job Number.
* @param state              Job state.  @see KSpeech::JobState.
*/
Q_SCRIPTABLE void KttsJobMgrPart::jobStateChanged(const QString &appId, int jobNum, int state)
{
    Q_UNUSED(appId);
    switch (state)
    {
        case KSpeech::jsQueued:
        {
            QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
            if (index.isValid())
                refreshJob(jobNum);
            else
            {
                JobInfo* job = retrieveJobInfo(jobNum);
                if (job) {
                    m_jobListModel->appendRow(*job);
                    // Should we select this job?
                    if (m_selectOnTextSet)
                    {
                        m_ui->m_jobTableView->setCurrentIndex(m_jobListModel->jobNumToIndex(jobNum));
                        m_selectOnTextSet = false;
                        slot_jobListView_clicked();
                    }
                }
            }
            // If a job not already selected, select this one.
            autoSelectInJobListView();
            break;
        }
        case KSpeech::jsSpeakable:
        {
            QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
            if (index.isValid()) {
                JobInfo* job = retrieveJobInfo(jobNum);
                if (job)
                    m_jobListModel->updateRow(index.row(), *job);
                else
                    m_jobListModel->removeRow(index.row());
            }
            break;
        }
        case KSpeech::jsFiltering:
        case KSpeech::jsSpeaking:
        case KSpeech::jsPaused:
        case KSpeech::jsInterrupted:
        case KSpeech::jsFinished:
        {
            QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
            if (index.isValid())
            {
                JobInfo job = m_jobListModel->getRow(index.row());
                job.state = state;
                m_jobListModel->updateRow(index.row(), job);
            }
            //m_ui->m_currentSentence->setPlainText(m_kspeech->getJobSentence(jobNum, 0));
            break;
        }
        case KSpeech::jsDeleted:
        {
            QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
            if (index.isValid())
                m_jobListModel->removeRow(index.row());
            autoSelectInJobListView();
            break;
        }
    }
}

Q_SCRIPTABLE void KttsJobMgrPart::slotJobFiltered(const QString& prefilterText, const QString& postfilterText)
{
    //m_ui->m_currentSentence->setPlainText(prefilterText);
    //m_ui->m_filteredCurrentSentence->setPlainText(postfilterText);
}

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
Q_SCRIPTABLE void KttsJobMgrPart::marker(const QString &appId, int jobNum, int markerType, const QString &markerData)
{
    Q_UNUSED(appId);
    if (KSpeech::mtSentenceBegin == markerType) {
        QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
        if (index.isValid())
        {
            JobInfo job = m_jobListModel->getRow(index.row());
            int seq = markerData.toInt();
            job.sentenceNum = seq;
            m_jobListModel->updateRow(index.row(), job);
            //m_ui->m_currentSentence->setPlainText(m_kspeech->getJobSentence(jobNum, seq));
        }
    }
    if (KSpeech::mtSentenceEnd == markerType) {
        //m_ui->m_currentSentence->setPlainText(QString());
    }
}


KttsJobMgrBrowserExtension::KttsJobMgrBrowserExtension(KttsJobMgrPart *parent)
    : KParts::BrowserExtension(parent)
{
}

KttsJobMgrBrowserExtension::~KttsJobMgrBrowserExtension()
{
}
