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
 
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h> 

#include <qfile.h>

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

#include <pluginconf.h>

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

      void load (KConfig *config, const QString &langGroup) {
         config->setGroup(langGroup);
         
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

      void save (KConfig *config, const QString &langGroup) {
         config->setGroup(langGroup);
         config->writeEntry ("hadifixExec",configWidget->hadifixURL->url());
         config->writeEntry ("mbrolaExec", configWidget->mbrolaURL->url());
         config->writeEntry ("voice",      configWidget->getVoiceFilename());
         config->writeEntry ("gender",     configWidget->isMaleVoice());
         config->writeEntry ("volume",     configWidget->volumeBox->value());
         config->writeEntry ("time",       configWidget->timeBox->value());
         config->writeEntry ("pitch",      configWidget->frequencyBox->value());
      }
      
      HadifixConfigUI *configWidget;
      
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
};

/** Constructor */
HadifixConf::HadifixConf( QWidget* parent, const char* name, const QStringList &) : 
   PlugInConf( parent, name ){
   kdDebug() << "HadifixConf::HadifixConf: Running" << endl;
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
   kdDebug() << "HadifixConf::~HadifixConf: Running" << endl;
   delete d;
}

void HadifixConf::load(KConfig *config, const QString &langGroup) {
   kdDebug() << "HadifixConf::load: Running" << endl;
   d->load (config, langGroup);
}

void HadifixConf::save(KConfig *config, const QString &langGroup) {
   kdDebug() << "HadifixConf::save: Running" << endl;
   d->save (config, langGroup);
}

void HadifixConf::defaults() {
   kdDebug() << "HadifixConf::defaults: Running" << endl;
   d->setDefaults();
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
      connect (d->hadifixProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
   }
   // Create a temp file name for the wave file.
   KTempFile tempFile (locateLocal("tmp", "hadifixplugin-"), ".wav");
   QString tmpWaveFile = tempFile.file()->name();
   tempFile.close();
   // Speak a German sentence as hadifix is a German tts
   // TODO: Actually, Hadifix does support English (and other languages?) as well.
   d->hadifixProc->synth ("KDE ist eine moderne grafische Arbeitsumgebung für Unix-Computer.",
                         d->configWidget->hadifixURL->url(),
                         d->configWidget->isMaleVoice(),
                         d->configWidget->mbrolaURL->url(),
                         d->configWidget->getVoiceFilename(),
                         d->configWidget->volumeBox->value(),
                         d->configWidget->timeBox->value(),
                         d->configWidget->frequencyBox->value(),
                         tmpWaveFile);
}

void HadifixConf::slotSynthFinished()
{
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
}
