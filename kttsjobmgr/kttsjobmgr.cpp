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
#include <QObject>
#include <QLabel>
#include <QSplitter>
#include <QClipboard>
#include <QPushButton>
#include <QList>
#include <QTreeView>
#include <QMimeData>

// KDE includes.
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kapplication.h>
#include <kinputdialog.h>
#include <ktextedit.h>
#include <kvbox.h>
#include <kdemacros.h>
#include <kparts/genericfactory.h>
#include <QtDBus>

// KTTS includes.
#include "kspeechdef.h"
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
    KAboutData *about = new KAboutData("kttsjobmgr", I18N_NOOP("KttsJobMgr"), "1.99");
    return about;
}

KttsJobMgrPart::KttsJobMgrPart(QWidget *parentWidget, QObject *parent, const QStringList& args) :
    KParts::ReadOnlyPart(parent),
    m_kspeech(QDBus::sessionBus().findInterface<org::kde::KSpeech>("org.kde.kttsd", "/org/kde/KSpeech"))
{
//DBusAbstractInterfacePrivate
    Q_UNUSED(args);
    m_kspeech->setParent(this);

    // Initialize some variables.
    m_selectOnTextSet = false;
    m_buttonBox = 0;

    // All the ktts components use the same catalogue.
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
    // All the buttons with "part_" at the start of their names will be enabled/disabled when a
    // job is selected in the Job List View that has multiple parts.

    QPushButton* btn;
    QString wt;
    btn = new QPushButton(KIcon("stop"), i18n("Hold"), hbox1);
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
    btn = new QPushButton(KIcon("redo"), i18n("R&estart"), hbox1);
    btn->setObjectName("job_restart");
    wt = i18n(
        "<p>Rewinds a job to the beginning and changes its state to Waiting.  If the job "
        "is the top speakable job in the list, it begins speaking.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_restart()));
    btn = new QPushButton(KIcon("edittrash"), i18n("Re&move"), hbox1);
    btn->setObjectName("job_remove");
    wt = i18n(
        "<p>Deletes the job.  If it is currently speaking, it stops speaking.  The next "
        "speakable job in the list begins speaking.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_remove()));
    btn = new QPushButton(KIcon("down"), i18n("&Later"), hbox1);
    btn->setObjectName("job_later");
    wt = i18n(
        "<p>Moves a job downward in the list so that it will be spoken later.  If the job "
        "is currently speaking, its state changes to Paused.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_move()));

    btn = new QPushButton(KIcon("2leftarrow"), i18n("Pre&vious Part"), hbox2);
    btn->setObjectName("part_prevpart");
    wt = i18n(
        "<p>Rewinds a multi-part job to the previous part.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_prev_par()));
    btn = new QPushButton(KIcon("1leftarrow"), i18n("&Previous Sentence"), hbox2);
    btn->setObjectName("job_prevsentence");
    wt = i18n(
        "<p>Rewinds a job to the previous sentence.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_prev_sen()));
    btn = new QPushButton(KIcon("1rightarrow"), i18n("&Next Sentence"), hbox2);
    btn->setObjectName("job_nextsentence");
    wt = i18n(
        "<p>Advances a job to the next sentence.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_next_sen()));
    btn = new QPushButton(KIcon("2rightarrow"), i18n("Ne&xt Part"), hbox2);
    btn->setObjectName("part_nextpart");
    wt = i18n(
        "<p>Advances a multi-part job to the next part.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_next_par()));

    btn = new QPushButton(KIcon("klipper"), i18n("&Speak Clipboard"), hbox3);
    btn->setObjectName("speak_clipboard");
    wt = i18n(
        "<p>Queues the current contents of the clipboard for speaking and sets its state "
        "to Waiting.  If the job is the topmost in the list, it begins speaking.  "
        "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    btn->setWhatsThis(wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_speak_clipboard()));
    btn = new QPushButton(KIcon("fileopen"), i18n("Spea&k File"), hbox3);
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
    enableJobPartActions(false);

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
    connect(m_kspeech, SIGNAL(markerSeen(QString&,QString&)),
        this, SLOT(markerSeen(QString&,QString&)));
    connect(m_kspeech, SIGNAL(sentenceStarted(QString&,uint,uint)),
        this, SLOT(sentenceStarted(QString&,uint,uint)));
    connect(m_kspeech, SIGNAL(sentenceFinished(QString&,uint,uint)),
        this, SLOT(sentenceFinished(QString&,uint,uint)));
    connect(m_kspeech, SIGNAL(textSet(QString&,uint)),
        this, SLOT(textSet(QString&,uint)));
    connect(m_kspeech, SIGNAL(textStarted(QString&,uint)),
        this, SLOT(textStarted(QString&,uint)));
    connect(m_kspeech, SIGNAL(textFinished(QString&,uint)),
        this, SLOT(textFinished(QString&,uint)));
    connect(m_kspeech, SIGNAL(textStopped(QString&,uint)),
        this, SLOT(textStopped(QString&,uint)));
    connect(m_kspeech, SIGNAL(textPaused(QString&,uint)),
        this, SLOT(textPaused(QString&,uint)));
    connect(m_kspeech, SIGNAL(textResumed(QString&,uint)),
        this, SLOT(textResumed(QString&,uint)));
    connect(m_kspeech, SIGNAL(textRemoved(QString&,uint)),
        this, SLOT(textRemoved(QString&,uint)));

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
    closeURL();
    delete m_jobListModel;
}

bool KttsJobMgrPart::openFile()
{
    return true;
}

bool KttsJobMgrPart::closeURL()
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
    enableJobPartActions((getCurrentJobPartCount() > 1));
}

/**
* Slots connected to buttons.
*/
void KttsJobMgrPart::slot_job_hold()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->pauseText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_resume()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->resumeText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_restart()
{
    uint jobNum = getCurrentJobNum();
    // kDebug() << "KttsJobMgrPart::slot_job_restart: jobNum = " << jobNum << endl;
    if (jobNum)
    {
        m_kspeech->startText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_prev_par()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        // Get current part number.
        uint partNum = m_kspeech->jumpToTextPart(0, jobNum);
        if (partNum > 1) m_kspeech->jumpToTextPart(--partNum, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_prev_sen()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveRelTextSentence(-1, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_next_sen()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveRelTextSentence(1, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_next_par()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        // Get current part number.
        uint partNum = m_kspeech->jumpToTextPart(0, jobNum);
        m_kspeech->jumpToTextPart(++partNum, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_remove()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->removeText(jobNum);
        m_currentSentence->clear();
    }
}

void KttsJobMgrPart::slot_job_move()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        m_kspeech->moveTextLater(jobNum);
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
        m_kspeech->changeTextTalker(talkerCode, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_speak_clipboard()
{
    // kDebug() << "KttsJobMgrPart::slot_speak_clipboard: running" << endl;
    
    // Get the clipboard object.
    QClipboard *cb = kapp->clipboard();

    // Copy text from the clipboard.
    QString text;
    const QMimeData* data = cb->mimeData();
    if (data)
    {
        if (data->hasFormat("text/html"))
        {
            if (m_kspeech->supportsMarkup(NULL, KSpeech::mtHtml))
                text = data->html();
        }
        if (data->hasFormat("text/ssml"))
        {
            if (m_kspeech->supportsMarkup(NULL, KSpeech::mtSsml))
            {
                QByteArray d = data->data("text/ssml");
                text = QString(d);
            }
        }
    }
    if (text.isEmpty())
        text = cb->text();

    // Speak it.
    if ( !text.isEmpty() )
    {
        uint jobNum = m_kspeech->setText(text, "");
        // kDebug() << "KttsJobMgrPart::slot_speak_clipboard: started jobNum " << jobNum << endl;
        m_kspeech->startText(jobNum);
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
        m_kspeech->setFile(result.fileNames[0], NULL, result.encoding);
    }
}

void KttsJobMgrPart::slot_refresh()
{
    // Clear TalkerID cache.
    m_talkerCodesToTalkerIDs.clear();
    // Get current job number.
    uint jobNum = getCurrentJobNum();
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
uint KttsJobMgrPart::getCurrentJobNum()
{
    uint jobNum = 0;
    QModelIndex index = m_jobListView->currentIndex();
    if (index.isValid())
        jobNum = m_jobListModel->getRow(index.row()).jobNum;
    return jobNum;
}

/**
* Get the number of parts in the currently-selected job in the Job List View.
* @return               Number of parts in currently-selected job.
*                       0 if no currently-selected job.
*/
int KttsJobMgrPart::getCurrentJobPartCount()
{
    int partCount = 0;
    QModelIndex index = m_jobListView->currentIndex();
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        partCount = job.partCount;
    }
    return partCount;
}

/**
* Refresh display of a single job in the JobListView.
* @param jobNum         Job Number.
*/
void KttsJobMgrPart::refreshJob(uint jobNum)
{
    QByteArray jobInfo = m_kspeech->getTextJobInfo(jobNum);
    QDataStream stream(&jobInfo, QIODevice::ReadOnly);
    JobInfo job;
    QString talkerCode;
    job.jobNum = jobNum;
    stream >> job.state;
    stream >> job.appId;
    stream >> talkerCode;
    stream >> job.sentenceNum;
    stream >> job.sentenceCount;
    stream >> job.partNum;
    stream >> job.partCount;
    job.talkerID = cachedTalkerCodeToTalkerID(talkerCode);
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
        m_jobListModel->updateRow(index.row(), job);    
}

/**
* Fill the Job List View.
*/
void KttsJobMgrPart::refreshJobList()
{
    // kDebug() << "KttsJobMgrPart::refreshJobList: Running" << endl;
    m_jobListModel->clear();
    JobInfoList jobInfoList;
    enableJobActions(false);
    enableJobPartActions(false);
    QString jobNumbers = m_kspeech->getTextJobNumbers();
    // kDebug() << "jobNumbers: " << jobNumbers << endl;
    QStringList jobNums = jobNumbers.split(",", QString::SkipEmptyParts);
    for (int ndx = 0; ndx < jobNums.count(); ++ndx)
    {
        QString jobNumStr = jobNums[ndx];
        // kDebug() << "jobNumStr: " << jobNumStr << endl;
        uint jobNum = jobNumStr.toUInt(0, 10);
        QByteArray jobInfo = m_kspeech->getTextJobInfo(jobNum);
        QDataStream stream(&jobInfo, QIODevice::ReadOnly);
        JobInfo job;
        job.jobNum = jobNum;
        QString talkerCode;
        stream >> job.state;
        stream >> job.appId;
        stream >> talkerCode;
        stream >> job.sentenceNum;
        stream >> job.sentenceCount;
        stream >> job.partNum;
        stream >> job.partCount;
        job.talkerID = cachedTalkerCodeToTalkerID(talkerCode);
        jobInfoList.append(job);
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
    {
        enableJobActions(false);
        enableJobPartActions(false);
    }
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
        QString talkerID = m_kspeech->talkerCodeToTalkerId(talkerCode);
        m_talkerCodesToTalkerIDs[talkerCode] = talkerID;
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

/**
* Enables or disables all the job part-related buttons.
* @param enable        True to enable the job par-related butons.  False to disable.
*/
void KttsJobMgrPart::enableJobPartActions(bool enable)
{
    if (!m_buttonBox) return;
#if defined Q_CC_MSVC && _MSC_VER < 1300
    QList<QPushButton *> l = qfindChildren<QPushButton *>( m_buttonBox, QRegExp("part_*") );
#else
    QList<QPushButton *> l = m_buttonBox->findChildren<QPushButton *>( QRegExp("part_*") );
#endif
    QListIterator<QPushButton *> i(l);

    while (i.hasNext())
        (i.next())->setEnabled( enable );
}

/** Slots connected to DBUS Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
Q_ASYNC void KttsJobMgrPart::kttsdStarted() { slot_refresh(); };

/**
* This signal is emitted when the speech engine/plugin encounters a marker in the text.
* @param appId          DCOP application ID of the application that queued the text.
* @param markerName     The name of the marker seen.
* @see markers
*/
Q_ASYNC void KttsJobMgrPart::markerSeen(const QString&, const QString&)
{
}

/**
 * This signal is emitted whenever a sentence begins speaking.
 * @param appId          DCOP application ID of the application that queued the text.
 * @param jobNum         Job number of the text job.
 * @param seq            Sequence number of the text.
 * @see getTextCount
 */
Q_ASYNC void KttsJobMgrPart::sentenceStarted(const QString&, const uint jobNum, const uint seq)
{
    // kDebug() << "KttsJobMgrPart::sentencedStarted: jobNum = " << jobNum << " seq = " << seq << endl;
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        job.state = KSpeech::jsSpeaking;
        job.sentenceNum = seq;
        m_jobListModel->updateRow(index.row(), job);
        m_currentSentence->setPlainText(m_kspeech->getTextJobSentence(jobNum, seq));
    }
}

/**
* This signal is emitted when a sentence has finished speaking.
* @param appId          DCOP application ID of the application that queued the text.
* @param jobNum         Job number of the text job.
* @param seq            Sequence number of the text.
* @see getTextCount
*/
Q_ASYNC void KttsJobMgrPart::sentenceFinished(const QString& /*appId*/, const uint /*jobNum*/, const uint /*seq*/)
{
    // kDebug() << "KttsJobMgrPart::sentencedFinished: jobNum = " << jobNum << endl;
/*
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsSpeaking));
    }
*/
}

/**
* This signal is emitted whenever a new text job is added to the queue.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*
* If the job is already in the list, refreshes it.
*/
Q_ASYNC void KttsJobMgrPart::textSet(const QString&, const uint jobNum)
{
    kDebug() << "KttsJobMgrPart::textSet: jobNum = " << jobNum << endl;
    
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
        refreshJob(jobNum);
    else
    {
        QByteArray jobInfo = m_kspeech->getTextJobInfo(jobNum);
        QDataStream stream(&jobInfo, QIODevice::ReadOnly);
        JobInfo job;
        job.jobNum = jobNum;
        QString talkerCode;
        stream >> job.state;
        stream >> job.appId;
        stream >> talkerCode;
        stream >> job.sentenceNum;
        stream >> job.sentenceCount;
        stream >> job.partNum;
        stream >> job.partCount;
        job.talkerID = cachedTalkerCodeToTalkerID(talkerCode);
        m_jobListModel->appendRow(job);
        // Should we select this job?
        if (m_selectOnTextSet)
        {
            m_jobListView->setCurrentIndex(m_jobListModel->jobNumToIndex(jobNum));
            m_selectOnTextSet = false;
            slot_jobListView_clicked();
        }
    }
    // If a job not already selected, select this one.
    autoSelectInJobListView();
}

/**
* This signal is emitted whenever a new part is appended to a text job.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
* @param partNum        Part number of the new part.  Parts are numbered starting
*                       at 1.
*/
Q_ASYNC void KttsJobMgrPart::textAppended(const QString& appId, const uint jobNum, const int /*partNum*/)
{
    textSet(appId, jobNum);
}

/**
* This signal is emitted whenever speaking of a text job begins.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textStarted(const QString&, const uint jobNum)
{
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        job.state = KSpeech::jsSpeaking;
        job.sentenceNum = 1;
        m_jobListModel->updateRow(index.row(), job);
    }
}

/**
* This signal is emitted whenever a text job is finished.  The job has
* been marked for deletion from the queue and will be deleted when another
* job reaches the Finished state. (Only one job in the text queue may be
* in state Finished at one time.)  If @ref startText or @ref resumeText is
* called before the job is deleted, it will remain in the queue for speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textFinished(const QString&, const uint jobNum)
{
    // kDebug() << "KttsJobMgrPart::textFinished: jobNum = " << jobNum << endl;
    refreshJob(jobNum);
    m_currentSentence->setPlainText(QString());
}

/**
* This signal is emitted whenever a speaking text job stops speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textStopped(const QString&, const uint jobNum)
{
    // kDebug() << "KttsJobMgrPart::textStopped: jobNum = " << jobNum << endl;
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        job.state = KSpeech::jsQueued;
        job.sentenceNum = 1;
        m_jobListModel->updateRow(index.row(), job);
    }
}

/**
* This signal is emitted whenever a speaking text job is paused.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textPaused(const QString&, const uint jobNum)
{
    // kDebug() << "KttsJobMgrPart::textPaused: jobNum = " << jobNum << endl;
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        job.state = KSpeech::jsPaused;
        m_jobListModel->updateRow(index.row(), job);
    }
}

/**
* This signal is emitted when a text job, that was previously paused, resumes speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textResumed(const QString&, const uint jobNum)
{
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
    {
        JobInfo job = m_jobListModel->getRow(index.row());
        job.state = KSpeech::jsSpeaking;
        m_jobListModel->updateRow(index.row(), job);
    }
}

/**
* This signal is emitted whenever a text job is deleted from the queue.
* The job is no longer in the queue when this signal is emitted.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
Q_ASYNC void KttsJobMgrPart::textRemoved(const QString&, const uint jobNum)
{
    QModelIndex index = m_jobListModel->jobNumToIndex(jobNum);
    if (index.isValid())
        m_jobListModel->removeRow(index.row());
    autoSelectInJobListView();
}

KttsJobMgrBrowserExtension::KttsJobMgrBrowserExtension(KttsJobMgrPart *parent)
    : KParts::BrowserExtension(parent)
{
}

KttsJobMgrBrowserExtension::~KttsJobMgrBrowserExtension()
{
}
