/***************************************************************************
    begin                : Mon Okt 14 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : gunnar@schmi-dt.de
    current mainainer:   : Gary Cramblitt <garycramblitt@comcast.net> 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HADIFIXCONF_H
#define HADIFIXCONF_H

// Qt includes.

// KTTS includes.
#include <pluginconf.h>

// Hadifix includes.
#include "ui_hadifixconfigui.h"

class KConfig;
class KProgressDialog;
class HadifixProc;

class HadifixConfPrivate : public QWidget, public Ui::HadifixConfigUI {
    friend class HadifixConf;

    Q_OBJECT

    private:
        HadifixConfPrivate(QWidget *parent);
        virtual ~HadifixConfPrivate();

        void setConfiguration (QString hadifixExec,  QString mbrolaExec,
                               QString voice,        bool male,
                               int volume, int time, int pitch,
                               QString codecName);
        void initializeVoices ();
        void initializeCharacterCodes();
        void setDefaultEncodingFromVoice();
        void setDefaults ();
        void load (KConfig *config, const QString &configGroup);
        void save (KConfig *config, const QString &configGroup);

        void findInitialConfig();
        QString findHadifixDataPath ();
        QString findExecutable (const QStringList &names, const QString &possiblePath);
        QStringList findVoices(QString mbrolaExec, const QString &hadifixDataPath);
        QStringList findSubdirs (const QStringList &baseDirs);

    private slots:
        int percentToSlider (int percentValue);
        int sliderToPercent (int sliderValue);
        void volumeBox_valueChanged (int percentValue);
        void timeBox_valueChanged (int percentValue);
        void frequencyBox_valueChanged (int percentValue);
        void volumeSlider_valueChanged (int sliderValue);
        void timeSlider_valueChanged (int sliderValue);
        void frequencySlider_valueChanged (int sliderValue);
        void init ();
        void addVoice (const QString &filename, bool isMale);
        void addVoice (const QString &filename, bool isMale, const QString &displayname);
        void setVoice (const QString &filename, bool isMale);
        QString getVoiceFilename();
        bool isMaleVoice();

    private:
        QString languageCode;
        QString defaultHadifixExec;
        QString defaultMbrolaExec;
        QStringList defaultVoices;
        QStringList codecList;

        // Wave file playing on play object.
        QString waveFile;
        // Synthesizer.
        HadifixProc* hadifixProc;
        // Progress Dialog.
        KProgressDialog* progressDlg;

        QMap<QString,int> maleVoices;
        QMap<int,QString> defaultVoicesMap;
        QMap<QString,int> femaleVoices;
};

class HadifixConf : public PlugInConf {
    Q_OBJECT

    public:
        /** Constructor */
        explicit HadifixConf( QWidget* parent = 0, const QStringList &args = QStringList());

        /** Destructor */
        ~HadifixConf();

        /** This method is invoked whenever the module should read its 
            configuration (most of the times from a config file) and update the 
            user interface. This happens when the user clicks the "Reset" button in 
            the control center, to undo all of his changes and restore the currently 
            valid settings. NOTE that this is not called after the modules is loaded,
            so you probably want to call this method in the constructor.*/
        void load(KConfig *config, const QString &configGroup);

        /** This function gets called when the user wants to save the settings in 
            the user interface, updating the config files or wherever the 
            configuration is stored. The method is called when the user clicks "Apply" 
            or "Ok". */
        void save(KConfig *config, const QString &configGroup);

        /** This function is called to set the settings in the module to sensible
            default values. It gets called when hitting the "Default" button. The 
            default values should probably be the same as the ones the application 
            uses when started without a config file. */
        void defaults();

        /**
        * This function informs the plugin of the desired language to be spoken
        * by the plugin.  The plugin should attempt to adapt itself to the
        * specified language code, choosing sensible defaults if necessary.
        * If the passed-in code is QString(), no specific language has
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
        void setDesiredLanguage(const QString &lang);

        /**
        * Return fully-specified talker code for the configured plugin.  This code
        * uniquely identifies the configured instance of the plugin and distinquishes
        * one instance from another.  If the plugin has not been fully configured,
        * i.e., cannot yet synthesize, return QString().
        * @return            Fully-specified talker code.
        */
        QString getTalkerCode();

    public slots:
        void configChanged(bool t = true){emit changed(t);}

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
