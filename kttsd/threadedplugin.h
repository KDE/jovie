//
// C++ Interface: ThreadedPlugIn
//
// Description: 
//    This is a threaded wrapper around plugins that do not provide
// asynchronous support.  When the plugin has completed an operation,
// it will postEvent to the speaker object to let it know that plugin
// has completed the operation.
//
// Author: Gary Cramblitt <garycramblitt@comcast.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _THREADEDPLUGIN_H_
#define _THREADEDPLUGIN_H_

#include <qthread.h>
#include <qmutex.h>

#include "pluginproc.h"

class Speaker;

class ThreadedPlugIn : public PlugInProc, public QThread 
{
    public:
        enum pluginAction
        {
            paNone = 0,
            paSayText = 1,
            paSynthText = 2
        };
            
        /**
        * Constructor.
        */
        ThreadedPlugIn(PlugInProc* plugin, QObject *parent = 0, const char *name = 0);

        /**
        * Destructor.
        */
        virtual ~ThreadedPlugIn();

        /**
        * Initializate the speech plugin.
        */
        virtual bool init(const QString &lang, KConfig *config);

        /** 
        * Say a text.  Synthesize and audibilize it.
        * @param text                    The text to be spoken.
        *
        * If the plugin supports asynchronous operation, it should return immediately
        * and emit sayFinished signal when synthesis and audibilizing is finished.
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
        * and emit synthFinished signal when synthesis is completed.
        */
        virtual void synthText(const QString &text, const QString &suggestedFilename);
        
        /**
        * Get the generated audio filename from synthText.
        * @return                        Name of the audio file the plugin generated.
        *                                Null if no such file.
        *
        * The plugin must not re-use the filename.
        */
        virtual QString getFilename();
        
        /**
        * Stop current operation (saying or synthesizing text).
        * This function only makes sense in asynchronus modes.
        * The plugin should return to the psIdle state.
        */
        virtual void stopText();
        
        /**
        * Return the current state of the plugin.
        * This function only makes sense in asynchronous mode.
        * @return                        The pluginState of the plugin.
        *
        * @ref pluginState
        */
        virtual pluginState getState();
        
        /**
        * Acknowleges a finished state and resets the plugin state to psIdle.
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
        */
        virtual bool supportsAsync();
        
        /**
        * Returns True if the plugin supports synthText method,
        * i.e., is able to synthesize text to a sound file without
        * audibilizing the text.
        * @return                        True if this plugin supports synthText method.
        */
        virtual bool supportsSynth();

    protected:
        /**
         * Base function, where the thread will start.
         */
        virtual void run();
    
    private:
        /**
        * Waits for the thread to go to sleep.
        */
        void waitThreadNotBusy();
        
        /**
        * The plugin we wrap.
        */
        PlugInProc* m_plugin;
        
        /**
        * An action requested of the plugin. 
        */
        pluginAction m_action;
        
        /**
        * A text buffer to go with an action (if applicable).
        */
        QString m_text;

        /**
        * Current state of the plugin.
        */        
        volatile pluginState m_state;
        
        /**
        * Mutext for accessing state variable.
        */
        QMutex m_stateMutex;
        
        /**
        * True when stopText was called but the plugin did not stop.
        */
        bool m_waitingStop;
        
        /**
        * Locked when thread is running.
        */
        QMutex m_threadRunningMutex;
        
        /**
        * Filename for generated synthesized text.
        */
        QString m_filename;
        
        /**
        * Thread wait condition.
        */
        QWaitCondition m_waitCondition;

        /**
        * Thread exit flag.
        */
        volatile bool m_requestExit;
        
        /**
        * Whether wrapped plugin supports separate synthesis.
        */
        bool m_supportsSynth;
};

#endif // _THREADEDPLUGIN_H_