/***************************************************************************
                          hadifixconf.cpp  - description
                             -------------------
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
#include <qgroupbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>

// KDE includes.
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
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
         artsServer = 0;
         playObj = 0;
         progressDlg = 0;
         findInitialConfig();
      };
      
      ~HadifixConfPrivate() {
         if (hadifixProc) hadifixProc->stopText();
         delete hadifixProc;
         if (playObj)
         {
            playObj->halt();
            if (!waveFile.isNull()) QFile::remove(waveFile);
         }
         delete playObj;
         delete artsServer;
         delete progressDlg;
      };
      
      #include "initialconfig.h"

      void setConfiguration (QString hadifixExec,  QString mbrolaExec,
                             QString voice,        bool male,
                             int volume, int time, int pitch)
      {
         configWidget->hadifixURL->setURL (hadifixExec);
         configWidget->mbrolaURL->setURL (mbrolaExec);
         configWidget->setVoice (voice, male);
         
         configWidget->volumeBox->setValue (volume);
         configWidget->timeBox->setValue (time);
         configWidget->frequencyBox->setValue (pitch);
      }

      void initializeVoices () {
         QStringList::iterator it;
         for (it = defaultVoices.begin(); it != defaultVoices.end(); ++it) {
            HadifixProc::VoiceGender gender;
            QString name = QFileInfo(*it).fileName();
            gender = HadifixProc::determineGender(defaultMbrolaExec, *it);
            if (gender == HadifixProc::MaleGender)
               configWidget->addVoice(*it, true, i18n("Male voice \""+name+"\""));
            else if (gender == HadifixProc::FemaleGender)
               configWidget->addVoice(*it, false, i18n("Female voice \""+name+"\""));
            else {
               if (name == "de1")
                  configWidget->addVoice(*it, false, i18n("Female voice \""+name+"\""));
               else {
                  configWidget->addVoice(*it, true,  i18n("Unknown voice \""+name+"\""));
                  configWidget->addVoice(*it, false, i18n("Unknown voice \""+name+"\""));
               }
            }
         }
      };

      void setDefaults () {
         QStringList::iterator it = defaultVoices.begin();
         HadifixProc::VoiceGender gender;
         gender = HadifixProc::determineGender(defaultMbrolaExec, *it);
         
         setConfiguration (defaultHadifixExec, defaultMbrolaExec,
                           *it, gender == HadifixProc::MaleGender,
                           100, 100, 100);
      };

      void load (KConfig *config, const QString &configGroup) {
         config->setGroup(configGroup);
         
         QStringList::iterator it = defaultVoices.begin();
         QString voice = config->readEntry("voice", *it);
         
         HadifixProc::VoiceGender gender;
         gender = HadifixProc::determineGender(defaultMbrolaExec, voice);
         bool isMale = (gender == HadifixProc::MaleGender);
         
         setConfiguration (
               config->readEntry ("hadifixExec",defaultHadifixExec),
               config->readEntry ("mbrolaExec", defaultMbrolaExec),
               config->readEntry ("voice",      voice),
               config->readBoolEntry("gender",  isMale),
               config->readNumEntry ("volume",  100),
               config->readNumEntry ("time",    100),
               config->readNumEntry ("pitch",   100)
         );
      };

      void save (KConfig *config, const QString &configGroup) {
         config->setGroup(configGroup);
         config->writeEntry ("hadifixExec", KStandardDirs::realFilePath(configWidget->hadifixURL->url()));
         config->writeEntry ("mbrolaExec", KStandardDirs::realFilePath(configWidget->mbrolaURL->url()));
         config->writeEntry ("voice",      configWidget->getVoiceFilename());
         config->writeEntry ("gender",     configWidget->isMaleVoice());
         config->writeEntry ("volume",     configWidget->volumeBox->value());
         config->writeEntry ("time",       configWidget->timeBox->value());
         config->writeEntry ("pitch",      configWidget->frequencyBox->value());
      }

      HadifixConfigUI *configWidget;

      QString languageCode;
      QString defaultHadifixExec;
      QString defaultMbrolaExec;
      QStringList defaultVoices;

      // Wave file playing on play object.
      QString waveFile;
      // Synthesizer.
      HadifixProc* hadifixProc;
      // aRts server object.
      KArtsServer* artsServer;
      // aRts play object.
      KDE::PlayObject* playObj;
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
   i18n("This plug in is distributed under the terms of the LGPL v2.");
   
   connect(d->configWidget->voiceButton, SIGNAL(clicked()), this, SLOT(voiceButton_clicked()));
   connect(d->configWidget->testButton, SIGNAL(clicked()), this, SLOT(testButton_clicked()));
   connect(d->configWidget, SIGNAL(changed(bool)), this, SLOT(configChanged (bool)));
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
                    .arg(QFileInfo(voiceFile).baseName(false))
                    .arg(gender)
                    .arg(volume)
                    .arg(rate)
                    .arg("Hadifix");
        }
    }
    return QString::null;
}

void HadifixConf::voiceButton_clicked () {
   KDialogBase *dialog = new KDialogBase (this, 0, true,
                                      i18n("Voice file - Hadifix plug in"),
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
      emit changed(true);
   }
   
   delete dialog;
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
   // Go to the mbrola website and click on "TTS" to learn more.  Anyway, if someone
   // gets those other languages working then we would need code something like this
   // (but this code is *wrong*.)
   //    KLocale* locale = KGlobal::locale();
   //    QString oldLangCode = locale->language();
   //    locale->setLanguage(d->languageCode);
   //    QString msg = locale->translate("K D E is a modern graphical desktop for Unix computers.");
   //    locale->setLanguage(oldLangCode);

   connect (d->hadifixProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
   d->hadifixProc->synth ("K D E ist eine moderne grafische Arbeitsumgebung für Unix-Computer.",
      KStandardDirs::realFilePath(d->configWidget->hadifixURL->url()),
      d->configWidget->isMaleVoice(),
      KStandardDirs::realFilePath(d->configWidget->mbrolaURL->url()),
      d->configWidget->getVoiceFilename(),
      d->configWidget->volumeBox->value(),
      d->configWidget->timeBox->value(),
      d->configWidget->frequencyBox->value(),
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
    // If currently playing (or finished playing), stop and delete play object.
    if (d->playObj)
    {
       d->playObj->halt();
       // Clean up.
       QFile::remove(d->waveFile);
    }
    delete d->playObj;
    delete d->artsServer;
    // Get new wavefile name.
    d->waveFile = d->hadifixProc->getFilename();
    d->hadifixProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    d->artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (d->artsServer->server());
    d->playObj = factory.createPlayObject (d->waveFile, true);
    d->playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let HadifixConfPrivate destructor take care of
    // cleaning up the play object.  However, because we've been called from DCOP,
    // this seems to be necessary.  The call to processEvents is problematic because
    // it can cause re-entrancy.
    while (d->playObj->state() == Arts::posPlaying) qApp->processEvents();
    d->playObj->halt();
    delete d->playObj;
    d->playObj = 0;
    delete d->artsServer;
    d->artsServer = 0;
    QFile::remove(d->waveFile);
    d->waveFile = QString::null;
    if (d->progressDlg) d->progressDlg->close();
}

void HadifixConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = d->hadifixProc->getFilename();
    // kdDebug() << "HadifixConf::slotSynthStopped: filename = " << filename << endl;
    if (!filename.isNull()) QFile::remove(filename);
}
