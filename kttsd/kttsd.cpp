/***************************************************** vim:set ts=4 sw=4 sts=4:
  kttsd.cpp
  KTTSD main class
  -------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <qfile.h>
#include <kfiledialog.h>
#include <dcopclient.h>

#include "kttsd.h"

KTTSD::KTTSD(QObject *parent, const char *name) : 
    QObject(parent, name),
    DCOPObject("kspeech")
{
    kdDebug() << "Running: KTTSD::KTTSD(const char *name)" << endl;
    // Do stuff here
    //setIdleTimeout(15); // 15 seconds idle timeout.
    
    speaker = 0;
    speechData = 0;
    if (!initializeSpeechData()) return;   
    if (!initializeSpeaker()) return;
    
    // Let's rock!
    speaker->start();

    kdDebug() << "emitting DCOP signal kttsdStarted()" << endl;
    kttsdStarted();
}

/*
* Create and initialize the SpeechData object.
*/
bool KTTSD::initializeSpeechData()
{
    // Create speechData object, and load configuration, checking if valid config loaded.
    bool created = false;
    if (!speechData)
    {
        created = true;
        speechData = new SpeechData();
    }
    // Load configuration.
    if (!speechData->readConfig())
    {
        KMessageBox::error(0, i18n("No default language defined. Please configure kttsd in the KDE Control center before use. Text to speech service exiting."), i18n("Text To Speech Error"));
        delete speechData;
        speechData = 0;
        ok = false;
        return false;
    }

    if (created)
    {
        connect (speechData, SIGNAL(textStarted(const QCString&, const uint)), 
            this, SLOT(slotTextStarted(const QCString&, const uint)));
        connect (speechData, SIGNAL(textFinished(const QCString&, const uint)), 
            this, SLOT(slotTextFinished(const QCString&, const uint)));
        connect (speechData, SIGNAL(textStopped(const QCString&, const uint)), 
            this, SLOT(slotTextStopped(const QCString&, const uint)));
        connect (speechData, SIGNAL(textPaused(const QCString&, const uint)), 
            this, SLOT(slotTextPaused(const QCString&, const uint)));
        connect (speechData, SIGNAL(textResumed(const QCString&, const uint)), 
            this, SLOT(slotTextResumed(const QCString&, const uint)));
        connect (speechData, SIGNAL(textSet(const QCString&, const uint)), 
            this, SLOT(slotTextSet(const QCString&, const uint)));
        connect (speechData, SIGNAL(textRemoved(const QCString&, const uint)), 
            this, SLOT(slotTextRemoved(const QCString&, const uint)));
    }

    return true;
}

/*
* Create and initialize the Speaker object.
*/
bool KTTSD::initializeSpeaker()
{
    kdDebug() << "Instantiating Speaker and running it as another thread" << endl;

    // By default, everything is ok, don't worry, be happy
    ok = true;

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
    
    connect (speaker, SIGNAL(sentenceStarted(QString, QString, const QCString&, const uint, const uint)), this,
        SLOT(slotSentenceStarted(QString, QString, const QCString&, const uint, const uint)));
    connect (speaker, SIGNAL(sentenceFinished(const QCString&, const uint, const uint)), this,
        SLOT(slotSentenceFinished(const QCString&, const uint, const uint)));

    return true;
}

/**
 * Destructor
 * Terminate speaker thread
 */
KTTSD::~KTTSD(){
    kdDebug() << "KTTSD::~KTTSD:: Stopping KTTSD service" << endl;
    if (speaker)
    {
        // Stop the speaker thread.
        speaker->requestExit();
        if (!speaker->wait(1000))
        {
            // We cannot wait for speaker to terminate, so blow it off and hope for the best.
            speaker->terminate();
            speaker->wait();
        }
        delete speaker;
    }
    delete speechData;
    kdDebug() << "emitting DCOP signal kttsdExiting()" << endl;
    kttsdExiting();
}

/***** DCOP exported functions *****/

/**
* Determine whether the currently-configured speech plugin supports a speech markup language.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
* @param markupType     The kttsd code for the desired speech markup language.
* @return               True if the plugin currently configured for the indicated
*                       talker supports the indicated speech markup language.
* @see kttsdMarkupType
* TODO: Waiting for plugin api.
*/
bool KTTSD::supportsMarkup(const QString&, const uint) { return false; }
        
/**
* Determine whether the currently-configured speech plugin supports markers in speech markup.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
* @return               True if the plugin currently configured for the indicated
*                       talker supports markers.
* TODO: Waiting on plugin API.
*/
bool KTTSD::supportsMarkers(const QString&) { return false; }
        
/**
* Say a warning.  The warning will be spoken when the current sentence
* stops speaking and takes precedence over Messages and regular text.  Warnings should only
* be used for high-priority messages requiring immediate user attention, such as
* "WARNING. CPU is overheating."
* @param warning        The warning to be spoken.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified language code,
*                       defaults to the user's default talker.
*/
void KTTSD::sayWarning(const QString &warning, const QString &talker){
    kdDebug() << "Running: KTTSD::sayWarning(const QString &warning, const QString &talker=NULL)" << endl;
    kdDebug() << "Adding '" << warning << "' to warning queue." << endl;
    speechData->enqueueWarning(warning, talker, getAppId());
}

/**
* Say a message.  The message will be spoken when the current text paragraph
* stops speaking.  Messages should be used for one-shot messages that can't wait for
* normal text messages to stop speaking, such as "You have mail.".
* @param message        The message to be spoken.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no talker has been configured for the specified language code,
*                       defaults to the user's default talker.
*/
void KTTSD::sayMessage(const QString &message, const QString &talker){
    kdDebug() << "Running: KTTSD::sayMessage(const QString &message, const QString &talker=NULL)" << endl;
    kdDebug() << "Adding '" << message << "' to message queue." << endl;
    speechData->enqueueMessage(message, talker, getAppId());
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
void KTTSD::setSentenceDelimiter(const QString &delimiter) { speechData->setSentenceDelimiter(delimiter, getAppId()); }
        
/**
* Queue a text job.  Does not start speaking the text.
* @param text           The message to be spoken.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default plugin.
*                       If no plugin has been configured for the specified language code,
*                       defaults to the user's default plugin.
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
uint KTTSD::setText(const QString &text, const QString &talker){
    kdDebug() << "Running: setText(const QString &text, const QString &talker=NULL)" << endl;
    kdDebug() << "Setting text: '" << text << "'" << endl;
    uint jobNum = speechData->setText(text, talker, getAppId());
    return jobNum;
}

/**
* Queue a text job from the contents of a file.  Does not start speaking the text.
* @param filename       Full path to the file to be spoken.  May be a URL.
* @param talker         Code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified language code,
*                       defaults to the user's default talker.
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
uint KTTSD::setFile(const QString &filename, const QString &talker)
{
    kdDebug() << "Running: setFile(const QString &filename, const QString &talker=NULL)" << endl;
    QFile file(filename);
    uint jobNum = 0;
    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream(&file);
        jobNum = speechData->setText(stream.read(), talker, getAppId());
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
int KTTSD::getTextCount(const uint jobNum) { return speechData->getTextCount(jobNum, getAppId()); }

/**
* Get the job number of the current text job.
* @return               Job number of the current text job. 0 if no jobs.
*
* Note that the current job may not be speaking. See @ref isSpeakingText.
* @see getTextJobState.
* @see isSpeakingText
*/
uint KTTSD::getCurrentTextJob() { return speechData->getCurrentTextJob(); }
        
/**
* Get the number of jobs in the text job queue.
* @return               Number of text jobs in the queue.  0 if none.
*/
uint KTTSD::getTextJobCount() { return speechData->getTextJobCount(); }

/**
* Get a comma-separated list of text job numbers in the queue.
* @return               Comma-separated list of text job numbers in the queue.
*/
QString KTTSD::getTextJobNumbers() { return speechData->getTextJobNumbers(); }
        
/**
* Get the state of a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @return               State of the job. -1 if invalid job number.
*
* @see kttsdJobState
*/
int KTTSD::getTextJobState(const uint jobNum) { return speechData->getTextJobState(jobNum, getAppId()); }

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
QByteArray KTTSD::getTextJobInfo(const uint jobNum) { return speechData->getTextJobInfo(jobNum, getAppId()); }

/**
* Return a sentence of a job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @param seq            Sequence number of the sentence.
* @return               The specified sentence in the specified job.  If no such
*                       job or sentence, returns "".
*/
QString KTTSD::getTextJobSentence(const uint jobNum, const uint seq)
{
    return speechData->getTextJobSentence(jobNum, seq, getAppId());
}
       
/**
* Determine if kttsd is currently speaking any text jobs.
* @return               True if currently speaking any text jobs.
*/
bool KTTSD::isSpeakingText() { return speechData->isSpeakingText(); }
        
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
void KTTSD::removeText(const uint jobNum){
    kdDebug() << "Running: KTTSD::removeText()" << endl;
    stopText();
    speechData->removeText(jobNum, getAppId());
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
void KTTSD::startText(const uint jobNum){
    kdDebug() << "Running: KTTSD::startText()" << endl;
    speechData->startText(jobNum, getAppId());
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
void KTTSD::stopText(const uint jobNum){
    kdDebug() << "Running: KTTSD::stopText()" << endl;
    speechData->stopText(jobNum, getAppId());
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
void KTTSD::pauseText(const uint jobNum){
    kdDebug() << "Running: KTTSD::pauseText()" << endl;
    speechData->pauseText(jobNum, getAppId());
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
void KTTSD::resumeText(const uint jobNum){
    kdDebug() << "Running: KTTSD::resumeText()" << endl;
    speechData->resumeText(jobNum, getAppId());
}

/**
* Change the talker for a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
* @param talker         New code for the language to be spoken in.  Example "en".
*                       If NULL, defaults to the user's default talker.
*                       If no plugin has been configured for the specified language code,
*                       defaults to the user's default talker.
*/
void KTTSD::changeTextTalker(const uint jobNum, const QString &talker)
{
    speechData->changeTextTalker(jobNum, talker, getAppId());
};

/**
* Move a text job down in the queue so that it is spoken later.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*
* If the job is currently speaking, it is paused.
* If the next job in the queue is speakable, it begins speaking.
*/
void KTTSD::moveTextLater(const uint jobNum) { speechData->moveTextLater(jobNum, getAppId()); }

/**
* Go to the previous paragraph in a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*/
void KTTSD::prevParText(const uint jobNum){
    kdDebug() << "Running: KTTSD::prevParText()" << endl;
    speechData->prevParText(jobNum, getAppId());
}

/**
* Go to the previous sentence in the queue.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*/
void KTTSD::prevSenText(const uint jobNum){
    kdDebug() << "Running: KTTSD::prevSenText()" << endl;
    speechData->prevSenText(jobNum, getAppId());
}

/**
* Go to next sentence in a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*/
void KTTSD::nextSenText(const uint jobNum){
    kdDebug() << "Running: KTTSD::nextSenText()" << endl;
    speechData->nextSenText(jobNum, getAppId());
}

/**
* Go to next paragraph in a text job.
* @param jobNum         Job number of the text job.
*                       If zero, applies to the last job queued by the application.
*/
void KTTSD::nextParText(const uint jobNum){
    kdDebug() << "Running: KTTSD::nextParText()" << endl;
    speechData->nextParText(jobNum, getAppId());
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
    // TODO:
}

/**
* Stop the service.
*/
void KTTSD::kttsdExit()
{
    stopText();
    kdDebug() << "emitting DCOP signal kttsdExiting()" << endl;
    kttsdExiting();
    kapp->quit();
}

/**
* Re-start %KTTSD.
*/
void KTTSD::reinit()
{
    // Restart ourself.
    kdDebug() << "Running: KTTSD::reinit()" << endl;
    kdDebug() << "Stopping KTTSD service" << endl;
    if (speechData->isSpeakingText()) pauseText();
    if (speaker)
    {
        // Create a separate thread to terminate and wait for the speaker to exit.
        // We cannot do it here, otherwise, the main QT event loop would block.
        speakerTerminator = new SpeakerTerminator(speaker, this, "speakerTerminator");
        // When speaker exits, finishing reinit in slot speakerFinished.
        connect (speakerTerminator, SIGNAL(speakerFinished()), 
            this, SLOT(speakerFinished()));
        // Stop the speaker.
        speakerTerminator->start();
    }
}

void KTTSD::speakerFinished()
{
    // Speaker has exited.  Finish reinit.
    kdDebug() << "KTTSD::speakerFinished: running" << endl;
    delete speaker;
    speaker = 0;

    kdDebug() << "Starting KTTSD service" << endl;
    if (!initializeSpeechData()) return;
    if (!initializeSpeaker()) return;
    speaker->start();
    kdDebug() << "emitting DCOP signal kttsdStarted()" << endl;
    kttsdStarted();
}

void KTTSD::configCommitted() {
    if (speaker) reinit();
}

// Slots for the speaker object
void KTTSD::slotSentenceStarted(QString, QString, const QCString& appId, 
    const uint jobNum, const uint seq) {
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal sentenceStarted with appId " << appId << " job number " << jobNum << "  seq number " << seq << endl;
    sentenceStarted(appId, jobNum, seq);
}

void KTTSD::slotSentenceFinished(const QCString& appId, const uint jobNum, const uint seq){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal sentenceFinished with appId " << appId << " job number " << jobNum << "  seq number " << seq << endl;
    sentenceFinished(appId, jobNum, seq);
}

// Slots for the speechData object
void KTTSD::slotTextStarted(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textStarted with appId " << appId << " job number " << jobNum << endl;
    textStarted(appId, jobNum);
}

void KTTSD::slotTextFinished(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textFinished with appId " << appId << " job number " << jobNum << endl;
    textFinished(appId, jobNum);
}

void KTTSD::slotTextStopped(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textStopped with appId " << appId << " job number " << jobNum << endl;
    textStopped(appId, jobNum);
}

void KTTSD::slotTextPaused(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textPaused with appId " << appId << " job number " << jobNum << endl;
    textPaused(appId, jobNum);
}

void KTTSD::slotTextResumed(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textResumed with appId " << appId << " job number " << jobNum << endl;
    textResumed(appId, jobNum);
}

//void KTTSD::slotTextSet(const QCString& appId, const uint jobNum){
void KTTSD::slotTextSet(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textSet with appId " << appId << " job number " << jobNum << endl;
    textSet(appId, jobNum);
}

void KTTSD::slotTextRemoved(const QCString& appId, const uint jobNum){
    // Emit DCOP signal.
    kdDebug() << "emitting DCOP signal textRemoved with appId " << appId << " job number " << jobNum << endl;
    textRemoved(appId, jobNum);
}

/**
 * Returns the senderId (appId) of the DCOP application that called us.
 * @return appId         The DCOP sendId of calling application.  NULL if called internally by kttsd itself.
 */
const QCString KTTSD::getAppId()
{
    DCOPClient* client = callingDcopClient();
    QCString appId;
    if (client) appId = client->senderId();
    return appId;
}

#include "kttsd.moc"

