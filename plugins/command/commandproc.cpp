#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kconfig.h>

#include "texttospeechsystem.h"
#include "commandproc.h"
#include "commandproc.moc"

/** Constructor */
CommandProc::CommandProc( QObject* parent, const char* name, const QStringList &args) : 
  PlugInProc( parent, name ){
  kdDebug() << "Running: CommandProc::CommandProc( QObject* parent, const char* name, const QStringList &args)" << endl;
  tts = 0;
}

/** Desctructor */
CommandProc::~CommandProc(){
  kdDebug() << "Running: CommandProc::~CommandProc()" << endl;
  if (tts != 0)
     delete tts;
}

/** Initializate the speech */
bool CommandProc::init(const QString &lang, KConfig *config){
  kdDebug() << "Running: CommandProc::init(const QString &lang, KConfig *config)" << endl;
  kdDebug() << "Initializing plug in: Command" << endl;

  tts = new TextToSpeechSystem ();
  // KConfig *config = new KConfig("kttsdrc");
  tts->readOptions(config, QString("Lang_")+lang);
  // delete config;

  return true;
}

/** Say a text
    text: The text to be speech
*/
void CommandProc::sayText(const QString &text){
  kdDebug() << "Running: CommandProc::sayText(const QString &text)" << endl;
  kdDebug() << "Saying text: '" << text << "' using Command plug in" << endl;

  tts->speak (text);
}

/**
 * Stop text
 */
void CommandProc::stopText(){
   kdDebug() << "Running: CommandProc::stopText()" << endl;
   // Bogus implementation by now
}
