#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kconfig.h>

#include "festivalcsproc.h"
#include "festivalcsproc.moc"

#include <unistd.h>

/** Constructor */
FestivalCSProc::FestivalCSProc( QObject* parent, const char* name, const QStringList &args) : 
  PlugInProc( parent, name ){
  kdDebug() << "Running: FestivalCSProc::FestivalCSProc( QObject* parent, const char* name, const QStringList &args)" << endl;
}

/** Desctructor */
FestivalCSProc::~FestivalCSProc(){
  kdDebug() << "Running: FestivalCSProc::~FestivalCSProc()" << endl;
}

/** Initializate the speech */
bool FestivalCSProc::init(){
  kdDebug() << "Running: FestivalCSProc::init()" << endl;
/*  kdDebug() << "Initializing plug in: FestivalCS" << endl;
  
  KConfig *config = new KConfig("proklamrc");
  config->setGroup("FestivalCS");
  
  int heap_size = 210000;  // default scheme heap size
  int load_init_files = 1; // we want the festival init files loaded
  
  festival_initialize(load_init_files,heap_size);
  
  if(config->readEntry("Arts")){
    kdDebug() << "Forcing Arts output" << endl;
    //(Parameter.set 'Audio_Command "artsplay $FILE")
    //int festival_eval_command(const EST_String &expr); 
    if(!festival_eval_command(EST_String("(Parameter.set 'Audio_Command \"artsplay $FILE\")"))){
      kdDebug() << "Error while running (Parameter.set 'Audio_Command \"artsplay $FILE\")" << endl;
    }
  }
*/ 
  return true;
}

/** Say a text
    text: The text to be speech
*/
void FestivalCSProc::sayText(const QString &text){
  kdDebug() << "Running: FestivalCSProc::sayText(const QString &text)" << endl;
/*  kdDebug() << "Saying text: '" << text << "' using FestivalCS plug in" << endl;
  //festival_say_text(text.latin1());
  festival_say_text("hello world");
  kdDebug() << "Finished saying text: '" << text << "'" << endl;*/
}
