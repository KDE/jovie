/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.

  This class holds a single utterance for synthesis and playback.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef _UTT_H_
#define _UTT_H_

// Qt includes.
#include <QString>

class SSMLConvert;
class PlugInProc;
class Player;
class SpeechJob;
class Stretcher;
class SpeechJob;

class UttPrivate;
class Utt
{
public:
    /**
    * Type of utterance.
    */
    enum uttType
    {
        utText,                      /**< Text */
        utInterruptMsg,              /**< Interruption text message */
        utInterruptSnd,              /**< Interruption sound file */
        utResumeMsg,                 /**< Resume text message */
        utResumeSnd,                 /**< Resume sound file */
        utMessage,                   /**< Message */
        utWarning,                   /**< Warning */
        utScreenReader               /**< Screen Reader Output */
    };

    /**
    * Processing state of an utterance.
    */
    enum uttState
    {
        usNone,                      /**< Null state. Brand new utterance. */
        usWaitingTransform,          /**< Waiting to be transformed (XSLT) */
        usTransforming,              /**< Transforming the utterance (XSLT). */
        usWaitingSay,                /**< Waiting to start synthesis. */
        usWaitingSynth,              /**< Waiting to be synthesized and audibilized. */
        usSaying,                    /**< Plugin is synthesizing and audibilizing. */
        usSynthing,                  /**< Plugin is synthesizing only. */
        usSynthed,                   /**< Plugin has finished synthesizing.  Ready for stretch. */
        usStretching,                /**< Adjusting speed. */
        usStretched,                 /**< Speed adjustment finished.  Ready for playback. */
        usPlaying,                   /**< Playing on Audio Player. */
        usPaused,                    /**< Paused due to user action. */
        usPreempted,                 /**< Paused due to Screen Reader Output. */
        usFinished                   /**< Ready for deletion. */
    };

    /**
    * Constructs an empty utterance.
    */
    Utt();
    
    /**
    * Constructs a message type of utterance that must be (optionally)
    * transformed from SSML, synthesized, and played.
    */
    Utt(uttType utType,
        const QString& appId,
        SpeechJob* job,
        const QString& sentence,
        PlugInProc* pluginproc);

    /**
    * Constructs a sound icon type of utterance.
    */
    Utt(uttType utType, const QString& appId, const QString& audioUrl);

    /**
    * Destructor.
    */
    ~Utt();

    /**
    * Type of utterance.
    */
    uttType utType() const;
    void setUtType(uttType type);
    
    /**
    * AppId of utterance.
    */
    QString appId() const;
    void setAppId(const QString& appId);
    
    /**
    * The job from which the utterance came.
    */    
    SpeechJob* job() const;
    void setJob(SpeechJob* job);

    /**
    * The text of the utterance.
    */
    QString sentence() const;
    void setSentence(const QString& sentence);
    
    /**
    * Sequence number of the sentence.  They start at 1.
    */
    int seq() const;
    void setSeq(int seq);
    
    /**
    * True if the sentence contains SSML markup.
    */
    bool isSsml() const;
    void setIsSsml(bool isSsml);
    
    /**
    * Processing state of the utterance.
    */
    uttState state() const;
    void setState(uttState state);
    
    /**
    * XSLT Transformer.
    */
    SSMLConvert* transformer() const;
    void setTransformer(SSMLConvert* transformer);
    
    /**
    * Synthesis plugin.
    */
    PlugInProc* plugin() const;
    void setPlugin(PlugInProc* plugin);
    
    /**
    * Audio Stretcher.
    */
    Stretcher* audioStretcher() const;
    void setAudioStretcher(Stretcher* stretcher);
    
    /**
    * Filename containing synthesized audio.  Null if
    * plugin has not yet synthesized the utterance, or if
    * plugin does not support synthesis. */
    QString audioUrl() const;
    void setAudioUrl(const QString& audioUrl);
    
    /**
    * The audio player audibilizing the utterance.  Null
    * if not currently audibilizing or if plugin doesn't
    * support synthesis.
    */            
    Player* audioPlayer() const;
    void setAudioPlayer(Player* player);
    
    /**
     * Determines the initial state of an utterance.  If the utterance contains
     * SSML, the state is set to usWaitingTransform.  Otherwise, if the plugin
     * supports async synthesis, sets to usWaitingSynth, otherwise usWaitingSay.
     * If an utterance has already been transformed, usWaitingTransform is
     * skipped to either usWaitingSynth or usWaitingSay.
     */
    void setInitialState();
    
    /**
     * Converts an utterance type enumerator to a displayable string.
     * Note this is not a translated string.
     * @param utType          Utterance type.
     * @return                Displayable string for utterance type.
     */
    static QString uttTypeToStr(uttType utType);

    /**
     * Converts an utterance state enumerator to a displayable string.
     * Note this is not a translated string.
     * @param state           Utterance state.
     */
    static QString uttStateToStr(uttState state);
    
    
private:
    /**
    * Determines whether the sentence contains SSML markup.
    */
    bool detectSsml();

private:
    UttPrivate* d;
};

#endif
