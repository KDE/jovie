#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>

#include <pluginconf.h>

#include "freettsconf.h"
#include "freettsconf.moc"

/** Constructor */
FreeTTSConf::FreeTTSConf( QWidget* parent, const char* name, const QStringList &args) : 
  FreeTTSConfigWidget( parent, name ){
  kdDebug() << "Running: FreeTTSConf::FreeTTSConf( QWidget* parent, const char* name, const QStringList &args)" << endl;
}

/** Desctructor */
FreeTTSConf::~FreeTTSConf(){
  kdDebug() << "Running: FreeTTSConf::~FreeTTSConf()" << endl;
}

void FreeTTSConf::load(KConfig *config){
  kdDebug() << "Running: FreeTTSConf::load()" << endl;
}

void FreeTTSConf::save(KConfig *config){
  kdDebug() << "Running: FreeTTSConf::save()" << endl;
}

void FreeTTSConf::defaults(){
  kdDebug() << "Running: FreeTTSConf::defaults()" << endl;
}
