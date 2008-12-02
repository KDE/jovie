/****************************************************************************
    Main speaking functions for the FreeTTS Plug in
    -------------------
    Copyright : (C) 2004 Paul Giannaros <ceruleanblaze@gmail.com>
    -------------------
    Original author: Paul Giannaros <ceruleanblaze@gmail.com>
    Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ******************************************************************************/

/*******************************************************************************
 *                                                                             *
 *     This program is free software; you can redistribute it and/or modify    *
 *     it under the terms of the GNU General Public License as published by    *
 *     the Free Software Foundation; version 2 of the License or               *
 *     (at your option) version 3.                                             *
 *                                                                             *
 ******************************************************************************/

#include "freettsproc.h" 

#include <QtCore/QFileInfo>

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <k3process.h>

/** Constructor */
FreeTTSProc::FreeTTSProc( QObject* parent, const QStringList& /*args*/) : 
    PlugInProc( parent, "freettsproc" ) {
    kDebug() << "Running: FreeTTSProc::FreeTTSProc";
    m_state = psIdle;
    m_waitingStop = false;
    m_freettsProc = 0;
}

/** Desctructor */
FreeTTSProc::~FreeTTSProc() {
    kDebug() << "Running: FreeTTSProc::~FreeTTSProc";
    if (m_freettsProc) {
        stopText();
        delete m_freettsProc;
    }
}

/** Initializate the speech */
bool FreeTTSProc::init(KConfig *config, const QString &configGroup) {
    kDebug() << "Running: FreeTTSProc::init()";
    kDebug() << "Initializing plug in: FreeTTS";
    KConfigGroup group = config->group(configGroup);
    m_freettsJarPath = group.readEntry("FreeTTSJarPath", "freetts.jar");
    kDebug() << "FreeTTSProc::init: path to freetts.jar: " << m_freettsJarPath;
    return true;
}

/** 
 * Say a text.  Synthesize and audibilize it.
 * @param text                    The text to be spoken.
 *
 * If the plugin supports asynchronous operation, it should return immediately.
 */
void FreeTTSProc::sayText(const QString &text) {
    synth(text, QString(), m_freettsJarPath);
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
void FreeTTSProc::synthText(const QString& text, const QString& suggestedFilename) {
    kDebug() << "Running: FreeTTSProc::synthText";
    synth(text, suggestedFilename, m_freettsJarPath);
}

void FreeTTSProc::synth(
    const QString &text,
    const QString &synthFilename,
    const QString& freettsJarPath) {

    kDebug() << "Running: FreeTTSProc::synth";

    if (m_freettsProc) {
        if (m_freettsProc->isRunning()) m_freettsProc->kill();
        delete m_freettsProc;
        m_freettsProc = 0;
    }

    m_freettsProc = new K3Process;
    connect(m_freettsProc, SIGNAL(processExited(K3Process*)),
           this, SLOT(slotProcessExited(K3Process*)));
    connect(m_freettsProc, SIGNAL(receivedStdout(K3Process*, char*, int)),
           this, SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(m_freettsProc, SIGNAL(receivedStderr(K3Process*, char*, int)),
           this, SLOT(slotReceivedStderr(K3Process*, char*, int)));
    connect(m_freettsProc, SIGNAL(wroteStdin(K3Process*)),
           this, SLOT(slotWroteStdin(K3Process* )));
    if (synthFilename.isNull())
        m_state = psSaying;
    else
        m_state = psSynthing;


    QString saidText = text;
    saidText += '\n';

    /// As freetts.jar doesn't seem to like being called from an absolute path, 
    /// we need to strip off the path to freetts.jar and pass it to 
    /// K3Process::setWorkingDirectory()
    /// We could just strip off 11 characters from the end of the path to freetts.jar, but thats
    /// not exactly very portable...
    QString filename = QFileInfo(freettsJarPath).baseName().append(QString(".").append(QFileInfo(freettsJarPath).suffix()));
    QString freettsJarDir = freettsJarPath.left((freettsJarPath.length() - filename.length()) - 1);

    m_freettsProc->setWorkingDirectory(freettsJarDir);
    kDebug() << "FreeTTSProc::synthText: moved to directory '" << freettsJarDir << "'";
    kDebug() << "FreeTTSProc::synthText: Running file: '" << filename << "'";
    *m_freettsProc << "java" << "-jar" << filename;

    /// Dump audio into synthFilename

    if (!synthFilename.isNull()) *m_freettsProc << "-dumpAudio" << synthFilename;

    m_synthFilename = synthFilename;

    kDebug() << "FreeTTSProc::synth: Synthing text: '" << saidText << "' using FreeTTS plug in";
    if (!m_freettsProc->start(K3Process::NotifyOnExit, K3Process::All)) {
        kDebug() << "FreeTTSProc::synth: Error starting FreeTTS process.  Is freetts.jar in the PATH?";
        m_state = psIdle;
        kDebug() << "K3Process args: " << m_freettsProc->args();
        return;
    }
    kDebug()<< "FreeTTSProc:synth: FreeTTS initialized";
    m_freettsProc->writeStdin(saidText.toLatin1(), saidText.length());
}

/**
 * Returning the filename of the synth'd text
 * @returns                    The filename of the last synth'd text
 */
QString FreeTTSProc::getFilename() {
    kDebug() << "FreeTTSProc::getFilename: returning " << m_synthFilename;
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
void FreeTTSProc::stopText() {
    kDebug() << "FreeTTSProc::stopText:: Running";
    if (m_freettsProc) {
        if (m_freettsProc->isRunning()) {
            kDebug() << "FreeTTSProc::stopText: killing FreeTTS.";
            m_waitingStop = true;
            m_freettsProc->kill();
        } 
        else m_state = psIdle;
    }
    else m_state = psIdle;
    kDebug() << "FreeTTSProc::stopText: FreeTTS stopped.";
}

void FreeTTSProc::slotProcessExited(K3Process*) {
    kDebug() << "FreeTTSProc:slotProcessExited: FreeTTS process has exited.";
    pluginState prevState = m_state;
    if (m_waitingStop) {
        m_waitingStop = false;
        m_state = psIdle;
        emit stopped();
    }
    else {
        m_state = psFinished;
        if (prevState == psSaying)
            emit sayFinished();
        else if (prevState == psSynthing)
            emit synthFinished();
    }
}

void FreeTTSProc::slotReceivedStdout(K3Process*, char* buffer, int buflen) {
    QString buf = QString::fromLatin1(buffer, buflen);
    kDebug() << "FreeTTSProc::slotReceivedStdout: Received output from FreeTTS: " << buf;
}

void FreeTTSProc::slotReceivedStderr(K3Process*, char* buffer, int buflen) {
    QString buf = QString::fromLatin1(buffer, buflen);
    kDebug() << "FreeTTSProc::slotReceivedStderr: Received error from FreeTTS: " << buf;
}

void FreeTTSProc::slotWroteStdin(K3Process*) {
    kDebug() << "FreeTTSProc::slotWroteStdin: closing Stdin";
    m_freettsProc->closeStdin();
}

/**
 * Return the current state of the plugin.
 * This function only makes sense in asynchronous mode.
 * @return                        The pluginState of the plugin.
 *
 * @see pluginState
 */
pluginState FreeTTSProc::getState() { 
    return m_state;
}

/**
 * Acknowledges a finished state and resets the plugin state to psIdle.
 *
 * If the plugin is not in state psFinished, nothing happens.
 * The plugin may use this call to do any post-processing cleanup,
 * for example, blanking the stored filename (but do not delete the file).
 * Calling program should call getFilename prior to ackFinished.
 */
void FreeTTSProc::ackFinished() {
    if (m_state == psFinished) {
        m_state = psIdle;
        m_synthFilename.clear();
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
bool FreeTTSProc::supportsAsync() { 
//     return true; 
    return true;
}

/**
 * Returns True if the plugIn supports synthText method,
 * i.e., is able to synthesize text to a sound file without
 * audibilizing the text.
 * @return                        True if this plugin supports synthText method.
 */
bool FreeTTSProc::supportsSynth() { 
//     return true; 
    return true;
}


#include "freettsproc.moc"

