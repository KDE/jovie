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

#include "kttsd.h"
#include "speaker.h"
#include "kttsdui.h"
#include "ktextbrowser.h"

#include "kttsd.moc"

KTTSD::KTTSD(QWidget *parent, const char *name) : kttsdUI(parent, name), DCOPObject("kspeech") {
    kdDebug() << "Running: KTTSD::KTTSD( QObject *parent, const char *name)" << endl;
    // Do stuff here
    //setIdleTimeout(15); // 15 seconds idle timeout.
    kdDebug() << "Instantiating Speaker and running it as another thread" << endl;

    // By default, everything is ok, don't worry, be happy
    ok = true;


    // Create speechData object, and load configuration checking for the return
    speechData = new SpeechData();
    if(!speechData->readConfig()){
        KMessageBox::error(0, i18n("No default language defined. Please configure kttsd in the KDE Control center before use. Text to speech service exiting."), i18n("Text To Speech Error"));
        ok = false;
        return;
    }

    connect (speechData, SIGNAL(textStarted()), this, SLOT(textStarted()));
    connect (speechData, SIGNAL(textFinished()), this, SLOT(textFinished()));
    connect (speechData, SIGNAL(textStopped()), this, SLOT(textStopped()));
    connect (speechData, SIGNAL(textPaused()), this, SLOT(textPaused()));
    connect (speechData, SIGNAL(textResumed()), this, SLOT(textResumed()));
    connect (speechData, SIGNAL(textSet()), this, SLOT(textSet()));
    connect (speechData, SIGNAL(textRemoved()), this, SLOT(textRemoved()));


    // Create speaker object and load plug ins, checking for the return
    speaker = new Speaker(speechData);
    int load = speaker->loadPlugIns();
    if(load == -1){
        KMessageBox::error(0, i18n("No speech synthesizer plugin found. This program cannot run without a speech synthesizer. Text to speech service exiting."), i18n("Text To Speech Error"));
        ok = false;
        return;
    } else if(load == 0){
        KMessageBox::error(0, i18n("A speech synthesizer plugin was not found or is corrupt"), i18n("Text To Speech Error"));
    }

    connect (speaker, SIGNAL(sentenceStarted(QString,QString)), this, SLOT(sentenceStarted(QString,QString)));
    connect (speaker, SIGNAL(sentenceFinished()), this, SLOT(sentenceFinished()));


    // Create system tray object
    tray = new KTTSDTray (this, "systemTrayIcon");
    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small);
    tray->setPixmap (icon);
    connect(tray, SIGNAL(quitSelected()), this, SLOT(quitSelected()));
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

/**
 * Destructor
 * Terminate speaker thread
 */
KTTSD::~KTTSD(){
    kdDebug() << "Running: KTTSD::~KTTSD()" << endl;
    kdDebug() << "Stoping KTTSD service" << endl;
    speaker->requestExit();
    speaker->wait();
    delete speaker;
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
void KTTSD::setText(const QString &text, const QString &language=NULL){
    kdDebug() << "Running: setText(const QString &text, const QString &language=NULL)" << endl;
    kdDebug() << "Setting text: '" << text << "'" << endl;
    speechData->setText(text, language);
    if (checkBoxShow->isChecked())
        show();
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

/***** Slots *****/
// Buttons within the dialog
void KTTSD::openSelected(){
    setText("This is a test text. The function for opening existing texts has not been implemented yet. You can use DCOP, though. Just enter 'dcop kttsd kspeech setText <text> <language>' on the command line. <language> show be something like 'en', 'es', 'de', etc.", "en");
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


// Buttons at the bottom of the dialog
void KTTSD::helpSelected(){
    kapp->invokeHelp();
}

void KTTSD::closeSelected(){
    hide();
}

void KTTSD::quitSelected(){
    kapp->quit();
}

// System tray context menu entries
void KTTSD::configureSelected(){

}

// Speak contents of the clipboard.
void KTTSD::speakClipboardSelected()
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

void KTTSD::aboutSelected(){
    aboutDlg->show();
}

// Slots for the speaker object
void KTTSD::sentenceStarted(QString text, QString){
    viewActiveText->setText (text);
}

void KTTSD::sentenceFinished(){
    viewActiveText->setText ("");
}

// Slots for the speechData object
void KTTSD::textStarted(){
    buttonOpen->setEnabled (false);
    buttonStart->setEnabled (false);
    buttonRestart->setEnabled (true);
    buttonStop->setEnabled (true);
    buttonPause->setEnabled (true);
    buttonNextSentence->setEnabled (true);
    buttonNextParagraph->setEnabled (true);
    buttonPrevSentence->setEnabled (true);
    buttonPrevParagraph->setEnabled (true);
}

void KTTSD::textFinished(){
    buttonOpen->setEnabled (true);
    buttonStart->setEnabled (true);
    buttonRestart->setEnabled (false);
    buttonStop->setEnabled (false);
    buttonPause->setEnabled (false);
    buttonNextSentence->setEnabled (false);
    buttonNextParagraph->setEnabled (false);
    buttonPrevSentence->setEnabled (false);
    buttonPrevParagraph->setEnabled (false);
    if (checkBoxHide->isChecked())
        hide();
}

void KTTSD::textStopped(){
    buttonOpen->setEnabled (true);
    buttonStart->setEnabled (true);
    buttonRestart->setEnabled (false);
    buttonStop->setEnabled (false);
    buttonPause->setEnabled (false);
    buttonNextSentence->setEnabled (false);
    buttonNextParagraph->setEnabled (false);
    buttonPrevSentence->setEnabled (false);
    buttonPrevParagraph->setEnabled (false);
}

void KTTSD::textPaused(){
    buttonPause->setOn (true);
    viewPaused->setHidden (false);
}

void KTTSD::textResumed(){
    buttonPause->setOn (false);
    viewPaused->setHidden (true);
}

void KTTSD::textSet(){
    buttonStart->setEnabled (true);
}

void KTTSD::textRemoved(){
    buttonStart->setEnabled (false);
}


/***** KTTSDTray *****/

KTTSDTray::KTTSDTray (QWidget *parent, const char *name) : KSystemTray (parent, name)
{
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("configure", KIcon::Small),
        i18n("&Configure kttsd..."), this, SIGNAL(configureSelected()));
    contextMenu()->insertSeparator();
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("klipper", KIcon::Small),
        i18n("&Speak clipboard contents"), this, SIGNAL(speakClipboardSelected()));
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("contents", KIcon::Small),
        i18n("kttsd &Handbook"), this, SIGNAL(helpSelected()));
    contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small),
        i18n("&About kttsd"), this, SIGNAL(aboutSelected()));
}

KTTSDTray::~KTTSDTray() {
}
