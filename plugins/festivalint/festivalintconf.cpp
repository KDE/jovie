/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalintconf.cpp
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

#include "festivalintconf.h"
#include "festivalintconf.moc"

/** Constructor */
FestivalIntConf::FestivalIntConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
   FestivalIntConfWidget( parent, name ){
   kdDebug() << "Running: FestivalIntConf::FestivalIntConf( QWidget* parent, const char* name, const QStringList &args)" << endl;
   festivalVoicesPath->setMode(KFile::Directory);
}

/** Desctructor */
FestivalIntConf::~FestivalIntConf(){
   kdDebug() << "Running: FestivalIntConf::~FestivalIntConf()" << endl;
}

void FestivalIntConf::load(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: FestivalIntConf::load(KConfig *config, const QString &langGroup)" << endl;
   kdDebug() << "Loading configuration for language " << langGroup << " with plug in " << "Festival" << endl;

   config->setGroup(langGroup);
   this->festivalVoicesPath->setURL(config->readPathEntry("VoicesPath"));
   this->forceArts->setChecked(config->readBoolEntry("Arts"));
   scanVoices();
   QString voiceSelected(config->readEntry("Voice"));
   for(uint index = 0 ; index < voiceList.count(); ++index){
      kdDebug() << "Testing: " << voiceSelected << " == " << voiceList[index].code << endl;
      if(voiceSelected == voiceList[index].code){
         kdDebug() << "Match!" << endl;
         this->selectVoiceCombo->setCurrentItem(index);  
         break;
      }
   }
}

void FestivalIntConf::save(KConfig *config, const QString &langGroup){
   kdDebug() << "Running: FestivalIntConf::save(KConfig *config, const QString &langGroup)" << endl;
   kdDebug() << "Saving configuration for language " << langGroup << " with plug in " << "Festival" << endl;

   config->setGroup(langGroup);
   config->writePathEntry("VoicesPath", this->festivalVoicesPath->url());
   config->writeEntry("Arts", this->forceArts->isChecked());
   config->writeEntry("Voice", voiceList[this->selectVoiceCombo->currentItem()].code);
}

void FestivalIntConf::defaults(){
   kdDebug() << "Running: FestivalIntConf::defaults()" << endl;
}

void FestivalIntConf::scanVoices(){
   kdDebug() << "Running: FestivalIntConf::scanVoices()" << endl;
   voiceList.clear();
   selectVoiceCombo->clear();
   KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festivalint/voices", true, false);
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
         kdDebug() << "For " << *it << " the path " << this->festivalVoicesPath->url() + voiceTemp.path << " exists" << endl;
      }
      voiceTemp.code = *it;
      voiceTemp.name = voices.readEntry("Name");
      voiceTemp.comment = voices.readEntry("Comment");
      voiceList.append(voiceTemp);
      selectVoiceCombo->insertItem(voiceTemp.name + " (" + voiceTemp.comment + ")");
   }
}

