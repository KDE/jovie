/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposproc.cpp
  Main speaking functions for the Epos Plug in
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// C++ includes.
#include <math.h>

// Qt includes.
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qtextcodec.h>

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
    delete m_eposServerProc;
}

/** Initialize the speech */
bool EposProc::init(KConfig* config, const QString& configGroup)
{
    // kdDebug() << "EposProc::init: Running" << endl;
    // kdDebug() << "Initializing plug in: Epos" << endl;
    // Retrieve path to epos executable.
    config->setGroup(configGroup);
    m_eposServerExePath = config->readEntry("EposServerExePath", "epos");
    m_eposClientExePath = config->readEntry("EposClientExePath", "say");
    m_eposLanguage = config->readEntry("Language", QString::null);
    m_time = config->readNumEntry("time", 100);
    m_pitch = config->readNumEntry("pitch", 100);
    m_eposServerOptions = config->readEntry("EposServerOptions", QString::null);
    m_eposClientOptions = config->readEntry("EposClientOptions", QString::null);
    kdDebug() << "EposProc::init: path to epos server: " << m_eposServerExePath << endl;
    kdDebug() << "EposProc::init: path to epos client: " << m_eposClientExePath << endl;

    QString codecString = config->readEntry("Codec", "Local");
    m_codec = codecNameToCodec(codecString);
    // Start the Epos server if not already started.
    if (!m_eposServerProc)
    {
        KProcess* m_eposServerProc = new KProcess;
        *m_eposServerProc << m_eposServerExePath;
        if (!m_eposServerOptions.isEmpty())
            *m_eposServerProc << m_eposServerOptions;
        connect(m_eposServerProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
        connect(m_eposServerProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
        m_eposServerProc->start(KProcess::DontCare, KProcess::AllOutput);
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
        m_codec, m_eposLanguage, m_time, m_pitch);
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
        m_codec, m_eposLanguage, m_time, m_pitch);
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
* @param codec                   Codec for encoding of text.
* @param eposLanguage            Epos language setting.  "czech", "slovak",
*                                or null (default language).
* @param time                    Speed percentage. 50 to 200. 200% = 2x normal.
* @param pitch                   Pitch persentage.  50 to 200.
*/
void EposProc::synth(
    const QString &text,
    const QString &suggestedFilename,
    const QString& eposServerExePath,
    const QString& eposClientExePath,
    const QString& eposServerOptions,
    const QString& eposClientOptions,
    QTextCodec *codec,
    const QString& eposLanguage,
    int time,
    int pitch)
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
        if (!eposServerOptions.isEmpty())
            *m_eposServerProc << eposServerOptions;
        connect(m_eposServerProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
        connect(m_eposServerProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
        m_eposServerProc->start(KProcess::DontCare, KProcess::AllOutput);
    }

    // Encode the text.
    // 1.a) encode the text
    m_encText = QCString();
    QTextStream ts (m_encText, IO_WriteOnly);
    ts.setCodec(codec);
    ts << text;
    ts << endl; // Some synths need this, eg. flite.

    // Quote the text as one parameter.
    // QString escText = KShellProcess::quote(encText);

    // kdDebug()<< "EposProc::synth: Creating Epos object" << endl;
    m_eposProc = new KProcess;
    m_eposProc->setUseShell(true);
    QString languageCode;
    if (eposLanguage == "czech")
        languageCode == "cz";
    else if (eposLanguage == "slovak")
        languageCode == "sk";
    if (!languageCode.isEmpty())
    {
        m_eposProc->setEnvironment("LANG", languageCode + "." + codec->mimeName());
        m_eposProc->setEnvironment("LC_CTYPE", languageCode + "." + codec->mimeName());
    }
    *m_eposProc << eposClientExePath;
    // Language.
    if (!eposLanguage.isEmpty())
        *m_eposProc << QString("--language=%1").arg(eposLanguage);
    // Rate (speed).
    // Map 50% to 200% onto 0 to 1000.
    // slider = alpha * (log(percent)-log(50))
    // with alpha = 1000/(log(200)-log(50))
    double alpha = 1000 / (log(200) - log(50));
    int slider = (int)floor (0.5 + alpha * (log(time)-log(50)));
    // Center at 0.
    slider = slider - 500;
    // Map -500 to 500 onto 45 to -45 then shift to 130 to 40 (85 midpoint).
    float stretchValue = (-float(slider) * 45.0 / 500.0) + 85.0;
    QString timeMsg = QString("--init_t=%1").arg(stretchValue, 0, 'f', 3);
    *m_eposProc << timeMsg;
    // Pitch.  Map 50% to 200% onto 50 to 200.  easy.
    QString pitchMsg = QString("--init_f=%1").arg(pitch);
    *m_eposProc << pitchMsg;
    // Output file.
    if (!suggestedFilename.isEmpty()) 
        *m_eposProc << "-o";
    if (!eposClientOptions.isEmpty())
        *m_eposProc << eposClientOptions;
    *m_eposProc << "-";   // Read from StdIn.
    if (!suggestedFilename.isEmpty()) 
        *m_eposProc << " >" + suggestedFilename;
    connect(m_eposProc, SIGNAL(processExited(KProcess*)),
        this, SLOT(slotProcessExited(KProcess*)));
    connect(m_eposProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(m_eposProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
    connect(m_eposProc, SIGNAL(wroteStdin(KProcess*)),
        this, SLOT(slotWroteStdin(KProcess* )));
    if (suggestedFilename.isEmpty())
        m_state = psSaying;
    else
        m_state = psSynthing;

    // Ok, let's rock.
    m_synthFilename = suggestedFilename;
    kdDebug() << "EposProc::synth: Synthing text: '" << text << "' using Epos plug in" << endl;
    if (!m_eposProc->start(KProcess::NotifyOnExit, KProcess::All))
    {
        kdDebug() << "EposProc::synth: Error starting Epos process.  Is epos in the PATH?" << endl;
        m_state = psIdle;
        return;
    }
    kdDebug()<< "EposProc:synth: Epos initialized" << endl;
    if (!m_eposProc->writeStdin(m_encText, m_encText.length()))
        kdDebug() << "EposProc::synth: Error writing to Epos client StdIn." << endl;
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
    } else m_state = psIdle;
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
    m_encText = QCString();
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
