/******************************************************************************
    Configuration widget and functions for FreeTTS (interactive) plug in
    -------------------
    Copyright : (C) 2004 Paul Giannaros <ceruleanblaze@gmail.com>
    -------------------
    Original author: Paul Giannaros <ceruleanblaze@gmail.com>
    Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *     This program is free software; you can redistribute it and/or modify   *
 *     it under the terms of the GNU General Public License as published by   *
 *     the Free Software Foundation; version 2 of the License or               *
 *     (at your option) version 3.                                             *
 *                                                                            *
 ******************************************************************************/

// FreeTTS includes.
#include "freettsconf.h"
#include "ui_freettsconfigwidget.h"

// Qt includes.
#include <QtGui/QLabel>
#include <QtCore/QFile>
#include <QtGui/QApplication>

// KDE includes.
#include <kdialog.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kprogressdialog.h>

// KTTS includes.
#include <pluginconf.h>
#include <testplayer.h>

/** Constructor */
FreeTTSConf::FreeTTSConf( QWidget* parent, const QStringList&/*args*/) :
    PlugInConf( parent, "freettsconf" ) {

    // kDebug() << "FreeTTSConf::FreeTTSConf: Running";
    m_freettsProc = 0;
        m_progressDlg = 0;
        setupUi(this);

    defaults();

    connect(freettsPath, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(freettsTest, SIGNAL(clicked()), this, SLOT(slotFreeTTSTest_clicked()));
}

/** Destructor */
FreeTTSConf::~FreeTTSConf() {
    // kDebug() << "Running: FreeTTSConf::~FreeTTSConf()";
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_freettsProc;
        delete m_progressDlg;
}

void FreeTTSConf::load(KConfig *c, const QString &configGroup) {
    // kDebug() << "FreeTTSConf::load: Running";

    KConfigGroup config(c, configGroup);
        QString freeTTSJar = config.readEntry("FreeTTSJarPath", QString());
        if (freeTTSJar.isEmpty())
        {
            KConfigGroup freettsConfig(c, "FreeTTS");
            freeTTSJar = freettsConfig.readEntry("FreeTTSJarPath", QString());
        }
    if (freeTTSJar.isEmpty())
        freeTTSJar = getLocation("freetts.jar");
        freettsPath->setUrl(KUrl::fromPath(freeTTSJar));
    /// If freettsPath is still empty, then we couldn't find the file in the path.
}

void FreeTTSConf::save(KConfig *c, const QString &configGroup){
    // kDebug() << "FreeTTSConf::save: Running";

    KConfigGroup freettsConfig(c, "FreeTTS");
    freettsConfig.writeEntry("FreeTTSJarPath",
        realFilePath(freettsPath->url().path()));

    KConfigGroup config(c, configGroup);
    if(freettsPath->url().path().isEmpty())
    KMessageBox::sorry(0, i18n("Unable to locate freetts.jar in your path.\nPlease specify the path to freetts.jar in the Properties tab before using KDE Text-to-Speech"), i18n("KDE Text-to-Speech"));
    config.writeEntry("FreeTTSJarPath",
    realFilePath(freettsPath->url().path()));
}

void FreeTTSConf::defaults(){
    // kDebug() << "Running: FreeTTSConf::defaults()";
    freettsPath->setUrl(KUrl(""));
}

void FreeTTSConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString FreeTTSConf::getTalkerCode()
{
    QString freeTTSJar = realFilePath(freettsPath->url().path());
    if (!freeTTSJar.isEmpty())
    {
        if (!getLocation(freeTTSJar).isEmpty())
        {
            return QString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .arg(m_languageCode)
                    .arg("fixed")
                    .arg("neutral")
                    .arg("medium")
                    .arg("medium")
                    .arg("FreeTTS");
        }
    }
    return QString();
}

// QString FreeTTSConf::getLocation(const QString &name) {
//     /// Iterate over the path and see if 'name' exists in it. Return the
//     /// full path to it if it does. Else return an empty QString.
//     kDebug() << "FreeTTSConf::getLocation: Searching for " << name << " in the path... ";
//     kDebug() << m_path;
//     for(QStringList::iterator it = m_path.begin(); it != m_path.end(); ++it) {
//         QString fullName = *it;
//         fullName += "/";
//         fullName += name;
//         /// The user either has the directory of the file in the path...
//         if(QFile::exists(fullName)) {
//             return fullName;
//             kDebug() << fullName;
//         }
//         /// ....Or the file itself
//         else if(QFileInfo(*it).baseName().append(QString(".").append(QFileInfo(*it).extension())) == name) {
//             return fullName;
//             kDebug() << fullName;
//         }
//     }
//     return "";
// }


void FreeTTSConf::slotFreeTTSTest_clicked()
{
    // kDebug() << "FreeTTSConf::slotFreeTTSTest_clicked(): Running";
        // If currently synthesizing, stop it.
    if (m_freettsProc)
        m_freettsProc->stopText();
    else
        {
        m_freettsProc = new FreeTTSProc();
                connect (m_freettsProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
        }
        // Create a temp file name for the wave file.
    KTemporaryFile tempFile;
    tempFile.setPrefix("freettsplugin-");
    tempFile.setSuffix(".wav");
    tempFile.setAutoRemove(false);
    tempFile.open();
    QString tmpWaveFile = tempFile.fileName();

    // Get test message in the language of the voice.
    QString testMsg = testMessage(m_languageCode);

        // Tell user to wait.
        m_progressDlg = new KProgressDialog(this,
            i18n("Testing"),
            i18n("Testing."));
        m_progressDlg->setModal(true);
        m_progressDlg->progressBar()->hide();
        m_progressDlg->setAllowCancel(true);

    // I think FreeTTS only officialy supports English, but if anyone knows of someone
    // whos built up a different language lexicon and has it working with FreeTTS gimme an email at ceruleanblaze@gmail.com
        connect (m_freettsProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
        m_freettsProc->synth(
            testMsg,
            tmpWaveFile,
            realFilePath(freettsPath->url().path()));

        // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
        // or if user clicks Cancel button.
        m_progressDlg->exec();
        disconnect (m_freettsProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
        if (m_progressDlg->wasCancelled()) m_freettsProc->stopText();
        delete m_progressDlg;
        m_progressDlg = 0;
}

void FreeTTSConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_freettsProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // Get new wavefile name.
    m_waveFile = m_freettsProc->getFilename();
    // Tell synth we're done.
    m_freettsProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    QFile::remove(m_waveFile);
    m_waveFile.clear();
    if (m_progressDlg) m_progressDlg->close();
}

void FreeTTSConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_freettsProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
}

#include "freettsconf.moc"

