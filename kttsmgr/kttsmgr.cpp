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
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtCore/QProcess>

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
#include <klocale.h>
#include <kicon.h>
#include <kconfig.h>
#include <kspeech.h>

// KTTSMgr includes.
#include "kttsmgr.h"

int main (int argc, char *argv[])
{
    KGlobal::locale()->setMainCatalog("kttsd");

    KAboutData aboutdata("kttsmgr", 0, ki18n("KTTSMgr"),
        "0.4.0", ki18n("Text-to-Speech Manager"),
        KAboutData::License_GPL, ki18n("(C) 2002, José Pablo Ezequiel Fernández"));
    aboutdata.addAuthor(ki18n("José Pablo Ezequiel Fernández"),ki18n("Original Author"),"pupeno@pupeno.com");
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Maintainer"),"garycramblitt@comcast.net");
    aboutdata.addAuthor(ki18n("Gunnar Schmi Dt"), ki18n("Contributor"),"gunnar@schmi-dt.de");
    aboutdata.addAuthor(ki18n("Olaf Schmidt"), ki18n("Contributor"),"ojschmidt@kde.org");
    aboutdata.addAuthor(ki18n("Paul Giannaros"), ki18n("Contributor"), "ceruleanblaze@gmail.com");
    aboutdata.addCredit(ki18n("Jorge Luis Arzola"), ki18n("Testing"), "arzolacub@hotmail.com");
    aboutdata.addCredit(ki18n("David Powell"), ki18n("Testing"), "achiestdragon@gmail.com");

    KCmdLineArgs::init( argc, argv, &aboutdata );


    KCmdLineOptions options;

    options.add("a");

    options.add("autoexit", ki18n("Exit when speaking is finished"));

    KCmdLineArgs::addCmdLineOptions( options );

    KUniqueApplication::addCmdLineOptions();

    KUniqueApplication::setOrganizationDomain("kde.org");
    KUniqueApplication::setApplicationName("KttsMgr");
    KUniqueApplication app;

    if(!KUniqueApplication::start())
    {
        kDebug() << "kttsmgr is already running";
        return (0);
    }

    QPixmap icon = KIconLoader::global()->loadIcon("preferences-desktop-text-to-speech", KIconLoader::Panel);
    aboutdata.setProgramLogo(icon.toImage());

    KttsMgrTray* tray = new KttsMgrTray();
    tray->show();

    int result = app.exec();
    delete tray;
    return result;
}

/* ------------------  KttsMgrTray class ----------------------- */

KttsMgrTray::KttsMgrTray(QWidget *parent):
    KSystemTrayIcon("preferences-desktop-text-to-speech", parent),
    m_kspeech(0)
{
    setObjectName("kttsmgrsystemtray");

//    QIcon icon = KIconLoader::global()->loadIcon("preferences-desktop-text-to-speech", KIconLoader::Small);
//    setIcon (icon);

    // Start KTTS daemon if enabled and if not already running.
    KConfig _config( "kttsdrc" );
    KConfigGroup config(&_config, "General");
    if (config.readEntry("EnableKttsd", false))
    {
        if (!isKttsdRunning())
        {
            QString error;
            if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error) != 0)
                kDebug() << "Starting KTTSD failed with message " << error;
            else
                isKttsdRunning();
        }
    }

    // Set up menu.
    QAction *act;

    actStop = contextMenu()->addAction (
        i18n("&Stop/Delete"), this, SLOT(stopSelected()));
    actStop->setIcon(KIcon("media-playback-stop"));
    actPause = contextMenu()->addAction (
        i18n("&Pause"), this, SLOT(pauseSelected()));
    actPause->setIcon(KIcon("media-playback-pause"));
    actResume = contextMenu()->addAction (
        i18n("&Resume"), this, SLOT(resumeSelected()));
    actResume->setIcon(KIcon("media-playback-start"));
    actRepeat = contextMenu()->addAction (
        i18n("R&epeat"), this, SLOT(repeatSelected()));
    actRepeat->setIcon(KIcon("view-refresh"));
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
    act->setIcon(KIcon("help-contents"));
    act = contextMenu()->addAction (
        i18n("&About KTTSMgr"), this, SLOT(aboutSelected()));
    act->setIcon(KIcon("preferences-desktop-text-to-speech"));

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
        status += "<br /><br />";
        status += getStatus();
        status += "</qt>";
        // kDebug() << "KttsMgrTray::event status = " << status;
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
            // kDebug() << "KttsMgrTray::getStatus: jobNum = " << jobNum;
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
    // kDebug() << "KttsMgrTray::exitWhenFinishedSpeaking: running";
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
    QString status = i18np("1 job", "%1 jobs", jobCount);
    if (jobCount > 0)
    {
        int jobNum = m_kspeech->getCurrentJob();
        if (jobNum != 0)
        {
            // kDebug() << "KttsMgrTray::getStatus: job = " << job;
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
    // kDebug() << "Running KttsMgrTray::quitSelected";
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
    QStringList lst;
	lst << "kcmkttsd" << "--caption" << i18n("KDE Text-to-Speech");
	QProcess::startDetached("kcmshell4",lst);
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
