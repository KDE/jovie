/***************************************************** vim:set ts=4 sw=4 sts=4:
  Jovie (KDE Text to Speech)
  
  The KDE Text-to-Speech API.
  ------------------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ******************************************************************************/

#ifndef JOVIE_H
#define JOVIE_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>

#include <kspeech.h>

class JoviePrivate;

/**
* Jovie -- the KDE Text-to-Speech API.
*
* Note: Applications do not use this class directly.
*/
class Jovie : public QObject
{
Q_OBJECT
public:
    /**
    * singleton accessor
    */
    static Jovie * Instance();

    /**
    * Destructor.
    */
    ~Jovie();

public: // PROPERTIES
    Q_PROPERTY(bool isSpeaking READ isSpeaking)
    Q_PROPERTY(QString version READ version)

public Q_SLOTS: // METHODS
    /**
    * Returns true if KTTSD is currently speaking.
    * @return               True if currently speaking.
    */
    bool isSpeaking() const;

    /**
    * Returns the version number of KTTSD.
    * @return               Version number string.
    */
    QString version() const;

    /**
    * Returns the friendly display name for the application.
    * @return               Application display name.
    *
    * If application has not provided a friendly name, the DBUS connection name is returned.
    */
    QString applicationName();

    /**
    * Sets a friendly display name for the application.
    * @param applicationName    Friendly name for the application.
    */    
    void setApplicationName(const QString &applicationName);

    /**
    * Returns the default talker for the application.
    * @return                   Talker.
    *
    * The default is "", which uses the default talker configured by user.
    */
    QString defaultTalker();

    /**
    * Sets the default talker for the application.
    * @param defaultTalker      Default talker.  Example: "en".
    */
    void setDefaultTalker(const QString &defaultTalker);

    /**
    * Returns the default priority for speech jobs submitted by the application.
    * @return                   Default job priority.
    *
    * @see JobPriority
    */
    int defaultPriority();

    /**
    * Sets the default priority for speech jobs submitted by the application.
    * @param defaultPriority    Default job priority.
    *
    * @see JobPriority
    */
    void setDefaultPriority(int defaultPriority);

    /**
    * Returns the regular expression used to perform Sentence Boundary
    * Detection (SBD) for the application.
    * @return               Sentence delimiter regular expression.
    *
    * The default sentence delimiter is
      @verbatim
          ([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))
      @endverbatim
    *
    * Note that backward slashes must be escaped.
    *
    * @see sentenceparsing
    */
    QString sentenceDelimiter();

    /**
    * Sets the regular expression used to perform Sentence Boundary
    * Detection (SBD) for the application.
    * @param sentenceDelimiter  Sentence delimiter regular expression.
    */
    void setSentenceDelimiter(const QString &sentenceDelimiter);

    /**
    * Returns whether speech jobs for this application are filtered using configured
    * filter plugins.
    * @return               True if filtering is on.
    *
    * Filtering is on by default.
    */
    bool filteringOn();

    /**
    * Sets whether speech jobs for this application are filtered using configured
    * filter plugins.
    * @param filteringOn    True to set filtering on.
    */
    void setFilteringOn(bool filteringOn);

    /**
    * Returns whether KTTSD will automatically attempt to configure new
    * talkers to meet required talker attributes.
    * @return               True if KTTSD will autoconfigure talkers.
    *
    * @see defaultTalker
    */
    bool autoConfigureTalkersOn();
    
    /** Sets whether KTTSD will automatically attempt to configure new
    * talkers to meet required talker attributes.
    * @param autoConfigureTalkersOn True to enable auto configuration.
    */
    void setAutoConfigureTalkersOn(bool autoConfigureTalkersOn);

    /**
    * Returns whether application is paused.
    * @return               True if application is paused.
    */
    bool isApplicationPaused();
    
    /**
    * Returns the full path name to XSLT file used to convert HTML
    * markup to speakable form.
    * @return               XSLT filename.
    */
    QString htmlFilterXsltFile();

    /**
    * Sets the full path name to an XSLT file used to convert HTML
    * markup to speakable form.
    * @param htmlFilterXsltFile XSLT filename.
    */
    void setHtmlFilterXsltFile(const QString &htmlFilterXsltFile);

    /**
    * Returns the full path name to XSLT file used to convert SSML
    * markup to a speakable form.
    * @return                   XSLT filename.
    */
    QString ssmlFilterXsltFile();

    /**
    * Sets the full path name to XSLT file used to convert SSML
    * markup to a speakable form.
    * @param ssmlFilterXsltFile XSLT filename.
    */
    void setSsmlFilterXsltFile(const QString &ssmlFilterXsltFile);

    /**
    * Returns whether this is a System Manager application.
    * @return                   True if the application is a System Manager.
    */
    bool isSystemManager();

    /**
    * Sets whether this is a System Manager application.
    * @param isSystemManager    True if this is a System Manager.
    *
    * System Managers are used to control and configure overall TTS output.
    * When True, many of the Jovie methods alter their behavior.
    */
    void setIsSystemManager(bool isSystemManager);

    /**
    * Creates and starts a speech job.  The job is created at the application's
    * default job priority using the default talker.
    * @param text               The text to be spoken.
    * @param options            Speech options.
    * @return                   Job Number for the new job.
    *
    * @see JobPriority
    * @see SayOptions
    */
    int say(const QString &text, int options);

    /**
    * Creates and starts a speech job from a specified file.
    * @param filename           Full path name of the file.
    * @param encoding           The encoding of the file.  Default UTF-8.
    * @return                   Job Number for the new job.
    *
    * The job is spoken using application's default talker.
    * @see defaultTalker
    *
    * Plain text is parsed into individual sentences using the current sentence delimiter.
    * Call @ref setSentenceDelimiter to change the sentence delimiter prior to calling sayFile.
    * Call @ref getSentenceCount to retrieve the sentence count after calling sayFile.
    *
    * The text may contain speech mark language, such as SMML,
    * provided that the speech plugin/engine support it.  In this case,
    * sentence parsing follows the semantics of the markup language.
    */
    int sayFile(const QString &filename, const QString &encoding);

    /**
    * Submits a speech job from the contents of the clipboard.
    * The job is spoken using application's default talker.
    * @return                   Job Number for the new job.
    *
    * @see defaultTalker
    */
    int sayClipboard();

    /**
    * Pauses speech jobs belonging to the application.
    * When called by a System Manager, pauses all jobs of all applications.
    */
    void pause();

    /**
    * Resumes speech jobs belonging to the application.
    * When called by a System Manager, resumes all jobs of all applications.
    */
    void resume();

    
    void stop();
    void cancel();

    QStringList outputModules();
    QStringList languagesByModule(const QString & module);

    // runtime slots to change the current speech configuration
    void setSpeed(int speed);
    int speed();
    void setPitch(int pitch);
    int pitch();
    void setVolume(int volume);
    int volume();

    void setOutputModule(const QString & module);
    void setLanguage(const QString & language);
    void setVoiceType(int voiceType);
    int voiceType();

    /**
    * Removes the specified job.  If the job is speaking, it is stopped.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    */
    void removeJob(int jobNum);

    /**
    * Removes all jobs belonging to the application.
    * When called from a System Manager, removes all jobs of all applications.
    */
    void removeAllJobs();

    /**
    * Returns the number of sentences in a job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @return                   Number of sentences in the job.
    */
    int getSentenceCount(int jobNum);

    /**
    * Returns the job number of the currently speaking job (any application).
    * @return                    Job Number
    */
    int getCurrentJob();

    /**
    * Returns the number of jobs belonging to the application
    * with the specified job priority.
    * @param priority           Job Priority.
    * @return                   Number of jobs.
    *
    * If priority is KSpeech::jpAll, returns the number of jobs belonging
    * to the application (all priorities).
    *
    * When called from a System Manager, returns count of all jobs of the
    * specified priority for all applications.
    *
    * @see JobPriority
    */
    int getJobCount(int priority);

    /**
    * Returns a list job numbers for the jobs belonging to the
    * application with the specified priority.
    * @param priority           Job Priority.
    * @return                   List of job numbers.  Note that the numbers
    *                           are strings.
    *
    * If priority is KSpeech::jpAll, returns the job numbers belonging
    * to the application (all priorities).
    *
    * When called from a System Manager, returns job numbers of the
    * specified priority for all applications.
    *
    * @see JobPriority
    */
    QStringList getJobNumbers(int priority);

    /**
    * Returns the state of a job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @return                   Job state.
    *
    * @see JobState
    */
    int getJobState(int jobNum);

    /**
    * Get information about a job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @return               A QDataStream containing information about the job.
    *                       Blank if no such job.
    *
    * The stream contains the following elements:
    *   - int priority      Job Type.
    *   - int state         Job state.
    *   - QString appId     DBUS senderId of the application that requested the speech job.
    *   - QString talker    Talker code as requested by application.
    *   - int sentenceNum   Current sentence being spoken.  Sentences are numbered starting at 1.
    *   - int sentenceCount Total number of sentences in the job.
    *   - QString applicationName Application's friendly name (if provided by app)
    *
    * If the job is currently filtering, waits for that to finish before returning.
    *
    * The following sample code will decode the stream:
            @verbatim
                QByteArray jobInfo = m_kspeech->getJobInfo(jobNum);
                if (jobInfo != QByteArray()) {
                    QDataStream stream(&jobInfo, QIODevice::ReadOnly);
                    qint32 priority;
                    qint32 state;
                    QString talker;
                    qint32 sentenceNum;
                    qint32 sentenceCount;
                    QString applicationName;
                    stream >> priority;
                    stream >> state;
                    stream >> appId;
                    stream >> talker;
                    stream >> sentenceNum;
                    stream >> sentenceCount;
                    stream >> applicationName;
                };
            @endverbatim
    */
    QByteArray getJobInfo(int jobNum);

    /**
    * Return a sentence of a job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @param sentenceNum    Sentence Number.  Sentence numbers start at 1.
    * @return               The specified sentence in the specified job.  If no such
    *                       job or sentence, returns "".
    */
    QString getJobSentence(int jobNum, int sentenceNum);

    /**
    * Return a list of full Talker Codes for configured talkers.
    * @return               List of Talker codes.
    */
    QStringList getTalkerCodes();

    /**
    * Given a talker, returns the Talker ID for the talker.
    * that will speak the job.
    * @param talker         Talker.  Example: "en".
    * @return               Talker ID.  A Talker ID is an internally-assigned
    *                       identifier for a talker.
    *
    * This method is normally used only by System Managers.
    */
    QString talkerToTalkerId(const QString &talker);

    /**
    * Returns a bitarray giving the capabilities of a talker.
    * @param talker         Talker.  Example: "en".
    * @return               A word with bits set according to the capabilities
    *                       of the talker.
    *
    * @see TalkerCapabilities1
    * @see getTalkerCapabilities2
    */
    int getTalkerCapabilities1(const QString &talker);

    /**
    * Returns a bitarray giving the capabilities of a talker.
    * @param talker         Talker.  Example: "en".
    * @return               A word with bits set according to the capabilities
    *                       of the talker.
    *
    * @see TalkerCapabilities2
    * @see getTalkerCapabilities1
    */
    int getTalkerCapabilities2(const QString &talker);

    /**
    * Return a list of the voice codes of voices available in the synthesizer
    * corresponding to a talker.
    * @param talker         Talker.  Example: "synthesizer='Festival'"
    * @return               List of voice codes.
    *
    * Voice codes are synthesizer specific.
    */
    QStringList getTalkerVoices(const QString &talker);

    /**
    * Change the talker of an already-submitted job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @param talker             Desired new talker.
    */
    void changeJobTalker(int jobNum, const QString &talker);

    /**
    * Move a job one position down in the queue so that it is spoken later.
    * If the job is already speaking, it is stopped and will resume
    * when processing next gets to it.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    *
    * Since there is only one ScreenReaderOutput, this method is meaningless
    * for ScreenReaderOutput jobs.
    */    
    void moveJobLater(int jobNum);

    /**
    * Advance or rewind N sentences in a job.
    * @param jobNum             Job Number.  If 0, the last job submitted by
    *                           the application.
    * @param n              Number of sentences to advance (positive) or rewind (negative)
    *                       in the job.
    * @return               Number of the sentence actually moved to.  Sentence numbers
    *                       are numbered starting at 1.
    *
    * If no such job, does nothing and returns 0.
    * If n is zero, returns the current sentence number of the job.
    * Does not affect the current speaking/not-speaking state of the job.
    *
    * Since ScreenReaderOutput jobs are not split into sentences, this method
    * is meaningless for ScreenReaderOutput jobs.
    */    
    int moveRelSentence(int jobNum, int n);

    /**
    * Display the KttsMgr program so that user can configure KTTS options.
    * Only one instance of KttsMgr is displayed.
    */
    void showManagerDialog();

    /**
    * Shuts down KTTSD.  Do not call this!
    */
    void kttsdExit();

    /**
    * post ctor helper method that instantiates the dbus adaptor class, and registers
    */
    void init();

    /**
    * Cause KTTSD to re-read its configuration.
    */
    void reinit();

    /** Called by DBusAdaptor so that KTTSD knows the application that
    * called it.
    * @param appId              DBUS connection name that called KSpeech.
    */
    void setCallingAppId(const QString& appId);

Q_SIGNALS: // SIGNALS
    /**
    * This signal is emitted when KTTSD starts.
    */
    void kttsdStarted();

    /**
    * This signal is emitted just before KTTS exits.
    */
    void kttsdExiting();

    /**
    * This signal is emitted each time the state of a job changes.
    * @param appId              The DBUS connection name of the application that
    *                           submitted the job.
    * @param jobNum             Job Number.
    * @param state              Job state.
    */
    void jobStateChanged(const QString &appId, int jobNum, int state);

    /**
    * This signal is emitted when a marker is processed.
    * Currently only emits mtSentenceBegin and mtSentenceEnd.
    * @param appId         The DBUS connection name of the application that submitted the job.
    * @param jobNum        Job Number of the job emitting the marker.
    * @param markerType    The type of marker.
    *                      Currently either mtSentenceBegin or mtSentenceEnd.
    * @param markerData    Data for the marker.
    *                      Currently, this is the sentence number of the sentence
    *                      begun or ended.  Sentence numbers begin at 1.
    */
    void marker(const QString &appId, int jobNum, int markerType, const QString &markerData);

private slots:
    void slotJobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state);
    void slotMarker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData);
    void slotFilteringFinished();
    
private:
    /**
    * The DBUS connection name of the last application that called KTTSD.
    */
    QString callingAppId();
    
    /*
    * Checks if KTTSD is ready to speak and at least one talker is configured.
    * If not, user is prompted to display the configuration dialog.
    */
    bool ready();
    
    /**
    * Create and initialize the Configuration Data object.
    */
    bool initializeConfigData();

    /*
    * Create and initialize the SpeechData object.
    * Deprecated, remove in KDE 5
    */
    bool initializeSpeechData();

    /*
    * Create and initialize the TalkerMgr object.
    */
    bool initializeTalkerMgr();

    /*
    * Create and initialize the speaker.
    */
    bool initializeSpeaker();

    /*
    * If a job number is 0, returns the default job number for a command.
    * Returns the job number of the last job queued by the application, or if
    * no such job, the current job number.
    * @param jobNum          The job number passed in the DBUS message.  May be 0.
    * @return                Default job number.  0 if no such job.
    */
    int applyDefaultJobNum(int jobNum);

    /*
    * Announces an event to kDebug.
    */
    void announceEvent(const QString& slotName, const QString& eventName);
    void announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
        int jobNum, KSpeech::MarkerType markerType, const QString& markerData);
    void announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
        int jobNum, KSpeech::JobState state);

private:
    /**
    * Constructor.
    * Note: Applications do not create instances of Jovie.
    * Instead create KSpeechInterface object.  See @ref programming.
    */
    Jovie(QObject *parent=0);

    JoviePrivate *d;
    static Jovie *m_instance;

};

#endif // JOVIE_H
