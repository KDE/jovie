/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Festival (Interactive) plug in
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <QFile>
#include <QApplication>
#include <QProgressBar>

// KDE includes.
#include <klocale.h>
#include <kdialog.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <kprogressdialog.h>

// KTTS includes.
#include <testplayer.h>

// Flite Plugin includes.
#include "fliteproc.h"
#include "fliteconf.h"
#include "fliteconf.moc"

/** Constructor */
FliteConf::FliteConf( QWidget* parent, const QStringList& /*args*/) :
    PlugInConf(parent, "fliteconf")
{
    // kDebug() << "FliteConf::FliteConf: Running" << endl;
    m_fliteProc = 0;
    m_progressDlg = 0;

    setupUi(this);

    defaults();

    connect(flitePath, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(fliteTest, SIGNAL(clicked()), this, SLOT(slotFliteTest_clicked()));
}

/** Destructor */
FliteConf::~FliteConf(){
    // kDebug() << "Running: FliteConf::~FliteConf()" << endl;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_fliteProc;
    delete m_progressDlg;
}

void FliteConf::load(KConfig *c, const QString &configGroup){
    // kDebug() << "FliteConf::load: Loading configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    KConfigGroup config(c, configGroup);
    QString fliteExe = config.readEntry("FliteExePath", QString());
    if (fliteExe.isEmpty())
    {
        KConfigGroup fliteConfig(c, "Flite");
        fliteExe = fliteConfig.readEntry("FliteExePath", "flite");
    }
    flitePath->setUrl(KUrl::fromPath(fliteExe));
}

void FliteConf::save(KConfig *c, const QString &configGroup){
    // kDebug() << "FliteConf::save: Saving configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    KConfigGroup fliteConfig(c, "Flite");
    fliteConfig.writeEntry("FliteExePath",
        realFilePath(flitePath->url().path()));
    KConfigGroup config(c, configGroup);
    config.writeEntry("FliteExePath",
        realFilePath(flitePath->url().path()));
}

void FliteConf::defaults(){
    // kDebug() << "FliteConf::defaults: Running" << endl;
    flitePath->setUrl(KUrl::fromPath("flite"));
}

void FliteConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString FliteConf::getTalkerCode()
{
    QString fliteExe = realFilePath(flitePath->url().path());
    if (!fliteExe.isEmpty())
    {
        if (!getLocation(fliteExe).isEmpty())
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
                    .arg("Festival Lite (flite)");
        }
    }
    return QString();
}

void FliteConf::slotFliteTest_clicked()
{
    // kDebug() << "FliteConf::slotFliteTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_fliteProc)
        m_fliteProc->stopText();
    else
    {
        m_fliteProc = new FliteProc();
        connect (m_fliteProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTemporaryFile tempFile;
    tempFile.setPrefix("fliteplugin-");
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

    // Play an English test.  Flite only supports English.
    connect (m_fliteProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_fliteProc->synth(
        testMsg,
        tmpWaveFile,
        realFilePath(flitePath->url().path()));

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_fliteProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    if (m_progressDlg->wasCancelled()) m_fliteProc->stopText();
    delete m_progressDlg;
    m_progressDlg = 0;
}

void FliteConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_fliteProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // Get new wavefile name.
    m_waveFile = m_fliteProc->getFilename();
    // Tell synth we're done.
    m_fliteProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    QFile::remove(m_waveFile);
    m_waveFile.clear();
    if (m_progressDlg) m_progressDlg->close();
}

void FliteConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_fliteProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
}
