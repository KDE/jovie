#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

#include "freettsproc.h"
#include "freettsproc.moc"

/** Constructor */
FreeTTSProc::FreeTTSProc( QObject* parent, const char* name, const QStringList &args) : 
  PlugInProc( parent, name ){
  kdDebug() << "Running: FreeTTSProc::FreeTTSProc( QObject* parent, const char* name, const QStringList &args)" << endl;
}

/** Desctructor */
FreeTTSProc::~FreeTTSProc(){
  kdDebug() << "Running: FreeTTSProc::~FreeTTSProc()" << endl;
}

/** Initializate the speech */
bool FreeTTSProc::init(){
  kdDebug() << "Running: FreeTTSProc::init()" << endl;
  kdDebug() << "Initializing plug in: FreeTTS" << endl;
  return true;
}

/** Say a text
    text: The text to be speech
*/
void FreeTTSProc::sayText(const QString &text){
  kdDebug() << "Running: FreeTTSProc::sayText(const QString &text)" << endl;
  kdDebug() << "Saying text: '" << text << "' using FreeTTS plug in" << endl;
}
