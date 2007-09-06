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

// C++ includes.
#include <math.h>

// Qt includes.
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtCore/QDir>
#include <QtGui/QSlider>
#include <QtXml/qdom.h>
#include <QtCore/QTextCodec>
#include <QtGui/QPixmap>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <k3procio.h>
#include <kprogressdialog.h>
#include <kicon.h>

// KTTS includes.
#include "testplayer.h"

// FestivalInt includes.
#include "festivalintproc.h"
#include "festivalintconf.h"
#include "festivalintconf.moc"

/** Constructor */
FestivalIntConf::FestivalIntConf( QWidget* parent, const QStringList& /*args*/) :
    PlugInConf(parent, "festivalintconf")
{
    // kDebug() << "FestivalIntConf::FestivalIntConf: Running";
    m_festProc = 0;
    m_progressDlg = 0;
    m_supportsSSML = FestivalIntProc::ssUnknown;

    setupUi(this);

    festivalPath->setMode(KFile::File | KFile::ExistingOnly);
    festivalPath->setFilter("*");

    // Build codec list and fill combobox.
    m_codecList = PlugInProc::buildCodecList();
    characterCodingBox->clear();
    characterCodingBox->addItems(m_codecList);

    //    defaults();

    connect(festivalPath, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotFestivalPath_textChanged()));
    connect(selectVoiceCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotSelectVoiceCombo_activated()));
    connect(selectVoiceCombo, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
    connect(testButton, SIGNAL(clicked()), this, SLOT(slotTest_clicked()));
    connect(rescan, SIGNAL(clicked()), this, SLOT(scanVoices()));
    connect(volumeBox, SIGNAL(valueChanged(int)),
            this, SLOT(volumeBox_valueChanged(int)));
    connect(timeBox, SIGNAL(valueChanged(int)),
            this, SLOT(timeBox_valueChanged(int)));
    connect(frequencyBox, SIGNAL(valueChanged(int)),
            this, SLOT(frequencyBox_valueChanged(int)));
    connect(volumeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(volumeSlider_valueChanged(int)));
    connect(timeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeSlider_valueChanged(int)));
    connect(frequencySlider, SIGNAL(valueChanged(int)),
            this, SLOT(frequencySlider_valueChanged(int)));
    connect(volumeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(frequencyBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(preloadCheckBox, SIGNAL(clicked()), this, SLOT(configChanged()));
    connect(characterCodingBox, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(characterCodingBox, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
}

/** Destructor */
FestivalIntConf::~FestivalIntConf(){
    // kDebug() << "FestivalIntConf::~FestivalIntConf: Running";
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_festProc;
    delete m_progressDlg;
}

/**
* Given a voice code, returns index into m_voiceList array (and voiceCombo box).
* -1 if not found.
*/
int FestivalIntConf::voiceCodeToListIndex(const QString& voiceCode) const
{
    for (int i = 0; i < m_voiceList.size(); ++i)
        if (m_voiceList[i].code == voiceCode) return i;
    return -1;
}

void FestivalIntConf::load(KConfig *c, const QString &configGroup){
    //kDebug() << "FestivalIntConf::load: Running";
    KConfigGroup festivalConfig(c, "FestivalInt");
    QString exePath = festivalConfig.readEntry("FestivalExecutablePath", "festival");
    QString exeLocation = getLocation(exePath);
    if (!exeLocation.isEmpty()) exePath = exeLocation;
    exePath = realFilePath(exePath);

    KConfigGroup config(c, configGroup);
    festivalPath->setUrl(KUrl::fromPath(config.readEntry("FestivalExecutablePath", exePath)));
    preloadCheckBox->setChecked(false);
    scanVoices();
    QString voiceSelected(config.readEntry("Voice"));
    int index = voiceCodeToListIndex(voiceSelected);
    if (index >= 0)
    {
        selectVoiceCombo->setCurrentIndex(index);
        preloadCheckBox->setChecked(m_voiceList[index].preload);
    }
    volumeBox->setValue(config.readEntry("volume", 100));
    timeBox->setValue(config.readEntry("time", 100));
    frequencyBox->setValue(config.readEntry("pitch", 100));
    preloadCheckBox->setChecked(config.readEntry(
                "Preload", preloadCheckBox->isChecked()));
    m_languageCode = config.readEntry("LanguageCode", m_languageCode);
    m_supportsSSML = static_cast<FestivalIntProc::SupportsSSML>(
            config.readEntry("SupportsSSML", int(FestivalIntProc::ssUnknown)));
    QString codecName = PlugInProc::codecIndexToCodecName(
            characterCodingBox->currentIndex(), m_codecList);
    codecName = config.readEntry("Codec", codecName);
    int codecNdx = PlugInProc::codecNameToListIndex(codecName, m_codecList);
    characterCodingBox->setCurrentIndex(codecNdx);
}

void FestivalIntConf::save(KConfig *c, const QString &configGroup){
    // kDebug() << "FestivalIntConf::save: Running";
    KConfigGroup festivalConfig(c, "FestivalInt");
    festivalConfig.writeEntry("FestivalExecutablePath", realFilePath(festivalPath->url().path()));

    KConfigGroup config(c, configGroup);
    config.writeEntry("FestivalExecutablePath", realFilePath(festivalPath->url().path()));
    config.writeEntry("Voice", m_voiceList[selectVoiceCombo->currentIndex()].code);
    config.writeEntry("volume", volumeBox->value());
    config.writeEntry("time", timeBox->value());
    config.writeEntry("pitch", frequencyBox->value());
    config.writeEntry("Preload", preloadCheckBox->isChecked());
    config.writeEntry("LanguageCode", m_voiceList[selectVoiceCombo->currentIndex()].languageCode);
    config.writeEntry("SupportsSSML", int(m_supportsSSML));
    int codec = characterCodingBox->currentIndex();
    config.writeEntry("Codec", PlugInProc::codecIndexToCodecName(codec, m_codecList));
}

void FestivalIntConf::defaults(){
    // kDebug() << "FestivalIntConf::defaults: Running";
    festivalPath->setUrl(KUrl("festival"));
    timeBox->setValue(100);
    timeBox_valueChanged(100);
    volumeBox->setValue(100);
    volumeBox_valueChanged(100);
    frequencyBox->setValue(100);
    frequencyBox_valueChanged(100);
    preloadCheckBox->setChecked(false);
    characterCodingBox->setCurrentIndex(
        PlugInProc::codecNameToListIndex("ISO 8859-1", m_codecList));
    scanVoices();
}

void FestivalIntConf::setDesiredLanguage(const QString &lang)
{
    // kDebug() << "FestivalIntConf::setDesiredLanguage: Running";
    m_languageCode = splitLanguageCode(lang, m_countryCode);
}

QString FestivalIntConf::getTalkerCode()
{
    if (!selectVoiceCombo->isEnabled()) return QString();
    QString exePath = realFilePath(festivalPath->url().path());
    if (exePath.isEmpty()) return QString();
    if (getLocation(exePath).isEmpty()) return QString();
    if (m_voiceList.count() == 0) return QString();
    QString normalTalkerCode;
    voiceStruct voiceTemp = m_voiceList[selectVoiceCombo->currentIndex()];
    // Determine volume attribute.  soft < 75% <= medium <= 125% < loud.
    QString volume = "medium";
    if (volumeBox->value() < 75) volume = "soft";
    if (volumeBox->value() > 125) volume = "loud";
    // Determine rate attribute.  slow < 75% <= medium <= 125% < fast.
    QString rate = "medium";
    if (timeBox->value() < 75) rate = "slow";
    if (timeBox->value() > 125) rate = "fast";
    normalTalkerCode = QString(
            "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
            "<prosody volume=\"%4\" rate=\"%5\" />"
            "<kttsd synthesizer=\"%6\" />")
            .arg(voiceTemp.languageCode)
            .arg(voiceTemp.code)
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
    // kDebug() << "FestivalIntCont::setDefaultVoice: Running";
    // If language code is known, auto pick first voice that matches the language code.
    if (!m_languageCode.isEmpty())
    {
        bool found = false;
        // First search for a match on both language code and country code.
        QString languageCode = m_languageCode;
        if (!m_countryCode.isNull()) languageCode += '_' + m_countryCode;
        // kDebug() << "FestivalIntConf::setDefaultVoice:: looking for default voice to match language code " << languageCode;
        int index = 0;
        // Prefer existing voice if it matches.
        if (currentVoiceIndex >= 0)
        {
            QString vlCode = m_voiceList[currentVoiceIndex].languageCode.left(languageCode.length());
            if (languageCode == vlCode)
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
                // kDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode;
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
                if (languageCode == vlCode)
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
                    // kDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode;
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
            // kDebug() << "FestivalIntConf::setDefaultVoice: auto picking voice code " << m_voiceList[index].code;
            selectVoiceCombo->setCurrentIndex(index);
            preloadCheckBox->setChecked(m_voiceList[index].preload);
            QString codecName = m_voiceList[index].codecName;
            int codecNdx = PlugInProc::codecNameToListIndex(codecName, m_codecList);
            characterCodingBox->setCurrentIndex(codecNdx);
            if (m_voiceList[index].volumeAdjustable)
            {
                volumeBox->setEnabled(true);
                volumeSlider->setEnabled(true);
            }
            else
            {
                volumeBox->setValue(100);
                volumeBox_valueChanged(100);
                volumeBox->setEnabled(false);
                volumeSlider->setEnabled(false);
            }
            if (m_voiceList[index].rateAdjustable)
            {
                timeBox->setEnabled(true);
                timeSlider->setEnabled(true);
            }
            else
            {
                timeBox->setValue(100);
                timeBox_valueChanged(100);
                timeBox->setEnabled(false);
                timeSlider->setEnabled(false);
            }
            if (m_voiceList[index].pitchAdjustable)
            {
                frequencyBox->setEnabled(true);
                frequencySlider->setEnabled(true);
            }
            else
            {
                frequencyBox->setValue(100);
                frequencyBox_valueChanged(100);
                frequencyBox->setEnabled(false);
                frequencySlider->setEnabled(false);
            }
            if ((int)index != currentVoiceIndex) configChanged();
        }
    }
}

/**
 * Given an XML node and child element name, returns the string value from the child element.
 * If no such child element, returns def.
 */
QString FestivalIntConf::readXmlString(QDomNode &node, const QString &elementName, const QString &def)
{
    QDomNode childNode = node.namedItem(elementName);
    if (!childNode.isNull())
        return childNode.toElement().text();
    else
        return def;
}

/**
 * Given an XML node and child element name, returns the boolean value from the child element.
 * If no such child element, returns def.
 */
bool FestivalIntConf::readXmlBool(QDomNode &node, const QString &elementName, bool def)
{
    QDomNode childNode = node.namedItem(elementName);
    if (!childNode.isNull())
        return (childNode.toElement().text() == "true");
    else
        return def;
}

void FestivalIntConf::scanVoices()
{
    // kDebug() << "FestivalIntConf::scanVoices: Running";
    // Get existing voice code (if any).
    QString currentVoiceCode;
    int index = selectVoiceCombo->currentIndex();
    if (index >= 0 && index < m_voiceList.count()) currentVoiceCode = m_voiceList[index].code;

    m_voiceList.clear();
    selectVoiceCombo->clear();
    selectVoiceCombo->addItem(i18n("Scanning... Please wait."));

    // Save current state of selectVoiceCombo box and disable.
    bool selectVoiceComboEnabled = selectVoiceCombo->isEnabled();
    selectVoiceCombo->setEnabled(false);

    // Clear existing list of supported voice codes.
    // m_supportedVoiceCodes.clear();
    selectVoiceCombo->clear();

    QString exePath = realFilePath(festivalPath->url().path());
    if (!getLocation(exePath).isEmpty())
    {
        // Set up a progress dialog.
        m_progressDlg = new KProgressDialog(this,
            i18n("Query Voices"),
            i18n("Querying Festival for available voices.  This could take up to 15 seconds."));
        m_progressDlg->setModal(true);
        m_progressDlg->progressBar()->hide();
        m_progressDlg->setAllowCancel(true);
        // TODO: This is a bug workaround.  Remove when no longer needed.
        m_progressDlg->setDefaultButton(KDialog::Cancel);

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
        // kDebug() << "FestivalIntConf::scanVoices: back from progressDlg->exec()";

        // Processing continues until either user clicks Cancel button, or until
        // Festival responds with the list.  When Festival responds with list,
        // the progress dialog is closed.

        disconnect (m_festProc, SIGNAL(queryVoicesFinished(const QStringList&)),
                    this, SLOT(slotQueryVoicesFinished(const QStringList&)));
        if (!m_progressDlg->wasCancelled()) m_festProc->stopText();
        delete m_progressDlg;
        m_progressDlg = 0;
        m_supportsSSML = m_festProc->supportsSSML();
    }

    if (!m_supportedVoiceCodes.isEmpty())
    {
        // User's desktop language setting.
        QString desktopLanguageCode = KGlobal::locale()->language();
        QString langAlpha;
        QString countryCode;
        QString modifier;
        QString charSet;
        KGlobal::locale()->splitLocale(desktopLanguageCode, langAlpha, countryCode, modifier, charSet);
        desktopLanguageCode = langAlpha.toLower();

        // Festival known voices list.
        QString voicesFilename = KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festivalint/voices";
        QDomDocument voicesDoc("Festival Voices");
        QFile voicesFile(voicesFilename);
        if (voicesFile.open(QIODevice::ReadOnly)) voicesDoc.setContent(&voicesFile);
        voicesFile.close();
        QDomNodeList voices = voicesDoc.elementsByTagName("voice");
        uint voicesCount = voices.count();
        if (voicesCount == 0)
            kDebug() << "FestivalIntConf::scanVoices: Unable to open " << voicesFilename << ".  Is KDEDIR defined?";

        // Iterate thru list of voice codes returned by Festival,
        // find matching entry in voices.xml file, and add to list of supported voices.
        KIcon maleIcon("male");
        KIcon femaleIcon("female");
        QStringList::ConstIterator itEnd = m_supportedVoiceCodes.constEnd();
        for(QStringList::ConstIterator it = m_supportedVoiceCodes.begin(); it != itEnd; ++it )
        {
            QString code = *it;
            bool found = false;
            for (uint index=0; index < voicesCount; ++index)
            {
                QDomNode voiceNode = voices.item(index);
                QString voiceCode = readXmlString(voiceNode, "code", QString());
                // kDebug() << "FestivalIntConf::scanVoices: Comparing code " << code << " to " << voiceCode;
                if (voiceCode == code)
                {
                    found = true;
                    voiceStruct voiceTemp;
                    voiceTemp.code = code;
                    voiceTemp.name = i18nc("FestivalVoiceName",
                        readXmlString(voiceNode, "name", "Unknown").toUtf8());
                    voiceTemp.languageCode = readXmlString(voiceNode, "language", m_languageCode);
                    voiceTemp.codecName = readXmlString(voiceNode, "codec", "ISO 8859-1");
                    voiceTemp.gender = readXmlString(voiceNode, "gender", "neutral");
                    voiceTemp.preload = readXmlBool(voiceNode, "preload", false);
                    voiceTemp.volumeAdjustable = readXmlBool(voiceNode, "volume-adjustable", true);
                    voiceTemp.rateAdjustable = readXmlBool(voiceNode, "rate-adjustable", true);
                    voiceTemp.pitchAdjustable = readXmlBool(voiceNode, "pitch-adjustable", true);
                    m_voiceList.append(voiceTemp);
                    QString voiceDisplayName = voiceTemp.name + " (" + voiceTemp.code + ')';
                    if (voiceTemp.gender == "male")
                        selectVoiceCombo->addItem(maleIcon, voiceDisplayName);
                    else if (voiceTemp.gender == "female")
                        selectVoiceCombo->addItem(femaleIcon, voiceDisplayName);
                    else
                        selectVoiceCombo->addItem(voiceDisplayName);
                    break;
                }
            }
            if (!found)
            {
                voiceStruct voiceTemp;
                voiceTemp.code = code;
                voiceTemp.name = i18n("Unknown");
                voiceTemp.languageCode = m_languageCode;
                voiceTemp.codecName = "ISO 8858-1";
                voiceTemp.gender = "neutral";
                voiceTemp.preload = false;
                voiceTemp.volumeAdjustable = true;
                voiceTemp.rateAdjustable = true;
                voiceTemp.pitchAdjustable = true;
                m_voiceList.append(voiceTemp);
                selectVoiceCombo->addItem(voiceTemp.name + " (" + voiceTemp.code + ')');
            }
        }
        selectVoiceCombo->setEnabled(true);
    } else kDebug() << "FestivalIntConf::scanVoices: No voices found";
    setDefaultVoice(voiceCodeToListIndex(currentVoiceCode));
    // Emit configChanged if the enabled state of the selectVoiceCombo has changed.
    // This occurs when user changes Festival EXE path, then clicks Rescan.
    if (selectVoiceComboEnabled != selectVoiceCombo->isEnabled()) configChanged();
}

void FestivalIntConf::slotQueryVoicesFinished(const QStringList &voiceCodes)
{
    // kDebug() << "FestivalIntConf::slotQueryVoicesFinished: voiceCodes.count() = " << voiceCodes.count();
    m_supportedVoiceCodes = voiceCodes;
    if (m_progressDlg) m_progressDlg->close();
}

void FestivalIntConf::slotTest_clicked()
{
    // kDebug() << "FestivalIntConf::slotTest_clicked: Running ";
    // If currently synthesizing, stop it.
    if (m_festProc)
        m_festProc->stopText();
    else
    {
        m_festProc = new FestivalIntProc();
        connect (m_festProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTemporaryFile *tempFile = new KTemporaryFile();
    tempFile->setPrefix("festivalintplugin-");
    tempFile->setSuffix(".wav");
    tempFile->open();
    QString tmpWaveFile = tempFile->fileName();
    delete tempFile;

    kDebug() << "FestivalIntConf::slotTest_clicked: tmpWaveFile = " << tmpWaveFile;

    // Get the code for the selected voice.
    QString voiceCode = m_voiceList[selectVoiceCombo->currentIndex()].code;

    // Get language code for the selected voice.
    QString languageCode = m_voiceList[selectVoiceCombo->currentIndex()].languageCode;

    // Get test message in the language of the voice.
    QString testMsg = testMessage(languageCode);

    // Get codec.
    QTextCodec* codec = PlugInProc::codecIndexToCodec(
        characterCodingBox->currentIndex(), m_codecList);

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(this,
        i18n("Testing"),
        i18n("Testing.  MultiSyn voices require several seconds to load.  Please be patient."));
    m_progressDlg->setModal(true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // kDebug() << "FestivalIntConf::slotTest_clicked: calling synth with voiceCode: " << voiceCode << " time percent: " << timeBox->value();
    connect (m_festProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_festProc->synth(
        realFilePath(festivalPath->url().path()),
        testMsg,
        tmpWaveFile,
        voiceCode,
        timeBox->value(),
        frequencyBox->value(),
        volumeBox->value(),
        languageCode,
        codec);

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
    // kDebug() << "FestivalIntConf::slotSynthFinished: Running";
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
    m_waveFile.clear();
    if (m_progressDlg) m_progressDlg->close();
}

void FestivalIntConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_festProc->getFilename();
    // kDebug() << "FestivalIntConf::slotSynthStopped: filename = " << filename;
    if (!filename.isNull()) QFile::remove(filename);
}

void FestivalIntConf::slotFestivalPath_textChanged()
{
    QString exePath = realFilePath(festivalPath->url().path());
    selectVoiceCombo->setEnabled(false);
    if (!exePath.isEmpty() && !getLocation(exePath).isEmpty())
    {
        rescan->setEnabled(true);
    } else rescan->setEnabled(false);
}

void FestivalIntConf::slotSelectVoiceCombo_activated()
{
    int index = selectVoiceCombo->currentIndex();
    QString codecName = m_voiceList[index].codecName;
    int codecNdx = PlugInProc::codecNameToListIndex(codecName, m_codecList);
    characterCodingBox->setCurrentIndex(codecNdx);
    preloadCheckBox->setChecked(
        m_voiceList[index].preload);
    if (m_voiceList[index].volumeAdjustable)
    {
        volumeBox->setEnabled(true);
        volumeSlider->setEnabled(true);
    }
    else
    {
        volumeBox->setValue(100);
        volumeBox_valueChanged(100);
        volumeBox->setEnabled(false);
        volumeSlider->setEnabled(false);
    }
    if (m_voiceList[index].rateAdjustable)
    {
        timeBox->setEnabled(true);
        timeSlider->setEnabled(true);
    }
    else
    {
        timeBox->setValue(100);
        timeBox_valueChanged(100);
        timeBox->setEnabled(false);
        timeSlider->setEnabled(false);
    }
    if (m_voiceList[index].pitchAdjustable)
    {
        frequencyBox->setEnabled(true);
        frequencySlider->setEnabled(true);
    }
    else
    {
        frequencyBox->setValue(100);
        frequencyBox_valueChanged(100);
        frequencyBox->setEnabled(false);
        frequencySlider->setEnabled(false);
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
    volumeSlider->setValue(percentToSlider(percentValue));
}

void FestivalIntConf::timeBox_valueChanged(int percentValue) {
    timeSlider->setValue (percentToSlider (percentValue));
}

void FestivalIntConf::frequencyBox_valueChanged(int percentValue) {
    frequencySlider->setValue(percentToSlider(percentValue));
}

void FestivalIntConf::volumeSlider_valueChanged(int sliderValue) {
    volumeBox->setValue(sliderToPercent(sliderValue));
}

void FestivalIntConf::timeSlider_valueChanged(int sliderValue) {
    timeBox->setValue (sliderToPercent (sliderValue));
}

void FestivalIntConf::frequencySlider_valueChanged(int sliderValue) {
    frequencyBox->setValue(sliderToPercent(sliderValue));
}
