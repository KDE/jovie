/***************************************************************************
    begin                : Mon Okt 14 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : gunnar@schmi-dt.de
    current mainainer:   : Gary Cramblitt <garycramblitt@comcast.net> 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// System includes.
#include <math.h>

// Qt includes. 
#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QVBoxLayout>
#include <QTextStream>

// KDE includes.
#include <ktempfile.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kprogressbar.h>
#include <kprogressdialog.h>
#include <kiconloader.h>

// KTTS includes.
#include <pluginconf.h>
#include <testplayer.h>
#include <talkercode.h>

// Hadifix includes.
#include "hadifixproc.h"
#include "hadifixconf.h"
#include "hadifixconf.moc"
#include "ui_voicefileui.h"

// ====================================================================
// HadifixConfPrivate

HadifixConfPrivate::HadifixConfPrivate(QWidget *parent) :
    QWidget(parent)
{
    hadifixProc = 0;
    progressDlg = 0;
    setupUi(this);
    findInitialConfig();
};

HadifixConfPrivate::~HadifixConfPrivate() {
    if (hadifixProc) hadifixProc->stopText();
    delete hadifixProc;
    if (!waveFile.isNull()) QFile::remove(waveFile);
    delete progressDlg;
};

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int HadifixConfPrivate::percentToSlider (int percentValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int HadifixConfPrivate::sliderToPercent (int sliderValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void HadifixConfPrivate::volumeBox_valueChanged (int percentValue) {
    volumeSlider->setValue (percentToSlider (percentValue));
}

void HadifixConfPrivate::timeBox_valueChanged (int percentValue) {
    timeSlider->setValue (percentToSlider (percentValue));
}

void HadifixConfPrivate::frequencyBox_valueChanged (int percentValue) {
    frequencySlider->setValue (percentToSlider (percentValue));
}

void HadifixConfPrivate::volumeSlider_valueChanged (int sliderValue) {
    volumeBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfPrivate::timeSlider_valueChanged (int sliderValue) {
    timeBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfPrivate::frequencySlider_valueChanged (int sliderValue) {
    frequencyBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfPrivate::init () {
    male = KGlobal::iconLoader()->loadIcon("male", K3Icon::Small);
    female = KGlobal::iconLoader()->loadIcon("female", K3Icon::Small);
}

void HadifixConfPrivate::addVoice (const QString &filename, bool isMale) {
    if (isMale) {
        if (!maleVoices.contains(filename)) {
            int id = voiceCombo->count();
            maleVoices.insert (filename, id);
            voiceCombo->addItem (male, filename);
        }
    }
    else {
        if (!femaleVoices.contains(filename)) {
            int id = voiceCombo->count();
            femaleVoices.insert (filename, id);
            voiceCombo->addItem (female, filename);
        }
    }
}

void HadifixConfPrivate::addVoice (const QString &filename, bool isMale, const QString &displayname) {
    addVoice (filename, isMale);

    if (isMale) {
        defaultVoicesMap [maleVoices [filename]] = filename;
        voiceCombo->setItemIcon (maleVoices [filename], male);
        voiceCombo->setItemText (maleVoices [filename], displayname);
    }
    else{
        defaultVoicesMap [femaleVoices [filename]] = filename;
        voiceCombo->setItemIcon (femaleVoices [filename], female);
        voiceCombo->setItemText (femaleVoices [filename], displayname);
    }
}

void HadifixConfPrivate::setVoice (const QString &filename, bool isMale) {
    addVoice (filename, isMale);
    if (isMale)
        voiceCombo->setCurrentIndex (maleVoices[filename]);
    else
        voiceCombo->setCurrentIndex (femaleVoices[filename]);
}

QString HadifixConfPrivate::getVoiceFilename() {
    int curr = voiceCombo->currentIndex();

    QString filename = voiceCombo->itemText(curr);
    if (defaultVoicesMap.contains(curr))
        filename = defaultVoicesMap[curr];

    return filename;
}

bool HadifixConfPrivate::isMaleVoice() {
    int curr = voiceCombo->currentIndex();
    QString filename = getVoiceFilename();

    if (maleVoices.contains(filename))
        return maleVoices[filename] == curr;
    else
        return false;
}

/**
 * Tries to find hadifix and mbrola by looking onto the hard disk. This is
 * neccessary because both hadifix and mbrola do not have standard
 * installation directories.
 */
void HadifixConfPrivate::findInitialConfig() {
    QString hadifixDataPath = findHadifixDataPath();

    defaultHadifixExec = findExecutable(QStringList("txt2pho"), hadifixDataPath+"/../");

    QStringList list; list += "mbrola"; list += "mbrola-linux-i386";
    defaultMbrolaExec = findExecutable(list, hadifixDataPath+"/../../mbrola/");

    defaultVoices = findVoices (defaultMbrolaExec, hadifixDataPath);
}

/** Tries to find the hadifix data path by looking into a number of files. */
QString HadifixConfPrivate::findHadifixDataPath () {
    QStringList files;
    files += "/etc/txt2pho";
    files += QDir::homePath()+"/.txt2phorc";

    QStringList::iterator it;
    for (it = files.begin(); it != files.end(); ++it) {

        QFile file(*it);
        if ( file.open(QIODevice::ReadOnly) ) {
            QTextStream stream(&file);

            while (!stream.atEnd()) {
                QString s = stream.readLine().trimmed();
                // look for a line "DATAPATH=..."

                if (s.startsWith("DATAPATH")) {
                    s = s.mid(8, s.length()-8).trimmed();
                    if (s.startsWith("=")) {
                        s = s.mid(1, s.length()-1).trimmed();
                        if (s.startsWith("/"))
                            return s;
                        else {
                            QFileInfo info (QFileInfo(*it).path() + "/" + s);
                            return info.absoluteFilePath();
                        }
                    }
                }
           }
           file.close();
       }
   }
   return "/usr/local/txt2pho/";
}

/** Tries to find the an executable by looking onto the hard disk. */
QString HadifixConfPrivate::findExecutable (const QStringList &names, const QString &possiblePath) {
    // a) Try to find it directly
    QStringList::ConstIterator it;
    QStringList::ConstIterator itEnd = names.constEnd();
    for (it = names.constBegin(); it != itEnd; ++it) {
        QString executable = KStandardDirs::findExe (*it);
        if (!executable.isNull() && !executable.isEmpty())
            return executable;
        }

    // b) Try to find it in the path specified by the second parameter
    for (it = names.constBegin(); it != itEnd; ++it) {
        QFileInfo info (possiblePath+*it);
        if (info.exists() && info.isExecutable() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }

    // Both tries failed, so the user has to locate the executable.
    return QString();
}

/** Tries to find the voice files by looking onto the hard disk. */
QStringList HadifixConfPrivate::findVoices(QString mbrolaExec, const QString &hadifixDataPath) {

    // First of all:
    // dereference links to the mbrola executable (if mbrolaExec is a link).
    for (int i = 0; i < 10; ++i) {
        // If we have a chain of more than ten links something is surely wrong.
        QFileInfo info (mbrolaExec);
        if (info.exists() && info.isSymLink())
            mbrolaExec = info.readLink();
        }

    // Second:
    // create a list of directories that possibly contain voice files
    QStringList list;

    // 2a) search near the mbrola executable
    QFileInfo info (mbrolaExec);
    if (info.exists() && info.isFile() && info.isExecutable()) {
        QString mbrolaPath = info.absolutePath();
        list += mbrolaPath;
    }

    // 2b) search near the hadifix data path
    info.setFile(hadifixDataPath + "../../mbrola");
    QString mbrolaPath = info.absolutePath() + "/mbrola";
    if (!list.contains(mbrolaPath))
        list += mbrolaPath;

    // 2c) broaden the search by adding subdirs (with a depth of 2)
    QStringList subDirs = findSubdirs (list);
    QStringList subSubDirs = findSubdirs (subDirs);
    list += subDirs;
    list += subSubDirs;

    // Third:
    // look into each of these directories and search for voice files.
    QStringList result;
    QStringList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QDir baseDir (*it, QString(),
                      QDir::Name|QDir::IgnoreCase, QDir::Files);
        QStringList files = baseDir.entryList();

        QStringList::iterator iter;
        for (iter = files.begin(); iter != files.end(); ++iter) {
            // Voice files start with "MBROLA", but are afterwards binary files
            QString filename = *it + "/" + *iter;
            QFile file (filename);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream stream(&file);
                if (!stream.atEnd()) {
                    QString s = stream.readLine();
                    if (s.startsWith("MBROLA"))
                        if (HadifixProc::determineGender(mbrolaExec, filename)
                            != HadifixProc::NoVoice)
                        result += filename;
                    file.close();
                }
            }
        }
    }
    return result;
}

/** Returns a list of subdirs (with absolute paths) */
QStringList HadifixConfPrivate::findSubdirs (const QStringList &baseDirs) {
    QStringList result;

    QStringList::ConstIterator it;
    QStringList::ConstIterator itEnd = baseDirs.constEnd();
    for (it = baseDirs.constBegin(); it != itEnd; ++it) {
        // a) get a list of directory names
        QDir baseDir (*it, QString(),
                      QDir::Name|QDir::IgnoreCase, QDir::Dirs);
        QStringList list = baseDir.entryList();

        // b) produce absolute paths
        QStringList::ConstIterator iter;
        QStringList::ConstIterator iterEnd = list.constEnd();
        for (iter = list.constBegin(); iter != iterEnd; ++iter) {
            if ((*iter != ".") && (*iter != ".."))
                result += *it + "/" + *iter;
        }
    }
    return result;
}

void HadifixConfPrivate::setConfiguration (QString hadifixExec,  QString mbrolaExec,
                QString voice,        bool male,
                int volume, int time, int pitch,
                QString codecName)
{
    hadifixURL->setURL (hadifixExec);
    mbrolaURL->setURL (mbrolaExec);
    setVoice (voice, male);

    volumeBox->setValue (volume);
    timeBox->setValue (time);
    frequencyBox->setValue (pitch);
    int codec = PlugInProc::codecNameToListIndex(codecName, codecList);
    characterCodingBox->setCurrentIndex(codec);
}

void HadifixConfPrivate::initializeVoices () {
    QStringList::iterator it;
    for (it = defaultVoices.begin(); it != defaultVoices.end(); ++it) {
        HadifixProc::VoiceGender gender;
        QString name = QFileInfo(*it).fileName();
        gender = HadifixProc::determineGender(defaultMbrolaExec, *it);
        if (gender == HadifixProc::MaleGender)
            addVoice(*it, true, i18n("Male voice \"%1\"", name));
        else if (gender == HadifixProc::FemaleGender)
            addVoice(*it, false, i18n("Female voice \"%1\"", name));
        else {
            if (name == "de1")
                addVoice(*it, false, i18n("Female voice \"%1\"", name));
            else {
                addVoice(*it, true, i18n("Unknown voice \"%1\"", name));
                addVoice(*it, false, i18n("Unknown voice \"%1\"", name));
            }
        }
    }
};

void HadifixConfPrivate::initializeCharacterCodes() {
    // Build codec list and fill combobox.
    codecList = PlugInProc::buildCodecList();
    characterCodingBox->clear();
    characterCodingBox->addItems(codecList);
}

void HadifixConfPrivate::setDefaultEncodingFromVoice() {
    QString voiceFile = getVoiceFilename();
    QString voiceCode = QFileInfo(voiceFile).baseName();
    voiceCode = voiceCode.left(2);
    QString codecName = "Local";
    if (voiceCode == "de") codecName = "ISO 8859-1";
    if (voiceCode == "hu") codecName = "ISO 8859-2";
    characterCodingBox->setCurrentIndex(PlugInProc::codecNameToListIndex(
        codecName, codecList));
}

void HadifixConfPrivate::setDefaults () {
    QStringList::iterator it = defaultVoices.begin();
    // Find a voice that matches language code, if any.
    if (!languageCode.isEmpty())
    {
        QString justLang = languageCode.left(2);
        for (;it != defaultVoices.end();++it)
        {
            QString voiceCode = QFileInfo(*it).baseName().left(2);
            if (voiceCode == justLang) break;
        }
        if (it == defaultVoices.end()) it = defaultVoices.begin();
    }
    HadifixProc::VoiceGender gender;
    gender = HadifixProc::determineGender(defaultMbrolaExec, *it);

    setConfiguration (defaultHadifixExec, defaultMbrolaExec,
                      *it, gender == HadifixProc::MaleGender,
                      100, 100, 100, "Local");
};

void HadifixConfPrivate::load (KConfig *config, const QString &configGroup) {
    config->setGroup(configGroup);

    QString voice = config->readEntry("voice", getVoiceFilename());

    HadifixProc::VoiceGender gender;
    gender = HadifixProc::determineGender(defaultMbrolaExec, voice);
    bool isMale = (gender == HadifixProc::MaleGender);

    QString defaultCodecName = "Local";
    // TODO: Need a better way to determine proper codec for each voice.
    // This will do for now.
    QString voiceCode = QFileInfo(voice).baseName();
    if (voiceCode.left(2) == "de") defaultCodecName = "ISO 8859-1";
    if (voiceCode.left(2) == "hu") defaultCodecName = "ISO 8859-2";

    setConfiguration (
        config->readEntry ("hadifixExec",defaultHadifixExec),
        config->readEntry ("mbrolaExec", defaultMbrolaExec),
        config->readEntry ("voice",      voice),
        config->readEntry("gender", QVariant(isMale)).toBool(),
        config->readEntry ("volume",  100),
        config->readEntry ("time",    100),
        config->readEntry ("pitch",   100),
        config->readEntry ("codec",      defaultCodecName)
    );
};

void HadifixConfPrivate::save (KConfig *config, const QString &configGroup) {
    config->setGroup(configGroup);
    config->writeEntry ("hadifixExec", PlugInConf::realFilePath(hadifixURL->url()));
    config->writeEntry ("mbrolaExec", PlugInConf::realFilePath(mbrolaURL->url()));
    config->writeEntry ("voice",      getVoiceFilename());
    config->writeEntry ("gender",     isMaleVoice());
    config->writeEntry ("volume",     volumeBox->value());
    config->writeEntry ("time",       timeBox->value());
    config->writeEntry ("pitch",      frequencyBox->value());
    config->writeEntry ("codec",      PlugInProc::codecIndexToCodecName(
                                      characterCodingBox->currentIndex(), codecList));
};

// ====================================================================
// HadifixConf

/** Constructor */
HadifixConf::HadifixConf( QWidget* parent, const QStringList &) : 
    PlugInConf( parent, "hadifixconf" ){
    // kDebug() << "HadifixConf::HadifixConf: Running" << endl;
    QVBoxLayout *layout = new QVBoxLayout (this);
    layout->setAlignment (Qt::AlignTop);

    d = new HadifixConfPrivate(this);

    QString file = locate("data", "LICENSES/LGPL_V2");
    i18n("This plugin is distributed under the terms of the GPL v2 or later.");

    connect(d->voiceButton, SIGNAL(clicked()), this, SLOT(voiceButton_clicked()));
    connect(d->testButton, SIGNAL(clicked()), this, SLOT(testButton_clicked()));
    connect(d->voiceCombo, SIGNAL(activated(int)), this, SLOT(voiceCombo_activated(int)));
    connect(d->volumeBox, SIGNAL(valueChanged(int)), d, SLOT(volumeBox_valueChanged(int)));
    connect(d->volumeSlider, SIGNAL(valueChanged(int)), d, SLOT(volumeSlider_valueChanged(int)));
    connect(d->timeBox, SIGNAL(valueChanged(int)), d, SLOT(timeBox_valueChanged(int)));
    connect(d->timeSlider, SIGNAL(valueChanged(int)), d, SLOT(timeSlider_valueChanged(int)));
    connect(d->frequencyBox, SIGNAL(valueChanged(int)), d, SLOT(frequencyBox_valueChanged(int)));
    connect(d->frequencySlider, SIGNAL(valueChanged(int)), d, SLOT(frequencySlider_valueChanged(int)));

    connect(d->characterCodingBox, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
    connect(d->voiceCombo, SIGNAL(activated(const QString)), this, SLOT(configChanged()));
    connect(d->voiceCombo, SIGNAL(activated(const QString)), this, SLOT(configChanged()));
    connect(d->volumeBox, SIGNAL(valueChanged(const QString)), this, SLOT(configChanged()));
    connect(d->timeBox, SIGNAL(valueChanged(const QString)), this, SLOT(configChanged()));
    connect(d->frequencyBox, SIGNAL(valueChanged(const QString)), this, SLOT(configChanged()));
    connect(d->hadifixURL, SIGNAL(textChanged(const QString)), this, SLOT(configChanged()));
    connect(d->mbrolaURL, SIGNAL(textChanged(const QString)), this, SLOT(configChanged()));

    d->initializeCharacterCodes();
    d->initializeVoices();
    d->setDefaults();
    layout->addWidget (d);
}

/** Destructor */
HadifixConf::~HadifixConf(){
   // kDebug() << "HadifixConf::~HadifixConf: Running" << endl;
   delete d;
}

void HadifixConf::load(KConfig *config, const QString &configGroup) {
   // kDebug() << "HadifixConf::load: Running" << endl;
   d->setDefaults();
   d->load (config, configGroup);
}

void HadifixConf::save(KConfig *config, const QString &configGroup) {
   // kDebug() << "HadifixConf::save: Running" << endl;
   d->save (config, configGroup);
}

void HadifixConf::defaults() {
   // kDebug() << "HadifixConf::defaults: Running" << endl;
   d->setDefaults();
}

void HadifixConf::setDesiredLanguage(const QString &lang)
{
    d->languageCode = lang;
}

QString HadifixConf::getTalkerCode()
{
    if (!d->hadifixURL->url().isEmpty() && !d->mbrolaURL->url().isEmpty())
    {
        QString voiceFile = d->getVoiceFilename();
        if (QFileInfo(voiceFile).exists())
        {
            // mbrola voice file names usually start with two-letter language code,
            // but this is by no means guaranteed.
            QString voiceCode = QFileInfo(voiceFile).baseName();
            QString voiceLangCode = voiceCode.left(2);
            if (d->languageCode.left(2) != voiceLangCode)
            {
               // Verify that first two letters of voice filename are a valid language code.
               // If they are, switch to that language.
               if (!TalkerCode::languageCodeToLanguage(voiceLangCode).isEmpty())
                   d->languageCode = voiceLangCode;
            }
            QString gender = "male";
            if (!d->isMaleVoice()) gender = "female";
            QString volume = "medium";
            if (d->volumeBox->value() < 75) volume = "soft";
            if (d->volumeBox->value() > 125) volume = "loud";
            QString rate = "medium";
            if (d->timeBox->value() < 75) rate = "slow";
            if (d->timeBox->value() > 125) rate = "fast";
            return QString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .arg(d->languageCode)
                    .arg(voiceCode)
                    .arg(gender)
                    .arg(volume)
                    .arg(rate)
                    .arg("Hadifix");
        }
    }
    return QString();
}

void HadifixConf::voiceButton_clicked () {
    KDialog *dialog = new KDialog (this, 
        i18n("Voice File - Hadifix Plugin"),
        KDialog::Ok|KDialog::Cancel);
    QWidget *w;
    Ui::VoiceFileWidget *widget;
    widget->setupUi(w);
    dialog->setMainWidget(w);

    widget->femaleOption->setChecked(!d->isMaleVoice());
    widget->maleOption->setChecked(d->isMaleVoice());
    widget->voiceFileURL->setURL(d->getVoiceFilename());

    if (dialog->exec() == QDialog::Accepted) {
        d->setVoice (widget->voiceFileURL->url(), widget->maleOption->isChecked());
        d->setDefaultEncodingFromVoice();
        emit changed(true);
    }

    delete dialog;
}

void HadifixConf::voiceCombo_activated(int /*index*/)
{
    d->setDefaultEncodingFromVoice();
}

void HadifixConf::testButton_clicked () {
    // If currently synthesizing, stop it.
    if (d->hadifixProc)
        d->hadifixProc->stopText();
    else
    {
        d->hadifixProc = new HadifixProc();
        connect (d->hadifixProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "hadifixplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->fileName();
    tempFile.close();

    // Tell user to wait.
    d->progressDlg = new KProgressDialog(d, 
        i18n("Testing"),
        i18n("Testing."),
        true);
    d->progressDlg->progressBar()->hide();
    d->progressDlg->setAllowCancel(true);

    // Speak a German sentence as hadifix is a German tts
    // TODO: Actually, Hadifix does support English (and other languages?) as well,
    // If you install the right voice files.  The hard part is finding and installing 
    // a working txt2pho for the desired language.  There seem to be some primitive french,
    // italian, and a few others, written in perl, but they have many issues.
    // Go to the mbrola website and click on "TTS" to learn more.

    // QString testMsg = "K D E ist eine moderne grafische Arbeitsumgebung für UNIX-Computer.";
    QString testMsg = testMessage(d->languageCode);
    connect (d->hadifixProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    d->hadifixProc->synth (testMsg,
        realFilePath(d->hadifixURL->url()),
        d->isMaleVoice(),
        realFilePath(d->mbrolaURL->url()),
        d->getVoiceFilename(),
        d->volumeBox->value(),
        d->timeBox->value(),
        d->frequencyBox->value(),
        PlugInProc::codecIndexToCodec(d->characterCodingBox->currentIndex(), d->codecList),
        tmpWaveFile);

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    d->progressDlg->exec();
    disconnect (d->hadifixProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    if (d->progressDlg->wasCancelled()) d->hadifixProc->stopText();
    delete d->progressDlg;
    d->progressDlg = 0;
}

void HadifixConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!d->progressDlg)
    {
        d->hadifixProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    d->progressDlg->showCancelButton(false);
    // Get new wavefile name.
    d->waveFile = d->hadifixProc->getFilename();
    // Tell synth we're done.
    d->hadifixProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(d->waveFile);
    QFile::remove(d->waveFile);
    d->waveFile.clear();
    if (d->progressDlg) d->progressDlg->close();
}

void HadifixConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = d->hadifixProc->getFilename();
    // kDebug() << "HadifixConf::slotSynthStopped: filename = " << filename << endl;
    if (!filename.isNull()) QFile::remove(filename);
}

