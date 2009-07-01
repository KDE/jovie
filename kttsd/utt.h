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

#ifndef UTT_H
#define UTT_H

// Qt includes.
#include <QString>

class SSMLConvert;
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
        usSaying,                    /**< Plugin is synthesizing and audibilizing. */
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
        const QString& sentence);

    /**
    * Constructs a sound icon type of utterance.
    */
    Utt(uttType utType, const QString& appId);

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

#endif // UTT_H
