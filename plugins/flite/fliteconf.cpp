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

// TQt includes.
#include <tqlayout.h>
#include <tqfile.h>
#include <tqapplication.h>

// KDE includes.
#include <klocale.h>
#include <kdialog.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// KTTS includes.
#include <testplayer.h>

// Flite Plugin includes.
#include "fliteproc.h"
#include "fliteconf.h"
#include "fliteconf.moc"

/** Constructor */
FliteConf::FliteConf( TQWidget* parent, const char* name, const TQStringList& /*args*/) :
    PlugInConf(parent, name)
{
    // kdDebug() << "FliteConf::FliteConf: Running" << endl;
    m_fliteProc = 0;
    m_progressDlg = 0;
    
    TQVBoxLayout *tqlayout = new TQVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "FliteConfigWidgetLayout");
    tqlayout->tqsetAlignment (TQt::AlignTop);
    m_widget = new FliteConfWidget(this, "FliteConfigWidget");
    tqlayout->addWidget(m_widget);
    
    defaults();
    
    connect(m_widget->flitePath, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->fliteTest, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotFliteTest_clicked()));
}

/** Destructor */
FliteConf::~FliteConf(){
    // kdDebug() << "Running: FliteConf::~FliteConf()" << endl;
    if (!m_waveFile.isNull()) TQFile::remove(m_waveFile);
    delete m_fliteProc;
    delete m_progressDlg;
}

void FliteConf::load(KConfig *config, const TQString &configGroup){
    // kdDebug() << "FliteConf::load: Loading configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    config->setGroup(configGroup);
    TQString fliteExe = config->readEntry("FliteExePath", TQString());
    if (fliteExe.isEmpty())
    {
        config->setGroup("Flite");
        fliteExe = config->readEntry("FliteExePath", "flite");
    }
    m_widget->flitePath->setURL(fliteExe);
}

void FliteConf::save(KConfig *config, const TQString &configGroup){
    // kdDebug() << "FliteConf::save: Saving configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    config->setGroup("Flite");
    config->writeEntry("FliteExePath", 
        realFilePath(m_widget->flitePath->url()));
    config->setGroup(configGroup);
    config->writeEntry("FliteExePath",
        realFilePath(m_widget->flitePath->url()));
}

void FliteConf::defaults(){
    // kdDebug() << "FliteConf::defaults: Running" << endl;
    m_widget->flitePath->setURL("flite");
}

void FliteConf::setDesiredLanguage(const TQString &lang)
{
    m_languageCode = lang;
}

TQString FliteConf::getTalkerCode()
{
    TQString fliteExe = realFilePath(m_widget->flitePath->url());
    if (!fliteExe.isEmpty())
    {
        if (!getLocation(fliteExe).isEmpty())
        {
            return TQString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .tqarg(m_languageCode)
                    .tqarg("fixed")
                    .tqarg("neutral")
                    .tqarg("medium")
                    .tqarg("medium")
                    .tqarg("Festival Lite (flite)");
        }
    }
    return TQString();
}

void FliteConf::slotFliteTest_clicked()
{
    // kdDebug() << "FliteConf::slotFliteTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_fliteProc)
        m_fliteProc->stopText();
    else
    {
        m_fliteProc = new FliteProc();
        connect (m_fliteProc, TQT_SIGNAL(stopped()), this, TQT_SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "fliteplugin-"), ".wav");
    TQString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Get test message in the language of the voice.
    TQString testMsg = testMessage(m_languageCode);

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_flite_testdlg",
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // Play an English test.  Flite only supports English.
    connect (m_fliteProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
    m_fliteProc->synth(
        testMsg,
        tmpWaveFile,
        realFilePath(m_widget->flitePath->url()));

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_fliteProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
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
    TQFile::remove(m_waveFile);
    m_waveFile = TQString();
    if (m_progressDlg) m_progressDlg->close();
}

void FliteConf::slotSynthStopped()
{
    // Clean up after canceling test.
    TQString filename = m_fliteProc->getFilename();
    if (!filename.isNull()) TQFile::remove(filename);
}
