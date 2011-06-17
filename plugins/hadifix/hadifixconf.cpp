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

// TQt includes. 
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqgroupbox.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqfile.h>

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
         if (!waveFile.isNull()) TQFile::remove(waveFile);
         delete progressDlg;
      };
      
      #include "initialconfig.h"

      void setConfiguration (TQString hadifixExec,  TQString mbrolaExec,
                             TQString voice,        bool male,
                             int volume, int time, int pitch,
                             TQString codecName)
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
         TQStringList::iterator it;
         for (it = defaultVoices.begin(); it != defaultVoices.end(); ++it) {
            HadifixProc::VoiceGender gender;
            TQString name = TQFileInfo(*it).fileName();
            gender = HadifixProc::determineGender(defaultMbrolaExec, *it);
            if (gender == HadifixProc::MaleGender)
                configWidget->addVoice(*it, true, i18n("Male voice \"%1\"").tqarg(name));
            else if (gender == HadifixProc::FemaleGender)
                configWidget->addVoice(*it, false, i18n("Female voice \"%1\"").tqarg(name));
            else {
               if (name == "de1")
                   configWidget->addVoice(*it, false, i18n("Female voice \"%1\"").tqarg(name));
               else {
                   configWidget->addVoice(*it, true,  i18n("Unknown voice \"%1\"").tqarg(name));
                   configWidget->addVoice(*it, false, i18n("Unknown voice \"%1\"").tqarg(name));
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
         TQString voiceFile = configWidget->getVoiceFilename();
         TQString voiceCode = TQFileInfo(voiceFile).baseName(false);
         voiceCode = voiceCode.left(2);
         TQString codecName = "Local";
         if (voiceCode == "de") codecName = "ISO 8859-1";
         if (voiceCode == "hu") codecName = "ISO 8859-2";
         configWidget->characterCodingBox->setCurrentItem(PlugInProc::codecNameToListIndex(
            codecName, codecList));
}

      void setDefaults () {
         TQStringList::iterator it = defaultVoices.begin();
         // Find a voice that matches language code, if any.
         if (!languageCode.isEmpty())
         {
            TQString justLang = languageCode.left(2);
            for (;it != defaultVoices.end();++it)
            {
               TQString voiceCode = TQFileInfo(*it).baseName(false).left(2);
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

      void load (KConfig *config, const TQString &configGroup) {
         config->setGroup(configGroup);
         
         TQString voice = config->readEntry("voice", configWidget->getVoiceFilename());
         
         HadifixProc::VoiceGender gender;
         gender = HadifixProc::determineGender(defaultMbrolaExec, voice);
         bool isMale = (gender == HadifixProc::MaleGender);

         TQString defaultCodecName = "Local";
         // TODO: Need a better way to determine proper codec for each voice.
         // This will do for now.
         TQString voiceCode = TQFileInfo(voice).baseName(false);
         if (voiceCode.left(2) == "de") defaultCodecName = "ISO 8859-1";
         if (voiceCode.left(2) == "hu") defaultCodecName = "ISO 8859-2";
         
         setConfiguration (
               config->readEntry ("hadifixExec",defaultHadifixExec),
               config->readEntry ("mbrolaExec", defaultMbrolaExec),
               config->readEntry ("voice",      voice),
               config->readBoolEntry("gender",  isMale),
               config->readNumEntry ("volume",  100),
               config->readNumEntry ("time",    100),
               config->readNumEntry ("pitch",   100),
               config->readEntry ("codec",      defaultCodecName)
         );
      };

      void save (KConfig *config, const TQString &configGroup) {
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

      TQString languageCode;
      TQString defaultHadifixExec;
      TQString defaultMbrolaExec;
      TQStringList defaultVoices;
      TQStringList codecList;

      // Wave file playing on play object.
      TQString waveFile;
      // Synthesizer.
      HadifixProc* hadifixProc;
      // Progress Dialog.
      KProgressDialog* progressDlg;
};

/** Constructor */
HadifixConf::HadifixConf( TQWidget* tqparent, const char* name, const TQStringList &) : 
   PlugInConf( tqparent, name ){
   // kdDebug() << "HadifixConf::HadifixConf: Running" << endl;
   TQVBoxLayout *tqlayout = new TQVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint(), "CommandConfigWidgetLayout");
   tqlayout->tqsetAlignment (TQt::AlignTop); 

   d = new HadifixConfPrivate();
   d->configWidget = new HadifixConfigUI (this, "configWidget");
   
   TQString file = locate("data", "LICENSES/LGPL_V2");
   i18n("This plugin is distributed under the terms of the GPL v2 or later.");
   
   connect(d->configWidget->voiceButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(voiceButton_clicked()));
   connect(d->configWidget->testButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(testButton_clicked()));
   connect(d->configWidget, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(configChanged (bool)));
   connect(d->configWidget->characterCodingBox, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
   connect(d->configWidget->voiceCombo, TQT_SIGNAL(activated(int)), this, TQT_SLOT(voiceCombo_activated(int)));
   d->initializeCharacterCodes();
   d->initializeVoices();
   d->setDefaults();
   tqlayout->addWidget (d->configWidget);
}

/** Destructor */
HadifixConf::~HadifixConf(){
   // kdDebug() << "HadifixConf::~HadifixConf: Running" << endl;
   delete d;
}

void HadifixConf::load(KConfig *config, const TQString &configGroup) {
   // kdDebug() << "HadifixConf::load: Running" << endl;
   d->setDefaults();
   d->load (config, configGroup);
}

void HadifixConf::save(KConfig *config, const TQString &configGroup) {
   // kdDebug() << "HadifixConf::save: Running" << endl;
   d->save (config, configGroup);
}

void HadifixConf::defaults() {
   // kdDebug() << "HadifixConf::defaults: Running" << endl;
   d->setDefaults();
}

void HadifixConf::setDesiredLanguage(const TQString &lang)
{
    d->languageCode = lang;
}

TQString HadifixConf::getTalkerCode()
{
    if (!d->configWidget->hadifixURL->url().isEmpty() && !d->configWidget->mbrolaURL->url().isEmpty())
    {
        TQString voiceFile = d->configWidget->getVoiceFilename();
        if (TQFileInfo(voiceFile).exists())
        {
            // mbrola voice file names usually start with two-letter language code,
            // but this is by no means guaranteed.
            TQString voiceCode = TQFileInfo(voiceFile).baseName(false);
            TQString voiceLangCode = voiceCode.left(2);
            if (d->languageCode.left(2) != voiceLangCode)
            {
               // Verify that first two letters of voice filename are a valid language code.
               // If they are, switch to that language.
               if (!TalkerCode::languageCodeToLanguage(voiceLangCode).isEmpty())
                   d->languageCode = voiceLangCode;
            }
            TQString gender = "male";
            if (!d->configWidget->isMaleVoice()) gender = "female";
            TQString volume = "medium";
            if (d->configWidget->volumeBox->value() < 75) volume = "soft";
            if (d->configWidget->volumeBox->value() > 125) volume = "loud";
            TQString rate = "medium";
            if (d->configWidget->timeBox->value() < 75) rate = "slow";
            if (d->configWidget->timeBox->value() > 125) rate = "fast";
            return TQString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .tqarg(d->languageCode)
                    .tqarg(voiceCode)
                    .tqarg(gender)
                    .tqarg(volume)
                    .tqarg(rate)
                    .tqarg("Hadifix");
        }
    }
    return TQString();
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

   if (dialog->exec() == TQDialog::Accepted) {
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
      connect (d->hadifixProc, TQT_SIGNAL(stopped()), this, TQT_SLOT(slotSynthStopped()));
   }
   // Create a temp file name for the wave file.
   KTempFile tempFile (locateLocal("tmp", "hadifixplugin-"), ".wav");
   TQString tmpWaveFile = tempFile.file()->name();
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

   // TQString testMsg = "K D E ist eine moderne grafische Arbeitsumgebung für UNIX-Computer.";
   TQString testMsg = testMessage(d->languageCode);
   connect (d->hadifixProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
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
   disconnect (d->hadifixProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
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
    TQFile::remove(d->waveFile);
    d->waveFile = TQString();
    if (d->progressDlg) d->progressDlg->close();
}

void HadifixConf::slotSynthStopped()
{
    // Clean up after canceling test.
    TQString filename = d->hadifixProc->getFilename();
    // kdDebug() << "HadifixConf::slotSynthStopped: filename = " << filename << endl;
    if (!filename.isNull()) TQFile::remove(filename);
}
