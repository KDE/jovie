/***************************************************** vim:set ts=4 sw=4 sts=4:
  commandproc.cpp
  Main speaking functions for the Command Plug in
  -------------------
  Copyright : (C) 2002 by Gunnar Schmi Dt and 2004 by Gary Cramblitt
  -------------------
  Original author: Gunnar Schmi Dt <kmouth@schmi-dt.de>
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
#include <qfile.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qvaluestack.h>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

// KTTS includes.
#include <pluginproc.h>

// Command Plugin includes.
#include "commandproc.h"
#include "commandproc.moc"

/** Constructor */
CommandProc::CommandProc( QObject* parent, const char* name, const QStringList& /*args*/) : 
    PlugInProc( parent, name )
{
    kdDebug() << "CommandProc::CommandProc: Running" << endl;
    m_commandProc = 0;
    m_state = psIdle;
    m_stdin = true;
    m_codec = 0;
    m_supportsSynth = false;
    m_waitingStop = false;
}

/** Destructor */
CommandProc::~CommandProc()
{
    kdDebug() << "CommandProc::~CommandProc: Running" << endl;
    if (m_commandProc)
    {
        if (m_commandProc->isRunning()) m_commandProc->kill();
        delete m_commandProc;
        // Don't delete synth file.  That is responsibility of caller.
        if (!m_textFilename.isNull()) QFile::remove(m_textFilename);
    }
}

/** Initialize */
bool CommandProc::init(const QString &lang, KConfig *config){
    kdDebug() << "CommandProc::init: Initializing plug in: Command for language " << lang << endl;

    config->setGroup(QString("Lang_")+lang);
    m_ttsCommand = config->readEntry("Command", "cat -");
    m_stdin = config->readBoolEntry("StdIn", true);
    m_language = lang;
    
    // Support separate synthesis if the TTS command contains %w macro.
    m_supportsSynth = (m_ttsCommand.contains("%w"));

    // Build codec list.
    QPtrList<QTextCodec>* codecList = new QPtrList<QTextCodec>;
    QTextCodec *codec;
    int i;
    for (i = 0; (codec = QTextCodec::codecForIndex(i)); i++)
        codecList->append (codec);
    
    QString codecString = config->readEntry("Codec", "Local");
    if (codecString == "Local")
        m_codec = Local;
    else if (codecString == "Latin1")
        m_codec = Latin1;
    else if (codecString == "Unicode")
        m_codec = Unicode;
    else {
        m_codec = Local;
        for (unsigned int i = 0; i < codecList->count(); i++ )
            if (codecString == codecList->at(i)->name())
                m_codec = UseCodec + i;
    }
    kdDebug() << "CommandProc::init: Initialized with command: " << m_ttsCommand << " codec: " << codecString << endl;
    return true;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void CommandProc::sayText(const QString &text)
{
    synth(text, QString::null,
        m_ttsCommand, m_stdin, m_codec, QTextCodec::codecForIndex(m_codec), m_language);
}

/**
* Synthesize text into an audio file, but do not send to the audio device.
* @param text                    The text to be synthesized.
* @param suggestedFilename       Full pathname of file to create.  The plugin
*                                may ignore this parameter and choose its own
*                                filename.  KTTSD will query the generated
*                                filename using getFilename().
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void CommandProc::synthText(const QString& text, const QString& suggestedFilename)
{
    synth(text, suggestedFilename,
        m_ttsCommand, m_stdin, m_codec, QTextCodec::codecForIndex(m_codec), m_language);
};

/**
* Say or Synthesize text.
* @param inputText               The text that shall be spoken
* @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
*                                synthesize and audibilize the text.
* @param userCmd                 The program that shall be executed for speaking
* @param stdIn                   True if the program shall recieve its data via standard input
* @param encoding                The number of the encoding that shall be used
* @param codec                   The QTextCodec if encoding==UseCodec
* @param language                The language code (used for the %l macro)
*/
void CommandProc::synth(const QString& inputText, const QString& suggestedFilename,
    const QString& userCmd, bool stdIn, int encoding, QTextCodec *codec, QString& language)
{
    if (m_commandProc)
    {
        if (m_commandProc->isRunning()) m_commandProc->kill();
        delete m_commandProc;
        m_commandProc = 0;
        m_synthFilename = QString::null;
        if (!m_textFilename.isNull()) QFile::remove(m_textFilename);
        m_textFilename = QString::null;
    }
    QString command = userCmd;
    QString text = inputText.stripWhiteSpace();
    if (text.isEmpty()) return;
    // 1. prepare the text:
    // 1.a) encode the text
    QByteArray encText;
    QTextStream ts (encText, IO_WriteOnly);
    if (encoding == Local)
        ts.setEncoding (QTextStream::Locale);
    else if (encoding == Latin1)
        ts.setEncoding (QTextStream::Latin1);
    else if (encoding == Unicode)
        ts.setEncoding (QTextStream::Unicode);
    else
        ts.setCodec (codec);
    ts << text;
    ts << endl; // Some synths need this, eg. flite.
    
    // 1.b) quote the text as one parameter
    QString escText = KShellProcess::quote(encText);
    
    // 1.c) create a temporary file for the text, if %f macro is used.
    if (command.contains("%f"))
    {
        KTempFile tempFile(locateLocal("tmp", "commandplugin-"), ".txt");
        QTextStream* fs = tempFile.textStream();
        if (encoding == Local)
            fs->setEncoding (QTextStream::Locale);
        else if (encoding == Latin1)
            fs->setEncoding (QTextStream::Latin1);
        else if (encoding == Unicode)
            fs->setEncoding (QTextStream::Unicode);
        else
            fs->setCodec (codec);
        *fs << text;
        *fs << endl;
        m_textFilename = tempFile.file()->name();
        tempFile.close();
    } else m_textFilename = QString::null;
    
    // 2. replace variables with values
    QValueStack<bool> stack;
    bool issinglequote=false;
    bool isdoublequote=false;
    int noreplace=0;
    QRegExp re_noquote("(\"|'|\\\\|`|\\$\\(|\\$\\{|\\(|\\{|\\)|\\}|%%|%t|%f|%l|%w)");
    QRegExp re_singlequote("('|%%|%t|%f|%l|%w)");
    QRegExp re_doublequote("(\"|\\\\|`|\\$\\(|\\$\\{|%%|%t|%f|%l|%w)");

    for	( int i = re_noquote.search(command);
        i != -1;
        i = (issinglequote?re_singlequote.search(command,i)
            :isdoublequote?re_doublequote.search(command,i)
            :re_noquote.search(command,i))
    )
    {
        if ((command[i]=='(') || (command[i]=='{')) // (...) or {...}
        {
            // assert(isdoublequote == false)
            stack.push(isdoublequote);
            if (noreplace > 0)
                // count nested braces when within ${...}
                noreplace++;
            i++;
        }
        else if (command[i]=='$')
        {
            stack.push(isdoublequote);
            isdoublequote = false;
            if ((noreplace > 0) || (command[i+1]=='{'))
                // count nested braces when within ${...}
                noreplace++;
            i+=2;
        }
        else if ((command[i]==')') || (command[i]=='}'))
            // $(...) or (...) or ${...} or {...}
        {
            if (!stack.isEmpty())
                isdoublequote = stack.pop();
            else
                qWarning("Parse error.");
            if (noreplace > 0)
                // count nested braces when within ${...}
                noreplace--;
            i++;
        }
        else if (command[i]=='\'')
        {
            issinglequote=!issinglequote;
            i++;
        }
        else if (command[i]=='"')
        {
            isdoublequote=!isdoublequote;
            i++;
        }
        else if (command[i]=='\\')
            i+=2;
        else if (command[i]=='`')
        {
            // Replace all `...` with safer $(...)
            command.replace (i, 1, "$(");
            QRegExp re_backticks("(`|\\\\`|\\\\\\\\|\\\\\\$)");
            for (	int i2=re_backticks.search(command,i+2);
                i2!=-1;
                i2=re_backticks.search(command,i2)
            )
            {
                if (command[i2] == '`')
                {
                    command.replace (i2, 1, ")");
                    i2=command.length(); // leave loop
                }
                else
                {   // remove backslash and ignore following character
                    command.remove (i2, 1);
                    i2++;
                }
            }
            // Leave i unchanged! We need to process "$("
        }
        else if (noreplace == 0) // do not replace macros within ${...}
        {
            QString match, v;
    
            // get match
            if (issinglequote)
                match=re_singlequote.cap();
            else if (isdoublequote)
                match=re_doublequote.cap();
            else
                match=re_noquote.cap();
    
            // substitue %variables
            if (match=="%%")
                v="%";
            else if (match=="%t")
                v=escText;
            else if (match=="%f")
                v=m_textFilename;
            else if (match=="%l")
                v=language;
            else if (match=="%w")
                v = suggestedFilename;
    
            // %variable inside of a quote?
            if (isdoublequote)
                v='"'+v+'"';
            else if (issinglequote)
                v="'"+v+"'";
    
            command.replace (i, match.length(), v);
            i+=v.length();
        }
        else
        {
            if (issinglequote)
                i+=re_singlequote.matchedLength();
            else if (isdoublequote)
                i+=re_doublequote.matchedLength();
            else
                i+=re_noquote.matchedLength();
        }
    }

    // 3. create a new process
    kdDebug() << "CommandProc::synth: running command: " << command << endl;
    m_commandProc = new KProcess;
    m_commandProc->setUseShell(true);
    *m_commandProc << command;
    connect(m_commandProc, SIGNAL(processExited(KProcess*)),
        this, SLOT(slotProcessExited(KProcess*)));
    connect(m_commandProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(m_commandProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
    connect(m_commandProc, SIGNAL(wroteStdin(KProcess*)),
        this, SLOT(slotWroteStdin(KProcess* )));
    
    // 4. start the process
    
    if (suggestedFilename.isNull())
        m_state = psSaying;
    else
    {
        m_synthFilename = suggestedFilename;
        m_state = psSynthing;
    }
    if (stdIn) {
        m_commandProc->start(KProcess::NotifyOnExit, KProcess::All);
        if (encText.size() > 0)
            m_commandProc->writeStdin(encText, encText.size());
        else
            m_commandProc->closeStdin();
    }
    else
        m_commandProc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString CommandProc::getFilename()
{
    kdDebug() << "CommandProc::getFilename: returning " << m_synthFilename << endl;
    return m_synthFilename;
}

/**
* Stop current operation (saying or synthesizing text).
* Important: This function may be called from a thread different from the
* one that called sayText or synthText.
* If the plugin cannot stop an in-progress @ref sayText or
* @ref synthText operation, it must not block waiting for it to complete.
* Instead, return immediately.
*
* If a plugin returns before the operation has actually been stopped,
* the plugin must emit the @ref stopped signal when the operation has
* actually stopped.
*
* The plugin should change to the psIdle state after stopping the
* operation.
*/
void CommandProc::stopText(){
    kdDebug() << "CommandProc::stopText: Running" << endl;
    if (m_commandProc)
    {
        if (m_commandProc->isRunning())
        {
            kdDebug() << "CommandProc::stopText: killing Command." << endl;
            m_waitingStop = true;
            m_commandProc->kill();
        } else m_state = psIdle;
    }else m_state = psIdle;
    kdDebug() << "CommandProc::stopText: Command stopped." << endl;
}

void CommandProc::slotProcessExited(KProcess*)
{
    kdDebug() << "CommandProc:slotProcessExited: Command process has exited." << endl;
    pluginState prevState = m_state;
    if (m_waitingStop)
    {
        m_waitingStop = false;
        m_state = psIdle;
        emit stopped();
    } else {
        m_state = psFinished;
        if (prevState == psSaying)
            emit sayFinished();
        else
            if (prevState == psSynthing)
                emit synthFinished();
    }
}

void CommandProc::slotReceivedStdout(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "CommandProc::slotReceivedStdout: Received output from Command: " << buf << endl;
}

void CommandProc::slotReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "CommandProc::slotReceivedStderr: Received error from Command: " << buf << endl;
}

void CommandProc::slotWroteStdin(KProcess*)
{
    kdDebug() << "CommandProc::slotWroteStdin: closing Stdin" << endl;
    m_commandProc->closeStdin();
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @see pluginState
*/
pluginState CommandProc::getState() { return m_state; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void CommandProc::ackFinished()
{
    if (m_state == psFinished)
    {
        m_state = psIdle;
        m_synthFilename = QString::null;
        if (!m_textFilename.isNull()) QFile::remove(m_textFilename);
        m_textFilename = QString::null;
    }
}

/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*
* If the plugin returns True, it must also implement @ref getState .
* It must also emit @ref sayFinished or @ref synthFinished signals when
* saying or synthesis is completed.
*/
bool CommandProc::supportsAsync() { return true; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool CommandProc::supportsSynth() { return m_supportsSynth; }
    
