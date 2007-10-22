/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, rewind,
  advance, change Talker, etc.
  -------------------
  Copyright : (C) 2004,2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
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

// KttsJobMgr includes.
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"

typedef KParts::GenericFactory<KttsJobMgrPart> KttsJobMgrPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkttsjobmgrpart, KttsJobMgrPartFactory )

KAboutData* KttsJobMgrPart::createAboutData()
{
    KAboutData *about = new KAboutData("kttsjobmgr", 0, ki18n("KttsJobMgr"), "1.99");
    return about;
}

KttsJobMgrPart::KttsJobMgrPart(QWidget *parentWidget, QObject *parent, const QStringList& args) :
    KParts::ReadOnlyPart(parent)
//    m_kspeech(QDBus::sessionBus().findInterface<org::kde::KSpeech>("org.kde.kttsd", "/KSpeech"))
{
//DBusAbstractInterfacePrivate
    Q_UNUSED(args);
    m_kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
    m_kspeech->setParent(this);

    // Establish ourself as a System Manager.
    m_kspeech->setApplicationName("KCMKttsMgr");
    m_kspeech->setIsSystemManager(true);

    // Initialize some variables.
    m_selectOnTextSet = false;
    m_buttonBox = 0;

    // All the ktts components use the same catalog.
    KGlobal::locale()->insertCatalog("kttsd");

    // Create a QVBox to host everything.
    KVBox* vBox = new KVBox(parentWidget);
    //vBox->setMargin(6);

    // Create a splitter to contain the Job List View and the current sentence.
    QSplitter* splitter = new QSplitter(vBox);
    splitter->setOrientation(Qt::Vertical);

    // Create Job List View widget and model.
    m_jobListView = new QTreeView( splitter );
    m_jobListModel = new JobInfoListModel();
    m_jobListView->setModel(m_jobListModel);

    // Select by row.
    m_jobListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_jobListView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Not a tree.
    m_jobListView->setRootIsDecorated(false);

    // TODO: Do not sort the list.
    // m_jobListView->setSorting(-1);

    QString jobListViewWT = i18n(
        "<p>These are all the text jobs.  The <b>State</b> column "
        "may be:"
        "<ul>"
        "<li><b>Queued</b> - the job is waiting and will not be spoken until its state "
        "is changed to <b>Waiting</b> by clicking the <b>Resume</b> or <b>Restart</b> buttons.</li>"
        "<li><b>Waiting</b> - the job is ready to be spoken.  It will be spoken when the jobs "
        "preceding it in the list have finished.</li>"
        "<li><b>Speaking</b> - the job is speaking.  The <b>Position</b> column shows the current "
        "sentence of the job being spoken.  You may pause a speaking job by clicking the "
        "<b>Hold</b> button.</li>"
        "<li><b>Paused</b> - the job is currently paused.  Paused jobs prevent jobs below them "
        "from speaking.  Use the <b>Resume</b> or <b>Restart</b> buttons to resume speaking the "
        "job, or click <b>Later</b> to move the job down in the list.</li>"
        "<li><b>Finished</b> - the job has finished speaking.  When a second job finishes, "
        "this one will be deleted.  You may click <b>Restart</b> to repeat the job.</li>"
        "</ul>"
        "<em>Note</em>: Messages, Warnings, and Screen Reader Output do not appear in this list.  "
        "See the Handbook for more information."
        "</p>");
    m_jobListView->setWhatsThis( jobListViewWT);

    // splitter->setResizeMode(m_jobListView, QSplitter::Stretch);

    // Create a VBox to hold buttons and current sentence.
    KVBox* bottomBox = new KVBox(splitter);

    // Create a box to hold buttons.
    m_buttonBox = new KVBox(bottomBox);
    m_buttonBox->setSpacing(6);

    // Create 3 HBoxes to host buttons.
    KHBox* hbox1 = new KHBox(m_buttonBox);
    hbox1->setSpacing(6);
    KHBox* hbox2 = new KHBox(m_buttonBox);
    hbox2->setSpacing(6);
    KHBox* hbox3 = new KHBox(m_buttonBox);
    hbox3->setSpacing(6);

    // Do not let button box stretch vertically.
    m_buttonBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    // All the buttons with "job_" at start of their names will be enabled/disabled when a job is
    // selected in the Job List View.

    QPushButton* btn;
    QString wt;
    btn = new QPushButton(KIcon("process-stop"), i18n("Hold"), hbox1);
    btn->setObjectName("job_hold");
    wt = i18n(
        "<p>Changes a job to Paused state.  If currently speaking, the job stops speaking.  "
        "Paused jobs prevent jobs that follow them from speaking, so either click "
        "<b>Resume</b> to make the job speakable, or click <b>Later</b> to move it "
        "down in the list.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_hold()));
    btn = new QPushButton(KIcon("exec"), i18n("Resume"), hbox1);
    btn->setObjectName("job_resume");
    wt = i18n(
        "<p>Resumes a paused job or changes a Queued job to Waiting.  If the job is the "
        "top speakable job in the list, it begins speaking.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_resume()));
    btn = new QPushButton(KIcon("edit-redo"), i18n("R&estart"), hbox1);
    btn->setObjectName("job_restart");
    wt = i18n(
        "<p>Rewinds a job to the beginning and changes its state to Waiting.  If the job "
        "is the top speakable job in the list, it begins speaking.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_restart()));
    btn = new QPushButton(KIcon("user-trash"), i18n("Re&move"), hbox1);
    btn->setObjectName("job_remove");
    wt = i18n(
        "<p>Deletes the job.  If it is currently speaking, it stops speaking.  The next "
        "speakable job in the list begins speaking.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_remove()));
    btn = new QPushButton(KIcon("go-down"), i18n("&Later"), hbox1);
    btn->setObjectName("job_later");
    wt = i18n(
        "<p>Moves a job downward in the list so that it will be spoken later.  If the job "
        "is currently speaking, its state changes to Paused.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_move()));

    btn = new QPushButton(KIcon("arrow-left"), i18n("&Previous Sentence"), hbox2);
    btn->setObjectName("job_prevsentence");
    wt = i18n(
        "<p>Rewinds a job to the previous sentence.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_prev_sen()));
    btn = new QPushButton(KIcon("arrow-right"), i18n("&Next Sentence"), hbox2);
    btn->setObjectName("job_nextsentence");
    wt = i18n(
        "<p>Advances a job to the next sentence.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_next_sen()));

    btn = new QPushButton(KIcon("klipper"), i18n("&Speak Clipboard"), hbox3);
    btn->setObjectName("speak_clipboard");
    wt = i18n(
        "<p>Queues the current contents of the clipboard for speaking and sets its state "
        "to Waiting.  If the job is the topmost in the list, it begins speaking.  "
        "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_speak_clipboard()));
    btn = new QPushButton(KIcon("document-open"), i18n("Spea&k File"), hbox3);
    btn->setObjectName("speak_file");
    wt = i18n(
        "<p>Prompts you for a file name and queues the contents of the file for speaking.  "
        "You must click the <b>Resume</b> button before the job will be speakable.  "
        "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_speak_file()));
    btn = new QPushButton(KIcon("translate"), i18n("Change Talker"), hbox3);
    btn->setObjectName("job_changetalker");
    wt = i18n(
        "<p>Prompts you with a list of your configured Talkers from the <b>Talkers</b> tab.  "
        "The job will be spoken using the selected Talker.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_change_talker()));
    btn = new QPushButton(KIcon("reload_page"), i18n("&Refresh"), hbox3);
    btn->setObjectName("refresh");
    wt = i18n(
        "<p>Refresh the list of jobs.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_refresh()));

    // Disable job buttons until a job is selected.
    enableJobActions(false);

    // Create a VBox for the current sentence and sentence label.
    KVBox* sentenceVBox = new KVBox(bottomBox);

    // Create a label for current sentence.
    QLabel* currentSentenceLabel = new QLabel(sentenceVBox);
    currentSentenceLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    currentSentenceLabel->setText(i18n("Current Sentence"));

    // Create a box to contain the current sentence.
    m_currentSentence = new KTextEdit(sentenceVBox);
    m_currentSentence->setReadOnly(true);
    m_currentSentence->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_currentSentence->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_currentSentence->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wt = i18n(
        "<p>The text of the sentence currently speaking.</p>");
    m_currentSentence->setWhatsThis(wt);

    // Set the main widget for the part.
    setWidget(vBox);

    connect(m_jobListView, SIGNAL(clicked(const QModelIndex&)),
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

    m_extension = new KttsJobMgrBrowserExtension(this);

    // Divide splitter in half.  ListView gets half.  Buttons and Current Sentence get half.
    int halfSplitterSize = splitter->height()/2;
    QList<int> splitterSizes;
    splitterSizes.append(halfSplitterSize);
    splitterSizes.append(halfSplitterSize);
    splitter->setSizes(splitterSizes);
}

KttsJobMgrPart::~KttsJobMgrPart()
{
    KGlobal::locale()->removeCatalog("kttsd");
    closeUrl();
    delete m_jobListModel;
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
    // Enable job buttons.
    enableJobActions(true);
}

/**
* Slots connected to buttons.
*/
void KttsJobMgrPart::slot_job_hold()
{
    m_kspeech->pause();
}

void KttsJobMgrPart::slot_job_resume()
{
    m_kspeech->resume();
}

void KttsJobMgrPart::slot_job_restart()
{
    int jobNum = getCurrentJobNum();
    // kDebug() << "KttsJobMgrPart::slot_job_restart: jobNum = " << jobNum;
    if (jobNum)
    {
        int seq = m_kspeech->moveRelSentence(jobNum, 0);
        m_kspeech->moveRelSentence(jobNum, -seq);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_prev_sen()
{
    int jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveRelSentence(jobNum, -1);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_next_sen()
{
    int jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveRelSentence(jobNum, 1);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_remove()
{
    int jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->removeJob(jobNum);
        m_currentSentence->clear();
    }
}

void KttsJobMgrPart::slot_job_move()
{
    int jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveJobLater(jobNum);
        refreshJobList();
        // Select the job we just moved.
        QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
        if (index.isValid())
        {
            m_jobListView->setCurrentIndex(index);
            slot_jobListView_clicked();
        }
    }
}

void KttsJobMgrPart::slot_job_change_talker()
{
    QModelIndex index = m_jobListView->currentIndex();
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        QString talkerID = job.talkerID;
        QStringList talkerIDs = m_talkerCodesToTalkerIDs.values();
        int ndx = talkerIDs.indexOf(talkerID);
        QString talkerCode;
        if (ndx >= 0) talkerCode = m_talkerCodesToTalkerIDs.keys()[ndx];
        SelectTalkerDlg dlg(widget(), "selecttalkerdialog", i18n("Select Talker"), talkerCode, true);
        int dlgResult = dlg.exec();
        if (dlgResult != KDialog::Accepted) return;
        talkerCode = dlg.getSelectedTalkerCode();
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

void KttsJobMgrPart::slot_refresh()
{
    // Clear TalkerID cache.
    m_talkerCodesToTalkerIDs.clear();
    // Get current job number.
    int jobNum = getCurrentJobNum();
    refreshJobList();
    // Select the previously-selected job.
    if (jobNum)
    {
        QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
        if (index.isValid())
        {
            m_jobListView->setCurrentIndex(index);
            slot_jobListView_clicked();
        }
    }
}


/**
* Get the Job Number of the currently-selected job in the Job List View.
* @return               Job Number of currently-selected job.
*                       0 if no currently-selected job.
*/
int KttsJobMgrPart::getCurrentJobNum()
{
    int jobNum = 0;
    QModelIndex index = m_jobListView->currentIndex();
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
    enableJobActions(false);
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
    if (m_jobListView->currentIndex().isValid()) return;
    // If empty, disable job buttons.
    
    if (m_jobListModel->rowCount() == 0)
        enableJobActions(false);
    else
    {
        // Select first item.
        m_jobListView->setCurrentIndex(m_jobListModel->index(0, 0));
        slot_jobListView_clicked();
    }
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

/**
* Enables or disables all the job-related buttons.
* @param enable        True to enable the job-related butons.  False to disable.
*/
void KttsJobMgrPart::enableJobActions(bool enable)
{
    if (!m_buttonBox) return;

#if defined Q_CC_MSVC && _MSC_VER < 1300
    QList<QPushButton *> l = qfindChildren<QPushButton *>( m_buttonBox, QRegExp("job_*") );
#else
    QList<QPushButton *> l = m_buttonBox->findChildren<QPushButton *>( QRegExp("job_*") );
#endif
    QListIterator<QPushButton *> i(l);

    while (i.hasNext())
        (i.next())->setEnabled( enable );

    if (enable)
    {
        // Later button only enables if currently selected list item is not bottom of list.
        QModelIndex index = m_jobListView->currentIndex();
        if (index.isValid())
        {
            bool enableLater = (index.row() < m_jobListModel->rowCount());

#if defined Q_CC_MSVC && _MSC_VER < 1300
            l = qfindChildren<QPushButton *>( m_buttonBox, "job_later" );
#else
            l = m_buttonBox->findChildren<QPushButton *>( "job_later" );
#endif
            QListIterator<QPushButton *> it(l); // iterate over the buttons
            while (it.hasNext())
                // for each found object...
                (it.next())->setEnabled( enableLater );
        }
    }
}

/** Slots connected to DBUS Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
Q_SCRIPTABLE void KttsJobMgrPart::kttsdStarted() { slot_refresh(); }


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
                        m_jobListView->setCurrentIndex(m_jobListModel->jobNumToIndex(jobNum));
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
            m_currentSentence->setPlainText(QString());
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
            m_currentSentence->setPlainText(m_kspeech->getJobSentence(jobNum, seq));
        }
    }
    if (KSpeech::mtSentenceEnd == markerType) {
        m_currentSentence->setPlainText(QString());
    }
}


KttsJobMgrBrowserExtension::KttsJobMgrBrowserExtension(KttsJobMgrPart *parent)
    : KParts::BrowserExtension(parent)
{
}

KttsJobMgrBrowserExtension::~KttsJobMgrBrowserExtension()
{
}
