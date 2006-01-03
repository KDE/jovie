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

// Qt includes. 
#include <qlayout.h>
#include <qlabel.h>
#include <q3groupbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
//Added by qt3to4:
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
#include <kdialogbase.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kprogress.h>

// KTTS includes.
#include <pluginconf.h>
#include <testplayer.h>
#include <talkercode.h>

// Hadifix includes.
#include "hadifixproc.h"
#include "voicefileui.h"
#include "hadifixconfigui.h"
#include "hadifixconf.h"
#include "hadifixconf.moc"

class HadifixConfPrivate {
   friend class HadifixConf;
   private:
      HadifixConfPrivate() {
         hadifixProc = 0;
         progressDlg = 0;
         findInitialConfig();
      };
      
      ~HadifixConfPrivate() {
         if (hadifixProc) hadifixProc->stopText();
         delete hadifixProc;
         if (!waveFile.isNull()) QFile::remove(waveFile);
         delete progressDlg;
      };
      
      #include "initialconfig.h"

      void setConfiguration (QString hadifixExec,  QString mbrolaExec,
                             QString voice,        bool male,
                             int volume, int time, int pitch,
                             QString codecName)
      {
         configWidget->hadifixURL->setURL (hadifixExec);
         configWidget->mbrolaURL->setURL (mbrolaExec);
         configWidget->setVoice (voice, male);
         
         configWidget->volumeBox->setValue (volume);
         configWidget->timeBox->setValue (time);
         configWidget->frequencyBox->setValue (pitch);
         int codec = PlugInProc::codecNameToListIndex(codecName, codecList);
         configWidget->characterCodingBox->setCurrentItem(codec);
      }

      void initializeVoices () {
         QStringList::iterator it;
         for (it = defaultVoices.begin(); it != defaultVoices.end(); ++it) {
            HadifixProc::VoiceGender gender;
            QString name = QFileInfo(*it).fileName();
            gender = HadifixProc::determineGender(defaultMbrolaExec, *it);
            if (gender == HadifixProc::MaleGender)
                configWidget->addVoice(*it, true, i18n("Male voice \"%1\"").arg(name));
            else if (gender == HadifixProc::FemaleGender)
                configWidget->addVoice(*it, false, i18n("Female voice \"%1\"").arg(name));
            else {
               if (name == "de1")
                   configWidget->addVoice(*it, false, i18n("Female voice \"%1\"").arg(name));
               else {
                   configWidget->addVoice(*it, true,  i18n("Unknown voice \"%1\"").arg(name));
                   configWidget->addVoice(*it, false, i18n("Unknown voice \"%1\"").arg(name));
               }
            }
         }
      };

      void initializeCharacterCodes() {
         // Build codec list and fill combobox.
         codecList = PlugInProc::buildCodecList();
         configWidget->characterCodingBox->clear();
         configWidget->characterCodingBox->insertStringList(codecList);
      }

      void setDefaultEncodingFromVoice() {
         QString voiceFile = configWidget->getVoiceFilename();
         QString voiceCode = QFileInfo(voiceFile).baseName(false);
         voiceCode = voiceCode.left(2);
         QString codecName = "Local";
         if (voiceCode == "de") codecName = "ISO 8859-1";
         if (voiceCode == "hu") codecName = "ISO 8859-2";
         configWidget->characterCodingBox->setCurrentItem(PlugInProc::codecNameToListIndex(
            codecName, codecList));
}

      void setDefaults () {
         QStringList::iterator it = defaultVoices.begin();
         // Find a voice that matches language code, if any.
         if (!languageCode.isEmpty())
         {
            QString justLang = languageCode.left(2);
            for (;it != defaultVoices.end();++it)
            {
               QString voiceCode = QFileInfo(*it).baseName(false).left(2);
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

      void load (KConfig *config, const QString &configGroup) {
         config->setGroup(configGroup);
         
         QString voice = config->readEntry("voice", configWidget->getVoiceFilename());
         
         HadifixProc::VoiceGender gender;
         gender = HadifixProc::determineGender(defaultMbrolaExec, voice);
         bool isMale = (gender == HadifixProc::MaleGender);

         QString defaultCodecName = "Local";
         // TODO: Need a better way to determine proper codec for each voice.
         // This will do for now.
         QString voiceCode = QFileInfo(voice).baseName(false);
         if (voiceCode.left(2) == "de") defaultCodecName = "ISO 8859-1";
         if (voiceCode.left(2) == "hu") defaultCodecName = "ISO 8859-2";
         
         setConfiguration (
               config->readEntry ("hadifixExec",defaultHadifixExec),
               config->readEntry ("mbrolaExec", defaultMbrolaExec),
               config->readEntry ("voice",      voice),
               config->readEntry("gender", QVariant(isMale)).toBool(),
               config->readNumEntry ("volume",  100),
               config->readNumEntry ("time",    100),
               config->readNumEntry ("pitch",   100),
               config->readEntry ("codec",      defaultCodecName)
         );
      };

      void save (KConfig *config, const QString &configGroup) {
         config->setGroup(configGroup);
         config->writeEntry ("hadifixExec", PlugInConf::realFilePath(configWidget->hadifixURL->url()));
         config->writeEntry ("mbrolaExec", PlugInConf::realFilePath(configWidget->mbrolaURL->url()));
         config->writeEntry ("voice",      configWidget->getVoiceFilename());
         config->writeEntry ("gender",     configWidget->isMaleVoice());
         config->writeEntry ("volume",     configWidget->volumeBox->value());
         config->writeEntry ("time",       configWidget->timeBox->value());
         config->writeEntry ("pitch",      configWidget->frequencyBox->value());
         config->writeEntry ("codec",      PlugInProc::codecIndexToCodecName(
                                              configWidget->characterCodingBox->currentItem(), codecList));
      }

      HadifixConfigUI *configWidget;

      QString languageCode;
      QString defaultHadifixExec;
      QString defaultMbrolaExec;
      QStringList defaultVoices;
      QStringList codecList;

      // Wave file playing on play object.
      QString waveFile;
      // Synthesizer.
      HadifixProc* hadifixProc;
      // Progress Dialog.
      KProgressDialog* progressDlg;
};

/** Constructor */
HadifixConf::HadifixConf( QWidget* parent, const char* name, const QStringList &) : 
   PlugInConf( parent, name ){
   // kdDebug() << "HadifixConf::HadifixConf: Running" << endl;
   QVBoxLayout *layout = new QVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint(), "CommandConfigWidgetLayout");
   layout->setAlignment (Qt::AlignTop); 

   d = new HadifixConfPrivate();
   d->configWidget = new HadifixConfigUI (this, "configWidget");
   
   QString file = locate("data", "LICENSES/LGPL_V2");
   i18n("This plugin is distributed under the terms of the GPL v2 or later.");
   
   connect(d->configWidget->voiceButton, SIGNAL(clicked()), this, SLOT(voiceButton_clicked()));
   connect(d->configWidget->testButton, SIGNAL(clicked()), this, SLOT(testButton_clicked()));
   connect(d->configWidget, SIGNAL(changed(bool)), this, SLOT(configChanged (bool)));
   connect(d->configWidget->characterCodingBox, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
   connect(d->configWidget->voiceCombo, SIGNAL(activated(int)), this, SLOT(voiceCombo_activated(int)));
   d->initializeCharacterCodes();
   d->initializeVoices();
   d->setDefaults();
   layout->addWidget (d->configWidget);
}

/** Destructor */
HadifixConf::~HadifixConf(){
   // kdDebug() << "HadifixConf::~HadifixConf: Running" << endl;
   delete d;
}

void HadifixConf::load(KConfig *config, const QString &configGroup) {
   // kdDebug() << "HadifixConf::load: Running" << endl;
   d->setDefaults();
   d->load (config, configGroup);
}

void HadifixConf::save(KConfig *config, const QString &configGroup) {
   // kdDebug() << "HadifixConf::save: Running" << endl;
   d->save (config, configGroup);
}

void HadifixConf::defaults() {
   // kdDebug() << "HadifixConf::defaults: Running" << endl;
   d->setDefaults();
}

void HadifixConf::setDesiredLanguage(const QString &lang)
{
    d->languageCode = lang;
}

QString HadifixConf::getTalkerCode()
{
    if (!d->configWidget->hadifixURL->url().isEmpty() && !d->configWidget->mbrolaURL->url().isEmpty())
    {
        QString voiceFile = d->configWidget->getVoiceFilename();
        if (QFileInfo(voiceFile).exists())
        {
            // mbrola voice file names usually start with two-letter language code,
            // but this is by no means guaranteed.
            QString voiceCode = QFileInfo(voiceFile).baseName(false);
            QString voiceLangCode = voiceCode.left(2);
            if (d->languageCode.left(2) != voiceLangCode)
            {
               // Verify that first two letters of voice filename are a valid language code.
               // If they are, switch to that language.
               if (!TalkerCode::languageCodeToLanguage(voiceLangCode).isEmpty())
                   d->languageCode = voiceLangCode;
            }
            QString gender = "male";
            if (!d->configWidget->isMaleVoice()) gender = "female";
            QString volume = "medium";
            if (d->configWidget->volumeBox->value() < 75) volume = "soft";
            if (d->configWidget->volumeBox->value() > 125) volume = "loud";
            QString rate = "medium";
            if (d->configWidget->timeBox->value() < 75) rate = "slow";
            if (d->configWidget->timeBox->value() > 125) rate = "fast";
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
   KDialogBase *dialog = new KDialogBase (this, 0, true,
                                      i18n("Voice File - Hadifix Plugin"),
                                      KDialogBase::Ok|KDialogBase::Cancel,
                                      KDialogBase::Ok, true);
   VoiceFileWidget *widget = new VoiceFileWidget(dialog);
   dialog->setMainWidget (widget);

   widget->femaleOption->setChecked(!d->configWidget->isMaleVoice());
   widget->maleOption->setChecked(d->configWidget->isMaleVoice());
   widget->voiceFileURL->setURL(d->configWidget->getVoiceFilename());
   widget->mbrola = d->defaultMbrolaExec;

   if (dialog->exec() == QDialog::Accepted) {
      d->configWidget->setVoice (widget->voiceFileURL->url(),
                                 widget->maleOption->isChecked());
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
   QString tmpWaveFile = tempFile.file()->name();
   tempFile.close();

    // Tell user to wait.
   d->progressDlg = new KProgressDialog(d->configWidget, "ktts_hadifix_testdlg",
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
      realFilePath(d->configWidget->hadifixURL->url()),
      d->configWidget->isMaleVoice(),
      realFilePath(d->configWidget->mbrolaURL->url()),
      d->configWidget->getVoiceFilename(),
      d->configWidget->volumeBox->value(),
      d->configWidget->timeBox->value(),
      d->configWidget->frequencyBox->value(),
      PlugInProc::codecIndexToCodec(d->configWidget->characterCodingBox->currentItem(), d->codecList),
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
    // kdDebug() << "HadifixConf::slotSynthStopped: filename = " << filename << endl;
    if (!filename.isNull()) QFile::remove(filename);
}
