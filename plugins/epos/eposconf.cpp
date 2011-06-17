/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Epos plug in
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

// C++ includes.
#include <math.h>

// TQt includes.
#include <tqfile.h>
#include <tqapplication.h>
#include <tqtextcodec.h>
#include <tqlayout.h>
#include <tqslider.h>

// KDE includes.
#include <kdialog.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <klocale.h>
#include <knuminput.h>

// KTTS includes.
#include <testplayer.h>

// Epos Plugin includes.
#include "eposproc.h"
#include "eposconf.h"
#include "eposconf.moc"

/** Constructor */
EposConf::EposConf( TQWidget* tqparent, const char* name, const TQStringList& /*args*/) :
    PlugInConf(tqparent, name)
{
    // kdDebug() << "EposConf::EposConf: Running" << endl;
    m_eposProc = 0;
    m_progressDlg = 0;

    TQVBoxLayout *tqlayout = new TQVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "EposConfigWidgetLayout");
    tqlayout->tqsetAlignment (TQt::AlignTop);
    m_widget = new EposConfWidget(this, "EposConfigWidget");
    tqlayout->addWidget(m_widget);

    // Build codec list and fill combobox.
    m_codecList = PlugInProc::buildCodecList();
    m_widget->characterCodingBox->clear();
    m_widget->characterCodingBox->insertStringList(m_codecList);

    defaults();

    connect(m_widget->eposServerPath, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->eposClientPath, TQT_SIGNAL(textChanged(const TQString&)),
            this, TQT_SLOT(configChanged()));
    connect(m_widget->timeBox, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(timeBox_valueChanged(int)));
    connect(m_widget->frequencyBox, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(frequencyBox_valueChanged(int)));
    connect(m_widget->timeSlider, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(timeSlider_valueChanged(int)));
    connect(m_widget->frequencySlider, TQT_SIGNAL(valueChanged(int)),
            this, TQT_SLOT(frequencySlider_valueChanged(int)));
    connect(m_widget->timeBox, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_widget->timeSlider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_widget->frequencyBox, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_widget->frequencySlider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    connect(m_widget->characterCodingBox, TQT_SIGNAL(activated(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->eposServerOptions, TQT_SIGNAL(textChanged(const TQString&)),
            this, TQT_SLOT(configChanged()));
    connect(m_widget->eposClientOptions, TQT_SIGNAL(textChanged(const TQString&)),
            this, TQT_SLOT(configChanged()));
    connect(m_widget->eposTest, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotEposTest_clicked()));
}

/** Destructor */
EposConf::~EposConf(){
    // kdDebug() << "Running: EposConf::~EposConf()" << endl;
    if (!m_waveFile.isNull()) TQFile::remove(m_waveFile);
    delete m_eposProc;
    delete m_progressDlg;
}

void EposConf::load(KConfig *config, const TQString &configGroup){
    // kdDebug() << "EposConf::load: Running " << endl;

    config->setGroup(configGroup);
    m_widget->eposServerPath->setURL(config->readEntry("EposServerExePath", "eposd"));
    m_widget->eposClientPath->setURL(config->readEntry("EposClientExePath", "say-epos"));
    m_widget->eposServerOptions->setText(config->readEntry("EposServerOptions", ""));
    m_widget->eposClientOptions->setText(config->readEntry("EposClientOptions", ""));
    TQString codecString = config->readEntry("Codec", "ISO 8859-2");
    int codec = PlugInProc::codecNameToListIndex(codecString, m_codecList);
    m_widget->timeBox->setValue(config->readNumEntry("time", 100));
    m_widget->frequencyBox->setValue(config->readNumEntry("pitch", 100));
    m_widget->characterCodingBox->setCurrentItem(codec);
}

/**
* Converts a language code into the language setting passed to Epos synth.
*/
TQString EposConf::languageCodeToEposLanguage(const TQString &languageCode)
{
    TQString eposLanguage;
    if (languageCode.left(2) == "cs") eposLanguage = "czech";
    if (languageCode.left(2) == "sk") eposLanguage = "slovak";
    return eposLanguage;
}

void EposConf::save(KConfig *config, const TQString &configGroup){
    // kdDebug() << "EposConf::save: Running" << endl;

    config->setGroup("Epos");
    config->writeEntry("EposServerExePath",
        realFilePath(m_widget->eposServerPath->url()));
    config->writeEntry("EposClientExePath", 
        realFilePath(m_widget->eposClientPath->url()));
    config->writeEntry("Language", languageCodeToEposLanguage(m_languageCode));
    config->setGroup(configGroup);
    config->writeEntry("EposServerExePath", 
        realFilePath(m_widget->eposServerPath->url()));
    config->writeEntry("EposClientExePath", 
        realFilePath(m_widget->eposClientPath->url()));
    config->writeEntry("EposServerOptions", m_widget->eposServerOptions->text());
    config->writeEntry("EposClientOptions", m_widget->eposClientOptions->text());
    config->writeEntry("time", m_widget->timeBox->value());
    config->writeEntry("pitch", m_widget->frequencyBox->value());
    int codec = m_widget->characterCodingBox->currentItem();
    config->writeEntry("Codec", PlugInProc::codecIndexToCodecName(codec, m_codecList));
}

void EposConf::defaults(){
    // kdDebug() << "EposConf::defaults: Running" << endl;
    // Epos server command changed from epos to eposd.  Epos client command changed from
    // say to say-epos.  These changes appeared around Epos v2.5.35.  Try for these automatically.
    TQString exeName = "eposd";
    if (realFilePath(exeName).isEmpty())
        if (!realFilePath("epos").isEmpty())
            exeName = "epos";
    m_widget->eposServerPath->setURL(exeName);
    exeName = "say-epos";
    if (realFilePath(exeName).isEmpty())
        if (!realFilePath("say").isEmpty())
            exeName = "say";
    m_widget->eposClientPath->setURL(exeName);
    m_widget->eposServerOptions->setText("");
    m_widget->eposClientOptions->setText("");
    m_widget->timeBox->setValue(100);
    timeBox_valueChanged(100);
    m_widget->frequencyBox->setValue(100);
    frequencyBox_valueChanged(100);
    int codec = PlugInProc::codecNameToListIndex("ISO 8859-2", m_codecList);
    m_widget->characterCodingBox->setCurrentItem(codec);
}

void EposConf::setDesiredLanguage(const TQString &lang)
{
    m_languageCode = lang;
}

TQString EposConf::getTalkerCode()
{
    TQString eposServerExe = realFilePath(m_widget->eposServerPath->url());
    TQString eposClientExe = realFilePath(m_widget->eposClientPath->url());
    if (!eposServerExe.isEmpty() && !eposClientExe.isEmpty())
    {
        if (!getLocation(eposServerExe).isEmpty() && !getLocation(eposClientExe).isEmpty())
        {
            TQString rate = "medium";
            if (m_widget->timeBox->value() < 75) rate = "slow";
            if (m_widget->timeBox->value() > 125) rate = "fast";
            return TQString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .tqarg(m_languageCode)
                    .tqarg("fixed")
                    .tqarg("neutral")
                    .tqarg("medium")
                    .tqarg(rate)
                    .tqarg("Epos TTS Synthesis System");
        }
    }
    return TQString();
}

void EposConf::slotEposTest_clicked()
{
    // kdDebug() << "EposConf::slotEposTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_eposProc)
        m_eposProc->stopText();
    else
    {
        m_eposProc = new EposProc();
        connect (m_eposProc, TQT_SIGNAL(stopped()), this, TQT_SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "eposplugin-"), ".wav");
    TQString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Get test message in the language of the voice.
    TQString testMsg = testMessage(m_languageCode);

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_epos_testdlg",
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // TODO: Whenever server options change, the server must be restarted.
    // TODO: Do codec names contain non-ASCII characters?
    connect (m_eposProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
    m_eposProc->synth(
        testMsg,
        tmpWaveFile,
        realFilePath(m_widget->eposServerPath->url()),
        realFilePath(m_widget->eposClientPath->url()),
        m_widget->eposServerOptions->text(),
        m_widget->eposClientOptions->text(),
        PlugInProc::codecIndexToCodec(m_widget->characterCodingBox->currentItem(), m_codecList),
        languageCodeToEposLanguage(m_languageCode),
        m_widget->timeBox->value(),
        m_widget->frequencyBox->value()
        );

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_eposProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
    if (m_progressDlg->wasCancelled()) m_eposProc->stopText();
    delete m_progressDlg;
    m_progressDlg = 0;
}

void EposConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_eposProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // Get new wavefile name.
    m_waveFile = m_eposProc->getFilename();
    // Tell synth we're done.
    m_eposProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    TQFile::remove(m_waveFile);
    m_waveFile = TQString();
    if (m_progressDlg) m_progressDlg->close();
}

void EposConf::slotSynthStopped()
{
    // Clean up after canceling test.
    TQString filename = m_eposProc->getFilename();
    if (!filename.isNull()) TQFile::remove(filename);
}

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int EposConf::percentToSlider(int percentValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int EposConf::sliderToPercent(int sliderValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void EposConf::timeBox_valueChanged(int percentValue) {
    m_widget->timeSlider->setValue (percentToSlider (percentValue));
}

void EposConf::frequencyBox_valueChanged(int percentValue) {
    m_widget->frequencySlider->setValue(percentToSlider(percentValue));
}

void EposConf::timeSlider_valueChanged(int sliderValue) {
    m_widget->timeBox->setValue (sliderToPercent (sliderValue));
}

void EposConf::frequencySlider_valueChanged(int sliderValue) {
    m_widget->frequencyBox->setValue(sliderToPercent(sliderValue));
}
