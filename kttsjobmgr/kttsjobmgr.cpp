/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, rewind,
  advance, change Talker, etc. 
  -------------------
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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
#include <qdockarea.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qclipboard.h>

// KDE includes.
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <klistview.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kapplication.h>
#include <kinputdialog.h>

// KTTS includes.
#include "kspeech.h"
#include "talkercode.h"
#include "selecttalkerwidget.h"
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
    selectOnTextSet = false;

    setInstance(KttsJobMgrFactory::instance());

    // All the ktts components use the same catalogue.
    KGlobal::locale()->insertCatalogue("kttsd");

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
    m_jobListView->addColumn(i18n("Talker ID"));
    m_jobListView->addColumn(i18n("State"));
    m_jobListView->addColumn(i18n("Position"));
    m_jobListView->addColumn(i18n("Sentences"));
    m_jobListView->addColumn(i18n("Part Num"));
    m_jobListView->addColumn(i18n("Parts"));

    // Do not sort the list.
    m_jobListView->setSorting(-1);

    // Splitter to resize Job ListView to minimum height.
    splitter->setResizeMode(m_jobListView, QSplitter::Stretch);

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
    act = new KAction(i18n("Change Talker"),
        KGlobal::iconLoader()->loadIconSet("translate", KIcon::Toolbar, 0, true),
        0, this, SLOT(slot_job_change_talker()), actionCollection(), "job_change_talker");
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
    uint jobNum = getCurrentJobNum();
    kdDebug() << "KttsJobMgrPart::slot_job_restart: jobNum = " << jobNum << endl;
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

QString KttsJobMgrPart::translatedGender(const QString &gender)
{
    if (gender == "male")
        return i18n("male");
    else if (gender == "female")
        return i18n("female");
    else if (gender == "neutral")
        return i18n("neutral gender", "neutral");
    else return gender;
}

QString KttsJobMgrPart::translatedVolume(const QString &volume)
{
    if (volume == "medium")
        return i18n("medium sound", "medium");
    else if (volume == "loud")
        return i18n("loud sound", "loud");
    else if (volume == "soft")
        return i18n("soft sound", "soft");
    else return volume;
}

QString KttsJobMgrPart::translatedRate(const QString &rate)
{
    if (rate == "medium")
        return i18n("medium speed", "medium");
    else if (rate == "fast")
        return i18n("fast speed", "fast");
    else if (rate == "slow")
        return i18n("slow speed", "slow");
    else return rate;
}

/**
 * Given a talker code, parses out the attributes.
 * @param talkerCode       The talker code.
 * @return languageCode    Language Code.
 * @return voice           Voice name.
 * @return gender          Gender.
 * @return volume          Volume.
 * @return rate            Rate.
 * @return plugInName      Name of Synthesizer plugin.
 */
void KttsJobMgrPart::parseTalkerCode(const QString &talkerCode,
    QString &languageCode,
    QString &voice,
    QString &gender,
    QString &volume,
    QString &rate,
    QString &plugInName)
{
    languageCode = talkerCode.section("lang=", 1, 1);
    languageCode = languageCode.section('"', 1, 1);
    voice = talkerCode.section("name=", 1, 1);
    voice = voice.section('"', 1, 1);
    gender = talkerCode.section("gender=", 1, 1);
    gender = gender.section('"', 1, 1);
    volume = talkerCode.section("volume=", 1, 1);
    volume = volume.section('"', 1, 1);
    rate = talkerCode.section("rate=", 1, 1);
    rate = rate.section('"', 1, 1);
    plugInName = talkerCode.section("synthesizer=", 1, 1);
    plugInName = plugInName.section('"', 1, 1);
}

/**
 * Converts a language code plus optional country code to language description.
 */
QString KttsJobMgrPart::languageCodeToLanguage(const QString &languageCode)
{
    QString twoAlpha;
    QString countryCode;
    QString charSet;
    QString language;
    if (languageCode == "other")
        language = i18n("Other");
    else
    {
        KGlobal::locale()->splitLocale(languageCode, twoAlpha, countryCode, charSet);
        language = KGlobal::locale()->twoAlphaToLanguageName(twoAlpha);
    }
    if (!countryCode.isEmpty())
        language += " (" + KGlobal::locale()->twoAlphaToCountryName(countryCode) + ")";
    return language;
}

/**
* Convert a Talker Code to a translated, displayable name.
*/
QString KttsJobMgrPart::talkerCodeToDisplayName(const QString &talkerCode)
{
    QString languageCode;
    QString voice;
    QString gender;
    QString volume;
    QString rate;
    QString plugInName;
    parseTalkerCode(talkerCode, languageCode, voice, gender, volume, rate, plugInName);
    QString display;
    if (!languageCode.isEmpty()) display = languageCodeToLanguage(languageCode);
    if (!plugInName.isEmpty()) display += "," + plugInName;
    if (!voice.isEmpty()) display += "," + voice;
    if (!gender.isEmpty()) display += "," + translatedGender(gender);
    if (!volume.isEmpty()) display += "," + translatedVolume(volume);
    if (!rate.isEmpty()) display += "," + translatedRate(rate);
    return display;
}

void KttsJobMgrPart::slot_job_change_talker()
{
    uint jobNum = getCurrentJobNum();
    if (jobNum)
    {
        QStringList talkerCodesList = getTalkers();
        // Create SelectTalker widget.
        SelectTalkerWidget* stw = new SelectTalkerWidget();
        stw->talkersList->setSelectionMode(QListView::Single);
        // A list of the items in the listview.
        QValueList<QListViewItem*> talkersListItems;
        QListViewItem* talkerItem = 0;
        // Fill rows and columns.
        uint talkerCodesListCount = talkerCodesList.count();
        for (uint index = 0; index < talkerCodesListCount; index++)
        {
            QString code = talkerCodesList[index];
            TalkerCode parsedTalkerCode(code);
            QString language = parsedTalkerCode.languageCode();
            QString synthName = parsedTalkerCode.plugInName();
            if (talkerItem)
                talkerItem =
                    new KListViewItem(stw->talkersList, talkerItem, language, synthName);
            else
                talkerItem =
                    new KListViewItem(stw->talkersList, language, synthName);
            updateTalkerItem(talkerItem, code);
            talkersListItems.append(talkerItem);
        }
        // Display the listview in a dialog.
        KDialogBase* dlg = new KDialogBase(
            KDialogBase::Swallow,
            i18n("Select Talker"),
            KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
            KDialogBase::Cancel,
            widget(),
            "selectTalker_dlg",
            true,
            true);
        dlg->setInitialSize(QSize(700, 300), false);
        dlg->setMainWidget(stw);
        // dlg->setHelp("configure-plugin", "kttsd");
        if (dlg->exec() == QDialog::Accepted)
        {
            talkerItem = stw->talkersList->selectedItem();
            if (talkerItem)
            {
                uint index = talkersListItems.findIndex(talkerItem);
                changeTextTalker(talkerCodesList[index], jobNum);
                refreshJob(jobNum);
            }
        }
        delete stw;
        delete dlg;
    }
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
        kdDebug() << "KttsJobMgrPart::slot_speak_clipboard: started jobNum " << jobNum << endl;
        startText(jobNum);
        // Set flag so that the job we just created will be selected when textSet signal is received.
        selectOnTextSet = true;        
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
        item->setText(jlvcTalker, talkerID);
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
    kdDebug() << "KttsJobMgrPart::refreshJobListView: Running" << endl;
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
 * Given an item in the talker listview and a talker code, sets the columns of the item.
 * @param talkerItem       QListViewItem.
 * @param talkerCode       Talker Code.
 */
void KttsJobMgrPart::updateTalkerItem(QListViewItem* talkerItem, const QString &talkerCode)
{
    TalkerCode parsedTalkerCode(talkerCode);
    QString fullLanguageCode = parsedTalkerCode.fullLanguageCode();
    if (!fullLanguageCode.isEmpty())
    {
        QString language = parsedTalkerCode.languageCodeToLanguage(fullLanguageCode);
        if (!language.isEmpty())
        {
            talkerItem->setText(tlvcLanguage, language);
        }
    }
    // Don't update the Synthesizer name with plugInName.  The former is a translated
    // name; the latter an English name.
    // if (!plugInName.isEmpty()) talkerItem->setText(tlvcSynthName, plugInName);
    if (!parsedTalkerCode.voice().isEmpty())
        talkerItem->setText(tlvcVoice, parsedTalkerCode.voice());
    if (!parsedTalkerCode.gender().isEmpty())
        talkerItem->setText(tlvcGender, translatedGender(parsedTalkerCode.gender()));
    if (!parsedTalkerCode.volume().isEmpty())
        talkerItem->setText(tlvcVolume, translatedVolume(parsedTalkerCode.volume()));
    if (!parsedTalkerCode.rate().isEmpty())
        talkerItem->setText(tlvcRate, translatedRate(parsedTalkerCode.rate()));
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
    kdDebug() << "KttsJobMgrPart::sentencedStarted: jobNum = " << jobNum << " seq = " << seq << endl;
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
ASYNC KttsJobMgrPart::sentenceFinished(const QCString&, const uint jobNum, const uint)
{
    kdDebug() << "KttsJobMgrPart::sentencedFinished: jobNum = " << jobNum << endl;
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
    kdDebug() << "KttsJobMgrPart::textFinished: jobNum = " << jobNum << endl;
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
    kdDebug() << "KttsJobMgrPart::textStopped: jobNum = " << jobNum << endl;
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
    kdDebug() << "KttsJobMgrPart::textPaused: jobNum = " << jobNum << endl;
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
