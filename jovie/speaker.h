/***************************************************** vim:set ts=4 sw=4 sts=4:
  Speaker class.
  
  This class is in charge of getting the messages, warnings and text from
  the queue and calling speech-dispatcher to actually speak the texts.
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

#ifndef SPEAKER_H
#define SPEAKER_H

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QEvent>

#include <kspeech.h>

// KTTSD includes.
#include "config-jovie.h"
#ifdef SPEECHD_FOUND
#include <libspeechd.h>
#endif

#ifdef OPENTTS_FOUND
#include <opentts/libopentts.h>
#endif

#include "filtermgr.h"
#include "appdata.h"
#include "speechjob.h"

/**
 * Struct used to keep a pool of FilterMgr objects.
 */
//struct PooledFilterMgr {
//    FilterMgr* filterMgr;       /* The FilterMgr object. */
//    bool busy;                  /* True if the FilterMgr is busy. */
//    SpeechJob* job;             /* The job the FilterMgr is filtering. */
//    TalkerCode* talkerCode;     /* TalkerCode object passed to FilterMgr. */
//};

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
    * singleton accessor
    */
    static Speaker * Instance();

    static void speechdCallback(size_t msg_id, size_t client_id, SPDNotificationType type);

    /**
    * Destructor.
    */
    ~Speaker();
    
    /**
    * (re)initializes the filtermgr
    */
    void init();

    /**
    * Tells the thread to exit.
    * TODO: Is this used anymore?
    */
    void requestExit();

    /**
    * Determine if kttsd is currently speaking any jobs.
    * @return               True if currently speaking any jobs.
    */
    bool isSpeaking();

    /**
    * Get application data.
    * If this is a new application, a new AppData object is created and initialized
    * with defaults.
    * Caller may set properties, but must not delete the returned AppData object.
    * Use releaseAppData instead.
    * @param appId          The DBUS senderId of the application.
    */
    AppData* getAppData(const QString& appId) const;

    /**
    * Queue and start a speech job.
    * @param appId          The DBUS senderId of the application.
    * @param text           The text to be spoken.
    * @param sayOptions        Option flags.  @see SayOptions.  Defaults to KSpeech::soNone.
    *
    * Based on the options, the text may contain the text to be spoken, with or withou
    * markup, or it may contain characters to be spelled out, or it may contain
    * the symbolic name of a keyboard key, or it may contain the name of a sound
    * icon.
    *
    * The job is given the applications current defaultPriority.  @see defaultPriority.
    * The job is assigned the applications current defaultTalker.  @see defaultTalker.
    */
    int say(const QString& appId, const QString& text, int sayOptions);

    /**
    * Change the talker for a job.
    * @param jobNum         Job number of the job.
    * @param talker         New code for the talker to do speaking.  Example "en".
    *                       If NULL, defaults to the user's default talker.
    *                       If no plugin has been configured for the specified Talker code,
    *                       defaults to the closest matching talker.
    *
    * @see talker
    */
    void setTalker(int jobNum, const QString &talker);

    /**
    * Given an appId, returns the last (most recently queued) Job Number with that appId,
    * or if no such job, the Job Number of the last (most recent) job in the queue.
    * @param appId          The DBUS senderId of the application.
    * @return               Job Number.
    * If no such job, returns 0.
    * If appId is NULL, returns the Job Number of the last job in the queue.
    */
    int findJobNumByAppId(const QString& appId) const;

    /**
    * Given an appId, returns the last (most recently queued) job with that appId.
    * @param appId          The DBUS senderId of the application.
    * @return               Pointer to the job.
    * If no such job, returns 0.
    * If appId is NULL, returns the last job in the queue.
    */
    SpeechJob* findLastJobByAppId(const QString& appId) const;

    /**
    * Return true if the application is paused.
    */
    bool isApplicationPaused(const QString& appId);

    /**
     * Stops the message currently being spoken on a given connection.
     * If there is no message being spoken, does nothing. (It doesn't touch the messages waiting in queues).
     */
    void stop();
    
    /**
     * Stops the currently spoken message from this connection (if there is any) and discards all the
     * queued messages from this connection.
     */
    void cancel();
    
    /**
    * Pauses the speech.
    */
    void pause();
    
    /**
    * Resumes the speech.
    */
    void resume();

    /**
     * Get the output modules available from speech-dispatcher
     */
    QStringList outputModules();
    
    /**
     * Get the languages available for the given output module
     */
    QStringList languagesByModule(const QString & module);

    void setSpeed(int speed);
    void setPitch(int pitch);
    void setVolume(int volume);
    
    void setOutputModule(const QString & module);
    void setLanguage(const QString & language);
    void setVoiceType(int voiceType);

    int speed();
    int pitch();
    int volume();
    QString outputModule();
    QString language();
    int voiceType();

signals:
    /**
     * This signal is emitted when a new job coming in is filtered (or not filtered if no filters
     * are on).
     * @param prefilterText     The text of the speech job
     * @param postfilterText    The text of the speech job after any filters have been applied
     */
    void newJobFiltered(const QString &prefilterText, const QString &postfilterText);

private slots:
    void slotServiceUnregistered(const QString& serviceName);

private:
    /**
    * Constructor.
    */
    Speaker();

    /**
    * get the list of available modules that are configured in speech-dispatcher
    */
    QStringList moduleNames();

    /**
    * Determines whether the given text is SSML markup.
    */
    bool isSsml(const QString &text);

    /**
    * Parses a block of text into sentences using the application-specified regular expression
    * or (if not specified), the default regular expression.
    * @param text           The message to be spoken.
    * @param appId          The DBUS senderId of the application.
    * @return               List of parsed sentences.
    */
    QStringList parseText(const QString &text, const QString &appId);

private:
    SpeakerPrivate* d;
    static Speaker * m_instance;
};

#endif // SPEAKER_H
