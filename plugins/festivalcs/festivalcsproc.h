#ifndef _FESTIVALCSPROC_H_
#define _FESTIVALCSPROC_H_

#include <qstringlist.h> 

#include <pluginproc.h>

class FestivalCSProc : public PlugInProc{
  Q_OBJECT 
  
  public:
    /** Constructor */
    FestivalCSProc( QObject* parent = 0, const char* name = 0, const QStringList &args = QStringList());
    
    /** Destructor */
    ~FestivalCSProc();
    
    /** Initializate the speech */
    bool init();
    
    /** Say a text
        text: The text to be speech
    */
    void sayText(const QString &text);
};

#endif
