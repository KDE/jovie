//
// C++ Implementation: kttsjobmgr
//
// Description: 
//
//
// Author: Gary Cramblitt <garycramblitt@comcast.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"

#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <klistview.h>
#include <kaction.h>
#include <qdockarea.h>
#include <qvbox.h>
#include <qgrid.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qclipboard.h>
#include <kfiledialog.h>
#include <kapplication.h>

#include "kspeech.h"

extern "C"
{
    /**
    * This function is the 'main' function of this part.  It takes
    * the form 'void *init_lib<library name>()  It always returns a
    * new factory object
    */
    void *init_libkttsjobmgr()
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
  KAboutData *about = new KAboutData("KttsJobMgr", I18N_NOOP("KttsJobMgr"), "1.99");
  return about;
}

KttsJobMgrPart::KttsJobMgrPart(QWidget *parent, const char *name) :
    DCOPStub("kttsd", "kspeech"),
    DCOPObject("kttsjobmgr_kspeechsink"),
    KParts::ReadOnlyPart(parent, name)
{
    // Initialize some variables.
    selectOnTextSet = false;
    
    setInstance(KttsJobMgrFactory::instance());
    
    // Create a QVBox to host everything.
    QVBox* vBox = new QVBox(parent);
    
    // Create a QDockArea to host the toolbar.
    QDockArea* toolBarDockArea = new QDockArea(Qt::Horizontal, QDockArea::Normal, vBox, "jobmgr_toolbar_dockarea");
    
    // Create three KToolBars.
    m_toolBar1 = new KToolBar(vBox, "jobmgr_toolbar1");
    m_toolBar1->setIconText(KToolBar::IconTextRight);
    m_toolBar1->setTitle(i18n("Text-to-Speech Job Manager Toolbar"));
    m_toolBar1->setMovingEnabled(true);
    // This is a temporary workaround until these toolbars size correctly when floated.
    m_toolBar1->setResizeEnabled(true);
    m_toolBar2 = new KToolBar(vBox, "jobmgr_toolbar2");
    m_toolBar2->setIconText(KToolBar::IconTextRight);
    m_toolBar2->setTitle(i18n("Text-to-Speech Job Manager Toolbar"));
    m_toolBar2->setMovingEnabled(true);
    m_toolBar2->setResizeEnabled(true);
    m_toolBar3 = new KToolBar(vBox, "jobmgr_toolbar3");
    m_toolBar3->setIconText(KToolBar::IconTextRight);
    m_toolBar3->setTitle(i18n("Text-to-Speech Job Manager Toolbar"));
    m_toolBar3->setMovingEnabled(true);
    m_toolBar3->setResizeEnabled(true);
    
    // Add the toolbars to the QDockArea.
    toolBarDockArea->setAcceptDockWindow(m_toolBar1, true);
    toolBarDockArea->moveDockWindow(m_toolBar1);
    toolBarDockArea->setAcceptDockWindow(m_toolBar2, true);
    toolBarDockArea->moveDockWindow(m_toolBar2);
    toolBarDockArea->setAcceptDockWindow(m_toolBar3, true);
    toolBarDockArea->moveDockWindow(m_toolBar3);
    
    // Create a splitter to contain the Job List View and the current sentence.
    QSplitter* splitter = new QSplitter(vBox);
    splitter->setOrientation(QSplitter::Vertical);
    
    // Create Job List View widget.
    m_jobListView = new KListView(splitter, "joblistview");
    m_jobListView->setSelectionModeExt(KListView::Single);
    m_jobListView->addColumn(i18n("Job Num"));
    m_jobListView->addColumn(i18n("Owner"));
    m_jobListView->addColumn(i18n("Language"));
    m_jobListView->addColumn(i18n("State"));
    m_jobListView->addColumn(i18n("Position"));
    m_jobListView->addColumn(i18n("Sentences"));
    m_jobListView->addColumn(i18n("Part Num"));
    m_jobListView->addColumn(i18n("Parts"));
    
    // Do not sort the list.
    m_jobListView->setSorting(-1);
    
    // Create a VBox for the current sentence and sentence label.
    QVBox* sentenceVBox = new QVBox(splitter);
    
    // Create a label for current sentence.
    QLabel* currentSentenceLabel = new QLabel(sentenceVBox);
    currentSentenceLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    currentSentenceLabel->setText(i18n("Current Sentence"));
    
    // Create a box to contain the current sentence.
    m_currentSentence = new QLabel(sentenceVBox);
    m_currentSentence->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_currentSentence->setAlignment(Qt::AlignAuto | Qt::AlignVCenter | Qt::WordBreak);
    
    // Set the main widget for the part.
    setWidget(vBox);
    
    // Set up toolbar buttons.
    setupActions();
    
    connect(m_jobListView, SIGNAL(selectionChanged(QListViewItem* )),
        this, SLOT(slot_selectionChanged(QListViewItem* )));

    // Fill the Job List View.
    refreshJobListView();
    // Select first item (if any).
    autoSelectInJobListView();
    
    // Connect DCOP Signals emitted by KTTSD to our own DCOP methods.
    connectDCOPSignal("kttsd", "kspeech",
        "kttsdStarted()",
        "kttsdStarted()",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "markerSeen(QCString,QString)",
        "markerSeen(QCString,QString)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "sentenceStarted(QCString,uint,uint)",
        "sentenceStarted(QCString,uint,uint)",
        false);
    connectDCOPSignal(0, 0,
        "sentenceFinished(QCString,uint,uint)",
        "sentenceFinished(QCString,uint,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textSet(QCString,uint)",
        "textSet(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textStarted(QCString,uint)",
        "textStarted(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textFinished(QCString,uint)",
        "textFinished(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textStopped(QCString,uint)",
        "textStopped(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textPaused(QCString,uint)",
        "textPaused(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textResumed(QCString,uint)",
        "textResumed(QCString,uint)",
        false);
    connectDCOPSignal("kttsd", "kspeech",
        "textRemoved(QCString,uint)",
        "textRemoved(QCString,uint)",
        false);
            
    m_extension = new KttsJobMgrBrowserExtension(this);

    m_jobListView->show();
}

KttsJobMgrPart::~KttsJobMgrPart()
{
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
* Set up toolbar.
*/
void KttsJobMgrPart::setupActions()
{
//    setXMLFile("kttsjobmgrui.rc");
    
    KAction* act;
    
    // All the buttons with "job_" at start of their names will be enabled/disabled when a job is
    // selected in the Job List View.
    // All the buttons with "part_" at the start of their names will be enabled/disabled when a
    // job is selected in the Job List View that has multiple parts.
    
    act = new KAction(i18n("&Hold"),
        KGlobal::iconLoader()->loadIconSet("stop", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_hold()), actionCollection(), "job_hold");
    act->plug(m_toolBar1);
    act = new KAction(i18n("&Resume"),
        KGlobal::iconLoader()->loadIconSet("exec", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_resume()), actionCollection(), "job_resume");
    act->plug(m_toolBar1);
    act = new KAction(i18n("R&estart"),
        KGlobal::iconLoader()->loadIconSet("redo", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_restart()), actionCollection(), "job_restart");
    act->plug(m_toolBar1);
    act = new KAction(i18n("Remove"),
        KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_remove()), actionCollection(), "job_remove");
    act->plug(m_toolBar1);
    act = new KAction(i18n("Later"),
        KGlobal::iconLoader()->loadIconSet("down", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_move()), actionCollection(), "job_move");
    act->plug(m_toolBar1);
        // TODO: Remove "todo_" from the name to enable this button when it is implemented.
    act = new KAction(i18n("Change Language"),
        KGlobal::iconLoader()->loadIconSet("translate", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_change_talker()), actionCollection(), "todo_job_change_talker");
    act->setEnabled(false);
    act->plug(m_toolBar1);

    act = new KAction(i18n("Previous Part"),
        KGlobal::iconLoader()->loadIconSet("2leftarrow", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_prev_par()), actionCollection(), "part_prev_par");
    act->plug(m_toolBar2);
    act = new KAction(i18n("Previous Sentence"),
        KGlobal::iconLoader()->loadIconSet("1leftarrow", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_prev_sen()), actionCollection(), "job_prev_sen");
    act->plug(m_toolBar2);
    act = new KAction(i18n("Next Sentence"),
        KGlobal::iconLoader()->loadIconSet("1rightarrow", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_next_sen()), actionCollection(), "job_next_sen");
    act->plug(m_toolBar2);
    act = new KAction(i18n("Next Part"),
        KGlobal::iconLoader()->loadIconSet("2rightarrow", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_next_par()), actionCollection(), "part_next_par");
    act->plug(m_toolBar2);
    
    act = new KAction(i18n("Speak Clipboard"),
        KGlobal::iconLoader()->loadIconSet("klipper", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_speak_clipboard()), actionCollection(), "speak_clipboard");
    act->plug(m_toolBar3);
    act = new KAction(i18n("Speak File"),
        KGlobal::iconLoader()->loadIconSet("fileopen", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_speak_file()), actionCollection(), "speak_file");
    act->plug(m_toolBar3);
    act = new KAction(i18n("Refresh"),
        KGlobal::iconLoader()->loadIconSet("reload_page", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_refresh()), actionCollection(), "refresh");
    act->plug(m_toolBar3);
    
    // Disable job buttons until a job is selected.
//    stateChanged("no_job_selected");
    enableJobActions(false);
    enableJobPartActions(false);
}

/**
* This slot is connected to the Job List View selectionChanged signal.
*/
void KttsJobMgrPart::slot_selectionChanged(QListViewItem*)
{
    // Enable job buttons.
//    stateChanged("job_selected");
    enableJobActions(true);
    enableJobPartActions((getCurrentJobPartCount() > 1));
}

/**
* Slots connected to toolbar buttons.
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
    kdDebug() << "Running KttsJobMgr::slot_job_restart" << endl;
    uint jobNum = getCurrentJobNum();
    kdDebug() << "jobNum: " << jobNum << endl;
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
    // TODO:
}

void KttsJobMgrPart::slot_speak_clipboard()
{
    // Get the clipboard object.
    QClipboard *cb = kapp->clipboard();

    // Copy text from the clipboard.
    QString text = cb->text();
    
    // Speak it.
    if ( !text.isNull() ) 
    {
        uint jobNum = setText(text, NULL);
        kdDebug() << "In KttsJobMgrPart::slot_speak_clipboard, started jobNum " << jobNum << endl;
        startText(jobNum);
        // Set flag so that the job we just created will be selected when textSet signal is received.
        selectOnTextSet = true;        
    }
}

void KttsJobMgrPart::slot_speak_file()
{
    QString filename = KFileDialog::getOpenFileName();
    setFile(filename, NULL);
}

void KttsJobMgrPart::slot_refresh()
{
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
        case kspeech::jsQueued: return        i18n("Queued");
        case kspeech::jsSpeakable: return     i18n("Waiting");
        case kspeech::jsSpeaking: return      i18n("Speaking");
        case kspeech::jsPaused: return        i18n("Paused");
        case kspeech::jsFinished: return      i18n("Finished");
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
    QString language;
    int seq;
    int sentenceCount;
    int partNum;
    int partCount;
    stream >> state;
    stream >> appId;
    stream >> language;
    stream >> seq;
    stream >> sentenceCount;
    stream >> partNum;
    stream >> partCount;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcLanguage, language);
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
    kdDebug() << "Running KttsJobMgrPart::refreshJobListView" << endl;
    m_jobListView->clear();
    enableJobActions(false);
    enableJobPartActions(false);
    QString jobNumbers = getTextJobNumbers();
    kdDebug() << "jobNumbers: " << jobNumbers << endl;
    QStringList jobNums = QStringList::split(",", jobNumbers);
    QListViewItem* lastItem = 0;
    QStringList::ConstIterator endJobNums(jobNums.constEnd());
    for( QStringList::ConstIterator it = jobNums.constBegin(); it != endJobNums; ++it)
    {
        QString jobNumStr = *it;
        kdDebug() << "jobNumStr: " << jobNumStr << endl;
        uint jobNum = jobNumStr.toUInt(0, 10);
        QByteArray jobInfo = getTextJobInfo(jobNum);
        QDataStream stream(jobInfo, IO_ReadOnly);
        int state;
        QCString appId;
        QString language;
        int seq;
        int sentenceCount;
        int partNum;
        int partCount;
        stream >> state;
        stream >> appId;
        stream >> language;
        stream >> seq;
        stream >> sentenceCount;
        stream >> partNum;
        stream >> partCount;
        // Append to list.
        if (lastItem)
            lastItem = new QListViewItem(m_jobListView, lastItem, jobNumStr, appId, language, 
                stateToStr(state), QString::number(seq), QString::number(sentenceCount),
                QString::number(partNum), QString::number(partCount));
        else
            lastItem = new QListViewItem(m_jobListView, jobNumStr, appId, language, 
                stateToStr(state), QString::number(seq), QString::number(sentenceCount),
                QString::number(partNum), QString::number(partCount));
    }
}
    
/**
* If nothing selected in Job List View and list not empty, select top item.
* If nothing selected and list is empty, disable job buttons on toolbar.
*/
void KttsJobMgrPart::autoSelectInJobListView()
{
    // If something selected, nothing to do.
    if (m_jobListView->selectedItem()) return;
    // If empty, disable job buttons on toolbar.
    QListViewItem* item = m_jobListView->firstChild();
    if (!item)
    {
        enableJobActions(false);
        enableJobPartActions(false);
    }
    else
        // Select first item.  Should fire itemSelected event which will enable job buttons on toolbar.
        m_jobListView->setSelected(item, true);
}

/**
* Enables or disables all the job-related buttons on the toolbar.
* @param enable        True to enable the job-related butons.  False to disable.
*/
void KttsJobMgrPart::enableJobActions(bool enable)
{
    for (uint index = 0; index < actionCollection()->count(); ++index)
    {
        KAction* act = actionCollection()->action(index);
        if (act)
        {
            QString actionName = act->name();
            if (actionName.left(4) == "job_") act->setEnabled(enable);
        }
    }
}

/**
* Enables or disables all the job part-related buttons on the toolbar.
* @param enable        True to enable the job par-related butons.  False to disable.
*/
void KttsJobMgrPart::enableJobPartActions(bool enable)
{
    for (uint index = 0; index < actionCollection()->count(); ++index)
    {
        KAction* act = actionCollection()->action(index);
        if (act)
        {
            QString actionName = act->name();
            if (actionName.left(5) == "part_") act->setEnabled(enable);
        }
    }
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
    kdDebug() << "Running KttsJobMgrPart::sentencedStarted with jobNum " << jobNum << endl;
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsSpeaking));
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
ASYNC KttsJobMgrPart::sentenceFinished(const QCString&, const uint jobNum, const uint)
{
    kdDebug() << "Running KttsJobMgrPart::sentencedFinished with jobNum " << jobNum << endl;
/*
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsSpeaking));
    }
*/
}

/**
* This signal is emitted whenever a new text job is added to the queue.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textSet(const QCString&, const uint jobNum)
{
    QByteArray jobInfo = getTextJobInfo(jobNum);
    QDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    QCString appId;
    QString language;
    int seq;
    int sentenceCount;
    int partNum;
    int partCount;
    stream >> state;
    stream >> appId;
    stream >> language;
    stream >> seq;
    stream >> sentenceCount;
    stream >> partNum;
    stream >> partCount;
    QListViewItem* item = new QListViewItem(m_jobListView, m_jobListView->lastItem(), 
        QString::number(jobNum), appId, language, 
        stateToStr(state), QString::number(seq), QString::number(sentenceCount),
        QString::number(partNum), QString::number(partCount));
    // Should we select this job?
    if (selectOnTextSet)
    {
        m_jobListView->setSelected(item, true);
        selectOnTextSet = false;
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
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStarted(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsSpeaking));
        item->setText(jlvcPosition, "1");
    }
}

/**
* This signal is emitted whenever a text job is finished.  The job has
* been marked for deletion from the queue and will be deleted when another
* job reaches the Finished state. (Only one job in the text queue may be
* in state Finished at one time.)  If @ref startText or @ref resumeText is
* called before the job is deleted, it will remain in the queue for speaking.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textFinished(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsFinished));
        // Update sentence pointer, since signal may not be emitted for final CR.
        refreshJob(jobNum);
    }
}

/**
* This signal is emitted whenever a speaking text job stops speaking.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStopped(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsQueued));
        item->setText(jlvcPosition, "0");
    }
}

/**
* This signal is emitted whenever a speaking text job is paused.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textPaused(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsPaused));
    }
}

/**
* This signal is emitted when a text job, that was previously paused, resumes speaking.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textResumed(const QCString&, const uint jobNum)
{
    QListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(kspeech::jsSpeaking));
    }
}

/**
* This signal is emitted whenever a text job is deleted from the queue.
* The job is no longer in the queue when this signal is emitted.
* @param appId          The DCOP senderId of the application that created the job.  NULL if kttsd.
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
