#ifndef _COMMANDCONF_H_
#define _COMMANDCONF_H_

#include <qstringlist.h>

#include <kconfig.h>

#include <pluginconf.h>

class TextToSpeechConfigurationWidget;

class CommandConf : public PlugInConf {
  Q_OBJECT 
  
  public:
    /** Constructor */
    CommandConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());
    
    /** Destructor */
    ~CommandConf();
    
    /** This method is invoked whenever the module should read its 
        configuration (most of the times from a config file) and update the 
        user interface. This happens when the user clicks the "Reset" button in 
        the control center, to undo all of his changes and restore the currently 
        valid settings. NOTE that this is not called after the modules is loaded,
        so you probably want to call this method in the constructor.*/
    void load(KConfig *config, const QString &langGroup);
    
    /** This function gets called when the user wants to save the settings in 
        the user interface, updating the config files or wherever the 
        configuration is stored. The method is called when the user clicks "Apply" 
        or "Ok". */
    void save(KConfig *config, const QString &langGroup);
    
    /** This function is called to set the settings in the module to sensible
        default values. It gets called when hitting the "Default" button. The 
        default values should probably be the same as the ones the application 
        uses when started without a config file. */
    void defaults();
  
  public slots:
    void configChanged(bool t = true){emit changed(t);};
    
  private:
    TextToSpeechConfigurationWidget *configWidget; 
};
#endif
