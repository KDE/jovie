/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.cpp
  KTTSD main class
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#include <qcstring.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <qfile.h>
#include <kfiledialog.h>
#include <kcmultidialog.h>
#include <dcopclient.h>

#include "kttsd.h"
//#include "speaker.h"
//#include "kttsdui.h"
#include "ktextbrowser.h"

KTTSD::KTTSD(QWidget *parent, const char *name) : kttsdUI(parent, name), DCOPObject("kspeech") {
    kdDebug() << "Running: KTTSD::KTTSD( QObject *parent, const char *name)" << endl;
    // Do stuff here
    //setIdleTimeout(15); // 15 seconds idle timeout.

    speaker = 0;
    speechData = 0;    
    if (!initializeSpeaker()) return;

    // Create system tray object
    tray = new KTTSDTray (this, "systemTrayIcon");
    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small);
    tray->setPixmap (icon);
    connect(tray, SIGNAL(quitSelected()), this, SLOT(quitSelected()));
    connect(tray, SIGNAL(configureSelected()), this, SLOT(configureSelected()));
    connect(tray, SIGNAL(speakClipboardSelected()), this, SLOT(speakClipboardSelected()));
    connect(tray, SIGNAL(aboutSelected()), this, SLOT(aboutSelected()));
    connect(tray, SIGNAL(helpSelected()), this, SLOT(helpSelected()));

    aboutDlg = new KAboutApplication (0, "KDE Text To Speech Deamon", false);

    buttonOpen->setIconSet (KGlobal::iconLoader()->loadIconSet("fileopen", KIcon::Small));
    connect(buttonOpen,  SIGNAL(clicked()), this, SLOT(openSelected()));
    buttonStart->setIconSet (KGlobal::iconLoader()->loadIconSet("player_play", KIcon::Small));
    connect(buttonStart, SIGNAL(clicked()), this, SLOT(startSelected()));
    buttonRestart->setIconSet (KGlobal::iconLoader()->loadIconSet("player_start", KIcon::Small));
    connect(buttonRestart, SIGNAL(clicked()), this, SLOT(startSelected()));
    buttonStop->setIconSet (KGlobal::iconLoader()->loadIconSet("player_stop", KIcon::Small));
    connect(buttonStop,  SIGNAL(clicked()), this, SLOT(stopSelected()));
    buttonPause->setIconSet (KGlobal::iconLoader()->loadIconSet("player_pause", KIcon::Small));
    connect(buttonPause, SIGNAL(toggled(bool)), this, SLOT(pauseSelected(bool)));

    buttonNextSentence->setIconSet (KGlobal::iconLoader()->loadIconSet("player_fwd", KIcon::Small));
    connect(buttonNextSentence, SIGNAL(clicked()), this, SLOT(nextSentenceSelected()));
    buttonNextParagraph->setIconSet (KGlobal::iconLoader()->loadIconSet("player_fwd", KIcon::Small));
    connect(buttonNextParagraph, SIGNAL(clicked()), this, SLOT(nextParagraphSelected()));
    buttonPrevSentence->setIconSet (KGlobal::iconLoader()->loadIconSet("player_rew", KIcon::Small));
    connect(buttonPrevSentence, SIGNAL(clicked()), this, SLOT(prevSentenceSelected()));
    buttonPrevParagraph->setIconSet (KGlobal::iconLoader()->loadIconSet("player_rew", KIcon::Small));
    connect(buttonPrevParagraph, SIGNAL(clicked()), this, SLOT(prevParagraphSelected()));

    buttonHelp->setGuiItem(KStdGuiItem::help());
    connect(buttonHelp,  SIGNAL(clicked()), this, SLOT(helpSelected()));
    buttonClose->setGuiItem(KStdGuiItem::close());
    connect(buttonClose, SIGNAL(clicked()), this, SLOT(closeSelected()));
#if KDE_VERSION >= KDE_MAKE_VERSION (3,1,90)
    buttonQuit->setGuiItem(KStdGuiItem::quit());
#endif // KDE 3.2
    connect(buttonQuit,  SIGNAL(clicked()), this, SLOT(quitSelected()));

    viewPaused->setHidden (true);

    // Let's rock!
    speaker->start();
    tray->show();
}

bool KTTSD::initializeSpeaker()
{
    kdDebug() << "Instantiating Speaker and running it as another thread" << endl;

    // By default, everything is ok, don't worry, be happy
    ok = true;

    // Create speechData object, and load configuration, checking if valid config loaded.
    speechData = new SpeechData();
    // If user has not yet configured KTTSD, display configuration dialog.
    if (!speechData->readConfig())
    {
        delete speechData;
        speechData = 0;
        configureSelected();
        speechData = new SpeechData();
        // If still no valid configuration, bail out.
        if (!speechData->readConfig())
        {
            KMessageBox::error(0, i18n("No default language defined. Please configure kttsd in the KDE Control center before use. Text to speech service exiting."), i18n("Text To Speech Error"));
            delete speechData;
            speechData = 0;
            ok = false;
            return false;
        }
    }

    connect (speechData, SIGNAL(textStarted()), this, SLOT(slotTextStarted()));
    connect (speechData, SIGNAL(textFinished()), this, SLOT(slotTextFinished()));
    connect (speechData, SIGNAL(textStopped()), this, SLOT(slotTextStopped()));
    connect (speechData, SIGNAL(textPaused()), this, SLOT(slotTextPaused()));
    connect (speechData, SIGNAL(textResumed()), this, SLOT(slotTextResumed()));
    connect (speechData, SIGNAL(textSet()), this, SLOT(slotTextSet()));
    connect (speechData, SIGNAL(textRemoved()), this, SLOT(slotTextRemoved()));

    // Create speaker object and load plug ins, checking for the return
    speaker = new Speaker(speechData);
    int load = speaker->loadPlugIns();
    if(load == -1){
        KMessageBox::error(0, i18n("No speech synthesizer plugin found. This program cannot run without a speech synthesizer. Text to speech service exiting."), i18n("Text To Speech Error"));
        delete speaker;
        speaker = 0;
        delete speechData;
        speechData = 0;
        ok = false;
        return false;
    } else if(load == 0){
        KMessageBox::error(0, i18n("A speech synthesizer plugin was not found or is corrupt"), i18n("Text To Speech Error"));
        delete speaker;
        speaker = 0;
        delete speechData;
        speechData = 0;
        ok = false;
        return false;
    }

    connect (speaker, SIGNAL(sentenceStarted(QString,QString)), this,
        SLOT(slotSentenceStarted(QString,QString)));
    connect (speaker, SIGNAL(sentenceFinished()), this,
        SLOT(slotSentenceFinished()));

    return true;
}

/**
 * Destructor
 * Terminate speaker thread
 */
KTTSD::~KTTSD(){
    kdDebug() << "Running: KTTSD::~KTTSD()" << endl;
    kdDebug() << "Stopping KTTSD service" << endl;
    delete aboutDlg;
    if (speaker)
    {
        speaker->requestExit();
        speaker->wait();
        delete speaker;
    }
    delete speechData;
}

/***** DCOP exported functions *****/
/**
 * DCOP exported function to say warnings
 */
void KTTSD::sayWarning(const QString &warning, const QString &language=NULL){
    kdDebug() << "Running: KTTSD::sayWarning(const QString &warning, const QString &language=NULL)" << endl;
    kdDebug() << "Adding '" << warning << "' to warning queue." << endl;
    speechData->enqueueWarning(warning, language);
}

/**
 * DCOP exported function to say messages
 */
void KTTSD::sayMessage(const QString &message, const QString &language=NULL){
    kdDebug() << "Running: KTTSD::sayMessage(const QString &message, const QString &language=NULL)" << endl;
    kdDebug() << "Adding '" << message << "' to message queue." << endl;
    speechData->enqueueMessage(message, language);
}

/**
 * DCOP exported function to set text
 */
void KTTSD::setText(const QString &text, const QString &language){
    kdDebug() << "Running: setText(const QString &text, const QString &language=NULL)" << endl;
    kdDebug() << "Setting text: '" << text << "'" << endl;
    DCOPClient* client = callingDcopClient();
    QCString appId;
    if (client) appId = client->senderId();
    speechData->setText(text, language, appId);
    if (checkBoxShow->isChecked())
        show();
}

/**
 * DCOP exported function to set text to contents of a file.
 */
void KTTSD::setFile(const QString &filename, const QString &language=NULL)
{
    kdDebug() << "Running: setFile(const QString &filename, const QString &language=NULL)" << endl;
    QFile file(filename);
    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream(&file);
        setText(stream.read(), language);
        file.close();
    }
}

/**
 * Remove the text
 */
void KTTSD::removeText(){
    kdDebug() << "Running: KTTSD::removeText()" << endl;
    stopText();
    speechData->removeText();
}

/**
 * Previous paragrah
 */
void KTTSD::prevParText(){
    kdDebug() << "Running: KTTSD::prevParText()" << endl;
    speechData->prevParText();
}

/**
 * Previous sentence
 */
void KTTSD::prevSenText(){
    kdDebug() << "Running: KTTSD::prevSenText()" << endl;
    speechData->prevSenText();
}

/**
 * Pause text
 */
void KTTSD::pauseText(){
    kdDebug() << "Running: KTTSD::pauseText()" << endl;
    speechData->pauseText();
}

/**
 * Pause text and go to the begining
 */
void KTTSD::stopText(){
    kdDebug() << "Running: KTTSD::stopText()" << endl;
    speechData->stopText();
}

/**
 * Start text at the beginning
 */
void KTTSD::startText(){
    kdDebug() << "Running: KTTSD::startText()" << endl;
    speechData->startText();
}

/**
 * Start or resume text where it was paused
 */
void KTTSD::resumeText(){
    kdDebug() << "Running: KTTSD::resumeText()" << endl;
    speechData->resumeText();
}

/**
 * Next sentence
 */
void KTTSD::nextSenText(){
    kdDebug() << "Running: KTTSD::nextSenText()" << endl;
    speechData->nextSenText();
}

/**
 * Next paragrah
 */
void KTTSD::nextParText(){
    kdDebug() << "Running: KTTSD::nextParText()" << endl;
    speechData->nextParText();
}

/**
 * Speak the clipboard contents.
 */
 
void KTTSD::speakClipboard()
{
    // Get the clipboard object.
    QClipboard *cb = kapp->clipboard();

    // Copy text from the clipboard.
    QString text = cb->text();
    
    // Speak it.
    if ( !text.isNull() ) 
    {
        setText(text);
        startText();
    }
}

/***** Slots *****/
// Buttons within the dialog
void KTTSD::openSelected()
{
    QString filename = KFileDialog::getOpenFileName();
    setFile(filename);

//    setText("This is a test text. The function for opening existing texts has not been implemented yet. You can use DCOP, though. Just enter 'dcop kttsd kspeech setText <text> <language>' on the command line. <language> should be something like 'en', 'es', 'de', etc.", "en");
}

void KTTSD::startSelected(){
    startText();
}

void KTTSD::stopSelected(){
    stopText();
}

void KTTSD::pauseSelected(bool paused){
    if (paused)
        pauseText();
    else
        resumeText();
}

void KTTSD::nextSentenceSelected(){
    nextSenText();
}

void KTTSD::prevSentenceSelected(){
    prevSenText();
}

void KTTSD::nextParagraphSelected(){
    nextParText();
}

void KTTSD::prevParagraphSelected(){
    prevParText();
}

void KTTSD::reinit()
{
    // Restart ourself.
    kdDebug() << "Running: KTTSD::reinit()" << endl;
    kdDebug() << "Stopping KTTSD service" << endl;
    if (speaker)
    {
        speaker->requestExit();
        speaker->wait();
        delete speaker;
        speaker = 0;
    }
    if (speechData) delete speechData;
    speechData = 0;
    slotTextStopped();
    slotTextRemoved();

    kdDebug() << "Starting KTTSD service" << endl;
    if (!initializeSpeaker()) return;
    speaker->start();
}

// Buttons at the bottom of the dialog
void KTTSD::helpSelected(){
    kapp->invokeHelp();
}

void KTTSD::speakClipboardSelected(){
    speakClipboard();
}

void KTTSD::configCommitted() {
    if (speaker) reinit();
}

void KTTSD::closeSelected(){
    hide();
}

void KTTSD::quitSelected(){
    stopText();
    kapp->quit();
}

// System tray context menu entries
void KTTSD::configureSelected(){
    KCMultiDialog* cfgDlg = new KCMultiDialog(KCMultiDialog::Plain, "Configure", this, "configKTTSD", true);
    cfgDlg->addModule("kcmkttsd", true);
    connect(cfgDlg, SIGNAL(configCommitted()), this, SLOT(configCommitted()));
    cfgDlg->exec();
}

void KTTSD::aboutSelected(){
    aboutDlg->show();
}

// Slots for the speaker object
void KTTSD::slotSentenceStarted(QString text, QString){
    viewActiveText->setText (text);
    emitDcopSignalNoParams("sentenceStarted()");
}

void KTTSD::slotSentenceFinished(){
    viewActiveText->setText ("");
    emitDcopSignalNoParams("sentenceFinished()");
}

// Slots for the speechData object
void KTTSD::slotTextStarted(){
    buttonOpen->setEnabled (false);
    buttonStart->setEnabled (false);
    buttonRestart->setEnabled (true);
    buttonStop->setEnabled (true);
    buttonPause->setEnabled (true);
    buttonNextSentence->setEnabled (true);
    buttonNextParagraph->setEnabled (true);
    buttonPrevSentence->setEnabled (true);
    buttonPrevParagraph->setEnabled (true);
    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    emitDcopSignalNoParams("textStarted()");
}

void KTTSD::slotTextFinished(){
    buttonOpen->setEnabled (true);
    buttonStart->setEnabled (true);
    buttonRestart->setEnabled (false);
    buttonStop->setEnabled (false);
    buttonPause->setEnabled (false);
    buttonNextSentence->setEnabled (false);
    buttonNextParagraph->setEnabled (false);
    buttonPrevSentence->setEnabled (false);
    buttonPrevParagraph->setEnabled (false);
    emitDcopSignalNoParams("textFinished()");
    if (checkBoxHide->isChecked())
        hide();
}

void KTTSD::slotTextStopped(){
    buttonOpen->setEnabled (true);
    buttonStart->setEnabled (true);
    buttonRestart->setEnabled (false);
    buttonStop->setEnabled (false);
    buttonPause->setEnabled (false);
    buttonNextSentence->setEnabled (false);
    buttonNextParagraph->setEnabled (false);
    buttonPrevSentence->setEnabled (false);
    buttonPrevParagraph->setEnabled (false);
    emitDcopSignalNoParams("textStopped()");
}

void KTTSD::slotTextPaused(){
    buttonPause->setOn (true);
    viewPaused->setHidden (false);
    emitDcopSignalNoParams("textPaused()");
}

void KTTSD::slotTextResumed(){
    buttonPause->setOn (false);
    viewPaused->setHidden (true);
    emitDcopSignalNoParams("textResumed()");
}

void KTTSD::slotTextSet(){
    buttonStart->setEnabled (true);
    emitDcopSignalNoParams("textSet()");
}

void KTTSD::slotTextRemoved(){
    buttonStart->setEnabled (false);
    emitDcopSignalNoParams("textRemoved()");
}

/**
 * Send a DCOP signal with no parameters.
 */
void KTTSD::emitDcopSignalNoParams(const QCString& signalName)
{
    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    emitDCOPSignal(signalName, params);
}

/***** KTTSDTray *****/

KTTSDTray::KTTSDTray (QWidget *parent, const char *name) : KSystemTray (parent, name)
{
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("configure", KIcon::Small),
        i18n("&Configure kttsd..."), this, SIGNAL(configureSelected()));
    contextMenu()->insertSeparator();
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("klipper", KIcon::Small),
        i18n("&Speak clipboard contents"), this, SIGNAL(speakClipboardSelected()));
    int id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("contents", KIcon::Small),
        i18n("kttsd &Handbook"), this, SIGNAL(helpSelected()));
    // Handbook not available yet.
    contextMenu()->setItemEnabled(id, false);
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small),
        i18n("&About kttsd"), this, SIGNAL(aboutSelected()));
}

KTTSDTray::~KTTSDTray() {
}

#include "kttsd.moc"

