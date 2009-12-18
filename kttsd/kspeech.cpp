/***************************************************** vim:set ts=4 sw=4 sts=4:
  KSpeech
  
  The KDE Text-to-Speech object.
  ------------------------------
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

#include <kspeech.h>

// Qt includes.
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>

// KDE includes.
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <krun.h>
#include <kaboutdata.h>


// KTTSD includes.
#include "talkermgr.h"
// define spd_debug here to avoid a link error in speech-dispatcher 0.6.7's header file for now
#define spd_debug spd_debug2
#include "speaker.h"

// KSpeech includes.
#include "kspeechadaptor.h"

/* KSpeechPrivate Class ================================================== */

class KSpeechPrivate
{
    KSpeechPrivate()
    {
    }
    
    ~KSpeechPrivate()
    {
    }

    friend class KSpeech;
    
protected:
    /*
    * The DBUS sender ID of last application to call KTTSD.
    */
    QString callingAppId;
};

/* KSpeech Class ========================================================= */

/* ---- Public Methods --------------------------------------------------- */

KSpeech::KSpeech(QObject *parent) :
    QObject(parent), d(new KSpeechPrivate())
{
    kDebug() << "KSpeech::KSpeech Running";
}

KSpeech::~KSpeech(){
    kDebug() << "KSpeech::~KSpeech:: Stopping KTTSD service";
    Speaker::Instance()->requestExit();
    delete d;
    announceEvent("~KSpeech", "kttsdExiting");
    emit kttsdExiting();
}

/* ---- DBUS exported functions ------------------------------------------ */

bool KSpeech::isSpeaking() const
{
    return Speaker::Instance()->isSpeaking();
}

QString KSpeech::version() const
{
    return KGlobal::mainComponent().aboutData()->version();
}

QString KSpeech::applicationName()
{
    return Speaker::Instance()->getAppData(callingAppId())->applicationName();
}

void KSpeech::setApplicationName(const QString &applicationName)
{
    kDebug() << "setting application name to : " << applicationName;
    Speaker::Instance()->getAppData(callingAppId())->setApplicationName(applicationName);
}

QString KSpeech::defaultTalker()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultTalker();
}

void KSpeech::setDefaultTalker(const QString &defaultTalker)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultTalker(defaultTalker);
}

int KSpeech::defaultPriority()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultPriority();
}

void KSpeech::setDefaultPriority(int defaultPriority)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultPriority((JobPriority)defaultPriority);
}

QString KSpeech::sentenceDelimiter()
{
    return Speaker::Instance()->getAppData(callingAppId())->sentenceDelimiter();
}

void KSpeech::setSentenceDelimiter(const QString &sentenceDelimiter)
{
    Speaker::Instance()->getAppData(callingAppId())->setSentenceDelimiter(sentenceDelimiter);
}

bool KSpeech::filteringOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->filteringOn();
}

void KSpeech::setFilteringOn(bool filteringOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setFilteringOn(filteringOn);
}

bool KSpeech::autoConfigureTalkersOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->autoConfigureTalkersOn();
}

void KSpeech::setAutoConfigureTalkersOn(bool autoConfigureTalkersOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setAutoConfigureTalkersOn(autoConfigureTalkersOn);
}

bool KSpeech::isApplicationPaused()
{
    return Speaker::Instance()->isApplicationPaused(callingAppId());
}

QString KSpeech::htmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->htmlFilterXsltFile();
}

void KSpeech::setHtmlFilterXsltFile(const QString &htmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setHtmlFilterXsltFile(htmlFilterXsltFile);
}

QString KSpeech::ssmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->ssmlFilterXsltFile();
}

void KSpeech::setSsmlFilterXsltFile(const QString &ssmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setSsmlFilterXsltFile(ssmlFilterXsltFile);
}

bool KSpeech::isSystemManager()
{
    return Speaker::Instance()->getAppData(callingAppId())->isSystemManager();
}

void KSpeech::setIsSystemManager(bool isSystemManager)
{
    Speaker::Instance()->getAppData(callingAppId())->setIsSystemManager(isSystemManager);
}

int KSpeech::say(const QString &text, int options) {
    // kDebug() << "KSpeech::say: Adding '" << text << "' to queue.";
    Speaker * speaker = Speaker::Instance();
    return speaker->say(speaker->getAppData(callingAppId())->applicationName(), text, options);
}

int KSpeech::sayFile(const QString &filename, const QString &encoding)
{
    // kDebug() << "KSpeech::setFile: Running";
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
        jobNum = Speaker::Instance()->say(callingAppId(), stream.readAll(), 0);
        file.close();
    }
    return jobNum;
}

int KSpeech::sayClipboard()
{
    // Get the clipboard object.
    QClipboard *cb = qApp->clipboard();

    // Copy text from the clipboard.
    QString text = cb->text();

    // Speak it.
    if (!text.isNull())
    {
        return Speaker::Instance()->say(callingAppId(), text, 0);
    } else
        return 0;
}

QStringList KSpeech::outputModules()
{
    return Speaker::Instance()->outputModules();
}

QStringList KSpeech::languagesByModule(const QString & module)
{
    return Speaker::Instance()->languagesByModule(module);
}

void KSpeech::setSpeed(int speed)
{
    if (speed < -100 || speed > 100) {
        kDebug() << "setSpeed called with out of range speed value: " << speed;
    }
    else {
        Speaker::Instance()->setSpeed(speed);
    }
}

void KSpeech::setPitch(int pitch)
{
    if (pitch < -100 || pitch > 100) {
        kDebug() << "setPitch called with out of range pitch value: " << pitch;
    }
    else {
        Speaker::Instance()->setPitch(pitch);
    }
}

void KSpeech::setVolume(int volume)
{
    if (volume < -100 || volume > 100) {
        kDebug() << "setVolume called with out of range volume value: " << volume;
    }
    else {
        Speaker::Instance()->setVolume(volume);
    }
}

void KSpeech::setOutputModule(const QString & module)
{
    Speaker::Instance()->setOutputModule(module);
}

void KSpeech::setLanguage(const QString & language)
{
    Speaker::Instance()->setLanguage(language);
}

void KSpeech::setVoiceType(int voiceType)
{
    Speaker::Instance()->setVoiceType(voiceType);
}

void KSpeech::stop()
{
    Speaker::Instance()->stop();
}

void KSpeech::cancel()
{
    Speaker::Instance()->pause();
}

void KSpeech::pause()
{
    Speaker::Instance()->pause();
}

void KSpeech::resume()
{
    Speaker::Instance()->resume();
}

void KSpeech::removeJob(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    Speaker::Instance()->removeJob(jobNum);
}

void KSpeech::removeAllJobs()
{
    Speaker::Instance()->removeAllJobs(callingAppId());
}

int KSpeech::getSentenceCount(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return Speaker::Instance()->sentenceCount(jobNum);
}

int KSpeech::getCurrentJob()
{
    return Speaker::Instance()->getCurrentJobNum();
}

int KSpeech::getJobCount(int priority)
{
    return Speaker::Instance()->jobCount(callingAppId(), (JobPriority)priority);
}

QStringList KSpeech::getJobNumbers(int priority)
{
    return Speaker::Instance()->jobNumbers(callingAppId(), (JobPriority)priority);
}

int KSpeech::getJobState(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return Speaker::Instance()->jobState(jobNum);
}

QByteArray KSpeech::getJobInfo(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return Speaker::Instance()->jobInfo(jobNum);
}

QString KSpeech::getJobSentence(int jobNum, int sentenceNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    return Speaker::Instance()->jobSentence(jobNum, sentenceNum);
}

QStringList KSpeech::getTalkerCodes()
{
    return TalkerMgr::Instance()->getTalkers();
}

QString KSpeech::talkerToTalkerId(const QString &talker)
{
    return TalkerMgr::Instance()->talkerCodeToTalkerId(talker);
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
    Speaker::Instance()->setTalker(jobNum, talker);
}

void KSpeech::moveJobLater(int jobNum)
{
    jobNum = applyDefaultJobNum(jobNum);
    Speaker::Instance()->moveJobLater(jobNum);
}

int KSpeech::moveRelSentence(int jobNum, int n)
{
    jobNum = applyDefaultJobNum(jobNum);
    int sentenceNum = Speaker::Instance()->moveRelSentence(jobNum, n);
    return sentenceNum;
}

void KSpeech::showManagerDialog()
{
    QString cmd = "kcmshell4 kcmkttsd --caption ";
    cmd += '\'' + i18n("KDE Text-to-Speech") + '\'';
    KRun::runCommand(cmd,NULL);
}

void KSpeech::kttsdExit()
{
   Speaker::Instance()->removeAllJobs("kttsd");
    announceEvent("kttsdExit", "kttsdExiting");
    emit kttsdExiting();
    qApp->quit();
}

void KSpeech::init()
{
    new KSpeechAdaptor(this);
    if (ready()) {
        QDBusConnection::sessionBus().registerObject("/KSpeech", this, QDBusConnection::ExportAdaptors);
    }
}

void KSpeech::reinit()
{
    // Restart ourself.
    kDebug() << "KSpeech::reinit: Running";
    kDebug() << "KSpeech::reinit: Stopping KTTSD service";
    //if (Speaker::Instance()->isSpeaking())
    //    Speaker::Instance()->pause();
    Speaker::Instance()->requestExit();
    QDBusConnection::sessionBus().unregisterObject("/KSpeech");
    if (ready()) {
        QDBusConnection::sessionBus().registerObject("/KSpeech", this, QDBusConnection::ExportAdaptors);
    }
}

void KSpeech::setCallingAppId(const QString& appId)
{
    d->callingAppId = appId;
}

/* ---- Private Methods ------------------------------------------ */

bool KSpeech::initializeConfigData()
{
    return true;
}

bool KSpeech::ready()
{
    // TODO: add a check here to see if kttsd is ready (Speaker::Instance() will always be true...)
    //if (Speaker::Instance())
    //    return true;
    //kDebug() << "KSpeech::ready: Starting KTTSD service";
//    if (!initializeSpeechData()) return false;
    if (!initializeTalkerMgr())
        return false;
    if (!initializeSpeaker())
        return false;
    announceEvent("ready", "kttsdStarted");
    emit kttsdStarted();
    return true;
}

bool KSpeech::initializeSpeechData()
{
    return true;
}

bool KSpeech::initializeTalkerMgr()
{
    TalkerMgr::Instance()->loadTalkers(KGlobal::config().data());
    return true;
}

bool KSpeech::initializeSpeaker()
{
    kDebug() << "KSpeech::initializeSpeaker: Instantiating Speaker";

    Speaker::Instance()->init();

    connect (Speaker::Instance(), SIGNAL(marker(const QString&, int, KSpeech::MarkerType, const QString&)),
        this, SLOT(slotMarker(const QString&, int, KSpeech::MarkerType, const QString&)));

    // Establish ourself as a System Manager application.
    Speaker::Instance()->getAppData("kttsd")->setIsSystemManager(true);

    return true;
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
    //Speaker::Instance()->doUtterances();
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
        jNum = Speaker::Instance()->findJobNumByAppId(callingAppId());
        if (!jNum) jNum = getCurrentJob();
        if (!jNum) jNum = Speaker::Instance()->findJobNumByAppId(QString());
    }
    return jNum;
}

void KSpeech::announceEvent(const QString& slotName, const QString& eventName)
{
    kDebug() << "KSpeech::" << slotName << ": emitting DBUS signal " << eventName;
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
