/***************************************************************************
                          hadifixproc.cpp  - description
                             -------------------
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
 
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtextcodec.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "hadifixproc.h"
#include "hadifixproc.moc"

class HadifixProcPrivate {
   friend class HadifixProc;
   private:
      HadifixProcPrivate () {
         hadifixProc = 0;
         waitingStop = false;
         state = psIdle;
         synthFilename = TQString();
         gender = false;
         volume = 100;
         time = 100;
         pitch = 100;
         codec = 0;
      };

      ~HadifixProcPrivate() {
        delete hadifixProc;
      };

      void load(KConfig *config, const TQString &configGroup) {
         config->setGroup(configGroup);
         hadifix  = config->readEntry ("hadifixExec",   TQString());
         mbrola   = config->readEntry ("mbrolaExec",    TQString());
         voice    = config->readEntry ("voice",         TQString());
         gender   = config->readBoolEntry("gender",     false);
         volume   = config->readNumEntry ("volume",     100);
         time     = config->readNumEntry ("time",       100);
         pitch    = config->readNumEntry ("pitch",      100);
         codec    = PlugInProc::codecNameToCodec(config->readEntry ("codec", "Local"));
      };

      TQString hadifix;
      TQString mbrola;
      TQString voice;
      bool gender;
      int volume;
      int time;
      int pitch;

      bool waitingStop;      
      KShellProcess* hadifixProc;
      volatile pluginState state;
      TQTextCodec* codec;
      TQString synthFilename;
};

/** Constructor */
HadifixProc::HadifixProc( TQObject* tqparent, const char* name, const TQStringList &) : 
   PlugInProc( tqparent, name ){
   // kdDebug() << "HadifixProc::HadifixProc: Running" << endl;
   d = 0;
}

/** Destructor */
HadifixProc::~HadifixProc(){
   // kdDebug() << "HadifixProc::~HadifixProc: Running" << endl;

   if (d != 0) {
      delete d;
      d = 0;
   }
}

/** Initializate the speech */
bool HadifixProc::init(KConfig *config, const TQString &configGroup){
   // kdDebug() << "HadifixProc::init: Initializing plug in: Hadifix" << endl;

   if (d == 0)
      d = new HadifixProcPrivate();
   d->load(config, configGroup);
   return true;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately
* and emit sayFinished signal when synthesis and audibilizing is finished.
* It must also implement the @ref getState method, which must return
* psFinished, when saying is completed.
*/
void HadifixProc::sayText(const TQString& /*text*/)
{
    kdDebug() << "HadifixProc::sayText: Warning, sayText not implemented." << endl;
    return;
}
    
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
void HadifixProc::synthText(const TQString &text, const TQString &suggestedFilename)
{
    if (d == 0) return;  // Caller should have called init.
    synth(text, d->hadifix, d->gender, d->mbrola, d->voice, d->volume,
        d->time, d->pitch, d->codec, suggestedFilename);
}

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
void HadifixProc::synth(TQString text,
                        TQString hadifix, bool isMale,
                        TQString mbrola,  TQString voice,
                        int volume, int time, int pitch,
                        TQTextCodec *codec,
                        const TQString waveFilename)
{
   // kdDebug() << "HadifixProc::synth: Saying text: '" << text << "' using Hadifix plug in" << endl;
   if (d == 0)
   {
      d = new HadifixProcPrivate();
   }
   if (hadifix.isNull() || hadifix.isEmpty())
      return;
   if (mbrola.isNull() || mbrola.isEmpty())
      return;
   if (voice.isNull() || voice.isEmpty())
      return;

   // If process exists, delete it so we can create a new one.
   // kdDebug() << "HadifixProc::synth: creating process" << endl;
   if (d->hadifixProc) delete d->hadifixProc;
   
   // Create process.
   d->hadifixProc = new KShellProcess;
   
   // Set up txt2pho and mbrola commands.
   // kdDebug() << "HadifixProc::synth: setting up commands" << endl;
   TQString hadifixCommand = d->hadifixProc->quote(hadifix);
   if (isMale)
      hadifixCommand += " -m";
   else
      hadifixCommand += " -f";

   TQString mbrolaCommand = d->hadifixProc->quote(mbrola);
   mbrolaCommand += " -e"; //Ignore fatal errors on unkown diphone
   mbrolaCommand += TQString(" -v %1").tqarg(volume/100.0); // volume ratio
   mbrolaCommand += TQString(" -f %1").tqarg(pitch/100.0);  // freqency ratio
   mbrolaCommand += TQString(" -t %1").tqarg(1/(time/100.0));   // time ratio
   mbrolaCommand += " " + d->hadifixProc->quote(voice);
   mbrolaCommand += " - " + d->hadifixProc->quote(waveFilename);

   // kdDebug() << "HadifixProc::synth: Hadifix command: " << hadifixCommand << endl;
   // kdDebug() << "HadifixProc::synth: Mbrola command: " << mbrolaCommand << endl;

   TQString command = hadifixCommand + "|" + mbrolaCommand;
   *(d->hadifixProc) << command;
   
   // Connect signals from process.
   connect(d->hadifixProc, TQT_SIGNAL(processExited(KProcess *)),
     this, TQT_SLOT(slotProcessExited(KProcess *)));
   connect(d->hadifixProc, TQT_SIGNAL(wroteStdin(KProcess *)),
     this, TQT_SLOT(slotWroteStdin(KProcess *)));
   
   // Store off name of wave file to be generated.
   d->synthFilename = waveFilename;
   // Set state, busy synthing.
   d->state = psSynthing;
   if (!d->hadifixProc->start(KProcess::NotifyOnExit, KProcess::Stdin))
   {
     kdDebug() << "HadifixProc::synth: start process failed." << endl;
     d->state = psIdle;
   } else {
     TQCString encodedText;
     if (codec) {
       encodedText = codec->fromUnicode(text);
       // kdDebug() << "HadifixProc::synth: encoding using " << codec->name() << endl;
     } else
       encodedText = text.latin1();  // Should not happen, but just in case.
     // Send the text to be synthesized to process.
     d->hadifixProc->writeStdin(encodedText, encodedText.length());
   }
}

/**
* Get the generated audio filename from call to @ref synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use or delete the filename.  The file may not
* be locked when this method is called.  The file will be deleted when
* KTTSD is finished using it.
*/
TQString HadifixProc::getFilename() { return d->synthFilename; }

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
void HadifixProc::stopText(){
    // kdDebug() << "Running: HadifixProc::stopText()" << endl;
    if (d->hadifixProc)
    {
        if (d->hadifixProc->isRunning())
        {
            // kdDebug() << "HadifixProc::stopText: killing Hadifix shell." << endl;
            d->waitingStop = true;
            d->hadifixProc->kill();
        } else d->state = psIdle;
    } else d->state = psIdle;
    // d->state = psIdle;
    // kdDebug() << "HadifixProc::stopText: Hadifix stopped." << endl;
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @see pluginState
*/
pluginState HadifixProc::getState() { return d->state; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void HadifixProc::ackFinished()
{
    if (d->state == psFinished)
    {
        d->state = psIdle;
        d->synthFilename = TQString();
    }
}

/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*
* If the plugin returns True, it must also implement @ref getState .
* It must also emit @ref sayFinished or @ref synthFinished signals when
* saying or synthesis is completed.
*/
bool HadifixProc::supportsAsync() { return true; }

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
bool HadifixProc::supportsSynth() { return true; }


void HadifixProc::slotProcessExited(KProcess*)
{
    // kdDebug() << "HadifixProc:hadifixProcExited: Hadifix process has exited." << endl;
    pluginState prevState = d->state;
    if (d->waitingStop)
    {
        d->waitingStop = false;
        d->state = psIdle;
        emit stopped();
    } else {
        d->state = psFinished;
        if (prevState == psSynthing)
            emit synthFinished();
    }
}

void HadifixProc::slotWroteStdin(KProcess*)
{
   // kdDebug() << "HadifixProc::slotWroteStdin: closing Stdin" << endl;
   d->hadifixProc->closeStdin();
}


/***************************************************************************/

/**
* Static function to determine whether the voice file is male or female.
* @param mbrola the mbrola executable
* @param voice the voice file
* @param output the output of mbrola will be written into this TQString*
* @return HadifixSpeech::MaleGender if the voice is male,
*         HadifixSpeech::FemaleGender if the voice is female,
*         HadifixSpeech::NoGender if the gender cannot be determined,
*         HadifixSpeech::NoVoice if there is an error in the voice file
*/
HadifixProc::VoiceGender HadifixProc::determineGender(TQString mbrola, TQString voice, TQString *output)
{
   TQString command = mbrola + " -i " + voice + " - -";

   // create a new process
   HadifixProc speech;
   KShellProcess proc;
   proc << command;
   connect(&proc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),
     &speech, TQT_SLOT(receivedStdout(KProcess *, char *, int)));
   connect(&proc, TQT_SIGNAL(receivedStderr(KProcess *, char *, int)),
     &speech, TQT_SLOT(receivedStderr(KProcess *, char *, int)));

   speech.stdOut = TQString();
   speech.stdErr = TQString();
   proc.start (KProcess::Block, KProcess::AllOutput);

   VoiceGender result;
   if (!speech.stdErr.isNull() && !speech.stdErr.isEmpty()) {
      if (output != 0)
         *output = speech.stdErr;
      result = NoVoice;
   }
   else {
      if (output != 0)
         *output = speech.stdOut;
      if (speech.stdOut.contains("female", false))
         result = FemaleGender;
      else if (speech.stdOut.contains("male", false))
         result = MaleGender;
      else
         result = NoGender;
   }
   
   return result;
}

void HadifixProc::receivedStdout (KProcess *, char *buffer, int buflen) {
   stdOut += TQString::tqfromLatin1(buffer, buflen);
}

void HadifixProc::receivedStderr (KProcess *, char *buffer, int buflen) {
   stdErr += TQString::tqfromLatin1(buffer, buflen);
}

/**
 * Returns the name of an XSLT stylesheet that will convert a valid SSML file
 * into a format that can be processed by the synth.  For example,
 * The Festival plugin returns a stylesheet that will convert SSML into
 * SABLE.  Any tags the synth cannot handle should be stripped (leaving
 * their text contents though).  The default stylesheet strips all
 * tags and converts the file to plain text.
 * @return            Name of the XSLT file.
 */
TQString HadifixProc::getSsmlXsltFilename()
{
    return KGlobal::dirs()->resourceDirs("data").last() + "kttsd/hadifix/xslt/SSMLtoTxt2pho.xsl";
}
