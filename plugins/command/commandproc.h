#ifndef _COMMANDPROC_H_
#define _COMMANDPROC_H_

#include <qstringlist.h> 

#include <pluginproc.h>

class TextToSpeechSystem;

class CommandProc : public PlugInProc{
  Q_OBJECT 
  
  public:
    /** Constructor */
    CommandProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());
    
    /** Destructor */
    ~CommandProc();
    
    /** Initializate the speech */
    bool init (const QString &lang, KConfig *config);
    
    /** Say a text
        text: The text to be speech
    */
    void sayText(const QString &text);
    
    /**
     * Stop text
     */
    virtual void stopText();      

  private:
    TextToSpeechSystem *tts;
};

#endif
