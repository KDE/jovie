/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposproc.cpp
  Main speaking functions for the Epos Plug in
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$

// Qt includes.
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qfile.h>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kprocess.h>

// Epos Plugin includes.
#include "eposproc.h"
#include "eposproc.moc"
 
/** Constructor */
EposProc::EposProc( QObject* parent, const char* name, const QStringList& ) : 
    PlugInProc( parent, name ){
    kdDebug() << "EposProc::EposProc: Running" << endl;
    m_state = psIdle;
    m_waitingStop = false;
    m_eposServerProc = 0;
    m_eposProc = 0;
}

/** Destructor */
EposProc::~EposProc(){
    kdDebug() << "EposProc::~EposProc:: Running" << endl;
    if (m_eposProc)
    {
        stopText();
        delete m_eposProc;
    }
    if (m_eposServerProc)
    {
        m_eposServerProc->kill();
        delete m_eposServerProc;
    }
}

/** Initialize the speech */
bool EposProc::init(const QString& lang, KConfig* config){
    // kdDebug() << "Running: EposProc::init(const QString &lang)" << endl;
    // kdDebug() << "Initializing plug in: Epos" << endl;
    // Retrieve path to epos executable.
    config->setGroup(QString("Lang_")+lang);
    m_eposServerExePath = config->readPathEntry("EposServerExePath", "epos");
    m_eposClientExePath = config->readPathEntry("EposClientExePath", "say");
    m_eposServerOptions = config->readEntry("EposServerOptions", "");
    m_eposClientOptions = config->readEntry("EposClientOptions", "");
    kdDebug() << "EposProc::init: path to epos server: " << m_eposServerExePath << endl;
    kdDebug() << "EposProc::init: path to epos client: " << m_eposClientExePath << endl;
    
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
    kdDebug() << "EposProc::init: Initialized with codec: " << codecString << endl;
    return true;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void EposProc::sayText(const QString &text)
{
    synth(text, QString::null, m_eposServerExePath, m_eposClientExePath,
        m_eposServerOptions, m_eposClientOptions,
        m_codec, QTextCodec::codecForIndex(m_codec));
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
void EposProc::synthText(const QString& text, const QString& suggestedFilename)
{
    synth(text, suggestedFilename, m_eposServerExePath, m_eposClientExePath,
        m_eposServerOptions, m_eposClientOptions,
        m_codec, QTextCodec::codecForIndex(m_codec));
};

/**
* Say or Synthesize text.
* @param text                    The text to be synthesized.
* @param suggestedFilename       If not Null, synthesize only to this filename, otherwise
*                                synthesize and audibilize the text.
* @param eposServerExePath       Path to the Epos server executable.
* @param eposClientExePath       Path to the Epos client executable.
* @param eposServerOptions       Options passed to Epos server executable.
* @param eposClientOptions       Options passed to Epos client executable (don't include -o).
* @param encoding                Codec index.
* @param codec                   Codec if encoding not Local, Latin1, or Unicode.
*/
void EposProc::synth(
    const QString &text,
    const QString &suggestedFilename,
    const QString& eposServerExePath,
    const QString& eposClientExePath,
    const QString& eposServerOptions,
    const QString& eposClientOptions,
    int encoding,
    QTextCodec *codec)
{
    // kdDebug() << "Running: EposProc::synth(const QString &text)" << endl;

    if (m_eposProc)
    {
        if (m_eposProc->isRunning()) m_eposProc->kill();
        delete m_eposProc;
        m_eposProc = 0;
    }
    // Start the Epos server if not already started.
    if (!m_eposServerProc)
    {
        KProcess* m_eposServerProc = new KProcess;
        *m_eposServerProc << eposServerExePath;
        if (!eposServerOptions.isNull())
            *m_eposServerProc << eposServerOptions;
        m_eposServerProc->start(KProcess::DontCare, KProcess::NoCommunication);
    }
    
    // Encode the text.
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
    
    // Quote the text as one parameter.
    QString escText = KShellProcess::quote(encText);
    
    // kdDebug()<< "EposProc::synth: Creating Epos object" << endl;
    m_eposProc = new KProcess;
    m_eposProc->setUseShell(true);
    *m_eposProc << eposClientExePath;
    if (!suggestedFilename.isNull()) 
        *m_eposProc << "-o";
    if (!eposClientOptions.isNull())
        *m_eposProc << eposClientOptions;
    // TODO: Set language (-l option) based on KTTSD language group.
    *m_eposProc << escText;
    if (!suggestedFilename.isNull()) 
        *m_eposProc << " >" + suggestedFilename;
    connect(m_eposProc, SIGNAL(processExited(KProcess*)),
        this, SLOT(slotProcessExited(KProcess*)));
    connect(m_eposProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(m_eposProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
//    connect(m_eposProc, SIGNAL(wroteStdin(KProcess*)),
//        this, SLOT(slotWroteStdin(KProcess* )));
    if (suggestedFilename.isNull())
        m_state = psSaying;
    else
        m_state = psSynthing;
    
    // Ok, let's rock.
    m_synthFilename = suggestedFilename;
    kdDebug() << "EposProc::synth: Synthing text: '" << text << "' using Epos plug in" << endl;
    if (!m_eposProc->start(KProcess::NotifyOnExit, KProcess::AllOutput))
    {
        kdDebug() << "EposProc::synth: Error starting Epos process.  Is epos in the PATH?" << endl;
        m_state = psIdle;
        return;
    }
    kdDebug()<< "EposProc:synth: Epos initialized" << endl;
}
        
/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString EposProc::getFilename()
{
    kdDebug() << "EposProc::getFilename: returning " << m_synthFilename << endl;
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
void EposProc::stopText(){
    kdDebug() << "EposProc::stopText:: Running" << endl;
    if (m_eposProc)
    {
        if (m_eposProc->isRunning())
        {
            kdDebug() << "EposProc::stopText: killing Epos." << endl;
            m_waitingStop = true;
            m_eposProc->kill();
        } else m_state = psIdle;
    }else m_state = psIdle;
    kdDebug() << "EposProc::stopText: Epos stopped." << endl;
}

void EposProc::slotProcessExited(KProcess*)
{
    kdDebug() << "EposProc:slotProcessExited: Epos process has exited." << endl;
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

void EposProc::slotReceivedStdout(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "EposProc::slotReceivedStdout: Received output from Epos: " << buf << endl;
}

void EposProc::slotReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "EposProc::slotReceivedStderr: Received error from Epos: " << buf << endl;
}

void EposProc::slotWroteStdin(KProcess*)
{
    kdDebug() << "EposProc::slotWroteStdin: closing Stdin" << endl;
    m_eposProc->closeStdin();
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @see pluginState
*/
pluginState EposProc::getState() { return m_state; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void EposProc::ackFinished()
{
    if (m_state == psFinished)
    {
        m_state = psIdle;
        m_synthFilename = QString::null;
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
bool EposProc::supportsAsync() { return true; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool EposProc::supportsSynth() { return true; }
    
