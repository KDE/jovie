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
#include <klineedit.h>
#include <knuminput.h>
#include <kpassdlg.h>

#include <pluginconf.h>

#include "festivalcsconf.h"
#include "festivalcsconf.moc"

/** Constructor */
FestivalCSConf::FestivalCSConf( QWidget* parent, const char* name, const QStringList &args) : 
  FestivalCSConfigWidget( parent, name ){
  kdDebug() << "Running: FestivalCSConf::FestivalCSConf( QWidget* parent, const char* name, const QStringList &args)" << endl;
}

/** Desctructor */
FestivalCSConf::~FestivalCSConf(){
  kdDebug() << "Running: FestivalCSConf::~FestivalCSConf()" << endl;
}

void FestivalCSConf::load(KConfig *config, const QString &langGroup){
  kdDebug() << "Running: FestivalCSConf::load(KConfig *config, const QString &langGroup)" << endl;
  kdDebug() << "Loading configuration for language " << langGroup << " with plug in " << "FestivalCS" << endl;
  config->setGroup(langGroup);

  host->setText(config->readEntry("Host"));
  portClient->setValue(config->readNumEntry("PortClient"));
  passwordClient->setText(config->readEntry("PasswordClient"));
  runServer->setChecked(config->readBoolEntry("RunServer"));
  festivalBin->setURL(config->readEntry("FestivalBin"));
  logFile->setURL(config->readEntry("LogFile"));
  passwordServer->setText(config->readEntry("PasswordServer"));
  portServer->setValue(config->readNumEntry("PortServer"));
  acessList->setText(config->readEntry("AcessList"));
  denyList->setText(config->readEntry("DenyList"));
  limitFunctions->setChecked(config->readBoolEntry("LimitFunctions"));
}

void FestivalCSConf::save(KConfig *config, const QString &langGroup){
  kdDebug() << "Running: FestivalCSConf::save(KConfig *config, const QString &langGroup)" << endl;
  kdDebug() << "Saving configuration for language " << langGroup << " with plug in " << "FestivalCS" << endl;
  config->setGroup(langGroup);
  config->writeEntry("Host", host->text());
  config->writeEntry("PortClient", portClient->value());
  config->writeEntry("PasswordClient", passwordClient->text());
  config->writeEntry("RunServer", runServer->isChecked());
  config->writeEntry("FestivalBin", festivalBin->url());
  config->writeEntry("LogFile", logFile->url());
  config->writeEntry("PasswordServer", passwordServer->text());
  config->writeEntry("PortServer", portServer->value());
  config->writeEntry("AcessList", acessList->text());
  config->writeEntry("DenyList", denyList->text());
  config->writeEntry("LimitFunctions", limitFunctions->isChecked());
}

void FestivalCSConf::defaults(){
  kdDebug() << "Running: FestivalCSConf::defaults()" << endl;
}
