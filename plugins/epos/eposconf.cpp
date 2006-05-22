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

// Qt includes.
#include <QFile>
#include <QApplication>
#include <QTextCodec>
#include <QSlider>

// KDE includes.
#include <kdialog.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kprogressdialog.h>
// KTTS includes.
#include <testplayer.h>

// Epos Plugin includes.
#include "eposproc.h"
#include "eposconf.h"
#include "eposconf.moc"

/** Constructor */
EposConf::EposConf( QWidget* parent, const QStringList& /*args*/) :
    PlugInConf(parent, "eposconf")
{
    // kDebug() << "EposConf::EposConf: Running" << endl;
    m_eposProc = 0;
    m_progressDlg = 0;

    setupUi(this);

    // Build codec list and fill combobox.
    m_codecList = PlugInProc::buildCodecList();
    characterCodingBox->clear();
    characterCodingBox->addItems(m_codecList);

    defaults();

    connect(eposServerPath, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(eposClientPath, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(timeBox, SIGNAL(valueChanged(int)),
            this, SLOT(timeBox_valueChanged(int)));
    connect(frequencyBox, SIGNAL(valueChanged(int)),
            this, SLOT(frequencyBox_valueChanged(int)));
    connect(timeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeSlider_valueChanged(int)));
    connect(frequencySlider, SIGNAL(valueChanged(int)),
            this, SLOT(frequencySlider_valueChanged(int)));
    connect(timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(frequencyBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(characterCodingBox, SIGNAL(activated(const QString&)),
        this, SLOT(configChanged()));
    connect(eposServerOptions, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(eposClientOptions, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(eposTest, SIGNAL(clicked()),
        this, SLOT(slotEposTest_clicked()));
}

/** Destructor */
EposConf::~EposConf(){
    // kDebug() << "Running: EposConf::~EposConf()" << endl;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_eposProc;
    delete m_progressDlg;
}

void EposConf::load(KConfig *config, const QString &configGroup){
    // kDebug() << "EposConf::load: Running " << endl;

    config->setGroup(configGroup);
    eposServerPath->setURL(config->readEntry("EposServerExePath", "epos"));
    eposClientPath->setURL(config->readEntry("EposClientExePath", "say"));
    eposServerOptions->setText(config->readEntry("EposServerOptions", ""));
    eposClientOptions->setText(config->readEntry("EposClientOptions", ""));
    QString codecString = config->readEntry("Codec", "ISO 8859-2");
    int codec = PlugInProc::codecNameToListIndex(codecString, m_codecList);
    timeBox->setValue(config->readEntry("time", 100));
    frequencyBox->setValue(config->readEntry("pitch", 100));
    characterCodingBox->setCurrentIndex(codec);
}

/**
* Converts a language code into the language setting passed to Epos synth.
*/
QString EposConf::languageCodeToEposLanguage(const QString &languageCode)
{
    QString eposLanguage;
    if (languageCode.left(2) == "cs") eposLanguage = "czech";
    if (languageCode.left(2) == "sk") eposLanguage = "slovak";
    return eposLanguage;
}

void EposConf::save(KConfig *config, const QString &configGroup){
    // kDebug() << "EposConf::save: Running" << endl;

    config->setGroup("Epos");
    config->writeEntry("EposServerExePath",
        realFilePath(eposServerPath->url()));
    config->writeEntry("EposClientExePath", 
        realFilePath(eposClientPath->url()));
    config->writeEntry("Language", languageCodeToEposLanguage(m_languageCode));
    config->setGroup(configGroup);
    config->writeEntry("EposServerExePath", 
        realFilePath(eposServerPath->url()));
    config->writeEntry("EposClientExePath", 
        realFilePath(eposClientPath->url()));
    config->writeEntry("EposServerOptions", eposServerOptions->text());
    config->writeEntry("EposClientOptions", eposClientOptions->text());
    config->writeEntry("time", timeBox->value());
    config->writeEntry("pitch", frequencyBox->value());
    int codec = characterCodingBox->currentIndex();
    config->writeEntry("Codec", PlugInProc::codecIndexToCodecName(codec, m_codecList));
}

void EposConf::defaults(){
    // kDebug() << "EposConf::defaults: Running" << endl;
    eposServerPath->setURL("epos");
    eposClientPath->setURL("say");
    eposServerOptions->setText("");
    eposClientOptions->setText("");
    timeBox->setValue(100);
    timeBox_valueChanged(100);
    frequencyBox->setValue(100);
    frequencyBox_valueChanged(100);
    int codec = PlugInProc::codecNameToListIndex("ISO 8859-2", m_codecList);
    characterCodingBox->setCurrentIndex(codec);
}

void EposConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString EposConf::getTalkerCode()
{
    QString eposServerExe = realFilePath(eposServerPath->url());
    QString eposClientExe = realFilePath(eposClientPath->url());
    if (!eposServerExe.isEmpty() && !eposClientExe.isEmpty())
    {
        if (!getLocation(eposServerExe).isEmpty() && !getLocation(eposClientExe).isEmpty())
        {
            QString rate = "medium";
            if (timeBox->value() < 75) rate = "slow";
            if (timeBox->value() > 125) rate = "fast";
            return QString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .arg(m_languageCode)
                    .arg("fixed")
                    .arg("neutral")
                    .arg("medium")
                    .arg(rate)
                    .arg("Epos TTS Synthesis System");
        }
    }
    return QString();
}

void EposConf::slotEposTest_clicked()
{
    // kDebug() << "EposConf::slotEposTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_eposProc)
        m_eposProc->stopText();
    else
    {
        m_eposProc = new EposProc();
        connect (m_eposProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "eposplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->fileName();
    tempFile.close();

    // Get test message in the language of the voice.
    QString testMsg = testMessage(m_languageCode);

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(this,
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // TODO: Whenever server options change, the server must be restarted.
    // TODO: Do codec names contain non-ASCII characters?
    connect (m_eposProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_eposProc->synth(
        testMsg,
        tmpWaveFile,
        realFilePath(eposServerPath->url()),
        realFilePath(eposClientPath->url()),
        eposServerOptions->text(),
        eposClientOptions->text(),
        PlugInProc::codecIndexToCodec(characterCodingBox->currentIndex(), m_codecList),
        languageCodeToEposLanguage(m_languageCode),
        timeBox->value(),
        frequencyBox->value()
        );

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_eposProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
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
    QFile::remove(m_waveFile);
    m_waveFile.clear();
    if (m_progressDlg) m_progressDlg->close();
}

void EposConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_eposProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
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
    timeSlider->setValue (percentToSlider (percentValue));
}

void EposConf::frequencyBox_valueChanged(int percentValue) {
    frequencySlider->setValue(percentToSlider(percentValue));
}

void EposConf::timeSlider_valueChanged(int sliderValue) {
    timeBox->setValue (sliderToPercent (sliderValue));
}

void EposConf::frequencySlider_valueChanged(int sliderValue) {
    frequencyBox->setValue(sliderToPercent(sliderValue));
}
