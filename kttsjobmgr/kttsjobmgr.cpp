/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, rewind,
  advance, change Talker, etc. 
  -------------------
  Copyright : (C) 2004,2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// QT includes.
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qclipboard.h>
#include <qpushbutton.h>
#include <qobjectlist.h>
#include <qwhatsthis.h>

#include <qmime.h>

// KDE includes.
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kapplication.h>
#include <kinputdialog.h>

// KTTS includes.
#include "kspeech.h"
#include "talkercode.h"
#include "selecttalkerdlg.h"
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"

extern "C"
{
    /**
    * This function is the 'main' function of this part.  It takes
    * the form 'void *init_lib<library name>()  It always returns a
    * new factory object
    */
    KDE_EXPORT void *init_libkttsjobmgrpart()
    {
        return new KttsJobMgrFactory;
    }
};

/**
* We need one static instance of the factory for our C 'main'
* function
*/
KInstance *KttsJobMgrFactory::s_instance = 0L;

KttsJobMgrFactory::~KttsJobMgrFactory()
{
    if (s_instance)
    {
        delete s_instance->aboutData();
        delete s_instance;
    }

    s_instance = 0;
}

QObject *KttsJobMgrFactory::createObject(QObject *parent, const char *name, const char*,
                               const QStringList& )
{
    QObject *obj = new KttsJobMgrPart((QWidget*)parent, name);
    emit objectCreated(obj);
    return obj;
}

KInstance *KttsJobMgrFactory::instance()
{
    if ( !s_instance )
        s_instance = new KInstance( aboutData() );
    return s_instance;
}

KAboutData *KttsJobMgrFactory::aboutData()
{
  KAboutData *about = new KAboutData("kttsjobmgr", I18N_NOOP("KttsJobMgr"), "1.99");
  return about;
}

KttsJobMgrPart::KttsJobMgrPart(QWidget *parent, const char *name) :
    DCOPStub("kttsd", "KSpeech"),
    DCOPObject("kttsjobmgr_kspeechsink"),
    KParts::ReadOnlyPart(parent, name)
{
    // Initialize some variables.
    m_selectOnTextSet = false;
    m_buttonBox = 0;

    setInstance(KttsJobMgrFactory::instance());

    // All the ktts components use the same catalogue.
    KGlobal::locale()->insertCatalogue("kttsd");

    // Create a QVBox to host everything.
    QVBox* vBox = new QVBox(parent);
    vBox->setMargin(6);

    // Create a splitter to contain the Job List View and the current sentence.
    QSplitter* splitter = new QSplitter(vBox);
    splitter->setOrientation(QSplitter::Vertical);

    // Create Job List View widget.
    m_jobListView = new KListView(splitter, "joblistview");
    m_jobListView->setSelectionModeExt(KListView::Single);
    m_jobListView->addColumn(i18n("Job Num"));
    m_jobListView->addColumn(i18n("Owner"));
    m_jobListView->addColumn(i18n("Talker ID"));
    m_jobListView->addColumn(i18n("State"));
    m_jobListView->addColumn(i18n("Position"));
    m_jobListView->addColumn(i18n("Sentences"));
    m_jobListView->addColumn(i18n("Part Num"));
    m_jobListView->addColumn(i18n("Parts"));

    // Do not sort the list.
    m_jobListView->setSorting(-1);

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
    QWhatsThis::add(m_jobListView, jobListViewWT);

    // splitter->setResizeMode(m_jobListView, QSplitter::Stretch);

    // Create a VBox to hold buttons and current sentence.
    QVBox* bottomBox = new QVBox(splitter);

    // Create a box to hold buttons.
    m_buttonBox = new QVBox(bottomBox);
    m_buttonBox->setSpacing(6);

    // Create 3 HBoxes to host buttons.
    QHBox* hbox1 = new QHBox(m_buttonBox);
    hbox1->setSpacing(6);
    QHBox* hbox2 = new QHBox(m_buttonBox);
    hbox2->setSpacing(6);
    QHBox* hbox3 = new QHBox(m_buttonBox);
    hbox3->setSpacing(6);

    // Do not let button box stretch vertically.
    m_buttonBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    // All the buttons with "job_" at start of their names will be enabled/disabled when a job is
    // selected in the Job List View.
    // All the buttons with "part_" at the start of their names will be enabled/disabled when a
    // job is selected in the Job List View that has multiple parts.

    QPushButton* btn;
    QString wt;
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("stop", KIcon::Small, 0, true),
                          i18n("Hold"), hbox1, "job_hold");
    wt = i18n(
            "<p>Changes a job to Paused state.  If currently speaking, the job stops speaking.  "
            "Paused jobs prevent jobs that follow them from speaking, so either click "
            "<b>Resume</b> to make the job speakable, or click <b>Later</b> to move it "
            "down in the list.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_hold()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("exec", KIcon::Small, 0, true),
                          i18n("Resume"), hbox1, "job_resume");
    wt = i18n(
            "<p>Resumes a paused job or changes a Queued job to Waiting.  If the job is the "
            "top speakable job in the list, it begins speaking.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_resume()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("redo", KIcon::Small, 0, true),
                          i18n("R&estart"), hbox1, "job_restart");
    wt = i18n(
            "<p>Rewinds a job to the beginning and changes its state to Waiting.  If the job "
            "is the top speakable job in the list, it begins speaking.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_restart()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small, 0, true),
                          i18n("Re&move"), hbox1, "job_remove");
    wt = i18n(
            "<p>Deletes the job.  If it is currently speaking, it stops speaking.  The next "
            "speakable job in the list begins speaking.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_remove()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("down", KIcon::Small, 0, true),
                          i18n("&Later"), hbox1, "job_later");
    wt = i18n(
            "<p>Moves a job downward in the list so that it will be spoken later.  If the job "
            "is currently speaking, its state changes to Paused.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_move()));

    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("2leftarrow", KIcon::Small, 0, true),
                          i18n("Pre&vious Part"), hbox2, "part_prevpart");
    wt = i18n(
            "<p>Rewinds a multi-part job to the previous part.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_prev_par()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("1leftarrow", KIcon::Small, 0, true),
                          i18n("&Previous Sentence"), hbox2, "job_prevsentence");
    wt = i18n(
            "<p>Rewinds a job to the previous sentence.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_prev_sen()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("1rightarrow", KIcon::Small, 0, true),
                          i18n("&Next Sentence"), hbox2, "job_nextsentence");
    wt = i18n(
            "<p>Advances a job to the next sentence.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_next_sen()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("2rightarrow", KIcon::Small, 0, true),
                          i18n("Ne&xt Part"), hbox2, "part_nextpart");
    wt = i18n(
            "<p>Advances a multi-part job to the next part.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_next_par()));

    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("klipper", KIcon::Small, 0, true),
                          i18n("&Speak Clipboard"), hbox3, "speak_clipboard");
    wt = i18n(
            "<p>Queues the current contents of the clipboard for speaking and sets its state "
            "to Waiting.  If the job is the topmost in the list, it begins speaking.  "
            "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_speak_clipboard()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("fileopen", KIcon::Small, 0, true),
                          i18n("Spea&k File"), hbox3, "speak_file");
    wt = i18n(
            "<p>Prompts you for a file name and queues the contents of the file for speaking.  "
            "You must click the <b>Resume</b> button before the job will be speakable.  "
            "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_speak_file()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("translate", KIcon::Small, 0, true),
                          i18n("Change Talker"), hbox3, "job_changetalker");
    wt = i18n(
            "<p>Prompts you with a list of your configured Talkers from the <b>Talkers</b> tab.  "
            "The job will be spoken using the selected Talker.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_job_change_talker()));
    btn = new QPushButton(KGlobal::iconLoader()->loadIconSet("reload_page", KIcon::Small, 0, true),
                          i18n("&Refresh"), hbox3, "refresh");
    wt = i18n(
            "<p>Refresh the list of jobs.</p>");
    QWhatsThis::add(btn, wt);
    connect (btn, SIGNAL(clicked()), this, SLOT(slot_refresh()));

    // Disable job buttons until a job is selected.
    enableJobActions(false);
    enableJobPartActions(false);

    // Create a VBox for the current sentence and sentence label.
    QVBox* sentenceVBox = new QVBox(bottomBox);

    // Create a label for current sentence.
    QLabel* currentSentenceLabel = new QLabel(sentenceVBox);
    currentSentenceLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    currentSentenceLabel->setText(i18n("Current Sentence"));

    // Create a box to contain the current sentence.
    m_currentSentence = new QLabel(sentenceVBox);
    m_currentSentence->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_currentSentence->setAlignment(Qt::AlignAuto | Qt::AlignTop | Qt::WordBreak);
    wt = i18n(
            "<p>The text of the sentence currently speaking.</p>");
    QWhatsThis::add(m_currentSentence, wt);

    // Set the main widget for the part.
    setWidget(vBox);

    connect(m_jobListView, SIGNAL(selectionChanged(QListViewItem* )),
        this, SLOT(slot_selectionChanged(QListViewItem* )));

    // Fill the Job List View.
    refreshJobListView();
    // Select first item (if any).
    autoSelectInJobListView();

    // Connect DCOP Signals emitted by KTTSD to our own DCOP methods.
    connectDCOPSignal("kttsd", "KSpeech",
        "kttsdStarted()",
        "kttsdStarted()",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "markerSeen(QCString,QString)",
        "markerSeen(QCString,QString)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "sentenceStarted(QCString,uint,uint)",
        "sentenceStarted(QCString,uint,uint)",
        false);
    connectDCOPSignal(0, 0,
        "sentenceFinished(QCString,uint,uint)",
        "sentenceFinished(QCString,uint,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textSet(QCString,uint)",
        "textSet(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textStarted(QCString,uint)",
        "textStarted(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textFinished(QCString,uint)",
        "textFinished(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textStopped(QCString,uint)",
        "textStopped(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textPaused(QCString,uint)",
        "textPaused(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textResumed(QCString,uint)",
        "textResumed(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textRemoved(QCString,uint)",
        "textRemoved(QCString,uint)",
        false);

    m_extension = new KttsJobMgrBrowserExtension(this);

    m_jobListView->show();

    // Divide splitter in half.  ListView gets half.  Buttons and Current Sentence get half.
    int halfSplitterSize = splitter->height()/2;
    QValueList<int> splitterSizes;
    splitterSizes.append(halfSplitterSize);
    splitterSizes.append(halfSplitterSize);
    splitter->setSizes(splitterSizes);
}

KttsJobMgrPart::~KttsJobMgrPart()
{
    KGlobal::locale()->removeCatalogue("kttsd");
    closeURL();
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
* This slot is connected to the Job List View selectionChanged signal.
*/
void KttsJobMgrPart::slot_selectionChanged(QListViewItem*)
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
        pauseText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_resume()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        resumeText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_restart()
{
    uint jobNum = getCurrentJobNum();
    // kdDebug() << "KttsJobMgrPart::slot_job_restart: jobNum = " << jobNum << endl;
    if (jobNum)
    {
        startText(jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_prev_par()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        // Get current part number.
        uint partNum = jumpToTextPart(0, jobNum);
        if (partNum > 1) jumpToTextPart(--partNum, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_prev_sen()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        moveRelTextSentence(-1, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_next_sen()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        moveRelTextSentence(1, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_next_par()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        // Get current part number.
        uint partNum = jumpToTextPart(0, jobNum);
        jumpToTextPart(++partNum, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_job_remove()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        removeText(jobNum);
        m_currentSentence->clear();
    }
}

void KttsJobMgrPart::slot_job_move()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        moveTextLater(jobNum);
        refreshJobListView();
        // Select the job we just moved.
        QListViewItem* item = findItemByJobNum(jobNum);
        if (item) m_jobListView->setSelected(item, true); 
    }
}

void KttsJobMgrPart::slot_job_change_talker()
{
    QListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        QString talkerID = item->text(jlvcTalkerID);
        QStringList talkerIDs = m_talkerCodesToTalkerIDs.values();
        int ndx = talkerIDs.findIndex(talkerID);
        QString talkerCode;
        if (ndx >= 0) talkerCode = m_talkerCodesToTalkerIDs.keys()[ndx];
        SelectTalkerDlg dlg(widget(), "selecttalkerdialog", i18n("Select Talker"), talkerCode, true);
        int dlgResult = dlg.exec();
        if (dlgResult != KDialogBase::Accepted) return;
        talkerCode = dlg.getSelectedTalkerCode();
        int jobNum = item->text(jlvcJobNum).toInt();
        changeTextTalker(talkerCode, jobNum);
        refreshJob(jobNum);
    }
}

void KttsJobMgrPart::slot_speak_clipboard()
{
    // Get the clipboard object.
    QClipboard *cb = kapp->clipboard();


    // Copy text from the clipboard.
    QString text;
    /* This code turned off until reliable xml/html processing can be achieved.
    QMimeSource* data = cb->data();
    if (data)
    {
        if (data->provides("text/html"))
        {
            QByteArray d = data->encodedData("text/html");
            text = QString(d);
            kdDebug() << "text/html: " << text << endl;
        }
    }
    if (text.isEmpty())  */
        text = cb->text();

    // Speak it.
    if ( !text.isEmpty() )
    {
        uint jobNum = setText(text, NULL);
        // kdDebug() << "KttsJobMgrPart::slot_speak_clipboard: started jobNum " << jobNum << endl;
        startText(jobNum);
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
        // kdDebug() << "KttsJobMgr::slot_speak_file: calling setFile with filename " <<
        //     result.fileNames[0] << " and encoding " << result.encoding << endl;
        setFile(result.fileNames[0], NULL, result.encoding);
    }
}

void KttsJobMgrPart::slot_refresh()
{
    // Clear TalkerID cache.
    m_talkerCodesToTalkerIDs.clear();
    // Get current job number.
    uint jobNum = getCurrentJobNum();
    refreshJobListView();
    // Select the previously-selected job.
    if (jobNum)
    {
        QListViewItem* item = findItemByJobNum(jobNum);
        if (item) m_jobListView->setSelected(item, true); 
    }
}


/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
QString KttsJobMgrPart::stateToStr(int state)
{
    switch( state )
    {
        case KSpeech::jsQueued: return        i18n("Queued");
        case KSpeech::jsSpeakable: return     i18n("Waiting");
        case KSpeech::jsSpeaking: return      i18n("Speaking");
        case KSpeech::jsPaused: return        i18n("Paused");
        case KSpeech::jsFinished: return      i18n("Finished");
        default: return                       i18n("Unknown");
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
    QListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        QString jobNumStr = item->text(jlvcJobNum);
        jobNum = jobNumStr.toUInt(0, 10);
    }
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
    QListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        QString partCountStr = item->text(jlvcPartCount);
        partCount = partCountStr.toUInt(0, 10);
    }
    return partCount;
}

/**
* Given a Job Number, returns the Job List View item containing the job.
* @param jobNum         Job Number.
* @return               QListViewItem containing the job or 0 if not found.
*/
QListViewItem* KttsJobMgrPart::findItemByJobNum(const uint jobNum)
{
    return m_jobListView->findItem(QString::number(jobNum), jlvcJobNum);
}

/**
* Refresh display of a single job in the JobListView.
* @param jobNum         Job Number.
*/
void KttsJobMgrPart::refreshJob(uint jobNum)
{
    QByteArray jobInfo = getTextJobInfo(jobNum);
    QDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    QCString appId;
    QString talker;
    int seq;
    int sentenceCount;
    int partNum;
    int partCount;
    stream >> state;
    stream >> appId;
    stream >> talker;
    stream >> seq;
    stream >> sentenceCount;
    stream >> partNum;
    stream >> partCount;
    QString talkerID = cachedTalkerCodeToTalkerID(talker);
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcTalkerID, talkerID);
        item->setText(jlvcState, stateToStr(state));
        item->setText(jlvcPosition, QString::number(seq));
        item->setText(jlvcSentences, QString::number(sentenceCount));
        item->setText(jlvcPartNum, QString::number(partNum));
        item->setText(jlvcPartCount, QString::number(partCount));
    }
}

/**
* Fill the Job List View.
*/
void KttsJobMgrPart::refreshJobListView()
{
    // kdDebug() << "KttsJobMgrPart::refreshJobListView: Running" << endl;
    m_jobListView->clear();
    enableJobActions(false);
    enableJobPartActions(false);
    QString jobNumbers = getTextJobNumbers();
    // kdDebug() << "jobNumbers: " << jobNumbers << endl;
    QStringList jobNums = QStringList::split(",", jobNumbers);
    QListViewItem* lastItem = 0;
    QStringList::ConstIterator endJobNums(jobNums.constEnd());
    for( QStringList::ConstIterator it = jobNums.constBegin(); it != endJobNums; ++it)
    {
        QString jobNumStr = *it;
        // kdDebug() << "jobNumStr: " << jobNumStr << endl;
        uint jobNum = jobNumStr.toUInt(0, 10);
        QByteArray jobInfo = getTextJobInfo(jobNum);
        QDataStream stream(jobInfo, IO_ReadOnly);
        int state;
        QCString appId;
        QString talkerCode;
        int seq;
        int sentenceCount;
        int partNum;
        int partCount;
        stream >> state;
        stream >> appId;
        stream >> talkerCode;
        stream >> seq;
        stream >> sentenceCount;
        stream >> partNum;
        stream >> partCount;
        QString talkerID = cachedTalkerCodeToTalkerID(talkerCode);
        // Append to list.
        if (lastItem)
            lastItem = new QListViewItem(m_jobListView, lastItem, jobNumStr, appId, talkerID, 
                stateToStr(state), QString::number(seq), QString::number(sentenceCount),
                QString::number(partNum), QString::number(partCount));
        else
            lastItem = new QListViewItem(m_jobListView, jobNumStr, appId, talkerID,
                stateToStr(state), QString::number(seq), QString::number(sentenceCount),
                QString::number(partNum), QString::number(partCount));
    }
}

/**
* If nothing selected in Job List View and list not empty, select top item.
* If nothing selected and list is empty, disable job buttons.
*/
void KttsJobMgrPart::autoSelectInJobListView()
{
    // If something selected, nothing to do.
    if (m_jobListView->selectedItem()) return;
    // If empty, disable job buttons.
    QListViewItem* item = m_jobListView->firstChild();
    if (!item)
    {
        enableJobActions(false);
        enableJobPartActions(false);
    }
    else
        // Select first item.  Should fire itemSelected event which will enable job buttons.
        m_jobListView->setSelected(item, true);
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
        QString talkerID = talkerCodeToTalkerId(talkerCode);
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
    QObjectList *l = m_buttonBox->queryList( "QPushButton", "job_*", true, true );
    QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
        // for each found object...
        ++it;
        ((QPushButton*)obj)->setEnabled( enable );
    }
    delete l; // delete the list, not the objects

    if (enable)
    {
        // Later button only enables if currently selected list item is not bottom of list.
        QListViewItem* item = m_jobListView->selectedItem();
        if (item)
        {
            bool enableLater = item->nextSibling();

            l = m_buttonBox->queryList( "QPushButton", "job_later", false, true );
            it = QObjectListIt( *l ); // iterate over the buttons
            if ( (obj = it.current()) != 0 ) {
                // for each found object...
                ((QPushButton*)obj)->setEnabled( enableLater );
            }
            delete l; // delete the list, not the objects
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
    QObjectList *l = m_buttonBox->queryList( "QPushButton", "part_*", true, true );
    QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
        // for each found object...
        ++it;
        ((QPushButton*)obj)->setEnabled( enable );
    }
    delete l; // delete the list, not the objects
}

/** DCOP Methods connected to DCOP Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
ASYNC KttsJobMgrPart::kttsdStarted() { slot_refresh(); };

/**
* This signal is emitted when the speech engine/plugin encounters a marker in the text.
* @param appId          DCOP application ID of the application that queued the text.
* @param markerName     The name of the marker seen.
* @see markers
*/
ASYNC KttsJobMgrPart::markerSeen(const QCString&, const QString&)
{
}

/**
 * This signal is emitted whenever a sentence begins speaking.
 * @param appId          DCOP application ID of the application that queued the text.
 * @param jobNum         Job number of the text job.
 * @param seq            Sequence number of the text.
 * @see getTextCount
 */
ASYNC KttsJobMgrPart::sentenceStarted(const QCString&, const uint jobNum, const uint seq)
{
    // kdDebug() << "KttsJobMgrPart::sentencedStarted: jobNum = " << jobNum << " seq = " << seq << endl;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsSpeaking));
        item->setText(jlvcPosition, QString::number(seq));
        m_currentSentence->setText(getTextJobSentence(jobNum, seq));
    }
}

/**
* This signal is emitted when a sentence has finished speaking.
* @param appId          DCOP application ID of the application that queued the text.
* @param jobNum         Job number of the text job.
* @param seq            Sequence number of the text.
* @see getTextCount
*/
ASYNC KttsJobMgrPart::sentenceFinished(const QCString& /*appId*/, const uint /*jobNum*/, const uint /*seq*/)
{
    // kdDebug() << "KttsJobMgrPart::sentencedFinished: jobNum = " << jobNum << endl;
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
*/
ASYNC KttsJobMgrPart::textSet(const QCString&, const uint jobNum)
{
    QByteArray jobInfo = getTextJobInfo(jobNum);
    QDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    QCString appId;
    QString talkerCode;
    int seq;
    int sentenceCount;
    int partNum;
    int partCount;
    stream >> state;
    stream >> appId;
    stream >> talkerCode;
    stream >> seq;
    stream >> sentenceCount;
    stream >> partNum;
    stream >> partCount;
    QString talkerID = cachedTalkerCodeToTalkerID(talkerCode);
    QListViewItem* item = new QListViewItem(m_jobListView, m_jobListView->lastItem(), 
        QString::number(jobNum), appId, talkerID, 
        stateToStr(state), QString::number(seq), QString::number(sentenceCount),
        QString::number(partNum), QString::number(partCount));
    // Should we select this job?
    if (m_selectOnTextSet)
    {
        m_jobListView->setSelected(item, true);
        m_selectOnTextSet = false;
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
ASYNC KttsJobMgrPart::textAppended(const QCString& appId, const uint jobNum, const int /*partNum*/)
{
    textSet(appId, jobNum);
}

/**
* This signal is emitted whenever speaking of a text job begins.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStarted(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsSpeaking));
        item->setText(jlvcPosition, "1");
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
ASYNC KttsJobMgrPart::textFinished(const QCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textFinished: jobNum = " << jobNum << endl;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsFinished));
        // Update sentence pointer, since signal may not be emitted for final CR.
        refreshJob(jobNum);
    }
    m_currentSentence->setText(QString::null);
}

/**
* This signal is emitted whenever a speaking text job stops speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStopped(const QCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textStopped: jobNum = " << jobNum << endl;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsQueued));
        item->setText(jlvcPosition, "1");
    }
}

/**
* This signal is emitted whenever a speaking text job is paused.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textPaused(const QCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textPaused: jobNum = " << jobNum << endl;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsPaused));
    }
}

/**
* This signal is emitted when a text job, that was previously paused, resumes speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textResumed(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsSpeaking));
    }
}

/**
* This signal is emitted whenever a text job is deleted from the queue.
* The job is no longer in the queue when this signal is emitted.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textRemoved(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    delete item;
    autoSelectInJobListView();
}

KttsJobMgrBrowserExtension::KttsJobMgrBrowserExtension(KttsJobMgrPart *parent)
    : KParts::BrowserExtension(parent, "KttsJobMgrBrowserExtension")
{
}

KttsJobMgrBrowserExtension::~KttsJobMgrBrowserExtension()
{
}
