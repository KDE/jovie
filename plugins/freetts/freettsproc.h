#ifndef _FREETTSPROC_H_
#define _FREETTSPROC_H_

#include <qstringlist.h> 

#include <pluginproc.h>

class FreeTTSProc : public PlugInProc{
  Q_OBJECT 
  
  public:
    /** Constructor */
    FreeTTSProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());
    
    /** Destructor */
    ~FreeTTSProc();
    
    /** Initializate the speech */
    bool init();
    
    /** Say a text
        text: The text to be speech
    */
    void sayText(const QString &text);
};

#endif
