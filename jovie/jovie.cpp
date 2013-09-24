/***************************************************** vim:set ts=4 sw=4 sts=4:
  Jovie

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

#include "jovie.h"

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
#include <KRun>
#include <kaboutdata.h>


// Jovie includes.
#include "talkermgr.h"
#include "talkercode.h"
// define spd_debug here to avoid a link error in speech-dispatcher 0.6.7's header file for now
#define spd_debug spd_debug2
#include "speaker.h"
#include "jovietrayicon.h"

#include "kspeechadaptor.h"

/* JoviePrivate Class ================================================== */

class JoviePrivate
{
    JoviePrivate()
    {
        trayIcon = new JovieTrayIcon();
    }

    ~JoviePrivate()
    {
        delete trayIcon;
    }

    friend class Jovie;

protected:
    /*
    * The DBUS sender ID of last application to call KTTSD.
    */
    QString callingAppId;
    
    /*
    * The tray icon.
    */
    JovieTrayIcon *trayIcon;
  
};

/* Jovie Class ========================================================= */

/* ---- Public Methods --------------------------------------------------- */
Jovie * Jovie::m_instance = NULL;

Jovie * Jovie::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new Jovie();
    }
    return m_instance;
}

Jovie::Jovie(QObject *parent) :
    QObject(parent), d(new JoviePrivate())
{
    kDebug() << "Jovie::Jovie Running";
}

Jovie::~Jovie()
{
    kDebug() << "Jovie::~Jovie:: Stopping Jovie service";
    Speaker::Instance()->requestExit();
    delete d;
    announceEvent(QLatin1String( "~Jovie" ), QLatin1String( "jovieExiting" ));
    emit kttsdExiting();
}

/* ---- DBUS exported functions ------------------------------------------ */

bool Jovie::isSpeaking() const
{
    return Speaker::Instance()->isSpeaking();
}

QString Jovie::version() const
{
    return KGlobal::mainComponent().aboutData()->version();
}

QString Jovie::applicationName()
{
    return Speaker::Instance()->getAppData(callingAppId())->applicationName();
}

void Jovie::setApplicationName(const QString &applicationName)
{
    kDebug() << "setting application name to : " << applicationName;
    Speaker::Instance()->getAppData(callingAppId())->setApplicationName(applicationName);
}

QString Jovie::defaultTalker()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultTalker();
}

void Jovie::setDefaultTalker(const QString &defaultTalker)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultTalker(defaultTalker);
}

void Jovie::setCurrentTalker(const TalkerCode &talker)
{
    Speaker::Instance()->setOutputModule(talker.outputModule());
    Speaker::Instance()->setLanguage(TalkerCode::languageCodeToLanguage(talker.language()));
    Speaker::Instance()->setVoiceType(talker.voiceType());
    Speaker::Instance()->setVolume(talker.volume());
    Speaker::Instance()->setSpeed(talker.rate());
    Speaker::Instance()->setPitch(talker.pitch());
}

int Jovie::defaultPriority()
{
    return Speaker::Instance()->getAppData(callingAppId())->defaultPriority();
}

void Jovie::setDefaultPriority(int defaultPriority)
{
    Speaker::Instance()->getAppData(callingAppId())->setDefaultPriority((KSpeech::JobPriority)defaultPriority);
}

QString Jovie::sentenceDelimiter()
{
    return Speaker::Instance()->getAppData(callingAppId())->sentenceDelimiter();
}

void Jovie::setSentenceDelimiter(const QString &sentenceDelimiter)
{
    Speaker::Instance()->getAppData(callingAppId())->setSentenceDelimiter(sentenceDelimiter);
}

bool Jovie::filteringOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->filteringOn();
}

void Jovie::setFilteringOn(bool filteringOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setFilteringOn(filteringOn);
}

bool Jovie::autoConfigureTalkersOn()
{
    return Speaker::Instance()->getAppData(callingAppId())->autoConfigureTalkersOn();
}

void Jovie::setAutoConfigureTalkersOn(bool autoConfigureTalkersOn)
{
    Speaker::Instance()->getAppData(callingAppId())->setAutoConfigureTalkersOn(autoConfigureTalkersOn);
}

bool Jovie::isApplicationPaused()
{
    return Speaker::Instance()->isApplicationPaused(callingAppId());
}

QString Jovie::htmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->htmlFilterXsltFile();
}

void Jovie::setHtmlFilterXsltFile(const QString &htmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setHtmlFilterXsltFile(htmlFilterXsltFile);
}

QString Jovie::ssmlFilterXsltFile()
{
    return Speaker::Instance()->getAppData(callingAppId())->ssmlFilterXsltFile();
}

void Jovie::setSsmlFilterXsltFile(const QString &ssmlFilterXsltFile)
{
    Speaker::Instance()->getAppData(callingAppId())->setSsmlFilterXsltFile(ssmlFilterXsltFile);
}

bool Jovie::isSystemManager()
{
    return Speaker::Instance()->getAppData(callingAppId())->isSystemManager();
}

void Jovie::setIsSystemManager(bool isSystemManager)
{
    Speaker::Instance()->getAppData(callingAppId())->setIsSystemManager(isSystemManager);
}

int Jovie::say(const QString &text, int options) {
    // kDebug() << "Jovie::say: Adding '" << text << "' to queue.";
    Speaker * speaker = Speaker::Instance();
    return speaker->say(speaker->getAppData(callingAppId())->applicationName(), text, options);
}

int Jovie::sayFile(const QString &filename, const QString &encoding)
{
    // kDebug() << "Jovie::setFile: Running";
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

int Jovie::sayClipboard()
{
    // Get the clipboard object.
    QClipboard *cb = qApp->clipboard();

    // Copy text from the clipboard.
    QString text = cb->text();

    // Speak it.
    if (!text.isNull())
    {
        return Speaker::Instance()->say(callingAppId(), text, 0);
    } else {
        return 0;
    }
}

QStringList Jovie::outputModules()
{
    return Speaker::Instance()->outputModules();
}

QStringList Jovie::languagesByModule(const QString & module)
{
    return Speaker::Instance()->languagesByModule(module);
}

QStringList Jovie::getPossibleTalkers()
{
    return Speaker::Instance()->getPossibleTalkers();
}

void Jovie::setSpeed(int speed)
{
    if (speed < -100 || speed > 100) {
        kDebug() << "setSpeed called with out of range speed value: " << speed;
    }
    else {
        Speaker::Instance()->setSpeed(speed);
    }
}

int Jovie::speed()
{
    return Speaker::Instance()->speed();
}

void Jovie::setPitch(int pitch)
{
    if (pitch < -100 || pitch > 100) {
        kDebug() << "setPitch called with out of range pitch value: " << pitch;
    }
    else {
        Speaker::Instance()->setPitch(pitch);
    }
}

int Jovie::pitch()
{
    return Speaker::Instance()->pitch();
}

void Jovie::setVolume(int volume)
{
    if (volume < -100 || volume > 100) {
        kDebug() << "setVolume called with out of range volume value: " << volume;
    }
    else {
        Speaker::Instance()->setVolume(volume);
    }
}

int Jovie::volume()
{
    return Speaker::Instance()->volume();
}

void Jovie::setOutputModule(const QString & module)
{
    Speaker::Instance()->setOutputModule(module);
}

void Jovie::setLanguage(const QString & language)
{
    Speaker::Instance()->setLanguage(language);
}

void Jovie::setVoiceType(int voiceType)
{
    Speaker::Instance()->setVoiceType(voiceType);
}

int Jovie::voiceType()
{
    return Speaker::Instance()->voiceType();
}

void Jovie::stop()
{
    Speaker::Instance()->stop();
}

void Jovie::cancel()
{
    Speaker::Instance()->cancel();
}

void Jovie::pause()
{
    Speaker::Instance()->pause();
}

void Jovie::resume()
{
    Speaker::Instance()->resume();
}

void Jovie::removeJob(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

void Jovie::removeAllJobs()
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

int Jovie::getSentenceCount(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

int Jovie::getCurrentJob()
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

int Jovie::getJobCount(int priority)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

QStringList Jovie::getJobNumbers(int priority)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return QStringList();
}

int Jovie::getJobState(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

QByteArray Jovie::getJobInfo(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return QByteArray();
}

QString Jovie::getJobSentence(int jobNum, int sentenceNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return QString();
}

QStringList Jovie::getTalkerCodes()
{
    return TalkerMgr::Instance()->getTalkers();
}

QString Jovie::talkerToTalkerId(const QString &talker)
{
    return TalkerMgr::Instance()->talkerCodeToTalkerId(talker);
}

int Jovie::getTalkerCapabilities1(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

int Jovie::getTalkerCapabilities2(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return 0;
}

QStringList Jovie::getTalkerVoices(const QString &talker)
{
    // TODO:
    Q_UNUSED(talker);
    return QStringList();
}

void Jovie::changeJobTalker(int jobNum, const QString &talker)
{
    jobNum = applyDefaultJobNum(jobNum);
    Speaker::Instance()->setTalker(jobNum, talker);
}

void Jovie::moveJobLater(int jobNum)
{
    kDebug() << "not implemented in speech-dispatcher yet";
}

int Jovie::moveRelSentence(int jobNum, int n)
{
    kDebug() << "not implemented in speech-dispatcher yet";
    return 0;
}

void Jovie::showManagerDialog()
{
    QString cmd = QLatin1String( "kcmshell4 kcmkttsd --caption " );
    cmd += QLatin1Char( '\'' ) + i18n("KDE Text-to-Speech") + QLatin1Char( '\'' );
    KRun::runCommand(cmd,NULL);
}

void Jovie::kttsdExit()
{
    announceEvent(QLatin1String( "kttsdExit" ), QLatin1String( "kttsdExiting" ));
    emit kttsdExiting();
    qApp->quit();
}

void Jovie::init()
{
    new KSpeechAdaptor(this);
    if (ready()) {
        QDBusConnection::sessionBus().registerObject(QLatin1String( "/KSpeech" ), this, QDBusConnection::ExportAdaptors);
    }
}

void Jovie::reinit()
{
    // Reload ourself.
    kDebug() << "Jovie::reinit: Running";
    //if (Speaker::Instance()->isSpeaking())
    //    Speaker::Instance()->pause();
    Speaker::Instance()->init();
    QDBusConnection::sessionBus().unregisterObject(QLatin1String( "/KSpeech" ));
    if (ready()) {
        QDBusConnection::sessionBus().registerObject(QLatin1String( "/KSpeech" ), this, QDBusConnection::ExportAdaptors);
    }
    
    d->trayIcon->slotUpdateTalkersMenu();
}

void Jovie::setCallingAppId(const QString& appId)
{
    d->callingAppId = appId;
}

/* ---- Private Methods ------------------------------------------ */

bool Jovie::initializeConfigData()
{
    return true;
}

bool Jovie::ready()
{
    // TODO: add a check here to see if kttsd is ready (Speaker::Instance() will always be true...)
    //if (Speaker::Instance())
    //    return true;
    //kDebug() << "Jovie::ready: Starting KTTSD service";
//    if (!initializeSpeechData()) return false;
    if (!initializeTalkerMgr())
        return false;
    if (!initializeSpeaker())
        return false;
    announceEvent(QLatin1String( "ready" ), QLatin1String( "kttsdStarted" ));
    emit kttsdStarted();
    return true;
}

bool Jovie::initializeSpeechData()
{
    return true;
}

bool Jovie::initializeTalkerMgr()
{
    TalkerMgr::Instance()->loadTalkers(KGlobal::config().data());
    return true;
}

bool Jovie::initializeSpeaker()
{
    kDebug() << "Jovie::initializeSpeaker: Instantiating Speaker";

    Speaker::Instance()->init();

    // Establish ourself as a System Manager application.
    Speaker::Instance()->getAppData(QLatin1String( "jovie" ))->setIsSystemManager(true);

    return true;
}


void Jovie::slotJobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state)
{
    announceEvent(QLatin1String( "slotJobStateChanged" ), QLatin1String( "jobStateChanged" ), appId, jobNum, state);
    emit jobStateChanged(appId, jobNum, state);
}

void Jovie::slotMarker(const QString& appId, int jobNum, KSpeech::MarkerType markerType, const QString& markerData)
{
    announceEvent(QLatin1String( "slotMarker" ), QLatin1String( "marker" ), appId, jobNum, markerType, markerData);
    emit marker(appId, jobNum, markerType, markerData);
}

void Jovie::slotFilteringFinished()
{
    //Speaker::Instance()->doUtterances();
}

QString Jovie::callingAppId()
{
    // TODO: What would be nice is if there were a way to get the
    // last DBUS sender() without having to add DBusMessage to every
    // slot.  Then it would not be necessary to hand-edit the adaptor.
    return d->callingAppId;
}

int Jovie::applyDefaultJobNum(int jobNum)
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

void Jovie::announceEvent(const QString& slotName, const QString& eventName)
{
    kDebug() << "Jovie::" << slotName << ": emitting DBUS signal " << eventName;
}

void Jovie::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, KSpeech::MarkerType markerType, const QString& markerData)
{
    kDebug() << "Jovie::" << slotName << ": emitting DBUS signal " << eventName
        << " with appId " << appId << " job number " << jobNum << " marker type " << markerType << " and data " << markerData << endl;
}

void Jovie::announceEvent(const QString& slotName, const QString& eventName, const QString& appId,
    int jobNum, KSpeech::JobState state)
{
    kDebug() << "Jovie::" << slotName << ": emitting DBUS signal " << eventName <<
        " with appId " << appId << " job number " << jobNum << " and state " << /* SpeechJob::jobStateToStr(state) << */ endl;
}
