/*************************************************** vim:set ts=4 sw=4 sts=4:
  This class contains a single speech job.
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

#include "speechjob.h"

/* -------------------------------------------------------------------------- */

class SpeechJobPrivate
{
public:
    SpeechJobPrivate(KSpeech::JobPriority priority) :
        jobNum(0),
        appId(),
        jobPriority(priority),
        talker(),
        state(KSpeech::jsQueued),
        sentences(),
        sentenceNum(0),
        seq(0),
        refCount(0)
    {};
    ~SpeechJobPrivate() {};
    friend class SpeechJob;

protected:
    /** Job number. */
    int jobNum;
    /** DBUS senderId of the application that requested the speech job. */
    QString appId;
    /** Priority of Job.  Text, Message, Warning, or Screen Reader Output */
    KSpeech::JobPriority jobPriority;
    /** Requested Talker code in which to speak the text. */
    QString talker;
    /** Job state. */
    KSpeech::JobState state;
    /** List of sentences in the job. */
    QStringList sentences;
    /** Current sentence being spoken.
        The first sentence is at seq 1, so if 0, not speaking. */
    int sentenceNum;
    /** Current sentence begin synthesized. */
    int seq;
    /** Reference count.  Used to delete job when all utterances
        have been deleted. **/
    int refCount;
};

/* -------------------------------------------------------------------------- */

SpeechJob::SpeechJob(KSpeech::JobPriority priority) :
    d(new SpeechJobPrivate(priority))
{
}

SpeechJob::~SpeechJob()
{
    emit jobStateChanged(d->appId, d->jobNum, KSpeech::jsDeleted);
    delete d;
}

int SpeechJob::jobNum() const { return d->jobNum; }
void SpeechJob::setJobNum(int jobNum) { d->jobNum = jobNum; }

QString SpeechJob::appId() const { return d->appId; }
void SpeechJob::setAppId(const QString &appId) { d->appId = appId; }

KSpeech::JobPriority SpeechJob::jobPriority() const { return d->jobPriority; }
void SpeechJob::setJobPriority(KSpeech::JobPriority priority) { d->jobPriority = priority; }

QString SpeechJob::talker() const { return d->talker; }
void SpeechJob::setTalker(const QString &talker) { d->talker = talker; }

KSpeech::JobState SpeechJob::state() const { return d->state; }
void SpeechJob::setState(KSpeech::JobState state)
{
    bool emitSignal = (state != d->state);
    d->state = state;
    if (emitSignal)
        emit jobStateChanged(d->appId, d->jobNum, state);
}

QStringList SpeechJob::sentences() const { return d->sentences; }
void SpeechJob::setSentences(const QStringList &sentences) { d->sentences = sentences; }

int SpeechJob::sentenceCount() const { return d->sentences.count(); }
int SpeechJob::sentenceNum() const { return d->sentenceNum; }
void SpeechJob::setSentenceNum(int sentenceNum) { d->sentenceNum = sentenceNum; }

int SpeechJob::seq() const { return d->seq; }
void SpeechJob::setSeq(int seq) { d->seq = seq; }

int SpeechJob::refCount() const { return d->refCount; }
void SpeechJob::incRefCount() { ++d->refCount; }
void SpeechJob::decRefCount() { --d->refCount; }


QString SpeechJob::getNextSentence() {
    int newSeq = d->seq + 1;
    if (newSeq > d->sentences.count())
        return QString();
    else {
        QString sentence = d->sentences[d->seq];
        d->seq = newSeq;
        return sentence;
    }
}

QByteArray SpeechJob::serialize() const
{
    QByteArray temp;
    QDataStream stream(&temp, QIODevice::WriteOnly);
    stream << (qint32)d->jobPriority;
    stream << (qint32)d->state;
    stream << d->appId;
    stream << d->talker;
    stream << (qint32)d->sentenceNum;
    stream << (qint32)(d->sentences.count());
    return temp;
}

/*static*/
QString SpeechJob::jobStateToStr(KSpeech::JobState state)
{
    switch ( state )
    {
        case KSpeech::jsQueued:      return "jsQueued";
        case KSpeech::jsFiltering:   return "jsFiltering";
        case KSpeech::jsSpeakable:   return "jsSpeakable";
        case KSpeech::jsSpeaking:    return "jsSpeaking";
        case KSpeech::jsPaused:      return "jsPaused";
        case KSpeech::jsInterrupted: return "jsInterrupted";
        case KSpeech::jsFinished:    return "jsFinished";
        case KSpeech::jsDeleted:     return "jsDeleted";
    }
    return QString();
}

#include "speechjob.moc"

