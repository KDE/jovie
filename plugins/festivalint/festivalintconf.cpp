/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Festival (Interactive) plug in
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

// C++ includes.
#include <math.h>

// Qt includes.
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qslider.h>

// KDE includes.
#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kprocio.h>
#include <kprogress.h>

// KTTS includes.
#include "pluginconf.h"
#include "testplayer.h"

// FestivalInt includes.
#include "festivalintproc.h"
#include "festivalintconf.h"
#include "festivalintconf.moc"

/** Constructor */
FestivalIntConf::FestivalIntConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
    PlugInConf(parent, name)
{
    // kdDebug() << "FestivalIntConf::FestivalIntConf: Running" << endl;
    m_festProc = 0;
    m_progressDlg = 0;

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "FestivalIntConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new FestivalIntConfWidget(this, "FestivalIntConfigWidget");
    layout->addWidget(m_widget);

    m_widget->festivalPath->setMode(KFile::File | KFile::ExistingOnly);
    m_widget->festivalPath->setFilter("*");
//    defaults();

    connect(m_widget->festivalPath, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotFestivalPath_textChanged()));
    connect(m_widget->selectVoiceCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotSelectVoiceCombo_activated()));
    connect(m_widget->selectVoiceCombo, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->testButton, SIGNAL(clicked()), this, SLOT(slotTest_clicked()));
    connect(m_widget->rescan, SIGNAL(clicked()), this, SLOT(scanVoices()));
    connect(m_widget->volumeBox, SIGNAL(valueChanged(int)),
            this, SLOT(volumeBox_valueChanged(int)));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)),
            this, SLOT(timeBox_valueChanged(int)));
    connect(m_widget->frequencyBox, SIGNAL(valueChanged(int)),
            this, SLOT(frequencyBox_valueChanged(int)));
    connect(m_widget->volumeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(volumeSlider_valueChanged(int)));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeSlider_valueChanged(int)));
    connect(m_widget->frequencySlider, SIGNAL(valueChanged(int)),
            this, SLOT(frequencySlider_valueChanged(int)));
    connect(m_widget->volumeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->frequencyBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->preloadCheckBox, SIGNAL(clicked()), this, SLOT(configChanged()));
}

/** Destructor */
FestivalIntConf::~FestivalIntConf(){
    // kdDebug() << "FestivalIntConf::~FestivalIntConf: Running" << endl;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_festProc;
    delete m_progressDlg;
}

/**
* Given a voice code, returns index into m_voiceList array (and voiceCombo box).
* -1 if not found.
*/
int FestivalIntConf::voiceCodeToListIndex(const QString voiceCode)
{
    for(uint index = 0 ; index < m_voiceList.count(); ++index){
        // kdDebug() << "Testing: " << voiceSelected << " == " << m_voiceList[index].code << endl;
        if(voiceCode == m_voiceList[index].code)
            return index;
    }
    return -1;
}

void FestivalIntConf::load(KConfig *config, const QString &configGroup){
    // kdDebug() << "FestivalIntConf::load: Running" << endl;

    config->setGroup("FestivalInt");
    m_widget->festivalPath->setURL(config->readPathEntry("FestivalExecutablePath", "festival"));
    config->setGroup(configGroup);
    QString exePath = realFilePath(m_widget->festivalPath->url());
    m_widget->festivalPath->setURL(config->readPathEntry("FestivalExecutablePath", exePath));
    m_widget->preloadCheckBox->setChecked(false);
    scanVoices();
    QString voiceSelected(config->readEntry("Voice"));
    int index = voiceCodeToListIndex(voiceSelected);
    if (index >= 0)
    {
        m_widget->selectVoiceCombo->setCurrentItem(index);
        m_widget->preloadCheckBox->setChecked(m_voiceList[index].preload);
    }
    m_widget->volumeBox->setValue(config->readNumEntry("volume", 100));
    m_widget->timeBox->setValue(config->readNumEntry("time", 100));
    m_widget->frequencyBox->setValue(config->readNumEntry("pitch", 100));
    m_widget->preloadCheckBox->setChecked(config->readBoolEntry(
         "Preload", m_widget->preloadCheckBox->isChecked()));
}

void FestivalIntConf::save(KConfig *config, const QString &configGroup){
    // kdDebug() << "FestivalIntConf::save: Running" << endl;

    config->setGroup("FestivalInt");
    config->writePathEntry("FestivalExecutablePath", realFilePath(m_widget->festivalPath->url()));
    config->setGroup(configGroup);
    config->writePathEntry("FestivalExecutablePath", realFilePath(m_widget->festivalPath->url()));
    config->writeEntry("Voice", m_voiceList[m_widget->selectVoiceCombo->currentItem()].code);
    config->writeEntry("volume", m_widget->volumeBox->value());
    config->writeEntry("time", m_widget->timeBox->value());
    config->writeEntry("pitch", m_widget->frequencyBox->value());
    config->writeEntry("Preload", m_widget->preloadCheckBox->isChecked());
}

void FestivalIntConf::defaults(){
    // kdDebug() << "FestivalIntConf::defaults: Running" << endl;
    m_widget->festivalPath->setURL("festival");
    m_widget->timeBox->setValue(100);
    timeBox_valueChanged(100);
    m_widget->volumeBox->setValue(100);
    volumeBox_valueChanged(100);
    m_widget->frequencyBox->setValue(100);
    frequencyBox_valueChanged(100);
    m_widget->preloadCheckBox->setChecked(false);
    scanVoices();
}

void FestivalIntConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = splitLanguageCode(lang, m_countryCode);
}

QString FestivalIntConf::getTalkerCode()
{
    if (!m_widget->selectVoiceCombo->isEnabled()) return QString::null;
    QString exePath = realFilePath(m_widget->festivalPath->url());
    if (exePath.isEmpty()) return QString::null;
    if (getLocation(exePath).isEmpty()) return QString::null;
    if (m_voiceList.count() == 0) return QString::null;
    QString normalTalkerCode;
    voiceStruct voiceTemp = m_voiceList[m_widget->selectVoiceCombo->currentItem()];
    // Determine volume attribute.  soft < 75% <= medium <= 125% < loud.
    QString volume = "medium";
    if (m_widget->volumeBox->value() < 75) volume = "soft";
    if (m_widget->volumeBox->value() > 125) volume = "loud";
    // Determine rate attribute.  slow < 75% <= medium <= 125% < fast.
    QString rate = "medium";
    if (m_widget->timeBox->value() < 75) rate = "slow";
    if (m_widget->timeBox->value() > 125) rate = "fast";
    normalTalkerCode = QString(
            "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
            "<prosody volume=\"%4\" rate=\"%5\" />"
            "<kttsd synthesizer=\"%6\" />")
            .arg(voiceTemp.languageCode)
            .arg(voiceTemp.name)
            .arg(voiceTemp.gender)
            .arg(volume)
            .arg(rate)
            .arg("Festival Interactive");
    return normalTalkerCode;
}

/**
 * Chooses a default voice given scanned list of voices in m_voiceList and current
 * language and country code, and updates controls.
 * @param currentVoiceIndex      This voice is preferred if it matches.
 */
void FestivalIntConf::setDefaultVoice(int currentVoiceIndex)
{
    // If language code is known, auto pick first voice that matches the language code.
    if (!m_languageCode.isEmpty())
    {
        bool found = false;
        // First search for a match on both language code and country code.
        QString languageCode = m_languageCode;
        if (!m_countryCode.isNull()) languageCode += "_" + m_countryCode;
        // kdDebug() << "FestivalIntConf::setDefaultVoice:: looking for default voice to match language code " << languageCode << endl;
        uint index = 0;
        // Prefer existing voice if it matches.
        if (currentVoiceIndex >= 0)
        {
            QString vlCode = m_voiceList[currentVoiceIndex].languageCode.left(languageCode.length());
            if (languageCode = vlCode)
            {
                found = true;
                index = currentVoiceIndex;
            }
        }
        if (!found)
        {
            for(index = 0 ; index < m_voiceList.count(); ++index)
            {
                QString vlCode = m_voiceList[index].languageCode.left(languageCode.length());
                // kdDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode << endl;
                if(languageCode == vlCode)
                {
                    found = true;
                    break;
                }
            }
        }
        // If not found, search for a match on just the language code.
        if (!found)
        {
            languageCode = m_languageCode;
            // Prefer existing voice if it matches.
            if (currentVoiceIndex >= 0)
            {
                QString vlCode = m_voiceList[currentVoiceIndex].languageCode.left(languageCode.length());
                if (languageCode = vlCode)
                {
                    found = true;
                    index = currentVoiceIndex;
                }
            }
            if (!found)
            {
                for(index = 0 ; index < m_voiceList.count(); ++index)
                {
                    QString vlCode = m_voiceList[index].languageCode.left(languageCode.length());
                    // kdDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode << endl;
                    if(languageCode == vlCode)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
        // If not found, pick first voice that is not "Unknown".
        if (!found)
        {
            for(index = 0 ; index < m_voiceList.count(); ++index)
            {
                if (m_voiceList[index].name != i18n("Unknown"))
                {
                    found = true;
                    break;
                }
            }
        }
        if (found)
        {
            // kdDebug() << "FestivalIntConf::setDefaultVoice: auto picking voice code " << m_voiceList[index].code << endl;
            m_widget->selectVoiceCombo->setCurrentItem(index);
            m_widget->preloadCheckBox->setChecked(m_voiceList[index].preload);
            if (m_voiceList[index].volumeAdjustable)
            {
                m_widget->volumeBox->setEnabled(true);
                m_widget->volumeSlider->setEnabled(true);
            }
            else
            {
                m_widget->volumeBox->setValue(100);
                volumeBox_valueChanged(100);
                m_widget->volumeBox->setEnabled(false);
                m_widget->volumeSlider->setEnabled(false);
            }
            if (m_voiceList[index].rateAdjustable)
            {
                m_widget->timeBox->setEnabled(true);
                m_widget->timeSlider->setEnabled(true);
            }
            else
            {
                m_widget->timeBox->setValue(100);
                timeBox_valueChanged(100);
                m_widget->timeBox->setEnabled(false);
                m_widget->timeSlider->setEnabled(false);
            }
            if (m_voiceList[index].pitchAdjustable)
            {
                m_widget->frequencyBox->setEnabled(true);
                m_widget->frequencySlider->setEnabled(true);
            }
            else
            {
                m_widget->frequencyBox->setValue(100);
                frequencyBox_valueChanged(100);
                m_widget->frequencyBox->setEnabled(false);
                m_widget->frequencySlider->setEnabled(false);
            }
            if ((int)index != currentVoiceIndex) configChanged();
        }
    }
}

void FestivalIntConf::scanVoices()
{
    // kdDebug() << "FestivalIntConf::scanVoices: Running" << endl;
    // Get existing voice code (if any).
    QString currentVoiceCode;
    int index = m_widget->selectVoiceCombo->currentItem();
    if (index < (int)m_voiceList.count()) currentVoiceCode = m_voiceList[index].code;

    m_voiceList.clear();
    m_widget->selectVoiceCombo->clear();
    m_widget->selectVoiceCombo->insertItem(i18n("Scanning..please wait."));

    // Save current state of selectVoiceCombo box and disable.
    bool selectVoiceComboEnabled = m_widget->selectVoiceCombo->isEnabled();
    m_widget->selectVoiceCombo->setEnabled(false);

    // Clear existing list of supported voice codes.
    m_supportedVoiceCodes.clear();
    m_widget->selectVoiceCombo->clear();

    QString exePath = realFilePath(m_widget->festivalPath->url());
    if (!getLocation(exePath).isEmpty())
    {
        // Set up a progress dialog.
        m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_queryvoices",
                                            i18n("Query Voices"),
                                            i18n("Querying Festival for available voices.  This could take up to 15 seconds."),
                                            true);
        m_progressDlg->progressBar()->hide();
        m_progressDlg->setAllowCancel(true);

        // Create Festival process and request a list of voice codes.
        if (m_festProc)
            m_festProc->stopText();
        else
        {
            m_festProc = new FestivalIntProc();
            connect (m_festProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
        }
        connect (m_festProc, SIGNAL(queryVoicesFinished(const QStringList&)),
                 this, SLOT(slotQueryVoicesFinished(const QStringList&)));
        m_festProc->queryVoices(exePath);

        // Display progress dialog modally.
        m_progressDlg->exec();
        // kdDebug() << "FestivalIntConf::scanVoices: back from progressDlg->exec()" << endl;

        // Processing continues until either user clicks Cancel button, or until
        // Festival responds with the list.  When Festival responds with list,
        // the progress dialog is closed.

        disconnect (m_festProc, SIGNAL(queryVoicesFinished(const QStringList&)),
                    this, SLOT(slotQueryVoicesFinished(const QStringList&)));
        if (!m_progressDlg->wasCancelled()) m_festProc->stopText();
        delete m_progressDlg;
        m_progressDlg = 0;
    }

    if (!m_supportedVoiceCodes.isEmpty())
    {
        // User's desktop language setting.
        QString desktopLanguageCode = KGlobal::locale()->language();
        QString twoAlpha;
        QString countryCode;
        QString charSet;
        KGlobal::locale()->splitLocale(desktopLanguageCode, twoAlpha, countryCode, charSet);
        desktopLanguageCode = twoAlpha.lower();
        // Festival known voices list.
        KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festivalint/voices",
                       true, false);
        QStringList::ConstIterator itEnd = m_supportedVoiceCodes.constEnd();
        for(QStringList::ConstIterator it = m_supportedVoiceCodes.begin(); it != itEnd; ++it )
        {
            QString code = *it;
            voices.setGroup(code);
            voiceStruct voiceTemp;
            voiceTemp.code = code;
            voiceTemp.name = voices.readEntry("Name", i18n("Unknown"));
            voiceTemp.languageCode = voices.readEntry("Language", m_languageCode);
            // Get translated comment, fall back to English comment, fall back to code.
            voiceTemp.comment = voices.readEntry("Comment["+desktopLanguageCode+"]",
                    voices.readEntry("Comment", code));
            voiceTemp.gender = voices.readEntry("Gender", "neutral");
            voiceTemp.preload = voices.readBoolEntry("Preload", false);
            voiceTemp.volumeAdjustable = voices.readBoolEntry("VolumeAdjustable", true);
            voiceTemp.rateAdjustable = voices.readBoolEntry("RateAdjustable", true);
            voiceTemp.pitchAdjustable = voices.readBoolEntry("PitchAdjustable", true);
            m_voiceList.append(voiceTemp);
            m_widget->selectVoiceCombo->insertItem(voiceTemp.name + " (" + voiceTemp.comment + ")");
        }
        m_widget->selectVoiceCombo->setEnabled(true);
    }
    setDefaultVoice(voiceCodeToListIndex(currentVoiceCode));
    // Emit configChanged if the enabled state of the selectVoiceCombo has changed.
    // This occurs when user changes Festival EXE path, then clicks Rescan.
    if (selectVoiceComboEnabled != m_widget->selectVoiceCombo->isEnabled()) configChanged();
}

void FestivalIntConf::slotQueryVoicesFinished(const QStringList &voiceCodes)
{
    m_supportedVoiceCodes = voiceCodes;
    if (m_progressDlg) m_progressDlg->close();
}

void FestivalIntConf::slotTest_clicked()
{
    // kdDebug() << "FestivalIntConf::slotTest_clicked: Running " << endl;
    // If currently synthesizing, stop it.
    if (m_festProc)
        m_festProc->stopText();
    else
    {
        m_festProc = new FestivalIntProc();
        connect (m_festProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "festivalintplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Get the code for the selected voice.
    QString voiceCode = m_voiceList[m_widget->selectVoiceCombo->currentItem()].code;

    // Use the translated name of the voice as the test message.
    QString testMsg = m_voiceList[m_widget->selectVoiceCombo->currentItem()].comment;
    // Fall back if none.
    if (testMsg == voiceCode) testMsg =
                i18n("K D E is a modern graphical desktop for UNIX computers.");

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "ktts_festivalint_testdlg",
                                        i18n("Testing"),
                                        i18n("Testing.  MultiSyn voices require several seconds to load.  Please be patient."),
                                        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // kdDebug() << "FestivalIntConf::slotTest_clicked: calling synth with voiceCode: " << voiceCode << " time percent: " << m_widget->timeBox->value() << endl;
    connect (m_festProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_festProc->synth(
            realFilePath(m_widget->festivalPath->url()),
    testMsg,
    tmpWaveFile,
    voiceCode,
    m_widget->timeBox->value(),
    m_widget->frequencyBox->value(),
    m_widget->volumeBox->value());

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_festProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    if (m_progressDlg->wasCancelled()) m_festProc->stopText();
    delete m_progressDlg;
    m_progressDlg = 0;
}

void FestivalIntConf::slotSynthFinished()
{
    // kdDebug() << "FestivalIntConf::slotSynthFinished: Running" << endl;
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_festProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // Get new wavefile name.
    m_waveFile = m_festProc->getFilename();
    // Tell synth we're done.
    m_festProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    QFile::remove(m_waveFile);
    m_waveFile = QString::null;
    if (m_progressDlg) m_progressDlg->close();
}

void FestivalIntConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_festProc->getFilename();
    // kdDebug() << "FestivalIntConf::slotSynthStopped: filename = " << filename << endl;
    if (!filename.isNull()) QFile::remove(filename);
}

void FestivalIntConf::slotFestivalPath_textChanged()
{
    QString exePath = realFilePath(m_widget->festivalPath->url());
    m_widget->selectVoiceCombo->setEnabled(false);
    if (!exePath.isEmpty() && !getLocation(exePath).isEmpty())
    {
        m_widget->rescan->setEnabled(true);
    } else m_widget->rescan->setEnabled(false);
}

void FestivalIntConf::slotSelectVoiceCombo_activated()
{
    int index = m_widget->selectVoiceCombo->currentItem();
    m_widget->preloadCheckBox->setChecked(
        m_voiceList[index].preload);
    if (m_voiceList[index].volumeAdjustable)
    {
        m_widget->volumeBox->setEnabled(true);
        m_widget->volumeSlider->setEnabled(true);
    }
    else
    {
        m_widget->volumeBox->setValue(100);
        volumeBox_valueChanged(100);
        m_widget->volumeBox->setEnabled(false);
        m_widget->volumeSlider->setEnabled(false);
    }
    if (m_voiceList[index].rateAdjustable)
    {
        m_widget->timeBox->setEnabled(true);
        m_widget->timeSlider->setEnabled(true);
    }
    else
    {
        m_widget->timeBox->setValue(100);
        timeBox_valueChanged(100);
        m_widget->timeBox->setEnabled(false);
        m_widget->timeSlider->setEnabled(false);
    }
    if (m_voiceList[index].pitchAdjustable)
    {
        m_widget->frequencyBox->setEnabled(true);
        m_widget->frequencySlider->setEnabled(true);
    }
    else
    {
        m_widget->frequencyBox->setValue(100);
        frequencyBox_valueChanged(100);
        m_widget->frequencyBox->setEnabled(false);
        m_widget->frequencySlider->setEnabled(false);
    }
}

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int FestivalIntConf::percentToSlider(int percentValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int FestivalIntConf::sliderToPercent(int sliderValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void FestivalIntConf::volumeBox_valueChanged(int percentValue) {
    m_widget->volumeSlider->setValue(percentToSlider(percentValue));
}

void FestivalIntConf::timeBox_valueChanged(int percentValue) {
    m_widget->timeSlider->setValue (percentToSlider (percentValue));
}

void FestivalIntConf::frequencyBox_valueChanged(int percentValue) {
    m_widget->frequencySlider->setValue(percentToSlider(percentValue));
}

void FestivalIntConf::volumeSlider_valueChanged(int sliderValue) {
    m_widget->volumeBox->setValue(sliderToPercent(sliderValue));
}

void FestivalIntConf::timeSlider_valueChanged(int sliderValue) {
    m_widget->timeBox->setValue (sliderToPercent (sliderValue));
}

void FestivalIntConf::frequencySlider_valueChanged(int sliderValue) {
    m_widget->frequencyBox->setValue(sliderToPercent(sliderValue));
}
