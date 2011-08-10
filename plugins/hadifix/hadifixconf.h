#ifndef _HADIFIXCONF_H_
#define _HADIFIXCONF_H_

#include <tqstringlist.h>

#include <kconfig.h>

#include <pluginconf.h>

class HadifixProc;
class HadifixConfPrivate;

class HadifixConf : public PlugInConf {
    Q_OBJECT
    TQ_OBJECT

    public:
        /** Constructor */
        HadifixConf( TQWidget* parent = 0, const char* name = 0, const TQStringList &args = TQStringList());

        /** Destructor */
        ~HadifixConf();

        /** This method is invoked whenever the module should read its 
            configuration (most of the times from a config file) and update the 
            user interface. This happens when the user clicks the "Reset" button in 
            the control center, to undo all of his changes and restore the currently 
            valid settings. NOTE that this is not called after the modules is loaded,
            so you probably want to call this method in the constructor.*/
        void load(KConfig *config, const TQString &configGroup);

        /** This function gets called when the user wants to save the settings in 
            the user interface, updating the config files or wherever the 
            configuration is stored. The method is called when the user clicks "Apply" 
            or "Ok". */
        void save(KConfig *config, const TQString &configGroup);

        /** This function is called to set the settings in the module to sensible
            default values. It gets called when hitting the "Default" button. The 
            default values should probably be the same as the ones the application 
            uses when started without a config file. */
        void defaults();

        /**
        * This function informs the plugin of the desired language to be spoken
        * by the plugin.  The plugin should attempt to adapt itself to the
        * specified language code, choosing sensible defaults if necessary.
        * If the passed-in code is TQString(), no specific language has
        * been chosen.
        * @param lang        The desired language code or Null if none.
        *
        * If the plugin is unable to support the desired language, that is OK.
        * Language codes are given by ISO 639-1 and are in lowercase.
        * The code may also include an ISO 3166 country code in uppercase
        * separated from the language code by underscore (_).  For
        * example, en_GB.  If your plugin supports the given language, but
        * not the given country, treat it as though the country
        * code were not specified, i.e., adapt to the given language.
        */
        void setDesiredLanguage(const TQString &lang);

        /**
        * Return fully-specified talker code for the configured plugin.  This code
        * uniquely identifies the configured instance of the plugin and distinquishes
        * one instance from another.  If the plugin has not been fully configured,
        * i.e., cannot yet synthesize, return TQString().
        * @return            Fully-specified talker code.
        */
        TQString getTalkerCode();

    public slots:
        void configChanged(bool t = true){emit changed(t);};

    private slots:
        virtual void voiceButton_clicked();
        virtual void testButton_clicked();
        virtual void voiceCombo_activated(int index);
        void slotSynthFinished();
        void slotSynthStopped();

    private:
        HadifixConfPrivate *d;
};
#endif
