/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSMgr System Tray Application
  -------------------------------
  Copyright:
  (C) 2004-2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>

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
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QToolTip>

// KDE includes.
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <ksystemtrayicon.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kaboutapplicationdialog.h>
#include <ktoolinvocation.h>
#include <kprocess.h>
#include <klocale.h>
#include <kicon.h>
#include <kconfig.h>
#include <kspeech.h>

// KTTSMgr includes.
#include "kttsmgr.h"

static const KCmdLineOptions options[] =
{
    { "a", 0, 0 },
    { "autoexit", I18N_NOOP("Exit when speaking is finished"), 0 },
    KCmdLineLastOption
};

int main (int argc, char *argv[])
{
    KGlobal::locale()->setMainCatalog("kttsd");

    KAboutData aboutdata("kttsmgr", I18N_NOOP("KTTSMgr"),
        "0.4.0", I18N_NOOP("Text-to-Speech Manager"),
        KAboutData::License_GPL, "(C) 2002, José Pablo Ezequiel Fernández");
    aboutdata.addAuthor("José Pablo Ezequiel Fernández",I18N_NOOP("Original Author"),"pupeno@pupeno.com");
    aboutdata.addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer"),"garycramblitt@comcast.net");
    aboutdata.addAuthor("Gunnar Schmi Dt", I18N_NOOP("Contributor"),"gunnar@schmi-dt.de");
    aboutdata.addAuthor("Olaf Schmidt", I18N_NOOP("Contributor"),"ojschmidt@kde.org");
    aboutdata.addAuthor("Paul Giannaros", I18N_NOOP("Contributor"), "ceruleanblaze@gmail.com");
    aboutdata.addCredit("Jorge Luis Arzola", I18N_NOOP("Testing"), "arzolacub@hotmail.com");
    aboutdata.addCredit("David Powell", I18N_NOOP("Testing"), "achiestdragon@gmail.com");

    KCmdLineArgs::init( argc, argv, &aboutdata );

    KCmdLineArgs::addCmdLineOptions( options );

    KUniqueApplication::addCmdLineOptions();

    KUniqueApplication::setOrganizationDomain("kde.org");
    KUniqueApplication::setApplicationName("KttsMgr");
    KUniqueApplication app;

    if(!KUniqueApplication::start())
    {
        kDebug() << "kttsmgr is already running" << endl;
        return (0);
    }

    QPixmap icon = KIconLoader::global()->loadIcon("kttsd", K3Icon::Panel);
    aboutdata.setProgramLogo(icon.toImage());

    KttsMgrTray* tray = new KttsMgrTray();
    tray->show();

    int result = app.exec();
    delete tray;
    return result;
}

/* ------------------  KttsMgrTray class ----------------------- */

KttsMgrTray::KttsMgrTray(QWidget *parent):
    KSystemTrayIcon("kttsd", parent),
    m_kspeech(0)
{
    setObjectName("kttsmgrsystemtray");

//    QIcon icon = KGlobal::iconLoader()->loadIcon("kttsd", K3Icon::Small);
//    setIcon (icon);

    // Start KTTS daemon if enabled and if not already running.
    KConfig config("kttsdrc");
    config.setGroup("General");
    if (config.readEntry("EnableKttsd", false))
    {
        if (!isKttsdRunning())
        {
            QString error;
            if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error) != 0)
                kDebug() << "Starting KTTSD failed with message " << error << endl;
            else
                isKttsdRunning();
        }
    }

    // Set up menu.
    QAction *act;

    actStop = contextMenu()->addAction (
        i18n("&Stop/Delete"), this, SLOT(stopSelected()));
    actStop->setIcon(KIcon("player_stop"));
    actPause = contextMenu()->addAction (
        i18n("&Pause"), this, SLOT(pauseSelected()));
    actPause->setIcon(KIcon("player_pause"));
    actResume = contextMenu()->addAction (
        i18n("&Resume"), this, SLOT(resumeSelected()));
    actResume->setIcon(KIcon("player_play"));
    actRepeat = contextMenu()->addAction (
        i18n("R&epeat"), this, SLOT(repeatSelected()));
    actRepeat->setIcon(KIcon("reload"));
    act = contextMenu()->addSeparator();
    actSpeakClipboard = contextMenu()->addAction (
        i18n("Spea&k Clipboard Contents"), this, SLOT(speakClipboardSelected()));
    act->setIcon(KIcon("klipper"));
    actConfigure = contextMenu()->addAction (
        i18n("&Configure"), this, SLOT(configureSelected()));
    actConfigure->setIcon(KIcon("configure"));
    act = contextMenu()->addSeparator();
    act = contextMenu()->addAction (
        i18n("KTTS &Handbook"), this, SLOT(helpSelected()));
    act->setIcon(KIcon("contents"));
    act = contextMenu()->addAction (
        i18n("&About KTTSMgr"), this, SLOT(aboutSelected()));
    act->setIcon(KIcon("kttsd"));

    connect(this, SIGNAL(quitSelected()),
                  SLOT(quitSelected()));
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                  SLOT(slotActivated(QSystemTrayIcon::ActivationReason)));
}

KttsMgrTray::~KttsMgrTray()
{
}

bool KttsMgrTray::event(QEvent *event)
{
    // TODO: This event only fires on X11 systems.
    // To make it work on all platforms, would have to constantly monitor status and update,
    // which would suck up huge amounts of CPU.
    if (event->type() == QEvent::ToolTip) {
        QString status = "<qt><b>KTTSMgr</b> - ";
        status += i18n("<qt>Text-to-Speech Manager");
        status += "<br/><br/>";
        status += getStatus();
        status += "</qt>";
        // kDebug() << "KttsMgrTray::event status = " << status << endl;
        setToolTip(status);
    }
    return false;
}

void KttsMgrTray::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
    // Convert left-click into a right-click.
    if (reason == QSystemTrayIcon::Trigger)
       contextMenu()->exec();
}

/*virtual*/ void KttsMgrTray::contextMenuAboutToShow(KMenu* menu)
{
    Q_UNUSED(menu);
    // Enable menu items based on current KTTSD state.
    bool kttsdRunning = isKttsdRunning();
    int jobState = -1;
    if (kttsdRunning)
    {
        int jobNum = m_kspeech->getCurrentJob();
        if (jobNum != 0)
        {
            // kDebug() << "KttsMgrTray::getStatus: jobNum = " << jobNum << endl;
            jobState = m_kspeech->getJobState(jobNum);
        }
    }
    actStop->setEnabled(jobState != -1);
    actPause->setEnabled(jobState == KSpeech::jsSpeaking);
    actResume->setEnabled(jobState == KSpeech::jsPaused);
    actRepeat->setEnabled(jobState != -1);
    actSpeakClipboard->setEnabled(kttsdRunning);
    bool configActive = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kcmshell_kcmkttsd"));
    actConfigure->setEnabled(!configActive);
}

void KttsMgrTray::exitWhenFinishedSpeaking()
{
    // kDebug() << "KttsMgrTray::exitWhenFinishedSpeaking: running" << endl;
    QStringList jobNumsList = m_kspeech->getJobNumbers(KSpeech::jpAll);
    uint jobNumsListCount = jobNumsList.count();
    // Since there can only be 2 Finished jobs at a time, more than 2 jobs means at least
    // one job is not Finished.
    if (jobNumsListCount > 2) return;
    // Exit if all jobs are Finished or there are no jobs.
    for (uint ndx=0; ndx < jobNumsListCount; ++ndx)
    {
        if (m_kspeech->getJobState(jobNumsList[ndx].toInt()) != KSpeech::jsFinished) return;
    }
    kapp->quit();
}

void KttsMgrTray::jobStateChanged(const QString &appId, int jobNum, int state)
{
    Q_UNUSED(appId);
    Q_UNUSED(jobNum);
    if (KSpeech::jsFinished == state)
        exitWhenFinishedSpeaking();
}

/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
QString KttsMgrTray::stateToStr(int state)
{
    switch( state )
    {
        case KSpeech::jsQueued: return        i18n("Queued");
        case KSpeech::jsSpeakable: return     i18n("Waiting");
        case KSpeech::jsSpeaking: return      i18n("Speaking");
        case KSpeech::jsPaused: return        i18n("Paused");
        case KSpeech::jsFinished: return      i18n("Finished");
        default: return                       i18n("Unknown");
    }
}

QString KttsMgrTray::getStatus()
{
    if (!isKttsdRunning()) return i18n("Text-to-Speech System is not running");
    int jobCount = m_kspeech->getJobCount(KSpeech::jpAll);
    QString status = i18np("1 job", "%n jobs", jobCount);
    if (jobCount > 0)
    {
        int jobNum = m_kspeech->getCurrentJob();
        if (jobNum != 0)
        {
            // kDebug() << "KttsMgrTray::getStatus: job = " << job << endl;
            int jobState = m_kspeech->getJobState(jobNum);
            int sentenceCount = m_kspeech->getSentenceCount(jobNum);
            int seq = m_kspeech->moveRelSentence(jobNum, 0);
            status += i18n(", current job %1 at sentence %2 of %3 sentences"
                , stateToStr(jobState), seq, sentenceCount);
        }
    }
    return status;
}

void KttsMgrTray::speakClipboardSelected()
{
    if (!isKttsdRunning())
    {
        QString error;
        if (KToolInvocation::startServiceByDesktopName("kttsd", QString(), &error) != 0)
            kError() << "Starting KTTSD failed with message " << error << endl;
    }
    m_kspeech->sayClipboard();
}

void KttsMgrTray::aboutSelected()
{
    KAboutApplicationDialog aboutDlg(KGlobal::mainComponent().aboutData());
    aboutDlg.exec();
}

void KttsMgrTray::helpSelected()
{
    KToolInvocation::invokeHelp(QString(),"kttsd");
}

void KttsMgrTray::quitSelected()
{
    // kDebug() << "Running KttsMgrTray::quitSelected" << endl;
    kapp->quit();
}

void KttsMgrTray::stopSelected()
{
    if (isKttsdRunning())
        m_kspeech->removeAllJobs();
}

void KttsMgrTray::pauseSelected()
{
    if (isKttsdRunning())
        m_kspeech->pause();
}

void KttsMgrTray::resumeSelected()
{
    if (isKttsdRunning())
        m_kspeech->resume();
}

void KttsMgrTray::repeatSelected()
{
    if (isKttsdRunning())
    {
        int jobNum = m_kspeech->getCurrentJob();
        int seq = m_kspeech->moveRelSentence(jobNum, 0);
        m_kspeech->moveRelSentence(jobNum, -seq);
    }
}

void KttsMgrTray::configureSelected()
{
    KProcess proc;
    proc << "kcmshell" << "kcmkttsd" << "--caption" << i18n("KDE Text-to-Speech");
    proc.start(KProcess::DontCare);
}

bool KttsMgrTray::isKttsdRunning()
{
    bool isRunning = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd"));
    if (isRunning) {
        if (!m_kspeech) {
            m_kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
            m_kspeech->setParent(this);
            m_kspeech->setApplicationName("KttsMgr");
            m_kspeech->setIsSystemManager(true);
            // If --autoexit option given, exit when speaking stops.
            KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
            if (args->isSet("autoexit"))
                connect(m_kspeech, SIGNAL(jobStateChanged(const QString&, int, int)),
                    this, SLOT(jobStateChanged(const QString&, int, int)));
        }
    } else {
        delete m_kspeech;
        m_kspeech = 0;
    }
    return isRunning;
}

#include "kttsmgr.moc"
