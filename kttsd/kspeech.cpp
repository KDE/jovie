/***************************************************** vim:set ts=4 sw=4 sts=4:
  KSpeech
  
  The KDE Text-to-Speech object.
  ------------------------------
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
#include <QClipboard>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>

// KDE includes.
#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <knotifyclient.h>
#include <krun.h>
#include <kaboutdata.h>
#include <kspeech.h>

// KTTS includes.
#include "notify.h"

// KTTSD includes.
#include "configdata.h"
#include "speechdata.h"
#include "talkermgr.h"
#include "speaker.h"

// KSpeech includes.
#include "kspeechadaptor_p.h"

/* KSpeechPrivate Class ================================================== */

class KSpeechPrivate
{
    KSpeechPrivate() :
        configData(NULL),
        talkerMgr(NULL),
        speechData(NULL),
        speaker(NULL)
    {
    }
    
    ~KSpeechPrivate()
    {
        delete configData;
        delete talkerMgr;
        delete speechData;
        delete speaker;
    }

    friend class KSpeech;
    
protected:
    /*
    * The DBUS sender ID of last application to call KTTSD.
    */
    QString callingAppId;
    
    /**
    * Configuration data.
    */
    ConfigData* configData;
    
    /*
    * TalkerMgr keeps a list of all the Talkers (synth plugins).
    */
    TalkerMgr* talkerMgr;

    /*
    * SpeechData containing all the data and the manipulating methods for all KTTSD
    */
    SpeechData* speechData;

    /*
    * Speaker that will be run as another thread, actually saying the messages, warnings, and texts
    */
    Speaker* speaker;
};

/* KSpeech Class ========================================================= */

/* ---- Public Methods --------------------------------------------------- */

KSpeech::KSpeech(QObject *parent) :
    QObject(parent), d(new KSpeechPrivate())
{
    kDebug() << "KSpeech::KSpeech Running" << endl;
    new KSpeechAdaptor(this);
    QDBus::sessionBus().registerObject("/KSpeech", this, QDBusConnection::ExportAdaptors);
    ready();
}

KSpeech::~KSpeech(){
    kDebug() << "KSpeech::~KSpeech:: Stopping KTTSD service" << endl;
    if (d->speaker) d->speaker->requestExit();
    delete d;
    announceEvent("~KSpeech", "kttsdExiting");
    kttsdExiting();
}

/* ---- DBUS exported functions ------------------------------------------ */

bool KSpeech::isSpeaking() const
{
    return d->speaker->isSpeaking();
}

QString KSpeech::version() const
{
    return kapp->aboutData()->version();
}

QString KSpeech::applicationName()
{
    return d->speechData->getAppData(callingAppId())->applicationName();
}

void KSpeech::setApplicationName(const QString &applicationName)
{
    d->speechData->getAppData(callingAppId())->setApplicationName(applicationName);
}

QString KSpeech::defaultTalker()
{
    return d->speechData->getAppData(callingAppId())->defaultTalker();
}

void KSpeech::setDefaultTalker(const QString &defaultTalker)
{
    d->speechData->getAppData(callingAppId())->setDefaultTalker(defaultTalker);
}

int KSpeech::defaultPriority()
{
    return d->speechData->getAppData(callingAppId())->defaultPriority();
}

void KSpeech::setDefaultPriority(int defaultPriority)
{
    d->speechData->getAppData(callingAppId())->setDefaultPriority((JobPriority)defaultPriority);
}

QString KSpeech::sentenceDelimiter()
{
    return d->speechData->getAppData(callingAppId())->sentenceDelimiter();
}

void KSpeech::setSentenceDelimiter(const QString &sentenceDelimiter)
{
    d->speechData->getAppData(callingAppId())->setSentenceDelimiter(sentenceDelimiter);
}

bool KSpeech::filteringOn()
{
    return d->speechData->getAppData(callingAppId())->filteringOn();
}

void KSpeech::setFilteringOn(bool filteringOn)
{
    d->speechData->getAppData(callingAppId())->setFilteringOn(filteringOn);
}

bool KSpeech::autoConfigureTalkersOn()
{
    return d->speechData->getAppData(callingAppId())->autoConfigureTalkersOn();
}

void KSpeech::setAutoConfigureTalkersOn(bool autoConfigureTalkersOn)
{
    d->speechData->getAppData(callingAppId())->setAutoConfigureTalkersOn(autoConfigureTalkersOn);
}

bool KSpeech::isApplicationPaused()
{
    return d->speechData->isApplicationPaused(callingAppId());
}

QString KSpeech::htmlFilterXsltFile()
{
    return d->speechData->getAppData(callingAppId())->htmlFilterXsltFile();
}

void KSpeech::setHtmlFilterXsltFile(const QString &htmlFilterXsltFile)
{
    d->speechData->getAppData(callingAppId())->setHtmlFilterXsltFile(htmlFilterXsltFile);
}

QString KSpeech::ssmlFilterXsltFile()
{
    return d->speechData->getAppData(callingAppId())->ssmlFilterXsltFile();
}

void KSpeech::setSsmlFilterXsltFile(const QString &ssmlFilterXsltFile)
{
    d->speechData->getAppData(callingAppId())->setSsmlFilterXsltFile(ssmlFilterXsltFile);
}

bool KSpeech::isSystemManager()
{
    return d->speechData->getAppData(callingAppId())->isSystemManager();
}

void KSpeech::setIsSystemManager(bool isSystemManager)
{
    d->speechData->getAppData(callingAppId())->setIsSystemManager(isSystemManager);
}

int KSpeech::say(const QString &text, int options) {
    if (!d->speaker) return 0;
    kDebug() << "KSpeech::say: Adding '" << text << "' to queue." << endl;
    int jobNum = d->speechData->say(callingAppId(), text, options);
    d->speaker->doUtterances();
    return jobNum;
}

int KSpeech::sayFile(const QString &filename, const QString &encoding)
{
    // kDebug() << "KSpeech::setFile: Running" << endl;
    if (!d->speaker) return 0;
    QFile file(filename);
    int jobNum = 0;
    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream(&file);
        if (!encoding.isEmpty())
        {
            QTextCodec* codec = QTextCodec::codecForName(encoding.toLatin1());
            if (codec) stream.setCodec(codec);
        }
        jobNum = d->speechData->say(callingAppId(), stream.readAll(), 0);
        file.close();
    }
    return jobNum;
}

int KSpeech::sayClipboard()
{
    // Get the clipboard object.
    QClipboard *cb = kapp->clipboard();

    // Copy text from the clipboard.
    QString text = cb->text();

    // Speak it.
    if (!text.isNull()) 
        return d->speechData->say(callingAppId(), text, 0);
    else
        return 0;
}

void KSpeech::pause()
{
    if (!d->speaker) return;
    d->speechData->pause(callingAppId());
    d->speaker->pause(callingAppId());
}

void KSpeech::resume()
{
    if (!d->speaker) return;
    d->speechData->resume(callingAppId());
    d->speaker->doUtterances();
}

void KSpeech::removeJob(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    d->speechData->removeJob(jobNum);
    d->speaker->removeJob(jobNum);
}

void KSpeech::removeAllJobs()
{
    d->speechData->removeAllJobs(callingAppId());
    d->speaker->removeAllJobs(callingAppId());
}

int KSpeech::getSentenceCount(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return d->speechData->sentenceCount(jobNum);
}

int KSpeech::getCurrentJob()
{
    return d->speaker->getCurrentJobNum();
}

int KSpeech::getJobCount(int priority)
{
    return d->speechData->jobCount(callingAppId(), (JobPriority)priority);
}

QStringList KSpeech::getJobNumbers(int priority)
{
    return d->speechData->jobNumbers(callingAppId(), (JobPriority)priority);
}

int KSpeech::getJobState(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return d->speechData->jobState(jobNum);
}

QByteArray KSpeech::getJobInfo(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return d->speechData->jobInfo(jobNum);
}

QString KSpeech::getJobSentence(int jobNum, int seq)
{
    jobNum = applyDefaultJobNum(jobNum);
    return d->speechData->jobSentence(jobNum, seq);
}

QStringList KSpeech::getTalkerCodes()
{
    if (!d->talkerMgr) return QStringList();
    return d->talkerMgr->getTalkers();
}

QString KSpeech::talkerToTalkerId(const QString &talker)
{
    return d->talkerMgr->talkerCodeToTalkerId(talker);
}

int KSpeech::getTalkerCapabilities1(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

int KSpeech::getTalkerCapabilities2(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

QStringList KSpeech::getTalkerVoices(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return QStringList();
}

void KSpeech::changeJobTalker(int jobNum, const QString &talker)
{
    jobNum = applyDefaultJobNum(jobNum);
    d->speechData->setTalker(jobNum, talker);
}

void KSpeech::moveJobLater(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    d->speaker->moveJobLater(jobNum);
}

int KSpeech::moveRelSentence(int jobNum, int n)
{
    jobNum = applyDefaultJobNum(jobNum);
    int seq = d->speaker->moveRelSentence(jobNum, n);
    return seq;
}

void KSpeech::showManagerDialog()
{
    QString cmd = "kcmshell kcmkttsd --caption ";
    cmd += "'" + i18n("KDE Text-to-Speech") + "'";
    KRun::runCommand(cmd);
}

void KSpeech::kttsdExit()
{
    d->speechData->removeAllJobs("kttsd");
    d->speaker->removeAllJobs("kttsd");
    announceEvent("kttsdExit", "kttsdExiting");
    kttsdExiting();
    kapp->quit();
}

void KSpeech::reinit()
{
    // Restart ourself.
    kDebug() << "KSpeech::reinit: Running" << endl;
    if (d->speaker)
    {
        kDebug() << "KSpeech::reinit: Stopping KTTSD service" << endl;
        if (d->speaker->isSpeaking())
            d->speaker->pause("kttsd");
        d->speaker->requestExit();
    }
    delete d->speaker;
    d->speaker = 0;
    delete d->talkerMgr;
    d->talkerMgr = 0;
    ready();
}

void KSpeech::setCallingAppId(const QString& appId)
{
    d->callingAppId = appId;
}

/* ---- Private Methods ------------------------------------------ */

bool KSpeech::initializeConfigData()
{
    if (d->configData) delete d->configData;
    d->configData = new ConfigData(KGlobal::config());
    return true;
}

bool KSpeech::ready()
{
    if (d->speaker) return true;
    kDebug() << "KSpeech::ready: Starting KTTSD service" << endl;
    if (!initializeSpeechData()) return false;
    if (!initializeTalkerMgr()) return false;
    if (!initializeSpeaker()) return false;
    d->speaker->doUtterances();
    announceEvent("ready", "kttsdStarted");
    kttsdStarted();
    return true;
}

bool KSpeech::initializeSpeechData()
{
    // Create speechData object.
    if (!d->speechData)
    {
        d->speechData = new SpeechData();
        connect (d->speechData, SIGNAL(jobStateChanged(const QString&, int, KSpeech::JobState)),
            this, SLOT(slotJobStateChanged(const QString&, int, KSpeech::JobState)));
        connect (d->speechData, SIGNAL(filteringFinished()),
            this, SLOT(slotFilteringFinished()));
        
        // TODO: Hook KNotify signal.
        // if (!connectDCOPSignal(0, 0, 
        //    "notifySignal(QString,QString,QString,QString,QString,int,int,int,int)",
        //    "notificationSignal(QString,QString,QString,QString,QString,int,int,int,int)",
        //    false)) kDebug() << "KTTSD:initializeSpeechData: connectDCOPSignal for knotify failed" << endl;
    }
    if (!d->configData) initializeConfigData();
    d->speechData->setConfigData(d->configData);

    // Establish ourself as a System Manager application.
    d->speechData->getAppData("kttsd")->setIsSystemManager(true);
    
    return true;
}

bool KSpeech::initializeTalkerMgr()
{
    if (!d->talkerMgr)
    {
        if (!d->speechData) initializeSpeechData();

        d->talkerMgr = new TalkerMgr(this, "kttsdtalkermgr");
        int load = d->talkerMgr->loadPlugIns(d->configData->config());
        // If no Talkers configured, try to autoconfigure one, first in the user's
        // desktop language, but if that fails, fallback to English.
        if (load < 0)
        {
            QString languageCode = KGlobal::locale()->language();
            if (d->talkerMgr->autoconfigureTalker(languageCode, d->configData->config()))
                load = d->talkerMgr->loadPlugIns(d->configData->config());
            else
            {
                if (d->talkerMgr->autoconfigureTalker("en", d->configData->config()))
                    load = d->talkerMgr->loadPlugIns(d->configData->config());
            }
        }
        if (load < 0)
        {
            // TODO: Would really like to eliminate ALL GUI stuff from kttsd.  Find
            // a better way to do this.
            delete d->speaker;
            d->speaker = 0;
            delete d->talkerMgr;
            d->talkerMgr = 0;
            delete d->speechData;
            d->speechData = 0;
            kDebug() << "KSpeech::initializeTalkerMgr: no Talkers have been configured." << endl;
            // Ask if user would like to run configuration dialog, but don't bug user unnecessarily.
            QString dontAskConfigureKTTS = "DontAskConfigureKTTS";
            KMessageBox::ButtonCode msgResult;
            if (KMessageBox::shouldBeShownYesNo(dontAskConfigureKTTS, msgResult))
            {
                if (KMessageBox::questionYesNo(
                    0,
                    i18n("KTTS has not yet been configured.  At least one Talker must be configured.  "
                        "Would you like to configure it now?"),
                    i18n("KTTS Not Configured"),
                    i18n("Configure"),
                    i18n("Do Not Configure"),
                    dontAskConfigureKTTS) == KMessageBox::Yes) msgResult = KMessageBox::Yes;
            }
            if (msgResult == KMessageBox::Yes) showManagerDialog();
            return false;
        }
    }
    d->speechData->setTalkerMgr(d->talkerMgr);
    return true;
}

bool KSpeech::initializeSpeaker()
{
    // kDebug() << "KSpeech::initializeSpeaker: Instantiating Speaker" << endl;

    if (!d->talkerMgr) initializeTalkerMgr();

    // Create speaker object and load plug ins;
    d->speaker = new Speaker(d->speechData, d->talkerMgr, this);

    connect (d->speaker, SIGNAL(marker(const QString&, int, KSpeech::MarkerType, const QString&)),
        this, SLOT(slotMarker(const QString&, int, KSpeech::MarkerType, const QString&)));
        
    d->speaker->setConfigData(d->configData);

    return true;
}

/**
* This signal is emitted by KNotify when a notification event occurs.
*    ds << event << fromApp << text << sound << file << present << level
*       << winId << eventId;
* default_presentation contains these ORed events: None=0, Sound=1, Messagebox=2, Logfile=4, Stderr=8,
* PassivePopup=16, Execute=32, Taskbar=64
*/
void KSpeech::notificationSignal( const QString& event, const QString& fromApp,
    const QString &text, const QString& sound, const QString& /*file*/,
    const int present, const int /*level*/, const int /*windId*/, const int /*eventId*/)
{
    if (!d->speaker) return;
    // kDebug() << "KTTSD:notificationSignal: event: " << event << " fromApp: " << fromApp << 
    //     " text: " << text << " sound: " << sound << " file: " << file << " present: " << present <<
    //    " level: " << level << " windId: " << windId << " eventId: " << eventId << endl;
    if ( d->configData->notify )
        if ( !d->configData->notifyExcludeEventsWithSound || sound.isEmpty() )
        {
            bool found = false;
            NotifyOptions notifyOptions;
            QString msg;
            QString talker;
            // Check for app-specific action.
            if ( d->configData->notifyAppMap.contains( fromApp ) )
            {
                NotifyEventMap notifyEventMap = d->configData->notifyAppMap[ fromApp ];
                if ( notifyEventMap.contains( event ) )
                {
                    found = true;
                    notifyOptions = notifyEventMap[ event ];
                } else {
                    // Check for app-specific default.
                    if ( notifyEventMap.contains( "default" ) )
                    {
                        found = true;
                        notifyOptions = notifyEventMap[ "default" ];
                        notifyOptions.eventName.clear();
                    }
                }
            }
            // If no app-specific action, try default.
            if ( !found )
            {
                switch ( d->configData->notifyDefaultPresent )
                {
                    case NotifyPresent::None:
                        found = false;
                        break;
                    case NotifyPresent::Dialog:
                        found = (
                            (present & KNotifyClient::Messagebox)
                            &&
                            !(present & KNotifyClient::PassivePopup)
                            );
                        break;
                    case NotifyPresent::Passive:
                        found = (
                            !(present & KNotifyClient::Messagebox)
                            &&
                            (present & KNotifyClient::PassivePopup)
                            );
                        break;
                    case NotifyPresent::DialogAndPassive:
                        found = (
                            (present & KNotifyClient::Messagebox)
                            &&
                            (present & KNotifyClient::PassivePopup)
                            );
                        break;
                    case NotifyPresent::All:
                        found = true;
                        break;
                }
                if ( found )
                    notifyOptions = d->configData->notifyDefaultOptions;
            }
            if ( found )
            {
                int action = notifyOptions.action;
                talker = notifyOptions.talker;
                switch ( action )
                {
                    case NotifyAction::DoNotSpeak:
                        break;
                    case NotifyAction::SpeakEventName:
                        if (notifyOptions.eventName.isEmpty())
                            msg = NotifyEvent::getEventName( fromApp, event );
                        else
                            msg = notifyOptions.eventName;
                        break;
                    case NotifyAction::SpeakMsg:
                        msg = text;
                        break;
                    case NotifyAction::SpeakCustom:
                        msg = notifyOptions.customMsg;
                        msg.replace( "%a", fromApp );
                        msg.replace( "%m", text );
                        if ( msg.contains( "%e" ) )
                        {
                            if ( notifyOptions.eventName.isEmpty() )
                                msg.replace( "%e", NotifyEvent::getEventName( fromApp, event ) );
                            else
                                msg.replace( "%e", notifyOptions.eventName );
                        }
                        break;
                }
            }
            // Queue msg if we should speak something.
            if ( !msg.isEmpty() )
            {
                QString fromApps = fromApp + ",knotify";
                d->speechData->getAppData(fromApp)->setDefaultTalker(talker);
                d->speechData->say(fromApp, msg, 0);
                d->speaker->doUtterances();
            }
        }
}

void KSpeech::slotJobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state)
{
    announceEvent("slotJobStateChanged", "jobStateChanged", appId, jobNum, state);
    emit jobStateChanged(appId, jobNum, state);
}

void KSpeech::slotMarker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData)
{
    announceEvent("slotMarker", "marker", appId, jobNum, markerType, markerData);
    emit marker(appId, jobNum, markerType, markerData);
}

void KSpeech::slotFilteringFinished()
{
    d->speaker->doUtterances();
}

QString KSpeech::callingAppId()
{
    // TODO: What would be nice is if there were a way to get the
    // last DBUS sender() without having to add DBusMessage to every
    // slot.  Then it would not be necessary to hand-edit the adaptor.
    return d->callingAppId;
}

int KSpeech::applyDefaultJobNum(int jobNum)
{
    int jNum = jobNum;
    if (!jNum)
    {
        jNum = d->speechData->findJobNumByAppId(callingAppId());
        if (!jNum) jNum = getCurrentJob();
        if (!jNum) jNum = d->speechData->findJobNumByAppId(0);
    }
    return jNum;
}

void KSpeech::announceEvent(const QString& slotName, const QString& eventName)
{
    kDebug() << "KSpeech::" << slotName << ": emitting DBUS signal " << eventName << endl;
}

void KSpeech::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, MarkerType markerType, const QString& markerData)
{
    kDebug() << "KSpeech::" << slotName << ": emitting DBUS signal " << eventName 
        << " with appId " << appId << " job number " << jobNum << " marker type " << markerType << " and data " << markerData << endl;
}

void KSpeech::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, JobState state)
{
    kDebug() << "KSpeech::" << slotName << ": emitting DBUS signal " << eventName <<
        " with appId " << appId << " job number " << jobNum << " and state " << SpeechJob::jobStateToStr(state) << endl;
}

#include "kspeech.moc"

