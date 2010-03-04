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

// Speaker includes.
#include "speaker.h"
#include "speaker.moc"

// System includes.

// Qt includes. 
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtDBus/QtDBus>

// KDE includes.
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/componentfactory.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kservicetypetrader.h>
#include <kspeech.h>
//#include <kio/job.h>

// KTTS includes.
#include "utils.h"
#include "talkercode.h"

// KTTSD includes.
//#include "talkermgr.h"
#include "ssmlconvert.h"


/**
* The Speaker class manages all speech requests coming in from DBus, does any
* filtering, and passes the resulting text on to speech-dispatcher through its
* C api.
*
* The message queues are maintained in speech-dispatcher itself.
*
* Text jobs in the text queue each have a state (queued, speakable,
* speaking, paused, finished).
*
* At the same time, it must issue the signals to inform programs
* what is happening.
*
* Finally, users can pause, restart, delete, advance, or rewind text jobs
* and Speaker must respond to these commands.  In some cases, utterances that
* have already been synthesized and are ready for audio output must be
* discarded in response to these commands.
*
* Some general guidelines for programmers modifying this code:
* - Avoid blocking at all cost.  If a plugin won't stopText, keep going.
*   You might have to wait for the plugin to complete before asking it
*   to perform the next operation, but in the meantime, there might be
*   other useful work that can be performed.
* - In no case allow the main thread Qt event loop to block.
* - Plugins that do not have asynchronous support are wrapped in the
*   ThreadedPlugin class, which attempts to make them as asynchronous as
*   it can, but there are limits.
* - doUtterances is the main worker method.  If in doubt, call doUtterances.
* - Because Speaker controls the ordering of utterances, most sequence-related
*   signals must be emitted by Speaker; not SpeechData.  Only the add
*   and delete job-related signals eminate from SpeechData.
* - The states of the 3 types of objects mentioned above (jobs, utterances,
*   and plugins) can interact in subtle ways.  Test fully.  For example, while
*   a text job might be in a paused state, the plugin could be synthesizing
*   in anticipation of resume, or sythesizing a Warning, Message, or
*   Screen Reader Output.  Meanwhile, while one of the utterances might
*   have a paused state, others from the same job could be synthing, waiting,
*   or finished.
*/


class SpeakerPrivate
{
    SpeakerPrivate() :
        connection(NULL),
        filterMgr(NULL),
        config(NULL)
    {
        if (!ConnectToSpeechd())
            kError() << "could not get a connection to speech-dispatcher"<< endl;

        filterMgr = new FilterMgr();
        filterMgr->init();
        
        config = new KConfig("kttsdrc");
    }
    
    ~SpeakerPrivate()
    {
        spd_close(connection);
        connection = NULL;
        
        // from speechdata class
        // kDebug() << "Running: SpeechDataPrivate::~SpeechDataPrivate";
        // Walk through jobs and emit jobStateChanged signal for each job.
        //foreach (SpeechJob* job, allJobs)
        //    delete job;
        //allJobs.clear();

        delete filterMgr;
        delete config;

        foreach (AppData* applicationData, appData)
            delete applicationData;
        appData.clear();
    }
    
    friend class Speaker;
    
protected:

    bool ConnectToSpeechd()
    {
        bool retval = false;
        connection = spd_open("kttsd", "main", NULL, SPD_MODE_THREADED);
        if (connection != NULL)
        {
            kDebug() << "successfully opened connection to speech dispatcher";
            connection->callback_begin = connection->callback_end = 
                connection->callback_cancel = connection->callback_pause = 
                connection->callback_resume = Speaker::speechdCallback;

            spd_set_notification_on(connection, SPD_BEGIN);
            spd_set_notification_on(connection, SPD_END);
            spd_set_notification_on(connection, SPD_CANCEL);
            spd_set_notification_on(connection, SPD_PAUSE);
            spd_set_notification_on(connection, SPD_RESUME);
            char ** modulenames = spd_list_modules(connection);
            while (modulenames != NULL && modulenames[0] != NULL)
            {
                outputModules << modulenames[0];
                modulenames++;
                kDebug() << "added module " << outputModules.last();
            }
            retval = true;
        }
        return retval;
    }

    // try to reconnect to speech-dispatcher, return true on success
    bool reconnect()
    {
        spd_close(connection);
        return ConnectToSpeechd();
    }

    void readTalkerData()
    {
        // Iterate through list of the TalkerCode IDs.
        KConfigGroup ttsconfig(config, "General");
        QStringList talkerIDsList = ttsconfig.readEntry("TalkerIDs", QStringList());
        // kDebug() << "TalkerListModel::loadTalkerCodesFromConfig: talkerIDsList = " << talkerIDsList;
        if (!talkerIDsList.isEmpty())
        {
            QStringList::ConstIterator itEnd = talkerIDsList.constEnd();
            for (QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it)
            {
                QString talkerID = *it;
                kDebug() << "TalkerListWidget::loadTalkerCodes: talkerID = " << talkerID;
                KConfigGroup talkGroup(config, "Talkers");
                QString talkerCode = talkGroup.readEntry(talkerID);
                TalkerCode tc = TalkerCode(talkerCode, true);
                kDebug() << "TalkerCodeWidget::loadTalkerCodes: talkerCode = " << talkerCode;
                //tc.setId(talkerID);
                // do something with the talker codes read in
            }
        }
    }

    /**
    * list of output modules speech-dispatcher has
    */
    QStringList outputModules;

    SPDConnection * connection;
    
    /**
    * Application data.
    */
    mutable QMap<QString, AppData*> appData;

    /**
    * the filter manager
    */
    FilterMgr * filterMgr;

    /**
    * Object holding all the configuration
    */
    KConfig *config;

};

/* Public Methods ==========================================================*/

Speaker * Speaker::m_instance = NULL;

Speaker * Speaker::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new Speaker();
    }
    return m_instance;
}

void Speaker::speechdCallback(size_t msg_id, size_t /*client_id*/, SPDNotificationType type)
{
    kDebug() << "speechdCallback called with messageid: " << msg_id << " and type: " << type;
    //KSpeech::JobState state(KSpeech::jsQueued);
    //switch (type) {
    //    case SPD_EVENT_BEGIN:
    //        state = KSpeech::jsSpeaking;
    //        break;
    //    case SPD_EVENT_END:
    //        state = KSpeech::jsFinished;
    //        break;
    //    case SPD_EVENT_INDEX_MARK:
    //        break;
    //    case SPD_EVENT_CANCEL:
    //        state = KSpeech::jsDeleted;
    //        break;
    //    case SPD_EVENT_PAUSE:
    //        state = KSpeech::jsPaused;
    //        break;
    //    case SPD_EVENT_RESUME:
    //        state = KSpeech::jsSpeaking;
    //        break;
    //}
}

Speaker::Speaker() :
    d(new SpeakerPrivate())
{
    // kDebug() << "Running: Speaker::Speaker()";
    // Connect ServiceUnregistered signal from DBUS so we know when apps have exited.
    connect (QDBusConnection::sessionBus().interface(), SIGNAL(serviceUnregistered(const QString&)),
        this, SLOT(slotServiceUnregistered(const QString&)));
}

Speaker::~Speaker(){
    kDebug() << "Running: Speaker::~Speaker()";
    delete d;
}

void Speaker::init()
{
    // from speechdata
    // Create an initial FilterMgr for the pool to save time later.
    delete d->filterMgr;
    d->filterMgr = new FilterMgr();
    d->filterMgr->init();
}

AppData* Speaker::getAppData(const QString& appId) const
{
    if (!d->appData.contains(appId))
        d->appData.insert(appId, new AppData(appId));
    return d->appData[appId];
}

bool Speaker::isSsml(const QString &text)
{
    /// This checks to see if the root tag of the text is a <speak> tag.
    QDomDocument ssml;
    ssml.setContent(text, false);  // No namespace processing.
    /// Check to see if this is SSML
    QDomElement root = ssml.documentElement();
    return (root.tagName() == "speak");
}

QStringList Speaker::moduleNames()
{
    return d->outputModules;
}

QStringList Speaker::parseText(const QString &text, const QString &appId /*=NULL*/)
{
    // There has to be a better way
    // kDebug() << "I'm getting: "<< text << " from application " << appId;
    if (isSsml(text)) {
        QStringList tempList(text);
        return tempList;
    }
    // See if app has specified a custom sentence delimiter and use it, otherwise use default.
    QRegExp sentenceDelimiter(getAppData(appId)->sentenceDelimiter());
    QString temp = text;
    // Replace spaces, tabs, and formfeeds with a single space.
    temp.replace(QRegExp("[ \\t\\f]+"), " ");
    // Replace sentence delimiters with tab.
    temp.replace(sentenceDelimiter, "\\1\t");
    // Replace remaining newlines with spaces.
    temp.replace('\n',' ');
    temp.replace('\r',' ');
    // Remove leading spaces.
    temp.replace(QRegExp("\\t +"), "\t");
    // Remove trailing spaces.
    temp.replace(QRegExp(" +\\t"), "\t");
    // Remove blank lines.
    temp.replace(QRegExp("\t\t+"),"\t");
    // Split into sentences.
    QStringList tempList = temp.split( '\t', QString::SkipEmptyParts);

//    for ( QStringList::Iterator it = tempList.begin(); it != tempList.end(); ++it ) {
//        kDebug() << "'" << *it << "'";
//    }
    return tempList;
}

int Speaker::say(const QString& appId, const QString& text, int sayOptions)
{
    QString filteredText = text;
	int jobNum = -1;

    AppData* appData = getAppData(appId);
    KSpeech::JobPriority priority = appData->defaultPriority();
    //kDebug() << "Speaker::say priority = " << priority;
    //kDebug() << "Running: Speaker::say appId = " << appId << " text = " << text;
    //QString talker = appData->defaultTalker();

    SPDPriority spdpriority = SPD_PROGRESS; // default to least priority
    switch (priority)
    {
        case KSpeech::jpScreenReaderOutput: /**< Screen Reader job. SPD_IMPORTANT */
            spdpriority = SPD_IMPORTANT;
            break;
        case KSpeech::jpWarning: /**< Warning job. SPD_NOTIFICATION */
            spdpriority = SPD_NOTIFICATION;
            break;
        case KSpeech::jpMessage: /**< Message job.SPD_MESSAGE */
            spdpriority = SPD_MESSAGE;
            break;
        case KSpeech::jpText: /**< Text job. SPD_TEXT */
            spdpriority = SPD_TEXT;
            break;
        case KSpeech::jpProgress: /**< Progress report. SPD_PROGRESS added KDE 4.4 */
            spdpriority = SPD_PROGRESS;
            break;
    }

    if (appData->filteringOn()) {
        filteredText = d->filterMgr->convert(text, NULL, appId);
    }

    emit newJobFiltered(text, filteredText);

    while (jobNum == -1 && d->connection != NULL)
    {
        switch (sayOptions)
        {
            case KSpeech::soNone: /**< No options specified.  Autodetected. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soPlainText: /**< The text contains plain text. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soHtml: /**< The text contains HTML markup. */
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soSsml: /**< The text contains SSML markup. */
                spd_set_data_mode(d->connection, SPD_DATA_SSML);
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                spd_set_data_mode(d->connection, SPD_DATA_TEXT);
                break;
            case KSpeech::soChar: /**< The text should be spoken as individual characters. */
                spd_set_spelling(d->connection, SPD_SPELL_ON);
                jobNum = spd_say(d->connection, spdpriority, filteredText.toUtf8().data());
                spd_set_spelling(d->connection, SPD_SPELL_OFF);
                break;
            case KSpeech::soKey: /**< The text contains a keyboard symbolic key name. */
                jobNum = spd_key(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
            case KSpeech::soSoundIcon: /**< The text is the name of a sound icon. */
                jobNum = spd_sound_icon(d->connection, spdpriority, filteredText.toUtf8().data());
                break;
        }
        if (jobNum == -1 && d->connection != NULL)
        {
            // job failure
            // try to reconnect once
            kDebug() << "trying to reconnect to speech dispatcher";
            if (!d->reconnect())
            {
                // replace this with an error stored in kttsd in a log? to be viewed on hover over kttsmgr?
                kDebug() << "could not connect to speech dispatcher";
            }
        }
    }

    if (jobNum != -1)
    {
        kDebug() << "incoming job with text: " << text;
        kDebug() << "saying post filtered text: " << filteredText;
    }

    //// Note: Set state last so job is fully populated when jobStateChanged signal is emitted.
    appData->jobList()->append(jobNum);
    return jobNum;
}

int Speaker::findJobNumByAppId(const QString& appId) const
{
    if (appId.isEmpty())
        return 0; // d->lastJobNum;
    else
        return getAppData(appId)->lastJobNum();
}

void Speaker::requestExit(){
    // kDebug() << "Speaker::requestExit: Running";
    //d->exitRequested = true;
}

bool Speaker::isSpeaking()
{
    return true; // TODO: ask speech-dispatcher somehow?
}

void Speaker::setTalker(int jobNum, const QString &talker)
{
    kDebug() << "this is not implemented yet in speech-dispatcher";
}

QStringList Speaker::outputModules()
{
    QStringList modules;

	if (d->connection) {
		char ** modulenames = spd_list_modules(d->connection);
		while (modulenames != NULL && modulenames[0] != NULL)
		{
			modules << modulenames[0];
			++modulenames;
		}
	}
	else {
		// emit some error message
	}
    return modules;
}

QStringList Speaker::languagesByModule(const QString & module)
{
    QStringList languages;
    if (spd_set_output_module(d->connection, module.toUtf8().data()) == 0)
    {
        SPDVoice ** voices = spd_list_synthesis_voices(d->connection);
        while (voices != NULL && voices[0] != NULL)
        {
            if (!languages.contains(voices[0]->language))
                languages << voices[0]->language;
            ++voices;
        }
    }
    return languages;
}

void Speaker::setSpeed(int speed)
{
    spd_set_voice_rate(d->connection, speed);
}

int Speaker::speed()
{
    return 0;
}

void Speaker::setPitch(int pitch)
{
    spd_set_voice_pitch(d->connection, pitch);
}

int Speaker::pitch()
{
    return 0;
}

void Speaker::setVolume(int volume)
{
    spd_set_volume(d->connection, volume);
}

int Speaker::volume()
{
    return 0;
}

void Speaker::setOutputModule(const QString & module)
{
    int result = spd_set_output_module(d->connection, module.toUtf8().data());
    // discard result for now, TODO: add error reporting
}

QString Speaker::outputModule()
{
    return QString();
}

void Speaker::setLanguage(const QString & language)
{
    int result = spd_set_language(d->connection, language.toUtf8().data());
    // discard result for now, TODO: add error reporting
}

QString Speaker::language()
{
    return QString();
}

void Speaker::setVoiceType(int voiceType)
{
    int result = spd_set_voice_type(d->connection, SPDVoiceType(voiceType));
    // discard result for now, TODO: add error reporting
}

int Speaker::voiceType()
{
    return 0;
}

void Speaker::stop()
{
    spd_stop(d->connection);
}

void Speaker::cancel()
{
    spd_cancel(d->connection);
}

void Speaker::pause()
{
    spd_pause(d->connection);
}

void Speaker::resume()
{
    spd_resume(d->connection);
}

bool Speaker::isApplicationPaused(const QString& appId)
{
    return getAppData(appId)->isApplicationPaused();
}

void Speaker::slotServiceUnregistered(const QString& serviceName)
{
    if (d->appData.contains(serviceName))
        d->appData[serviceName]->setUnregistered(true);
}
