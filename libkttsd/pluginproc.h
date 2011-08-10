/***************************************************** vim:set ts=4 sw=4 sts=4:
  This file is the template for the processing plug ins.
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  Copyright : (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef _PLUGINPROC_H_
#define _PLUGINPROC_H_

#include <tqobject.h>
#include <tqstring.h>
#include <kdemacros.h>
#include "kdeexportfix.h"
#include <kconfig.h>

/**
* @interface PlugInProc
*
* pluginproc - the KDE Text-to-Speech Deamon Plugin API.
*
* @version 1.0 Draft 1
*
* This class defines the interface that plugins to KTTSD must implement.
*
* @warning The pluginproc interface is still being developed and is likely
* to change in the future.
*
* A KTTSD Plugin interfaces between KTTSD and a speech engine.  It provides the methods
* used by KTTSD to synthesize and/or audibilize text into speech.
*
* @section Goals
*
* The ideal plugin has the following features (listed most important to least important):
* - It can synthesize text into an audio file without sending the audio to
*   the audio device. If the plugin can do this, the @ref supportsSynth function
*   should return True.
* - When @ref stopText is called, is able to immediately stop an in-progress
*   synthesis or saying operation.
* - It can operate asynchronously, i.e., returns immediately from a
*   @ref sayText or @ref synthText call and emits signals @ref sayFinished or
*   @ref synthFinished when completed.  If the plugin can do this, the @ref supportsAsync
*   function should return True.
*
* As a plugin author, your goal is to provide all 3 of these features.  However,
* the speech engine you are working with might not be able to support all three
* features.
*
* If a plugin cannot do all 3 of the features above, the next best combinations
* are (from best to worst):
*
* - @ref supportsSynth returns True, @ref supportsAsync returns False, and
*   @stopText is able to immediately stop synthesis.
* - @ref supportsSynth returns True, @ref supportsAsync returns False, and
*   @stopText returns immediately without stopping synthesis.
* - @ref supportsAsync returns True, @ref supportsSynth returns False, and
*   @ref stopText is able to immediately stop saying.
* - Both @ref supportsSynth and @ref supportsAsync both return False, and
*   @ref stopText is able to immediately stop saying.
* - @ref supportsAsync returns True, @ref supportsSynth returns False, and
*   @ref stopText returns immediately without stopping saying.
* - Both @ref supportsSynth and @ref supportsAsync both return False, and
*   @ref stopText returns immediately without stopping saying.
*
* Notice that aynchronous support is not essential because KTTSD is able to
* provide aynchronous support by running the plugin in a separate thread.
* The ability to immediately stop audio output (or support separate synthesis
* only) is more important.
*
* @section Minimum Implementations
*
* All plugins should implement @ref init in order to initialize the speech engine,
* set language codes, etc.
*
* If @ref supportsSynth return False, a plugin must implement @ref sayText .
*
* If @ref supportsSynth returns True, a plugin must implement the following methods:
* - @ref synthText
* - @ref getFilename
* - @ref ackFinished
* The plugin need not implement @ref sayText .
*
* If @ref supportsAsync returns True, the plugin must implement @ref getState .
*
* If @ref supportsAsync returns True, and @ref supportsSynth returns True, 
* a plugin must emit @ref synthFinished signal when synthesis is completed.
*
* If @ref supportsAsync returns True, and @ref supportsSynth returns False, 
* a plugin must emit @ref sayFinished signal when saying is completed.
*
* If @ref supportsAsync returns False, do not emit signals @ref sayFinished
* or @ref synthFinished .
*
* @section Implementation Guidelines
*
* In no case, will a plugin need to perform more than one @ref sayText or
* @ref synthText at a time.  In other words, in asynchronous mode, KTTSD will
* not call @ref sayText or @ref synthText again until @ref @getState returns
* psFinished.
*
* If @ref supportsAsync returns False, KTTSD will run the plugin in a separate
* TQThread.  As a consequence, the plugin must not make use of the KDE Library,
* when @ref sayText or @ref synthText is called,
* with the exception of KProcess and family (KProcIO, KShellProcess).
* This restriction comes about because the KDE Libraries make use of the
* main TQt event loop, which unfortunately, runs only in the main thread.
* This restriction will likely be lifted in TQt 4 and later.
*
* Since the KDE library is not available from the @ref sayText and @ref synthText methods,
* it is best if the plugin reads configuration settings in the @ref init method.
* The KConfig object is passed as an argument to @ref init .
*
* If the synthesis engine requires a long initialization time (more than a second),
* it is best if the plugin loads the speech engine from the @ref init method.
* Otherwise, it will be more memory efficient to wait until @ref sayText or 
* @ref synthText is called, because it is possible that the plugin will be created
* and initialized, but never used.
*
* All plugins, whether @ref supportsAsync returns True or not, should try to
* implement @ref stopText .  If a plugin returns False from @ref supportsAsync,
* @ref stopText will be called from the main thread, while @ref sayText and/or
* @ref synthText will be called from a separate thread.  Hence, it will be
* possible for @ref stopText to be called while @ref sayText or @ref synthText is
* running.  Keep this in mind when implementing the code.
*
* If the plugin returns True from @ref supportsAsync, you will of course
* need to deal with similar issues.  If you have to use TQThreads
* to implement asynchronous support, do not be concerned about emitting
* the @ref sayFinished or @ref synthFinished signals from your threads, since
* KTTSD will convert the received signals into postEvents and
* return immediately.
*
* If it is not possible for @ref stopText to stop an in-progress operation, it
* must not wait for the operation to complete, since this would block KTTSD.
* Instead, simply return immediately.  Usually, KTTSD will perform other operations
* while waiting for the plugin to complete its operation.  (See threadedplugin.cpp.)
*
* If the @ref stopText implementation returns before the operation has actually
* completed, it must emit the @ref stopped() signal when it is actually completed.
*
* If a plugin returns True from @ref supportsAsync, and @ref stopText is called,
* when the plugin has stopped or completed the operation, it must return psIdle
* on the next call to @ref getState ; not psFinished.  The following state diagram
* might be helpful to understand this:
*
 @verbatim
          psIdle <<----------------------------------------------------------
         /      \                                                           ^
  psSaying      psSynthing --- stopText called and operation completed -->> ^
         \      /                                                           ^
        psFinished --- ackFinished called ------------------------------->> ^
 @endverbatim
*
* If your plugin can't immediately stop an in-progress operation, the easiest
* way to handle this is to set a flag when stopText is called, and then in your
* getState() implementation, if the operation has completed, change the
* psFinished state to psIdle, if the flag is set.  See the flite plugin for
* example code.
*         
* If a plugin returns True from @ref supportsSynth, KTTSD will pass a suggested
* filename in the @ref synthText call.  The plugin should synthesize the text
* into an audio file with the suggested name.  If the synthesis engine does not
* permit this, i.e., it will pick a filename of its own, that is OK.  In either
* case, the actual filename produced should be returned in @ref getFilename .
* In no case may the plugin re-use this filename once @ref getFilename has been
* called.  If for some reason the synthesis engine cannot support this, the
* plugin should copy the file to the suggested filename.  The file must not be
* locked when @ref getFilename is called.  The file will be deleted when
* KTTSD is finished using it.
*
* The preferred audio file format is wave, since this is the only format
* guaranteed to be supported by KDE (aRts).  Other formats may or may not be
* supported on a user's machine.
*
* The plugin destructor should take care of terminating the speech engine.
*
* @section Error-handling Error Handling
*
* Plugins may emit the @ref error signal when an error occurs.
*
* When an error occurs, plugins should attempt to recover as best they can and
* continue accepting @ref sayText or @ref synthText calls.  For example,
* if a speech engine emits an error in response to certain characters embedded
* in synthesizing text, the plugin should discard the text and
* emit signal @ref error with True as the first argument and the speech
* engine's error message as the second argument.  The plugin should then
* treat the operation as a completed operation, i.e., return psFinished when
* @ref getState is called.
*
* If the speech engine crashes, the plugin should emit signal @ref error with
* True as the first argument and then attempt to restart the speech engine. 
* The plugin will need to implement some protection against an infinite
* restart loop and emit the @ref error signal with False as the first argument
* if this occurs.
*
* If a plugin emits the @ref error signal with False as the first argument,
* KTTSD will no longer call the plugin.
*
* @section PlugInConf
*
* The plugin should implement a configuration dialog where the user can specify
* options specific to the speech engine.  This dialog is displayed in the KDE
* Control Center and also in the KTTS Manager (kttsmgr).  See pluginconf.h.
*
* If the user changes any of the settings while the plugin is created,
* the plugin will be destroyed and re-created.
*/

class TQTextCodec;

enum pluginState
{
    psIdle = 0,                  /**< Plugin is not doing anything. */
    psSaying = 1,                /**< Plugin is synthesizing and audibilizing. */
    psSynthing = 2,              /**< Plugin is synthesizing. */
    psFinished = 3               /**< Plugin has finished synthesizing.  Audio file is ready. */
};

class KDE_EXPORT PlugInProc : virtual public TQObject{
    Q_OBJECT
  TQ_OBJECT

    public:
        enum CharacterCodec {
            Local    = 0,
            Latin1   = 1,
            Unicode  = 2,
            UseCodec = 3
        };

        /**
        * Constructor.
        */
        PlugInProc( TQObject *parent = 0, const char *name = 0);

        /**
        * Destructor.
        * Plugin must terminate the speech engine.
        */
        virtual ~PlugInProc();

        /**
        * Initialize the speech engine.
        * @param config          Settings object.
        * @param configGroup     Settings Group.
        *
        * Sample code for reading configuration:
        *
          @verbatim
            config->setGroup(configGroup);
            m_fliteExePath = config->readEntry("FliteExePath", "flite");
            kdDebug() << "FliteProc::init: path to flite: " << m_fliteExePath << endl;
            config->setGroup(configGroup);
          @endverbatim
        */
        virtual bool init(KConfig *config, const TQString &configGroup);

        /** 
        * Say a text.  Synthesize and audibilize it.
        * @param text                    The text to be spoken.
        *
        * If the plugin supports asynchronous operation, it should return immediately
        * and emit sayFinished signal when synthesis and audibilizing is finished.
        * It must also implement the @ref getState method, which must return
        * psFinished, when saying is completed.
        */
        virtual void sayText(const TQString &text);

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
        virtual void synthText(const TQString &text, const TQString &suggestedFilename);

        /**
        * Get the generated audio filename from call to @ref synthText.
        * @return                        Name of the audio file the plugin generated.
        *                                Null if no such file.
        *
        * The plugin must not re-use or delete the filename.  The file may not
        * be locked when this method is called.  The file will be deleted when
        * KTTSD is finished using it.
        */
        virtual TQString getFilename();

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
        * Returns the name of an XSLT stylesheet that will convert a valid SSML file
        * into a format that can be processed by the synth.  For example,
        * The Festival plugin returns a stylesheet that will convert SSML into
        * SABLE.  Any tags the synth cannot handle should be stripped (leaving
        * their text contents though).  The default stylesheet strips all
        * tags and converts the file to plain text.
        * @return            Name of the XSLT file.
        */
        virtual TQString getSsmlXsltFilename();

        /**
        * Given the name of a codec, returns the TQTextCodec for the name.
        * Handles the following "special" codec names:
        *   Local               The user's current Locale codec.
        *   Latin1              Latin1 (ISO 8859-1)
        *   Unicode             UTF-16
        * @param codecName      Name of desired codec.
        * @return               The codec object.  Calling program must not delete this object
        *                       as it is a reference to an existing TQTextCodec object.
        *
        * Caution: Do not pass translated codec names to this routine.
        */
        static TQTextCodec* codecNameToCodec(const TQString &codecName);

        /**
        * Builds a list of codec names, suitable for display in a TQComboBox.
        * The list includes the 3 special codec names (translated) at the top:
        *   Local               The user's current Locale codec.
        *   Latin1              Latin1 (ISO 8859-1)
        *   Unicode             UTF-16
        */
        static TQStringList buildCodecList();

        /**
        * Given the name of a codec, returns index into the codec list.
        * Handles the following "special" codec names:
        *   Local               The user's current Locale codec.
        *   Latin1              Latin1 (ISO 8859-1)
        *   Unicode             UTF-16
        * @param codecName      Name of the codec.
        * @param codecList      List of codec names. The first 3 entries may be translated names.
        * @return               TQTextCodec object.  Caller must not delete this object.
        *
        * Caution: Do not pass translated codec names to this routine in codecName parameter.
        */
        static int codecNameToListIndex(const TQString &codecName, const TQStringList &codecList);

        /**
        * Given index into codec list, returns the codec object.
        * @param codecNum       Index of the codec.
        * @param codecList      List of codec names. The first 3 entries may be translated names.
        * @return               TQTextCodec object.  Caller must not delete this object.
        */
        static TQTextCodec* codecIndexToCodec(int codecNum, const TQStringList &codecList);

        /**
        * Given index into codec list, returns the codec Name.
        * Handles the following "special" codec names:
        *   Local               The user's current Locale codec.
        *   Latin1              Latin1 (ISO 8859-1)
        *   Unicode             UTF-16
        * @param codecNum       Index of the codec.
        * @param codecList      List of codec names. The first 3 entries may be translated names.
        * @return               Untranslated name of the codec.
        */
        static TQString codecIndexToCodecName(int codecNum, const TQStringList &codecList);

    signals:
        /**
        * Emitted when synthText() finishes and plugin supports asynchronous mode.
        */
        void synthFinished();
        /**
        * Emitted when sayText() finishes and plugin supports asynchronous mode.
        */
        void sayFinished();
        /**
        * Emitted when stopText() has been called and plugin stops asynchronously.
        */
        void stopped();
        /**
        * Emitted if an error occurs.
        * @param keepGoing               True if the plugin can continue processing.
        *                                False if the plugin cannot continue, for example,
        *                                the speech engine could not be started.
        * @param msg                     Error message.
        *
        * When an error occurs, plugins should attempt to recover as best they can
        * and continue accepting @ref sayText or @ref synthText calls.  For example,
        * if the speech engine emits an error while synthesizing text, the plugin
        * should return True along with error message.
        *
        * @see Error-handling
        *
        */
        void error(bool keepGoing, const TQString &msg);
};

#endif // _PLUGINPROC_H_
