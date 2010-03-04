/***************************************************** vim:set ts=4 sw=4 sts=4:
  Kitty
  
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

#include "kitty.h"

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

#include "kspeechadaptor.h"

/* KittyPrivate Class ================================================== */

class KittyPrivate
{
    KittyPrivate()
    {
    }
    
    ~KittyPrivate()
    {
    }

    friend class Kitty;
    
protected:
    /*
    * The DBUS sender ID of last application to call KTTSD.
    */
    QString callingAppId;
};

/* Kitty Class ========================================================= */

/* ---- Public Methods --------------------------------------------------- */
Kitty * Kitty::m_instance = NULL;

Kitty * Kitty::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new Kitty();
    }
    return m_instance;
}

Kitty::Kitty(QObject *parent) :
    QObject(parent), d(new KittyPrivate())
{
    kDebug() << "Kitty::Kitty Running";
}

Kitty::~Kitty(){
    kDebug() << "Kitty::~Kitty:: Stopping KTTSD service";
    Speaker::Instance()->requestExit();
    delete d;
    announceEvent("~Kitty", "kttsdExiting");
    emit kttsdExiting();
}

/* ---- DBUS exported functions ------------------------------------------ */

bool Kitty::isSpeaking() const
{
    return Speaker::Instance()->isSpeaking();
}

QString Kitty::version() const
{
    return KGlobal::mainComponent().aboutData()->version();
}

QString Kitty::applicationName()
{
    return Speaker::Instance()->getAppData(callingAppId())->applicationName();
}

void Kitty::setApplicationName(const QString &applicationName)
{
    kDebug() << "setting application name to : " << applicationName;
    Speaker::Instance()->getAppData(callingAppId())->setApplicationName(applicationName);
}

QString Kitty::defaultTalker()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultTalker();
}

void Kitty::setDefaultTalker(const QString &defaultTalker)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultTalker(defaultTalker);
}

int Kitty::defaultPriority()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultPriority();
}

void Kitty::setDefaultPriority(int defaultPriority)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultPriority((KSpeech::JobPriority)defaultPriority);
}

QString Kitty::sentenceDelimiter()
{
    return Speaker::Instance()->getAppData(callingAppId())->sentenceDelimiter();
}

void Kitty::setSentenceDelimiter(const QString &sentenceDelimiter)
{
    Speaker::Instance()->getAppData(callingAppId())->setSentenceDelimiter(sentenceDelimiter);
}

bool Kitty::filteringOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->filteringOn();
}

void Kitty::setFilteringOn(bool filteringOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setFilteringOn(filteringOn);
}

bool Kitty::autoConfigureTalkersOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->autoConfigureTalkersOn();
}

void Kitty::setAutoConfigureTalkersOn(bool autoConfigureTalkersOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setAutoConfigureTalkersOn(autoConfigureTalkersOn);
}

bool Kitty::isApplicationPaused()
{
    return Speaker::Instance()->isApplicationPaused(callingAppId());
}

QString Kitty::htmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->htmlFilterXsltFile();
}

void Kitty::setHtmlFilterXsltFile(const QString &htmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setHtmlFilterXsltFile(htmlFilterXsltFile);
}

QString Kitty::ssmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->ssmlFilterXsltFile();
}

void Kitty::setSsmlFilterXsltFile(const QString &ssmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setSsmlFilterXsltFile(ssmlFilterXsltFile);
}

bool Kitty::isSystemManager()
{
    return Speaker::Instance()->getAppData(callingAppId())->isSystemManager();
}

void Kitty::setIsSystemManager(bool isSystemManager)
{
    Speaker::Instance()->getAppData(callingAppId())->setIsSystemManager(isSystemManager);
}

int Kitty::say(const QString &text, int options) {
    // kDebug() << "Kitty::say: Adding '" << text << "' to queue.";
    Speaker * speaker = Speaker::Instance();
    return speaker->say(speaker->getAppData(callingAppId())->applicationName(), text, options);
}

int Kitty::sayFile(const QString &filename, const QString &encoding)
{
    // kDebug() << "Kitty::setFile: Running";
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

int Kitty::sayClipboard()
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

QStringList Kitty::outputModules()
{
    return Speaker::Instance()->outputModules();
}

QStringList Kitty::languagesByModule(const QString & module)
{
    return Speaker::Instance()->languagesByModule(module);
}

void Kitty::setSpeed(int speed)
{
    if (speed < -100 || speed > 100) {
        kDebug() << "setSpeed called with out of range speed value: " << speed;
    }
    else {
        Speaker::Instance()->setSpeed(speed);
    }
}

int Kitty::speed()
{
    return Speaker::Instance()->speed();
}

void Kitty::setPitch(int pitch)
{
    if (pitch < -100 || pitch > 100) {
        kDebug() << "setPitch called with out of range pitch value: " << pitch;
    }
    else {
        Speaker::Instance()->setPitch(pitch);
    }
}

int Kitty::pitch()
{
    return Speaker::Instance()->pitch();
}

void Kitty::setVolume(int volume)
{
    if (volume < -100 || volume > 100) {
        kDebug() << "setVolume called with out of range volume value: " << volume;
    }
    else {
        Speaker::Instance()->setVolume(volume);
    }
}

int Kitty::volume()
{
    return Speaker::Instance()->volume();
}

void Kitty::setOutputModule(const QString & module)
{
    Speaker::Instance()->setOutputModule(module);
}

void Kitty::setLanguage(const QString & language)
{
    Speaker::Instance()->setLanguage(language);
}

void Kitty::setVoiceType(int voiceType)
{
    Speaker::Instance()->setVoiceType(voiceType);
}

int Kitty::voiceType()
{
    return Speaker::Instance()->voiceType();
}

void Kitty::stop()
{
    Speaker::Instance()->stop();
}

void Kitty::cancel()
{
    Speaker::Instance()->pause();
}

void Kitty::pause()
{
    Speaker::Instance()->pause();
}

void Kitty::resume()
{
    Speaker::Instance()->resume();
}

void Kitty::removeJob(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

void Kitty::removeAllJobs()
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

int Kitty::getSentenceCount(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

int Kitty::getCurrentJob()
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

int Kitty::getJobCount(int priority)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

QStringList Kitty::getJobNumbers(int priority)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return QStringList();
}

int Kitty::getJobState(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

QByteArray Kitty::getJobInfo(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return QByteArray();
}

QString Kitty::getJobSentence(int jobNum, int sentenceNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

QStringList Kitty::getTalkerCodes()
{
    return TalkerMgr::Instance()->getTalkers();
}

QString Kitty::talkerToTalkerId(const QString &talker)
{
    return TalkerMgr::Instance()->talkerCodeToTalkerId(talker);
}

int Kitty::getTalkerCapabilities1(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

int Kitty::getTalkerCapabilities2(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

QStringList Kitty::getTalkerVoices(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return QStringList();
}

void Kitty::changeJobTalker(int jobNum, const QString &talker)
{
    jobNum = applyDefaultJobNum(jobNum);
    Speaker::Instance()->setTalker(jobNum, talker);
}

void Kitty::moveJobLater(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

int Kitty::moveRelSentence(int jobNum, int n)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

void Kitty::showManagerDialog()
{
    QString cmd = "kcmshell4 kcmkttsd --caption ";
    cmd += '\'' + i18n("KDE Text-to-Speech") + '\'';
    KRun::runCommand(cmd,NULL);
}

void Kitty::kttsdExit()
{
    announceEvent("kttsdExit", "kttsdExiting");
    emit kttsdExiting();
    qApp->quit();
}

void Kitty::init()
{
    new KSpeechAdaptor(this);
    if (ready()) {
        QDBusConnection::sessionBus().registerObject("/KSpeech", this, QDBusConnection::ExportAdaptors);
    }
}

void Kitty::reinit()
{
    // Restart ourself.
    kDebug() << "Kitty::reinit: Running";
    kDebug() << "Kitty::reinit: Stopping KTTSD service";
    //if (Speaker::Instance()->isSpeaking())
    //    Speaker::Instance()->pause();
    Speaker::Instance()->requestExit();
    QDBusConnection::sessionBus().unregisterObject("/KSpeech");
    if (ready()) {
        QDBusConnection::sessionBus().registerObject("/KSpeech", this, QDBusConnection::ExportAdaptors);
    }
}

void Kitty::setCallingAppId(const QString& appId)
{
    d->callingAppId = appId;
}

/* ---- Private Methods ------------------------------------------ */

bool Kitty::initializeConfigData()
{
    return true;
}

bool Kitty::ready()
{
    // TODO: add a check here to see if kttsd is ready (Speaker::Instance() will always be true...)
    //if (Speaker::Instance())
    //    return true;
    //kDebug() << "Kitty::ready: Starting KTTSD service";
//    if (!initializeSpeechData()) return false;
    if (!initializeTalkerMgr())
        return false;
    if (!initializeSpeaker())
        return false;
    announceEvent("ready", "kttsdStarted");
    emit kttsdStarted();
    return true;
}

bool Kitty::initializeSpeechData()
{
    return true;
}

bool Kitty::initializeTalkerMgr()
{
    TalkerMgr::Instance()->loadTalkers(KGlobal::config().data());
    return true;
}

bool Kitty::initializeSpeaker()
{
    kDebug() << "Kitty::initializeSpeaker: Instantiating Speaker";

    Speaker::Instance()->init();

    // Establish ourself as a System Manager application.
    Speaker::Instance()->getAppData("kttsd")->setIsSystemManager(true);

    return true;
}


void Kitty::slotJobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state)
{
    announceEvent("slotJobStateChanged", "jobStateChanged", appId, jobNum, state);
    emit jobStateChanged(appId, jobNum, state);
}

void Kitty::slotMarker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData)
{
    announceEvent("slotMarker", "marker", appId, jobNum, markerType, markerData);
    emit marker(appId, jobNum, markerType, markerData);
}

void Kitty::slotFilteringFinished()
{
    //Speaker::Instance()->doUtterances();
}

QString Kitty::callingAppId()
{
    // TODO: What would be nice is if there were a way to get the
    // last DBUS sender() without having to add DBusMessage to every
    // slot.  Then it would not be necessary to hand-edit the adaptor.
    return d->callingAppId;
}

int Kitty::applyDefaultJobNum(int jobNum)
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

void Kitty::announceEvent(const QString& slotName, const QString& eventName)
{
    kDebug() << "Kitty::" << slotName << ": emitting DBUS signal " << eventName;
}

void Kitty::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, KSpeech::MarkerType markerType, const QString& markerData)
{
    kDebug() << "Kitty::" << slotName << ": emitting DBUS signal " << eventName 
        << " with appId " << appId << " job number " << jobNum << " marker type " << markerType << " and data " << markerData << endl;
}

void Kitty::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, KSpeech::JobState state)
{
    kDebug() << "Kitty::" << slotName << ": emitting DBUS signal " << eventName <<
        " with appId " << appId << " job number " << jobNum << " and state " << /* SpeechJob::jobStateToStr(state) << */ endl;
}
