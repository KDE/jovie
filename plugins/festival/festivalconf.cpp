/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalconf.cpp
  Configuration widget and functions for Festival plug in
  ------------------- 
  Copyright : (C) 2002 by Jos� Pablo Ezequiel "Pupeno" Fern�ndez
              (C) 2003 by Jos� Pablo Ezequiel "Pupeno" Fern�ndez
  -------------------
  Original author: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org>
  Current Maintainer: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
 
 // $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qdir.h> 

#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <pluginconf.h>

#include "festivalconf.h"
#include "festivalconf.moc"

/** Constructor */
FestivalConf::FestivalConf( QWidget* parent, const char* name, const QStringList &args) :
   FestivalConfWidget( parent, name ){
   kdDebug() << "Running: FestivalConf::FestivalConf( QWidget* parent, const char* name, const QStringList &args)" << endl;
   festivalVoicesPath->setMode(KFile::Directory);
}

/** Desctructor */
FestivalConf::~FestivalConf(){
   kdDebug() << "Running: FestivalConf::~FestivalConf()" << endl;
}

void FestivalConf::load(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: FestivalConf::load(KConfig *config, const QString &langGroup)" << endl;
   kdDebug() << "Loading configuration for language " << langGroup << " with plug in " << "Festival" << endl;

   config->setGroup(langGroup);
   this->festivalVoicesPath->setURL(config->readPathEntry("VoicesPath"));
   this->forceArts->setChecked(config->readBoolEntry("Arts"));
   scanVoices();
   QString voiceSelected(config->readEntry("Voice"));
   for(int index = 0 ; index < voiceList.count(); ++index){
      kdDebug() << "Testing: " << voiceSelected << " == " << voiceList[index].code << endl;
      if(voiceSelected == voiceList[index].code){
         kdDebug() << "Match!" << endl;
         this->selectVoiceCombo->setCurrentItem(index);  
         break;
      }
   }
}

void FestivalConf::save(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: FestivalConf::save(KConfig *config, const QString &langGroup)" << endl;
   kdDebug() << "Saving configuration for language " << langGroup << " with plug in " << "Festival" << endl;

   config->setGroup(langGroup);
   config->writePathEntry("VoicesPath", this->festivalVoicesPath->url());
   config->writeEntry("Arts", this->forceArts->isChecked());
   config->writeEntry("Voice", voiceList[this->selectVoiceCombo->currentItem()].code);
}

void FestivalConf::defaults(){
   kdDebug() << "Running: FestivalConf::defaults()" << endl;
}

void FestivalConf::scanVoices(){
   kdDebug() << "Running: FestivalConf::scanVoices()" << endl;
   voiceList.clear();
   selectVoiceCombo->clear();
   KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/proklam/festival/voices", true, false);
   QStringList groupList = voices.groupList();
   QDir mainPath(this->festivalVoicesPath->url());
   voice voiceTemp;
   for(QStringList::Iterator it = groupList.begin(); it != groupList.end(); ++it ){
      voices.setGroup(*it);
      voiceTemp.path = voices.readEntry("Path");
      mainPath.setPath(this->festivalVoicesPath->url() + voiceTemp.path);
      if(!mainPath.exists()){
         kdDebug() << "For " << *it << " the path " << this->festivalVoicesPath->url() + voiceTemp.path << " doesn't exist" << endl;
         continue;
      } else {
         kdDebug() << "For " << *it << " the path " << this->festivalVoicesPath->url() + voiceTemp.path << " exist" << endl;
      }
      voiceTemp.code = *it;
      voiceTemp.name = voices.readEntry("Name");
      voiceTemp.comment = voices.readEntry("Comment");
      voiceList.append(voiceTemp);
      selectVoiceCombo->insertItem(voiceTemp.name + " (" + voiceTemp.comment + ")");
   }
}

