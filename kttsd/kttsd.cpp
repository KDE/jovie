/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD main class
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// Qt includes.
#include <qcstring.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <qfile.h>

// KDE includes.
#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <dcopclient.h>
#include <knotifyclient.h>
#include <krun.h>

#include "kttsd.h"

/**
* This is the "main" module of KTTSD.  It performs the following functions:
* - Creates and destroys SpeechData and Speaker objects.
* - Receives DCOP calls and dispatches them to SpeechData and Speaker.
* - Receives signals from SpeechData and Speaker and converts them to DCOP signals.
*
* Note that most of the real tts work occurs in Speaker.
*/

KTTSD::KTTSD(QObject *parent, const char *name) : 
    DCOPObject("kspeech"),
    QObject(parent, name)
{
    // kdDebug() << "KTTSD::KTTSD Running" << endl;
    m_speaker = 0;
    m_speechData = 0;
    ready();
}

/*
* Create and initialize the SpeechData object.
*/
bool KTTSD::initializeSpeechData()
{
    // Create speechData object, and load configuration.
    if (!m_speechData)
    {
        m_speechData = new SpeechData();
        connect (m_speechData, SIGNAL(textSet(const QCString&, const uint)), 
            this, SLOT(slotTextSet(const QCString&, const uint)));
        connect (m_speechData, SIGNAL(textAppended(const QCString&, const uint, const int)),
            this, SLOT(slotTextAppended(const QCString&, const uint, const int)));
        connect (m_speechData, SIGNAL(textRemoved(const QCString&, const uint)), 
            this, SLOT(slotTextRemoved(const QCString&, const uint)));

        // Hook KNotify signal.
        if (!connectDCOPSignal(0, 0, 
            "notifySignal(QString,QString,QString,QString,QString,int,int,int,int)",
            "notificationSignal(QString,QString,QString,QString,QString,int,int,int,int)",
            false)) kdDebug() << "KTTSD:initializeSpeechData: connectDCOPSignal for knotify failed" << endl;
    }
    // Load configuration.
    m_speechData->readConfig();

    return true;
}

/*
* Create and initialize the Speaker object.
*/
bool KTTSD::initializeSpeaker()
{
    // kdDebug() << "KTTSD::initializeSpeaker: Instantiating Speaker" << endl;

    // Create speaker object and load plug ins, checking for the return
    m_speaker = new Speaker(m_speechData);
    int load = m_speaker->loadPlugIns();
    if(load < 0)
    {
        delete m_speaker;
        m_speaker = 0;
        delete m_speechData;
        m_speechData = 0;
        kdDebug() << "KTTSD::initializeSpeaker: no Talkers have been configured." << endl;
        // Ask if user would like to run configuration dialog, but don't bug user unnecessarily.
        QString dontAskConfigureKTTS = "DontAskConfigureKTTS";
        KMessageBox::ButtonCode msgResult;
        if (KMessageBox::shouldBeShownYesNo(dontAskConfigureKTTS, msgResult))
        {
            if (KMessageBox::questionYesNo(
                0,
                i18n("KTTS has not yet been configured.  At least one Talker must be configured.  "
                    "Would you like to configure it now?"),
                i18n("KTTS Not Configured"),
                i18n("&Yes"),
                i18n("&No"),
                dontAskConfigureKTTS) == KMessageBox::Yes) msgResult = KMessageBox::Yes;
        }
        if (msgResult == KMessageBox::Yes) showDialog();
        return false;
    }

    connect (m_speaker, SIGNAL(textStarted(const QCString&, const uint)), 
        this, SLOT(slotTextStarted(const QCString&, const uint)));
    connect (m_speaker, SIGNAL(textFinished(const QCString&, const uint)), 
        this, SLOT(slotTextFinished(const QCString&, const uint)));
    connect (m_speaker, SIGNAL(textResumed(const QCString&, const uint)), 
        this, SLOT(slotTextResumed(const QCString&, const uint)));
    connect (m_speaker, SIGNAL(sentenceStarted(QString, QString, const QCString&, const uint, const uint)),
        this, SLOT(slotSentenceStarted(QString, QString, const QCString&, const uint, const uint)));
    connect (m_speaker, SIGNAL(sentenceFinished(const QCString&, const uint, const uint)), this,
        SLOT(slotSentenceFinished(const QCString&, const uint, const uint)));
    connect (m_speaker, SIGNAL(textStopped(const QCString&, const uint)), 
        this, SLOT(slotTextStopped(const QCString&, const uint)));
    connect (m_speaker, SIGNAL(textPaused(const QCString&, const uint)), 
        this, SLOT(slotTextPaused(const QCString&, const uint)));

    return true;
}

/**
 * Destructor
 * Terminate speaker thread
 */
KTTSD::~KTTSD(){
    kdDebug() << "KTTSD::~KTTSD:: Stopping KTTSD service" << endl;
    if (m_speaker) m_speaker->requestExit();
    delete m_speaker;
    delete m_speechData;
    kdDebug() << "KTTSD::~KTTSD: Emitting DCOP signal kttsdExiting()" << endl;
    kttsdExiting();
}

/***** DCOP exported functions *****/

/**
* Determine whether the currently-configured speech plugin supports a speech markup language.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
* @param markupType     The kttsd code for the desired speech markup language.
* @return               True if the plugin currently configured for the indicated
*                       talker supports the indicated speech markup language.
* @see kttsdMarkupType
*/
bool KTTSD::supportsMarkup(const QString& talker /*=NULL*/, const uint markupType /*=0*/)
{
    if (markupType != kspeech::mtSsml) return false;
    if (!m_speaker) return false;
    return m_speaker->supportsMarkup(talker, markupType);
}

/**
* Determine whether the currently-configured speech plugin supports markers in speech markup.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
* @return               True if the plugin currently configured for the indicated
*                       talker supports markers.
* TODO: Waiting on plugin API.
*/
bool KTTSD::supportsMarkers(const QString& /*talker=NULL*/) { return false; }

/**
* Say a message as soon as possible, interrupting any other speech in progress.
* IMPORTANT: This method is reserved for use by Screen Readers and should not be used
* by any other applications.
* @param msg            The message to be spoken.
* @param talker         Code for the to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
*
* If an existing Screen Reader output is in progress, it is stopped and discarded and
* replaced with this new message.
*/
void KTTSD::sayScreenReaderOutput(const QString &msg, const QString &talker /*=NULL*/)
{
    if (!m_speaker) return;
    m_speechData->setScreenReaderOutput(msg, talker, getAppId());
    m_speaker->doUtterances();
}

/**
* Say a warning.  The warning will be spoken when the current sentence
* stops speaking and takes precedence over Messages and regular text.  Warnings should only
* be used for high-priority messages requiring immediate user attention, such as
* "WARNING. CPU is overheating."
* @param warning        The warning to be spoken.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
*/
void KTTSD::sayWarning(const QString &warning, const QString &talker /*=NULL*/){
    // kdDebug() << "KTTSD::sayWarning: Running" << endl;
    if (!m_speaker) return;
    kdDebug() << "KTTSD::sayWarning: Adding '" << warning << "' to warning queue." << endl;
    m_speechData->enqueueWarning(warning, talker, getAppId());
    m_speaker->doUtterances();
}

/**
* Say a message.  The message will be spoken when the current sentence stops speaking
* but after any warnings have been spoken.
* Messages should be used for one-shot messages that can't wait for
* normal text messages to stop speaking, such as "You have mail.".
* @param message        The message to be spoken.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no talker has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
*/
void KTTSD::sayMessage(const QString &message, const QString &talker /*=NULL*/)
{
    // kdDebug() << "KTTSD::sayMessage: Running" << endl;
    if (!m_speaker) return;
    kdDebug() << "KTTSD::sayMessage: Adding '" << message << "' to message queue." << endl;
    m_speechData->enqueueMessage(message, talker, getAppId());
    m_speaker->doUtterances();
}

/**
* Sets the GREP pattern that will be used as the sentence delimiter.
* @param delimiter      A valid GREP pattern.
*
* The default sentence delimiter is
    @verbatim
        ([\\.\\?\\!\\:\\;])\\s
    @endverbatim
*
* Note that backward slashes must be escaped.
*
* Changing the sentence delimiter does not affect other applications.
* @see sentenceparsing
*/
void KTTSD::setSentenceDelimiter(const QString &delimiter)
{
    if (!m_speaker) return;
    m_speechData->setSentenceDelimiter(delimiter, getAppId());
}

/**
* Queue a text job.  Does not start speaking the text.
* @param text           The message to be spoken.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default plugin.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
* @return               Job number.
*
* Plain text is parsed into individual sentences using the current sentence delimiter.
* Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
* Call @ref getTextCount to retrieve the sentence count after calling setText.
*
* The text may contain speech mark language, such as Sable, JSML, or SMML,
* provided that the speech plugin/engine support it.  In this case,
* sentence parsing follows the semantics of the markup language.
*
* Call @ref startText to mark the job as speakable and if the
* job is the first speakable job in the queue, speaking will begin.
* @see getTextCount
* @see startText
*/
uint KTTSD::setText(const QString &text, const QString &talker /*=NULL*/)
{
    // kdDebug() << "KTTSD::setText: Running" << endl;
    if (!m_speaker) return 0;
    kdDebug() << "KTTSD::setText: Setting text: '" << text << "'" << endl;
    uint jobNum = m_speechData->setText(text, talker, getAppId());
    return jobNum;
}

/**
* Adds another part to a text job.  Does not start speaking the text.
* (thread safe)
* @param text           The message to be spoken.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application,
*                       but if no such job, applies to the last job queued by any application.
* @return               Part number for the added part.  Parts are numbered starting at 1.
*
* The text is parsed into individual sentences.  Call getTextCount to retrieve
* the sentence count.  Call startText to mark the job as speakable and if the
* job is the first speakable job in the queue, speaking will begin.
* @see setText.
* @see startText.
*/
int KTTSD::appendText(const QString &text, const uint jobNum /*=0*/)
{
    if (!m_speaker) return 0;
    return m_speechData->appendText(text, applyDefaultJobNum(jobNum), getAppId());
}

/**
* Queue a text job from the contents of a file.  Does not start speaking the text.
* @param filename       Full path to the file to be spoken.  May be a URL.
* @param talker         Code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
* @return               Job number.  0 if an error occurs.
*
* Plain text is parsed into individual sentences using the current sentence delimiter.
* Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling setText.
* Call @ref getTextCount to retrieve the sentence count after calling setText.
*
* The text may contain speech mark language, such as Sable, JSML, or SMML,
* provided that the speech plugin/engine support it.  In this case,
* sentence parsing follows the semantics of the markup language.
*
* Call @ref startText to mark the job as speakable and if the
* job is the first speakable job in the queue, speaking will begin.
* @see getTextCount
* @see startText
*/
uint KTTSD::setFile(const QString &filename, const QString &talker /*=NULL*/)
{
    // kdDebug() << "KTTSD::setFile: Running" << endl;
    if (!m_speaker) return 0;
    QFile file(filename);
    uint jobNum = 0;
    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream(&file);
        jobNum = m_speechData->setText(stream.read(), talker, getAppId());
        file.close();
    }
    return jobNum;
}

/**
* Get the number of sentences in a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @return               The number of sentences in the job.  -1 if no such job.
*
* The sentences of a job are given sequence numbers from 1 to the number returned by this
* method.  The sequence numbers are emitted in the @ref sentenceStarted and
* @ref sentenceFinished signals.
*/
int KTTSD::getTextCount(const uint jobNum /*=0*/)
{
    if (!m_speaker) return -1;
    return m_speechData->getTextCount(applyDefaultJobNum(jobNum));
}

/**
* Get the job number of the current text job.
* @return               Job number of the current text job. 0 if no jobs.
*
* Note that the current job may not be speaking. See @ref isSpeakingText.
* @see getTextJobState.
* @see isSpeakingText
*/
uint KTTSD::getCurrentTextJob()
{
    if (!m_speaker) return 0;
    return m_speaker->getCurrentTextJob();
}

/**
* Get the number of jobs in the text job queue.
* @return               Number of text jobs in the queue.  0 if none.
*/
uint KTTSD::getTextJobCount()
{
    if (!m_speaker) return 0;
    return m_speechData->getTextJobCount();
}

/**
* Get a comma-separated list of text job numbers in the queue.
* @return               Comma-separated list of text job numbers in the queue.
*/
QString KTTSD::getTextJobNumbers()
{
    if (!m_speaker) return QString::null;
    return m_speechData->getTextJobNumbers();
}

/**
* Get the state of a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @return               State of the job. -1 if invalid job number.
*
* @see kttsdJobState
*/
int KTTSD::getTextJobState(const uint jobNum /*=0*/)
{
    if (!m_speaker) return -1;
    return m_speechData->getTextJobState(applyDefaultJobNum(jobNum));
}

/**
* Get information about a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @return               A QDataStream containing information about the job.
*                       Blank if no such job.
*
* The stream contains the following elements:
*   - int state         Job state.
*   - QCString appId    DCOP senderId of the application that requested the speech job.
*   - QString talker    Language code in which to speak the text.
*   - int seq           Current sentence being spoken.  Sentences are numbered starting at 1.
*   - int sentenceCount Total number of sentences in the job.
*
* The following sample code will decode the stream:
    @verbatim
    QByteArray jobInfo = getTextJobInfo(jobNum);
    QDataStream stream(jobInfo, IO_ReadOnly);
    int state;
    QCString appId;
    QString talker;
    int seq;
    int sentenceCount;
    stream >> state;
    stream >> appId;
    stream >> talker;
    stream >> seq;
    stream >> sentenceCount;
    @endverbatim
*/
QByteArray KTTSD::getTextJobInfo(const uint jobNum /*=0*/)
{
    return m_speechData->getTextJobInfo(applyDefaultJobNum(jobNum));
}

/**
* Given a Talker Code, returns the Talker ID of the talker that would speak
* a text job with that Talker Code.
* @param talkerCode     Talker Code.
* @return               Talker ID of the talker that would speak the text job.
*/
QString KTTSD::talkerCodeToTalkerId(const QString& talkerCode)
{
    if (!m_speaker) return QString::null;
    return m_speaker->talkerCodeToTalkerId(talkerCode);
}

/**
* Return a sentence of a job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @param seq            Sequence number of the sentence.
* @return               The specified sentence in the specified job.  If no such
*                       job or sentence, returns "".
*/
QString KTTSD::getTextJobSentence(const uint jobNum /*=0*/, const uint seq /*=1*/)
{
    return m_speechData->getTextJobSentence(applyDefaultJobNum(jobNum), seq);
}

/**
* Determine if kttsd is currently speaking any text jobs.
* @return               True if currently speaking any text jobs.
*/
bool KTTSD::isSpeakingText()
{
    if (!m_speaker) return false;
    return m_speaker->isSpeakingText();
}

/**
* Remove a text job from the queue.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* The job is deleted from the queue and the @ref textRemoved signal is emitted.
*
* If there is another job in the text queue, and it is marked speakable,
* that job begins speaking.
*/
void KTTSD::removeText(const uint jobNum /*=0*/)
{
    kdDebug() << "KTTSD::removeText: Running" << endl;
    if (!m_speaker) return;
    m_speaker->removeText(applyDefaultJobNum(jobNum));
}

/**
* Start a text job at the beginning.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* Rewinds the job to the beginning.
*
* The job is marked speakable.
* If there are other speakable jobs preceeding this one in the queue,
* those jobs continue speaking and when finished, this job will begin speaking.
* If there are no other speakable jobs preceeding this one, it begins speaking.
*
* The @ref textStarted signal is emitted when the text job begins speaking.
* When all the sentences of the job have been spoken, the job is marked for deletion from
* the text queue and the @ref textFinished signal is emitted.
*/
void KTTSD::startText(const uint jobNum /*=0*/)
{
    kdDebug() << "KTTSD::startText: Running" << endl;
    if (!m_speaker) return;
    m_speaker->startText(applyDefaultJobNum(jobNum));
}

/**
* Stop a text job and rewind to the beginning.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* The job is marked not speakable and will not be speakable until @ref startText or @ref resumeText
* is called.
*
* If there are speaking jobs preceeding this one in the queue, they continue speaking.
* If the job is currently speaking, the @ref textStopped signal is emitted and the job stops speaking.
* Depending upon the speech engine and plugin used, speeking may not stop immediately
* (it might finish the current sentence).
*/
void KTTSD::stopText(const uint jobNum /*=0*/)
{
    kdDebug() << "KTTSD::stopText: Running" << endl;
    if (!m_speaker) return;
    m_speaker->stopText(applyDefaultJobNum(jobNum));
}

/**
* Pause a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* The job is marked as paused and will not be speakable until @ref resumeText or
* @ref startText is called.
*
* If there are speaking jobs preceeding this one in the queue, they continue speaking.
* If the job is currently speaking, the @ref textPaused signal is emitted and the job stops speaking.
* Depending upon the speech engine and plugin used, speeking may not stop immediately
* (it might finish the current sentence).
* @see resumeText
*/
void KTTSD::pauseText(const uint jobNum /*=0*/)
{
    kdDebug() << "KTTSD::pauseText: Running" << endl;
    if (!m_speaker) return;
    m_speaker->pauseText(applyDefaultJobNum(jobNum));
}

/**
* Start or resume a text job where it was paused.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* The job is marked speakable.
*
* If the job was not paused, it is the same as calling @ref startText.
*
* If there are speaking jobs preceeding this one in the queue, those jobs continue speaking and,
* when finished this job will begin speaking where it left off.
*
* The @ref textResumed signal is emitted when the job resumes.
* @see pauseText
*/
void KTTSD::resumeText(const uint jobNum /*=0*/)
{
    kdDebug() << "KTTSD::resumeText: Running" << endl;
    if (!m_speaker) return;
    m_speaker->resumeText(applyDefaultJobNum(jobNum));
}

/**
* Get a list of the talkers configured in KTTS.
* @return               A QStringList of fully-specified talker codes, one
*                       for each talker user has configured.
*
* @see talkers
*/
QStringList KTTSD::getTalkers()
{
    if (!m_speaker) return QStringList();
    return m_speaker->getTalkers();
}

/**
* Change the talker for a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @param talker         New code for the talker to do the speaking.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified Talker code,
*                       defaults to the closest matching talker.
*/
void KTTSD::changeTextTalker(const uint jobNum /*=0*/, const QString &talker /*=NULL*/)
{
    m_speechData->changeTextTalker(applyDefaultJobNum(jobNum), talker);
}

/**
* Get the user's default talker.
* @return               A fully-specified talker code.
*
* @see talkers
* @see getTalkers
*/
QString KTTSD::userDefaultTalker()
{
    if (!m_speaker) return QString::null;
    return m_speaker->userDefaultTalker();
}

/**
* Move a text job down in the queue so that it is spoken later.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* If the job is currently speaking, it is paused.
* If the next job in the queue is speakable, it begins speaking.
*/
void KTTSD::moveTextLater(const uint jobNum /*=0*/)
{
    if (!m_speaker) return;
    m_speaker->moveTextLater(applyDefaultJobNum(jobNum));
}

/**
* Jump to the first sentence of a specified part of a text job.
* @param partNum        Part number of the part to jump to.  Parts are numbered starting at 1.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application,
*                       but if no such job, applies to the last job queued by any application.
* @return               Part number of the part actually jumped to.
*
* If partNum is greater than the number of parts in the job, jumps to last part.
* If partNum is 0, does nothing and returns the current part number.
* If no such job, does nothing and returns 0.
* Does not affect the current speaking/not-speaking state of the job.
*/
int KTTSD::jumpToTextPart(const int partNum, const uint jobNum /*=0*/)
{
    if (!m_speaker) return 0;
    return m_speaker->jumpToTextPart(partNum, applyDefaultJobNum(jobNum));
}

/**
* Advance or rewind N sentences in a text job.
* @param n              Number of sentences to advance (positive) or rewind (negative) in the job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application,
*                       but if no such job, applies to the last job queued by any application.
* @return               Sequence number of the sentence actually moved to.  Sequence numbers
*                       are numbered starting at 1.
*
* If no such job, does nothing and returns 0.
* If n is zero, returns the current sequence number of the job.
* Does not affect the current speaking/not-speaking state of the job.
*/
uint KTTSD::moveRelTextSentence(const int n, const uint jobNum /*=0*/)
{
    if (!m_speaker) return 0;
    return m_speaker->moveRelTextSentence(n, applyDefaultJobNum(jobNum));
}

/**
* Add the clipboard contents to the text queue and begin speaking it.
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

/**
* Displays the %KTTS Manager dialog.  In this dialog, the user may backup or skip forward in
* any text job by sentence or paragraph, rewind jobs, pause or resume jobs, or
* delete jobs.
*/
void KTTSD::showDialog()
{
    KRun::runCommand("kttsmgr");
}

/**
* Stop the service.
*/
void KTTSD::kttsdExit()
{
    stopText();
    kdDebug() << "KTTSD::kttsdExit: Emitting DCOP signal kttsdExiting()" << endl;
    kttsdExiting();
    kapp->quit();
}

/**
* Re-start %KTTSD.
*/
void KTTSD::reinit()
{
    // Restart ourself.
    kdDebug() << "KTTSD::reinit: Running" << endl;
    if (m_speaker)
    {
        kdDebug() << "KTTSD::reinit: Stopping KTTSD service" << endl;
        if (m_speaker->isSpeakingText()) pauseText();
        m_speaker->requestExit();
    }
    delete m_speaker;
    m_speaker = 0;
    ready();
}

/*
* Checks if KTTSD is ready to speak and at least one talker is configured.
* If not, user is prompted to display the configuration dialog.
*/
bool KTTSD::ready()
{
    if (m_speaker) return true;
    kdDebug() << "KTTSD::ready: Starting KTTSD service" << endl;
    if (!initializeSpeechData()) return false;
    if (!initializeSpeaker()) return false;
    m_speaker->doUtterances();
    kdDebug() << "KTTSD::ready: Emitting DCOP signal kttsdStarted()" << endl;
    kttsdStarted();
    return true;
}

void KTTSD::configCommitted() {
    if (m_speaker) reinit();
}

/**
* This signal is emitted by KNotify when a notification event occurs.
*/
void KTTSD::notificationSignal( const QString&, const QString&,
                                     const QString &text, const QString&, const QString&,
                                     const int present, const int, const int, const int )
{
    if (!m_speaker) return;
    if (!text.isNull())
        if ( m_speechData->notify )
            if ( m_speechData->notifyPassivePopupsOnly 
                or (present & KNotifyClient::PassivePopup) )
                {
                    m_speechData->enqueueMessage(text, NULL, getAppId());
                    m_speaker->doUtterances();
                }
}

// Slots for the speaker object
void KTTSD::slotSentenceStarted(QString, QString, const QCString& appId, 
    const uint jobNum, const uint seq) {
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotSentenceStarted: Emitting DCOP signal sentenceStarted with appId " << appId << " job number " << jobNum << "  seq number " << seq << endl;
    sentenceStarted(appId, jobNum, seq);
}

void KTTSD::slotSentenceFinished(const QCString& appId, const uint jobNum, const uint seq){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotSentenceFinished: Emitting DCOP signal sentenceFinished with appId " << appId << " job number " << jobNum << "  seq number " << seq << endl;
    sentenceFinished(appId, jobNum, seq);
}

// Slots for the speechData and speaker objects.
void KTTSD::slotTextStarted(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextStarted: Emitting DCOP signal textStarted with appId " << appId << " job number " << jobNum << endl;
    textStarted(appId, jobNum);
}

void KTTSD::slotTextFinished(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextFinished: Emitting DCOP signal textFinished with appId " << appId << " job number " << jobNum << endl;
    textFinished(appId, jobNum);
}

void KTTSD::slotTextStopped(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextStopped: Emitting DCOP signal textStopped with appId " << appId << " job number " << jobNum << endl;
    textStopped(appId, jobNum);
}

void KTTSD::slotTextPaused(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextPaused: Emitting DCOP signal textPaused with appId " << appId << " job number " << jobNum << endl;
    textPaused(appId, jobNum);
}

void KTTSD::slotTextResumed(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextResumed: Emitting DCOP signal textResumed with appId " << appId << " job number " << jobNum << endl;
    textResumed(appId, jobNum);
}

//void KTTSD::slotTextSet(const QCString& appId, const uint jobNum){
void KTTSD::slotTextSet(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextSet: Emitting DCOP signal textSet with appId " << appId << " job number " << jobNum << endl;
    textSet(appId, jobNum);
}

void KTTSD::slotTextAppended(const QCString& appId, const uint jobNum, const int partNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextAppended: Emitting DCOP signal textAppended with appId " <<
        appId << " job number " << jobNum << " part number " << partNum << endl;
    textAppended(appId, jobNum, partNum);
}

void KTTSD::slotTextRemoved(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "KTTSD::slotTextRemoved: Emitting DCOP signal textRemoved with appId " << appId << " job number " << jobNum << endl;
    textRemoved(appId, jobNum);
}

/**
 * Returns the senderId (appId) of the DCOP application that called us.
 * @return              The DCOP sendId of calling application.
 */
const QCString KTTSD::getAppId()
{
    DCOPClient* client = callingDcopClient();
    QCString appId;
    if (client) appId = client->senderId();
    return appId;
}

/**
* If a job number is 0, returns the default job number for a command.
* Returns the job number of the last job queued by the application, or if
* no such job, the current job number.
* @return                Default job number.  0 if no such job.
*/
uint KTTSD::applyDefaultJobNum(const uint jobNum)
{
    uint jNum = jobNum;
    if (!jNum)
    {
        jNum = m_speechData->findAJobNumByAppId(getAppId());
        if (!jNum) jNum = getCurrentTextJob();
        if (!jNum) jNum = m_speechData->findAJobNumByAppId(0);
    }
    return jNum;
}

#include "kttsd.moc"

