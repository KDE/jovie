/***************************************************************************
                          hadifixproc.h  - description
                             -------------------
    begin                : Mon Okt 14 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : gunnar@schmi-dt.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef HADIFIXPROC_H
#define HADIFIXPROC_H

#include <QtCore/QStringList> 

#include <pluginproc.h>

class K3Process;

class HadifixProcPrivate;
class HadifixProc : public PlugInProc{
  Q_OBJECT 
  
  public:
    enum VoiceGender {
        MaleGender      =  2,
        FemaleGender    =  1,
        NoGender        =  0,
        NoVoice         = -1
    };
    
    /** Constructor */
    explicit HadifixProc( QObject* parent = 0, const QStringList &args = QStringList());
    
    /** Destructor */
    ~HadifixProc();
    
    /** Initializate the speech */
    virtual bool init (KConfig *config, const QString &configGroup);
    
    /** 
    * Say a text.  Synthesize and audibilize it.
    * @param text                    The text to be spoken.
    *
    * If the plugin supports asynchronous operation, it should return immediately
    * and emit sayFinished signal when synthesis and audibilizing is finished.
    * It must also implement the @ref getState method, which must return
    * psFinished, when saying is completed.
    */
    virtual void sayText(const QString &text);
    
    /**
    * Synthesize text into an audio file, but do not send to the audio device.
    * @param text                    The text to be synthesized.
    * @param suggestedFilename       Full pathname of file to create.  The plugin
    *                                may ignore this parameter and choose its own
    *                                filename.  KTTSD will query the generated
    *                                filename using getFilename().
    *
    * If the plugin supports asynchronous operation, it should return immediately
    * and emit @ref synthFinished signal when synthesis is completed.
    * It must also implement the @ref getState method, which must return
    * psFinished, when synthesis is completed.
    */
    virtual void synthText(const QString &text, const QString &suggestedFilename);
    
    /**
    * Get the generated audio filename from call to @ref synthText.
    * @return                        Name of the audio file the plugin generated.
    *                                Null if no such file.
    *
    * The plugin must not re-use or delete the filename.  The file may not
    * be locked when this method is called.  The file will be deleted when
    * KTTSD is finished using it.
    */
    virtual QString getFilename();
    
    /**
    * Stop current operation (saying or synthesizing text).
    * Important: This function may be called from a thread different from the
    * one that called sayText or synthText.
    * If the plugin cannot stop an in-progress @ref sayText or
    * @ref synthText operation, it must not block waiting for it to complete.
    * Instead, return immediately.
    *
    * If a plugin returns before the operation has actually been stopped,
    * the plugin must emit the @ref stopped signal when the operation has
    * actually stopped.
    *
    * The plugin should change to the psIdle state after stopping the
    * operation.
    */
    virtual void stopText();
    
    /**
    * Return the current state of the plugin.
    * This function only makes sense in asynchronous mode.
    * @return                        The pluginState of the plugin.
    *
    * @see pluginState
    */
    virtual pluginState getState();
    
    /**
    * Acknowledges a finished state and resets the plugin state to psIdle.
    *
    * If the plugin is not in state psFinished, nothing happens.
    * The plugin may use this call to do any post-processing cleanup,
    * for example, blanking the stored filename (but do not delete the file).
    * Calling program should call getFilename prior to ackFinished.
    */
    virtual void ackFinished();
    
    /**
    * Returns True if the plugin supports asynchronous processing,
    * i.e., returns immediately from sayText or synthText.
    * @return                        True if this plugin supports asynchronous processing.
    *
    * If the plugin returns True, it must also implement @ref getState .
    * It must also emit @ref sayFinished or @ref synthFinished signals when
    * saying or synthesis is completed.
    */
    virtual bool supportsAsync();
    
    /**
    * Returns True if the plugin supports synthText method,
    * i.e., is able to synthesize text to a sound file without
    * audibilizing the text.
    * @return                        True if this plugin supports synthText method.
    *
    * If the plugin returns True, it must also implement the following methods:
    * - @ref synthText
    * - @ref getFilename
    * - @ref ackFinished
    *
    * If the plugin returns True, it need not implement @ref sayText .
    */
    virtual bool supportsSynth();
    
    /**
    * Synthesize text using a specified configuration.
    * @param text            The text to synthesize.
    * @param hadifix         Command to run hadifix (txt2pho).
    * @param isMale          True to use male voice.
    * @param mbrola          Command to run mbrola.
    * @param voice           Voice file for mbrola to use.
    * @param volume          Volume percent. 100 = normal
    * @param time            Speed percent. 100 = normal
    * @param pitch           Frequency. 100 = normal
    * @param waveFilename    Name of file to synthesize to.
    */
    void synth(QString text,
                            QString hadifix, bool isMale,
                            QString mbrola,  QString voice,
                            int volume, int time, int pitch,
                            QTextCodec* codec,
                            const QString waveFilename);
    
    /**
    * Static function to determine whether the voice file is male or female.
    * @param mbrola the mbrola executable
    * @param voice the voice file
    * @param output the output of mbrola will be written into this QString*
    * @return HadifixSpeech::MaleGender if the voice is male,
    *         HadifixSpeech::FemaleGender if the voice is female,
    *         HadifixSpeech::NoGender if the gender cannot be determined,
    *         HadifixSpeech::NoVoice if there is an error in the voice file
    */
    static VoiceGender determineGender(QString mbrola, QString voice, QString *output = 0);

    /**
     * Returns the name of an XSLT stylesheet that will convert a valid SSML file
     * into a format that can be processed by the synth.  For example,
     * The Festival plugin returns a stylesheet that will convert SSML into
     * SABLE.  Any tags the synth cannot handle should be stripped (leaving
     * their text contents though).  The default stylesheet strips all
     * tags and converts the file to plain text.
     * @return            Name of the XSLT file.
     */
    virtual QString getSsmlXsltFilename();

  private slots:
    void slotProcessExited(K3Process*);
    void slotWroteStdin(K3Process*);
    
    void receivedStdout (K3Process *, char *buffer, int buflen);
    void receivedStderr (K3Process *, char *buffer, int buflen);
    
  private:
     HadifixProcPrivate *d;
     
     QString stdOut;
     QString stdErr;
};

#endif
