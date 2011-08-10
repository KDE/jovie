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
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqsplitter.h>
#include <tqclipboard.h>
#include <tqpushbutton.h>
#include <tqobjectlist.h>
#include <tqwhatsthis.h>

#include <tqmime.h>

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
#include <ktextedit.h>

// KTTS includes.
#include "kspeech.h"
#include "talkercode.h"
#include "selecttalkerdlg.h"
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"

K_EXPORT_COMPONENT_FACTORY( libkttsjobmgrpart, KttsJobMgrFactory )

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

TQObject *KttsJobMgrFactory::createObject(TQObject *tqparent, const char *name, const char*,
                               const TQStringList& )
{
    TQObject *obj = new KttsJobMgrPart((TQWidget*)tqparent, name);
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

KttsJobMgrPart::KttsJobMgrPart(TQWidget *tqparent, const char *name) :
    DCOPStub("kttsd", "KSpeech"),
    DCOPObject("kttsjobmgr_kspeechsink"),
    KParts::ReadOnlyPart(TQT_TQOBJECT(tqparent), name)
{
    // Initialize some variables.
    m_selectOnTextSet = false;
    m_buttonBox = 0;

    setInstance(KttsJobMgrFactory::instance());

    // All the ktts components use the same catalogue.
    KGlobal::locale()->insertCatalogue("kttsd");

    // Create a TQVBox to host everything.
    TQVBox* vBox = new TQVBox(tqparent);
    vBox->setMargin(6);

    // Create a splitter to contain the Job List View and the current sentence.
    TQSplitter* splitter = new TQSplitter(vBox);
    splitter->setOrientation(Qt::Vertical);

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

    TQString jobListViewWT = i18n(
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
    TQWhatsThis::add(m_jobListView, jobListViewWT);

    // splitter->setResizeMode(m_jobListView, TQSplitter::Stretch);

    // Create a VBox to hold buttons and current sentence.
    TQVBox* bottomBox = new TQVBox(splitter);

    // Create a box to hold buttons.
    m_buttonBox = new TQVBox(bottomBox);
    m_buttonBox->setSpacing(6);

    // Create 3 HBoxes to host buttons.
    TQHBox* hbox1 = new TQHBox(m_buttonBox);
    hbox1->setSpacing(6);
    TQHBox* hbox2 = new TQHBox(m_buttonBox);
    hbox2->setSpacing(6);
    TQHBox* hbox3 = new TQHBox(m_buttonBox);
    hbox3->setSpacing(6);

    // Do not let button box stretch vertically.
    m_buttonBox->tqsetSizePolicy(TQSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Fixed));

    // All the buttons with "job_" at start of their names will be enabled/disabled when a job is
    // selected in the Job List View.
    // All the buttons with "part_" at the start of their names will be enabled/disabled when a
    // job is selected in the Job List View that has multiple parts.

    TQPushButton* btn;
    TQString wt;
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("stop", KIcon::Small, 0, true),
                          i18n("Hold"), hbox1, "job_hold");
    wt = i18n(
            "<p>Changes a job to Paused state.  If currently speaking, the job stops speaking.  "
            "Paused jobs prevent jobs that follow them from speaking, so either click "
            "<b>Resume</b> to make the job speakable, or click <b>Later</b> to move it "
            "down in the list.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_hold()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("exec", KIcon::Small, 0, true),
                          i18n("Resume"), hbox1, "job_resume");
    wt = i18n(
            "<p>Resumes a paused job or changes a Queued job to Waiting.  If the job is the "
            "top speakable job in the list, it begins speaking.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_resume()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("redo", KIcon::Small, 0, true),
                          i18n("R&estart"), hbox1, "job_restart");
    wt = i18n(
            "<p>Rewinds a job to the beginning and changes its state to Waiting.  If the job "
            "is the top speakable job in the list, it begins speaking.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_restart()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("edittrash", KIcon::Small, 0, true),
                          i18n("Re&move"), hbox1, "job_remove");
    wt = i18n(
            "<p>Deletes the job.  If it is currently speaking, it stops speaking.  The next "
            "speakable job in the list begins speaking.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_remove()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("down", KIcon::Small, 0, true),
                          i18n("&Later"), hbox1, "job_later");
    wt = i18n(
            "<p>Moves a job downward in the list so that it will be spoken later.  If the job "
            "is currently speaking, its state changes to Paused.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_move()));

    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("2leftarrow", KIcon::Small, 0, true),
                          i18n("Pre&vious Part"), hbox2, "part_prevpart");
    wt = i18n(
            "<p>Rewinds a multi-part job to the previous part.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_prev_par()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("1leftarrow", KIcon::Small, 0, true),
                          i18n("&Previous Sentence"), hbox2, "job_prevsentence");
    wt = i18n(
            "<p>Rewinds a job to the previous sentence.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_prev_sen()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("1rightarrow", KIcon::Small, 0, true),
                          i18n("&Next Sentence"), hbox2, "job_nextsentence");
    wt = i18n(
            "<p>Advances a job to the next sentence.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_next_sen()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("2rightarrow", KIcon::Small, 0, true),
                          i18n("Ne&xt Part"), hbox2, "part_nextpart");
    wt = i18n(
            "<p>Advances a multi-part job to the next part.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_next_par()));

    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("klipper", KIcon::Small, 0, true),
                          i18n("&Speak Clipboard"), hbox3, "speak_clipboard");
    wt = i18n(
            "<p>Queues the current contents of the clipboard for speaking and sets its state "
            "to Waiting.  If the job is the topmost in the list, it begins speaking.  "
            "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_speak_clipboard()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("fileopen", KIcon::Small, 0, true),
                          i18n("Spea&k File"), hbox3, "speak_file");
    wt = i18n(
            "<p>Prompts you for a file name and queues the contents of the file for speaking.  "
            "You must click the <b>Resume</b> button before the job will be speakable.  "
            "The job will be spoken by the topmost Talker in the <b>Talkers</b> tab.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_speak_file()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("translate", KIcon::Small, 0, true),
                          i18n("Change Talker"), hbox3, "job_changetalker");
    wt = i18n(
            "<p>Prompts you with a list of your configured Talkers from the <b>Talkers</b> tab.  "
            "The job will be spoken using the selected Talker.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_job_change_talker()));
    btn = new TQPushButton(KGlobal::iconLoader()->loadIconSet("reload_page", KIcon::Small, 0, true),
                          i18n("&Refresh"), hbox3, "refresh");
    wt = i18n(
            "<p>Refresh the list of jobs.</p>");
    TQWhatsThis::add(btn, wt);
    connect (btn, TQT_SIGNAL(clicked()), this, TQT_SLOT(slot_refresh()));

    // Disable job buttons until a job is selected.
    enableJobActions(false);
    enableJobPartActions(false);

    // Create a VBox for the current sentence and sentence label.
    TQVBox* sentenceVBox = new TQVBox(bottomBox);

    // Create a label for current sentence.
    TQLabel* currentSentenceLabel = new TQLabel(sentenceVBox);
    currentSentenceLabel->tqsetSizePolicy(TQSizePolicy(TQSizePolicy::Preferred, TQSizePolicy::Fixed));
    currentSentenceLabel->setText(i18n("Current Sentence"));

    // Create a box to contain the current sentence.
    m_currentSentence = new KTextEdit(sentenceVBox);
    m_currentSentence->setReadOnly(true);
    m_currentSentence->setWordWrap(TQTextEdit::WidgetWidth);
    m_currentSentence->setWrapPolicy(TQTextEdit::AtWordOrDocumentBoundary);
    m_currentSentence->setHScrollBarMode(TQScrollView::AlwaysOff);
    m_currentSentence->setVScrollBarMode(TQScrollView::Auto);
    wt = i18n(
            "<p>The text of the sentence currently speaking.</p>");
    TQWhatsThis::add(m_currentSentence, wt);

    // Set the main widget for the part.
    setWidget(vBox);

    connect(m_jobListView, TQT_SIGNAL(selectionChanged(TQListViewItem* )),
        this, TQT_SLOT(slot_selectionChanged(TQListViewItem* )));

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
        "markerSeen(TQCString,TQString)",
        "markerSeen(TQCString,TQString)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "sentenceStarted(TQCString,uint,uint)",
        "sentenceStarted(TQCString,uint,uint)",
        false);
    connectDCOPSignal(0, 0,
        "sentenceFinished(TQCString,uint,uint)",
        "sentenceFinished(TQCString,uint,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textSet(TQCString,uint)",
        "textSet(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textStarted(TQCString,uint)",
        "textStarted(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textFinished(TQCString,uint)",
        "textFinished(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textStopped(TQCString,uint)",
        "textStopped(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textPaused(TQCString,uint)",
        "textPaused(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textResumed(TQCString,uint)",
        "textResumed(TQCString,uint)",
        false);
    connectDCOPSignal("kttsd", "KSpeech",
        "textRemoved(TQCString,uint)",
        "textRemoved(TQCString,uint)",
        false);

    m_extension = new KttsJobMgrBrowserExtension(this);

    m_jobListView->show();

    // Divide splitter in half.  ListView gets half.  Buttons and Current Sentence get half.
    int halfSplitterSize = splitter->height()/2;
    TQValueList<int> splitterSizes;
    splitterSizes.append(halfSplitterSize);
    splitterSizes.append(halfSplitterSize);
    splitter->setSizes(splitterSizes);
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
* This slot is connected to the Job List View selectionChanged signal.
*/
void KttsJobMgrPart::slot_selectionChanged(TQListViewItem*)
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
        TQListViewItem* item = findItemByJobNum(jobNum);
        if (item) m_jobListView->setSelected(item, true); 
    }
}

void KttsJobMgrPart::slot_job_change_talker()
{
    TQListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        TQString talkerID = item->text(jlvcTalkerID);
        TQStringList talkerIDs = m_talkerCodesToTalkerIDs.values();
        int ndx = talkerIDs.findIndex(talkerID);
        TQString talkerCode;
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
    TQClipboard *cb = kapp->tqclipboard();


    // Copy text from the clipboard.
    TQString text;
    TQMimeSource* data = cb->data();
    if (data)
    {
        if (data->provides("text/html"))
        {
            if (supportsMarkup(NULL, KSpeech::mtHtml))
            {
                TQByteArray d = data->tqencodedData("text/html");
                text = TQString(d);
            }
        }
        if (data->provides("text/ssml"))
        {
            if (supportsMarkup(NULL, KSpeech::mtSsml))
            {
                TQByteArray d = data->tqencodedData("text/ssml");
                text = TQString(d);
            }
        }
    }
    if (text.isEmpty())
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
        TQListViewItem* item = findItemByJobNum(jobNum);
        if (item) m_jobListView->setSelected(item, true); 
    }
}


/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
TQString KttsJobMgrPart::stateToStr(int state)
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
    TQListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        TQString jobNumStr = item->text(jlvcJobNum);
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
    TQListViewItem* item = m_jobListView->selectedItem();
    if (item)
    {
        TQString partCountStr = item->text(jlvcPartCount);
        partCount = partCountStr.toUInt(0, 10);
    }
    return partCount;
}

/**
* Given a Job Number, returns the Job List View item containing the job.
* @param jobNum         Job Number.
* @return               TQListViewItem containing the job or 0 if not found.
*/
TQListViewItem* KttsJobMgrPart::findItemByJobNum(const uint jobNum)
{
    return m_jobListView->findItem(TQString::number(jobNum), jlvcJobNum);
}

/**
* Refresh display of a single job in the JobListView.
* @param jobNum         Job Number.
*/
void KttsJobMgrPart::refreshJob(uint jobNum)
{
    TQByteArray jobInfo = getTextJobInfo(jobNum);
    TQDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    TQCString appId;
    TQString talker;
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
    TQString talkerID = cachedTalkerCodeToTalkerID(talker);
    TQListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcTalkerID, talkerID);
        item->setText(jlvcState, stateToStr(state));
        item->setText(jlvcPosition, TQString::number(seq));
        item->setText(jlvcSentences, TQString::number(sentenceCount));
        item->setText(jlvcPartNum, TQString::number(partNum));
        item->setText(jlvcPartCount, TQString::number(partCount));
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
    TQString jobNumbers = getTextJobNumbers();
    // kdDebug() << "jobNumbers: " << jobNumbers << endl;
    TQStringList jobNums = TQStringList::split(",", jobNumbers);
    TQListViewItem* lastItem = 0;
    TQStringList::ConstIterator endJobNums(jobNums.constEnd());
    for( TQStringList::ConstIterator it = jobNums.constBegin(); it != endJobNums; ++it)
    {
        TQString jobNumStr = *it;
        // kdDebug() << "jobNumStr: " << jobNumStr << endl;
        uint jobNum = jobNumStr.toUInt(0, 10);
        TQByteArray jobInfo = getTextJobInfo(jobNum);
        TQDataStream stream(jobInfo, IO_ReadOnly);
        int state;
        TQCString appId;
        TQString talkerCode;
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
        TQString talkerID = cachedTalkerCodeToTalkerID(talkerCode);
        // Append to list.
        if (lastItem)
            lastItem = new TQListViewItem(m_jobListView, lastItem, jobNumStr, appId, talkerID, 
                stateToStr(state), TQString::number(seq), TQString::number(sentenceCount),
                TQString::number(partNum), TQString::number(partCount));
        else
            lastItem = new TQListViewItem(m_jobListView, jobNumStr, appId, talkerID,
                stateToStr(state), TQString::number(seq), TQString::number(sentenceCount),
                TQString::number(partNum), TQString::number(partCount));
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
    TQListViewItem* item = m_jobListView->firstChild();
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
TQString KttsJobMgrPart::cachedTalkerCodeToTalkerID(const TQString& talkerCode)
{
    // If in the cache, return that.
    if (m_talkerCodesToTalkerIDs.contains(talkerCode))
        return m_talkerCodesToTalkerIDs[talkerCode];
    else
    {
        // Otherwise, retrieve Talker ID from KTTSD and cache it.
        TQString talkerID = talkerCodeToTalkerId(talkerCode);
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
    TQObjectList *l = m_buttonBox->queryList( TQPUSHBUTTON_OBJECT_NAME_STRING, "job_*", true, true );
    TQObjectListIt it( *l ); // iterate over the buttons
    TQObject *obj;

    while ( (obj = it.current()) != 0 ) {
        // for each found object...
        ++it;
        ((TQPushButton*)obj)->setEnabled( enable );
    }
    delete l; // delete the list, not the objects

    if (enable)
    {
        // Later button only enables if currently selected list item is not bottom of list.
        TQListViewItem* item = m_jobListView->selectedItem();
        if (item)
        {
            bool enableLater = item->nextSibling();

            l = m_buttonBox->queryList( TQPUSHBUTTON_OBJECT_NAME_STRING, "job_later", false, true );
            it = TQObjectListIt( *l ); // iterate over the buttons
            if ( (obj = it.current()) != 0 ) {
                // for each found object...
                ((TQPushButton*)obj)->setEnabled( enableLater );
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
    TQObjectList *l = m_buttonBox->queryList( TQPUSHBUTTON_OBJECT_NAME_STRING, "part_*", true, true );
    TQObjectListIt it( *l ); // iterate over the buttons
    TQObject *obj;

    while ( (obj = it.current()) != 0 ) {
        // for each found object...
        ++it;
        ((TQPushButton*)obj)->setEnabled( enable );
    }
    delete l; // delete the list, not the objects
}

/** DCOP Methods connected to DCOP Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
ASYNC KttsJobMgrPart::kttsdStarted() { slot_refresh(); }

/**
* This signal is emitted when the speech engine/plugin encounters a marker in the text.
* @param appId          DCOP application ID of the application that queued the text.
* @param markerName     The name of the marker seen.
* @see markers
*/
ASYNC KttsJobMgrPart::markerSeen(const TQCString&, const TQString&)
{
}

/**
 * This signal is emitted whenever a sentence begins speaking.
 * @param appId          DCOP application ID of the application that queued the text.
 * @param jobNum         Job number of the text job.
 * @param seq            Sequence number of the text.
 * @see getTextCount
 */
ASYNC KttsJobMgrPart::sentenceStarted(const TQCString&, const uint jobNum, const uint seq)
{
    // kdDebug() << "KttsJobMgrPart::sentencedStarted: jobNum = " << jobNum << " seq = " << seq << endl;
    TQListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsSpeaking));
        item->setText(jlvcPosition, TQString::number(seq));
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
ASYNC KttsJobMgrPart::sentenceFinished(const TQCString& /*appId*/, const uint /*jobNum*/, const uint /*seq*/)
{
    // kdDebug() << "KttsJobMgrPart::sentencedFinished: jobNum = " << jobNum << endl;
/*
    TQListViewItem* item = findItemByJobNum(jobNum);
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
ASYNC KttsJobMgrPart::textSet(const TQCString&, const uint jobNum)
{
    TQByteArray jobInfo = getTextJobInfo(jobNum);
    TQDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    TQCString appId;
    TQString talkerCode;
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
    TQString talkerID = cachedTalkerCodeToTalkerID(talkerCode);
    TQListViewItem* item = new TQListViewItem(m_jobListView, m_jobListView->lastItem(), 
        TQString::number(jobNum), appId, talkerID, 
        stateToStr(state), TQString::number(seq), TQString::number(sentenceCount),
        TQString::number(partNum), TQString::number(partCount));
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
ASYNC KttsJobMgrPart::textAppended(const TQCString& appId, const uint jobNum, const int /*partNum*/)
{
    textSet(appId, jobNum);
}

/**
* This signal is emitted whenever speaking of a text job begins.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStarted(const TQCString&, const uint jobNum)
{
    TQListViewItem* item = findItemByJobNum(jobNum);
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
ASYNC KttsJobMgrPart::textFinished(const TQCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textFinished: jobNum = " << jobNum << endl;
    TQListViewItem* item = findItemByJobNum(jobNum);
    if (item)
    {
        item->setText(jlvcState, stateToStr(KSpeech::jsFinished));
        // Update sentence pointer, since signal may not be emitted for final CR.
        refreshJob(jobNum);
    }
    m_currentSentence->setText(TQString());
}

/**
* This signal is emitted whenever a speaking text job stops speaking.
* @param appId          The DCOP senderId of the application that created the job.
* @param jobNum         Job number of the text job.
*/
ASYNC KttsJobMgrPart::textStopped(const TQCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textStopped: jobNum = " << jobNum << endl;
    TQListViewItem* item = findItemByJobNum(jobNum);
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
ASYNC KttsJobMgrPart::textPaused(const TQCString&, const uint jobNum)
{
    // kdDebug() << "KttsJobMgrPart::textPaused: jobNum = " << jobNum << endl;
    TQListViewItem* item = findItemByJobNum(jobNum);
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
ASYNC KttsJobMgrPart::textResumed(const TQCString&, const uint jobNum)
{
    TQListViewItem* item = findItemByJobNum(jobNum);
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
ASYNC KttsJobMgrPart::textRemoved(const TQCString&, const uint jobNum)
{
    TQListViewItem* item = findItemByJobNum(jobNum);
    delete item;
    autoSelectInJobListView();
}

KttsJobMgrBrowserExtension::KttsJobMgrBrowserExtension(KttsJobMgrPart *tqparent)
    : KParts::BrowserExtension(tqparent, "KttsJobMgrBrowserExtension")
{
}

KttsJobMgrBrowserExtension::~KttsJobMgrBrowserExtension()
{
}
