/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  
  This class is in charge of getting the messages, warnings and text from
  the queue and calling the plugins to actually speak the texts.
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

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QEvent>

#include <kspeech.h>

// KTTSD includes.
#include "pluginproc.h"
#include "utt.h"

class ConfigData;
class SpeechData;
class Player;
class TalkerMgr;

/**
* Iterator for queue of utterances.
*/
typedef QList<Utt>::iterator uttIterator;

// Timer interval for checking whether audio playback is finished.
const int timerInterval = 500;

/**
 * This class is in charge of getting the messages, warnings and text from
 * the queue and call the plug ins function to actually speak the texts.
 */
class SpeakerPrivate;
class Speaker : public QObject
{
Q_OBJECT

public:
    /**
    * Constructor.
    * @param speechData    Pointer to SpeechData object.
    * @param talkerMgr     Pointer to TalkerMgr object.
    */
    Speaker(
        SpeechData* speechData,
        TalkerMgr* talkerMgr,
        QObject *parent = 0);

    /**
    * Destructor.
    */
    ~Speaker();
    
    /**
    * Sets pointer to the configuration data object.
    */
    void setConfigData(ConfigData* configData);

    /**
    * Tells the thread to exit.
    * TODO: Is this used anymore?
    */
    void requestExit();

    /**
    * Main processing loop.  Dequeues utterances and sends them to the
    * plugins and/or Audio Player.
    */
    void doUtterances();

    /**
    * Determine if kttsd is currently speaking any jobs.
    * @return               True if currently speaking any jobs.
    */
    bool isSpeaking();

    /**
    * Get the job number of the current job speaking.
    * @return               Job number of the current job. 0 if no jobs.
    *
    * @see isSpeakingText
    */
    int getCurrentJobNum();

    /**
    * Delete all utterances for a specified job number.
    * @param jobNum         Job number of the job.
    *
    * If there is another job in the text queue, and it is marked speakable,
    * that job begins speaking.
    */
    void removeJob(int jobNum);

    /**
    * Delete all utterances for all jobs for a specified application.
    * @param appId          DBUS sender ID of the application.
    *
    * If an utterance is playing, it is stopped.
    */
    void removeAllJobs(const QString& appId);

    /**
    * Pause any playing utterances for an application.
    * @param appId         DBUS sender id of the application.
    *
    * The application must be paused before calling this method.
    * See SpeechData::pause().
    */
    void pause(const QString& appId);

    /**
    * Move a job down in the queue so that it is spoken later.
    * @param jobNum         Job number.
    *
    * If the job is currently speaking, it is paused.
    * If the next job in the queue is speakable, it begins speaking.
    */
    void moveJobLater(int jobNum);

    /**
    * Advance or rewind N sentences in a job.
    * @param jobNum         Job number of the job.
    * @param n              Number of sentences to advance (positive) or rewind (negative)
    *                       in the job.
    * @return               Sentence number of the sentence actually moved to.
    *                       Sentence numbers are numbered starting at 1.
    *
    * If no such job, does nothing and returns 0.
    * If n is zero, returns the current sentence number of the job.
    * Does not affect the current speaking/not-speaking state of the job.
    */
    int moveRelSentence(int jobNum, int n);

signals:
    /**
    * Emitted when a marker is processed.
    * Currently only emits mtSentenceBegin and mtSentenceEnd.
    * @param appId         The DBUS sender ID of the application that queued the job.
    * @param jobNum        Job Number of the job emitting the marker.
    * @param markerType    The type of marker.
    *                      Currently either mtSentenceBegin or mtSentenceEnd.
    * @param markerData    Data for the marker.
    *                      Currently, this is the sentence number of the sentence
    *                      begun or ended.  Sentence numbers begin at 1.
    */
    void marker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData);

protected:
    /**
    * Processes events posted by ThreadedPlugIns.
    */
    virtual bool event ( QEvent * e );

private slots:
    /**
    * Received from PlugIn objects when they finish asynchronous synthesis.
    */
    void slotSynthFinished();
    /**
    * Received from PlugIn objects when they finish asynchronous synthesis
    * and audibilizing.
    */
    void slotSayFinished();
    /**
    * Received from PlugIn objects when they asynchronously stopText.
    */
    void slotStopped();
    /**
    * Received from audio stretcher when stretching (speed adjustment) is finished.
    */
    void slotStretchFinished();
    /**
    * Received from transformer (SSMLConvert) when transforming is finished.
    */
    void slotTransformFinished();
    /** Received from PlugIn object when they encounter an error.
    * @param keepGoing               True if the plugin can continue processing.
    *                                False if the plugin cannot continue, for example,
    *                                the speech engine could not be started.
    * @param msg                     Error message.
    */
    void slotError(bool keepGoing, const QString &msg);
    /**
    * Received from Timer when it fires.
    * Check audio player to see if it is finished.
    */
    void slotTimeout();

private:
    /**
    * Converts a plugin state enumerator to a displayable string.
    * @param state           Plugin state.
    * @return                Displayable string for plugin state.
    */
    QString pluginStateToStr(pluginState state);

    /**
    * Converts a job state enumerator to a displayable string.
    * @param state           Job state.
    * @return                Displayable string for job state.
    */
    QString jobStateToStr(int state);

    /**
    * Gets the next utterance of the specified priority to be spoken from
    * speechdata and adds it to the queue.
    * @param requestedPriority     Job priority to check for.
    * @return                      True if one or more utterances were added to the queue.
    *
    * If priority is KSpeech::jpAll, checks for waiting ScreenReaderOutput,
    * Warnings, Messages, or Text, in that order.
    * If Warning or Message and interruption messages have been configured,
    * adds those to the queue as well.
    * Determines which plugin should be used for the utterance.
    */
    bool getNextUtterance(KSpeech::JobPriority requestedPriority);

    /**
    * Given an iterator pointing to the m_uttQueue, deletes the utterance
    * from the queue.  If the utterance is currently being processed by a
    * plugin or the Audio Player, halts that operation and deletes Audio Player.
    * Also takes care of deleting temporary audio file.
    * @param it                      Iterator pointer to m_uttQueue.
    * @return                        Iterator pointing to the next utterance in the
    *                                queue, or m_uttQueue.end().
    */
    uttIterator deleteUtterance(uttIterator it);

    /**
    * Given an iterator pointing to the m_uttQueue, starts playing audio if
    *   1) An audio file is ready to be played, and
    *   2) It is not already playing.
    * If another audio player is already playing, pauses it before starting
    * the new audio player.
    * @param it                      Iterator pointer to m_uttQueue.
    * @return                        True if an utterance began playing or resumed.
    */
    bool startPlayingUtterance(uttIterator it);

    /**
    * Delete any utterances in the queue with this jobNum.
    * @param jobNum          The Job Number of the utterance(s) to delete.
    * If currently processing any deleted utterances, stop them.
    */
    void deleteUtteranceByJobNum(int jobNum);

    /**
    * Takes care of emitting reading interrupted/resumed and sentence started signals.
    * Should be called just before audibilizing an utterance.
    * @param it                      Iterator pointer to m_uttQueue.
    */
    void prePlaySignals(uttIterator it);

    /**
    * Takes care of emitting sentenceFinished signal.
    * Should be called immediately after an utterance has completed playback.
    * @param it                      Iterator pointer to m_uttQueue.
    */
    void postPlaySignals(uttIterator it);

    /**
    * Constructs a temporary filename for plugins to use as a suggested filename
    * for synthesis to write to.
    * @return                        Full pathname of suggested file.
    */
    QString makeSuggestedFilename();

    /**
    * Creates and returns a player object based on user option.
    */
    Player* createPlayerObject();

private:
    SpeakerPrivate* d;
};

#endif // _SPEAKER_H_
