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

#ifndef SPEECHJOB_H
#define SPEECHJOB_H

// Qt includes.
#include <QtCore/QList>
#include <QtCore/QByteArray>

// KDE includes.
#include <kspeech.h>

/**
 * @class SpeechJob
 *
 * Object containing a speech job.
 */

class SpeechJobPrivate;
class SpeechJob : public QObject {

    Q_OBJECT
    
    Q_PROPERTY(int jobNum READ jobNum WRITE setJobNum)
    Q_PROPERTY(QString appId READ appId WRITE setAppId)
    Q_PROPERTY(KSpeech::JobPriority jobPriority READ jobPriority WRITE setJobPriority)
    Q_PROPERTY(QString talker READ talker WRITE setTalker)
    Q_PROPERTY(KSpeech::JobState state READ state WRITE setState)
    Q_PROPERTY(QStringList sentences READ sentences WRITE setSentences)
    Q_PROPERTY(int sentenceCount READ sentenceCount)
    Q_PROPERTY(int sentenceNum READ sentenceNum WRITE setSentenceNum)
    Q_PROPERTY(int seq READ seq WRITE setSeq)
    Q_PROPERTY(QString getNextSentence READ getNextSentence)
    Q_PROPERTY(QByteArray serialize READ serialize)

public:
    SpeechJob(KSpeech::JobPriority priority=KSpeech::jpText);
    ~SpeechJob();
    
    /** Job number. */
    int jobNum() const;
    void setJobNum(int jobNum);
    /** DBUS senderId of the application that requested the speech job. */
    QString appId() const;
    void setAppId(const QString &appId);
    /** Type of job.  Text, Message, Warning, or Screen Reader Output */
    KSpeech::JobPriority jobPriority() const;
    void setJobPriority(KSpeech::JobPriority jobPriority);
    /** Requested Talker code in which to speak the text. */
    QString talker() const;
    void setTalker(const QString &talker);
    /** Job state. */
    KSpeech::JobState state() const;
    void setState(KSpeech::JobState state);
    /** List of sentences in the job. */
    QStringList sentences() const;
    void setSentences(const QStringList &sentences);
    /** Count of sentences in the job. */
    int sentenceCount() const;
    /** Current sentence begin spoken.
        The first sentence is at 1, so if 0, not speaking. */
    int sentenceNum() const;
    void setSentenceNum(int sentenceNum);
    /** Current sentence being synthesized.  First sentence is 1. */
    int seq() const;
    void setSeq(int seq);
    int refCount() const;
    void incRefCount();
    void decRefCount();

    /**
     * Returns the next sentence in the Job.
     * Increments sentenceNum.
     * If we run out of sentences, returns QString().
     */
    QString getNextSentence();

    /**
     * Converts the job into a byte stream.
     * @return               A QDataStream containing information about the job.
     *                       Blank if no such job.
     *
     * The stream contains the following elements:
     *   - int priority      Job Priority
     *   - int state         Job state.
     *   - QString appId     DBUS senderId of the application that requested the speech job.
     *   - QString talker    Language code in which to speak the text.
     *   - int sentenceNum   Current sentence being spoken.  Sentences are numbered starting at 1.
     *   - int sentenceCount Total number of sentences in the job.
     *
     * Note that sequence numbers apply to the entire job.
     * They do not start from 1 at the beginning of each part.
     *
     * The following sample code will decode the stream:
           @verbatim
             QByteArray jobInfo = serialize();
             QDataStream stream(jobInfo, QIODevice::ReadOnly);
             qint32 priority;
             qint32 state;
             QString appId;
             QString talker;
             qint32 sentenceNum;
             qint32 sentenceCount;
             stream >> priority;
             stream >> state;
             stream >> appId;
             stream >> talker;
             stream >> sentenceNum;
             stream >> sentenceCount;
           @endverbatim
     */
    QByteArray serialize() const;
    
    /**
     * Converts a job state enumerator to a displayable string.
     * @param state           Job state.
     * @return                Displayable string for job state.
     */
    static QString jobStateToStr(KSpeech::JobState state);
    
Q_SIGNALS:
    void jobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state);
    
private:
    SpeechJobPrivate* d;
};

#endif      // SPEECHJOB_H
