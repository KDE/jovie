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

// Qt includes

#include <QtCore/QTextCodec>
#include <QtCore/QByteArray>

// KDE includes
#include <kdebug.h>
#include <kconfig.h>
#include <k3process.h>
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
         synthFilename.clear();
         gender = false;
         volume = 100;
         time = 100;
         pitch = 100;
         codec = 0;
      }

      ~HadifixProcPrivate() {
        delete hadifixProc;
      }

      void load(KConfig *c, const QString &configGroup) {
         KConfigGroup config(c, configGroup);
         hadifix  = config.readEntry ("hadifixExec",   QString());
         mbrola   = config.readEntry ("mbrolaExec",    QString());
         voice    = config.readEntry ("voice",         QString());
         gender   = config.readEntry("gender", false);
         volume   = config.readEntry ("volume",     100);
         time     = config.readEntry ("time",       100);
         pitch    = config.readEntry ("pitch",      100);
         codec    = PlugInProc::codecNameToCodec(config.readEntry ("codec", "Local"));
      }

      QString hadifix;
      QString mbrola;
      QString voice;
      bool gender;
      int volume;
      int time;
      int pitch;

      bool waitingStop;      
      K3ShellProcess* hadifixProc;
      volatile pluginState state;
      QTextCodec* codec;
      QString synthFilename;
};

/** Constructor */
HadifixProc::HadifixProc( QObject* parent, const QStringList &) : 
   PlugInProc( parent, "hadifixproc" ){
   // kDebug() << "HadifixProc::HadifixProc: Running";
   d = 0;
}

/** Destructor */
HadifixProc::~HadifixProc(){
   // kDebug() << "HadifixProc::~HadifixProc: Running";

   if (d != 0) {
      delete d;
      d = 0;
   }
}

/** Initializate the speech */
bool HadifixProc::init(KConfig *config, const QString &configGroup){
   // kDebug() << "HadifixProc::init: Initializing plug in: Hadifix";

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
void HadifixProc::sayText(const QString& /*text*/)
{
    kDebug() << "HadifixProc::sayText: Warning, sayText not implemented.";
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
void HadifixProc::synthText(const QString &text, const QString &suggestedFilename)
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
void HadifixProc::synth(QString text,
                        QString hadifix, bool isMale,
                        QString mbrola,  QString voice,
                        int volume, int time, int pitch,
                        QTextCodec *codec,
                        const QString waveFilename)
{
   // kDebug() << "HadifixProc::synth: Saying text: '" << text << "' using Hadifix plug in";
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
   // kDebug() << "HadifixProc::synth: creating process";
   if (d->hadifixProc) delete d->hadifixProc;
   
   // Create process.
   d->hadifixProc = new K3ShellProcess;
   
   // Set up txt2pho and mbrola commands.
   // kDebug() << "HadifixProc::synth: setting up commands";
   QString hadifixCommand = d->hadifixProc->quote(hadifix);
   if (isMale)
      hadifixCommand += " -m";
   else
      hadifixCommand += " -f";

   QString mbrolaCommand = d->hadifixProc->quote(mbrola);
   mbrolaCommand += " -e"; //Ignore fatal errors on unknown diphone
   mbrolaCommand += QString(" -v %1").arg(volume/100.0); // volume ratio
   mbrolaCommand += QString(" -f %1").arg(pitch/100.0);  // freqency ratio
   mbrolaCommand += QString(" -t %1").arg(1/(time/100.0));   // time ratio
   mbrolaCommand += ' ' + d->hadifixProc->quote(voice);
   mbrolaCommand += " - " + d->hadifixProc->quote(waveFilename);

   // kDebug() << "HadifixProc::synth: Hadifix command: " << hadifixCommand;
   // kDebug() << "HadifixProc::synth: Mbrola command: " << mbrolaCommand;

   QString command = hadifixCommand + '|' + mbrolaCommand;
   *(d->hadifixProc) << command;
   
   // Connect signals from process.
   connect(d->hadifixProc, SIGNAL(processExited(K3Process *)),
     this, SLOT(slotProcessExited(K3Process *)));
   connect(d->hadifixProc, SIGNAL(wroteStdin(K3Process *)),
     this, SLOT(slotWroteStdin(K3Process *)));
   
   // Store off name of wave file to be generated.
   d->synthFilename = waveFilename;
   // Set state, busy synthing.
   d->state = psSynthing;
   if (!d->hadifixProc->start(K3Process::NotifyOnExit, K3Process::Stdin))
   {
     kDebug() << "HadifixProc::synth: start process failed.";
     d->state = psIdle;
   } else {
     QByteArray encodedText;
     if (codec) {
       encodedText = codec->fromUnicode(text);
       // kDebug() << "HadifixProc::synth: encoding using " << codec->name();
     } else
       encodedText = text.toLatin1();  // Should not happen, but just in case.
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
QString HadifixProc::getFilename() { return d->synthFilename; }

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
    // kDebug() << "Running: HadifixProc::stopText()";
    if (d->hadifixProc)
    {
        if (d->hadifixProc->isRunning())
        {
            // kDebug() << "HadifixProc::stopText: killing Hadifix shell.";
            d->waitingStop = true;
            d->hadifixProc->kill();
        } else d->state = psIdle;
    } else d->state = psIdle;
    // d->state = psIdle;
    // kDebug() << "HadifixProc::stopText: Hadifix stopped.";
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
        d->synthFilename.clear();
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


void HadifixProc::slotProcessExited(K3Process*)
{
    // kDebug() << "HadifixProc:hadifixProcExited: Hadifix process has exited.";
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

void HadifixProc::slotWroteStdin(K3Process*)
{
   // kDebug() << "HadifixProc::slotWroteStdin: closing Stdin";
   d->hadifixProc->closeStdin();
}


/***************************************************************************/

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
HadifixProc::VoiceGender HadifixProc::determineGender(QString mbrola, QString voice, QString *output)
{
   QString command = mbrola + " -i " + voice + " - -";

   // create a new process
   HadifixProc speech;
   K3ShellProcess proc;
   proc << command;
   connect(&proc, SIGNAL(receivedStdout(K3Process *, char *, int)),
     &speech, SLOT(receivedStdout(K3Process *, char *, int)));
   connect(&proc, SIGNAL(receivedStderr(K3Process *, char *, int)),
     &speech, SLOT(receivedStderr(K3Process *, char *, int)));

   speech.stdOut.clear();
   speech.stdErr.clear();
   proc.start (K3Process::Block, K3Process::AllOutput);

   VoiceGender result;
   if (!speech.stdErr.isNull() && !speech.stdErr.isEmpty()) {
      if (output != 0)
         *output = speech.stdErr;
      result = NoVoice;
   }
   else {
      if (output != 0)
         *output = speech.stdOut;
      if (speech.stdOut.contains("female", Qt::CaseInsensitive))
         result = FemaleGender;
      else if (speech.stdOut.contains("male", Qt::CaseInsensitive))
         result = MaleGender;
      else
         result = NoGender;
   }
   
   return result;
}

void HadifixProc::receivedStdout (K3Process *, char *buffer, int buflen) {
   stdOut += QString::fromLatin1(buffer, buflen);
}

void HadifixProc::receivedStderr (K3Process *, char *buffer, int buflen) {
   stdErr += QString::fromLatin1(buffer, buflen);
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
QString HadifixProc::getSsmlXsltFilename()
{
    return KGlobal::dirs()->resourceDirs("data").last() + "kttsd/hadifix/xslt/SSMLtoTxt2pho.xsl";
}
