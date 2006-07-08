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

// Qt includes.

// KDE includes.

// KTTS includes.
#include <pluginproc.h>
#include <utils.h>

// KTTSD includes.
#include "speechjob.h"

// Utt includes.
#include "utt.h"

class UttPrivate
{
public:
    UttPrivate() :
        utType(Utt::utText),
        appId(QString()),
        job(NULL),
        sentence(QString()),
        seq(0),
        isSsml(false),
        state(Utt::usNone),
        transformer(NULL),
        plugin(NULL),
        audioStretcher(NULL),
        audioUrl(QString()),
        audioPlayer(NULL) {}
        
    ~UttPrivate() {}
    
    friend class Utt;
    
protected:
    Utt::uttType utType;         /* The type of utterance (text, msg, screen reader) */
    QString appId;               /* The DBUS sender ID of the application for this utterance. */
    SpeechJob* job;              /* The job from which utterance came. */
    QString sentence;            /* Text of the utterance. */
    int seq;                     /* Sequence number of the sentence. */
    bool isSsml;                 /* True if the utterance contains SSML markup. */
    Utt::uttState state;         /* Processing state of the utterance. */
    SSMLConvert* transformer;    /* XSLT transformer. */
    PlugInProc* plugin;          /* The plugin that synthesizes the utterance. */
    Stretcher* audioStretcher;   /* Audio stretcher object.  Adjusts speed. */
    QString audioUrl;            /* Filename containing synthesized audio.  Null if
                                    plugin has not yet synthesized the utterance, or if
                                    plugin does not support synthesis. */
    Player* audioPlayer;         /* The audio player audibilizing the utterance.  Null
                                    if not currently audibilizing or if plugin doesn't
                                    support synthesis. */
};

Utt::Utt() : d(new UttPrivate()){}

Utt::Utt(uttType utType,
    const QString& appId,
    SpeechJob* job,
    const QString& sentence,
    PlugInProc* pluginproc) :
    
    d(new UttPrivate())
{
    d->utType = utType;
    d->appId = appId;
    d->job = job;
    if (job) job->incRefCount();
    d->sentence = sentence;
    d->plugin = pluginproc;
    if (job)
        d->seq = job->seq();
    if (sentence.isEmpty())
        d->isSsml = detectSsml();
    setInitialState();
}

/**
* Constructs a sound icon type of utterance.
*/
Utt::Utt(uttType utType, const QString& appId, const QString& audioUrl) :
    d(new UttPrivate())
{
    d->utType = utType;
    d->appId = appId;
    d->audioUrl = audioUrl;
    setInitialState();    
}

Utt::~Utt() {
    if (d->job)
        d->job->decRefCount();
    delete d;
}
    
Utt::uttType Utt::utType() const { return d->utType; }
void Utt::setUtType(uttType type) { d->utType = type; }
QString Utt::appId() const { return d->appId; }
void Utt::setAppId(const QString& appId) { d->appId = appId; }
SpeechJob* Utt::job() const { return d->job; }
void Utt::setJob(SpeechJob* job)
{
    if (d->job)
        d->job->decRefCount();
    d->job = job;
    if (job)
        job->incRefCount();
}
QString Utt::sentence() const { return d->sentence; }
void Utt::setSentence(const QString& sentence)
{
    d->sentence = sentence;
    d->isSsml = detectSsml();
}
int Utt::seq() const { return d->seq; }
void Utt::setSeq(int seq) { d->seq = seq; }
bool Utt::isSsml() const { return d->isSsml; }
void Utt::setIsSsml(bool isSsml) { d->isSsml = isSsml; }
Utt::uttState Utt::state() const { return d->state; }

void Utt::setState(uttState state)
{
    bool stateChanged = (state != d->state);
    d->state = state;
    if (stateChanged && d->job)
        switch (state)
        {
            case usNone:
            case usWaitingTransform:
            case usTransforming:
            case usWaitingSay:
            case usWaitingSynth:
                break;
            case usSaying:
                d->job->setSentenceNum(d->seq);
                d->job->setState(KSpeech::jsSpeaking);
                break;
            case usSynthing:
            case usSynthed:
            case usStretching:
            case usStretched:
                break;
            case usPlaying:
                d->job->setSentenceNum(d->seq);
                d->job->setState(KSpeech::jsSpeaking);
                break;
            case usPaused:
                d->job->setState(KSpeech::jsPaused);
                break;
            case usPreempted:
                d->job->setState(KSpeech::jsInterrupted);
                break;
            case usFinished:
                if (d->seq == d->job->sentenceCount())
                    d->job->setState(KSpeech::jsFinished);
                break;
        };
}

SSMLConvert* Utt::transformer() const { return d->transformer; }
void Utt::setTransformer(SSMLConvert* transformer) { d->transformer = transformer; }
PlugInProc* Utt::plugin() const { return d->plugin; }
void Utt::setPlugin(PlugInProc* plugin) { d->plugin = plugin; }
Stretcher* Utt::audioStretcher() const { return d->audioStretcher; }
void Utt::setAudioStretcher(Stretcher* stretcher) { d->audioStretcher = stretcher; }
QString Utt::audioUrl() const { return d->audioUrl; }
void Utt::setAudioUrl(const QString& audioUrl) { d->audioUrl = audioUrl; }

Player* Utt::audioPlayer() const { return d->audioPlayer; }
void Utt::setAudioPlayer(Player* player) { d->audioPlayer = player; }

bool Utt::detectSsml()
{
    if (d->sentence.isEmpty())
        return false;
    else
        return KttsUtils::hasRootElement( d->sentence, "speak" );
}

void Utt::setInitialState()
{
    if ((d->state != usTransforming) && d->isSsml)
    {
        d->state = usWaitingTransform;
        return;
    }
    if (d->plugin) {
        if (d->plugin->supportsSynth())
            d->state = usWaitingSynth;
        else
            d->state = usWaitingSay;
    } else {
        if (!d->audioUrl.isEmpty() && (utInterruptSnd == d->utType || utResumeSnd == d->utType))
            d->state = usStretched;
    }
}

/*static*/
QString Utt::uttTypeToStr(uttType utType)
{
    switch (utType)
    {
        case utText:         return "utText";
        case utInterruptMsg: return "utInterruptMsg";
        case utInterruptSnd: return "utInterruptSnd";
        case utResumeMsg:    return "utResumeMsg";
        case utResumeSnd:    return "utResumeSnd";
        case utMessage:      return "utMessage";
        case utWarning:      return "utWarning";
        case utScreenReader: return "utScreenReader";
    }
    return QString();
}

/*static*/ 
QString Utt::uttStateToStr(uttState state)
{
    switch (state)
    {
        case usNone:                return "usNone";
        case usWaitingTransform:    return "usWaitingTransform";
        case usTransforming:        return "usTransforming";
        case usWaitingSay:          return "usWaitingSay";
        case usWaitingSynth:        return "usWaitingSynth";
        case usSaying:              return "usSaying";
        case usSynthing:            return "usSynthing";
        case usSynthed:             return "usSynthed";
        case usStretching:          return "usStretching";
        case usStretched:           return "usStretched";
        case usPlaying:             return "usPlaying";
        case usPaused:              return "usPaused";
        case usPreempted:           return "usPreempted";
        case usFinished:            return "usFinished";
    }
    return QString();
}
