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
#include <ksystemtray.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kaboutapplication.h>
#include <dcopclient.h>
#include <ktoolinvocation.h>
#include <kprocess.h>
#include <klocale.h>
#include <kicon.h>
#include <kconfig.h>

// KTTSMgr includes.
#include "kspeech.h"
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

    if(!KUniqueApplication::start())
    {
        kDebug() << "kttsmgr is already running" << endl;
        return (0);
    }

    KUniqueApplication app;

    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", K3Icon::Panel);
    aboutdata.setProgramLogo(icon.toImage());

    KttsMgrTray* tray = new KttsMgrTray();
    tray->show();

    int result = app.exec();
    delete tray;
    return result;
}

/* ------------------  KttsMgrTray class ----------------------- */

KttsMgrTray::KttsMgrTray(QWidget *parent):
    DCOPStub("kttsd", "KSpeech"),
    DCOPObject("kkttsmgr_kspeechsink"),
    KSystemTray(parent)
{
    setObjectName("kttsmgrsystemtray");
    
    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", K3Icon::Small);
    setPixmap (icon);
    
    // Start KTTS daemon if enabled and if not already running.
    KConfig config("kttsdrc");
    config.setGroup("General");
    if (config.readEntry("EnableKttsd", false))
    {
        if (!isKttsdRunning())
        {
            QString error;
            if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error))
                kDebug() << "Starting KTTSD failed with message " << error << endl;
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

    connect(this, SIGNAL(quitSelected()), this, SLOT(quitSelected()));
    
    // If --autoexit option given, exit when speaking stops.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("autoexit"));
    {
        connectDCOPSignal("kttsd", "KSpeech",
            "textFinished(QByteArray,uint)",
            "textFinished(QByteArray,uint)",
            false);
    }
}

KttsMgrTray::~KttsMgrTray()
{
}

bool KttsMgrTray::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QString status = "<qt><b>KTTSMgr</b> - ";
        status += i18n("<qt>Text-to-Speech Manager");
        status += "<br/><br/>";
        status += getStatus();
        status += "</qt>";
        QToolTip::showText(helpEvent->globalPos(), status);
    }
    return QWidget::event(event);
}

void KttsMgrTray::mousePressEvent(QMouseEvent* ev)
{
    // Convert left-click into a right-click.
    if (ev->button() == Qt::LeftButton) {
        ev->accept();
        QMouseEvent* myEv = new QMouseEvent(
            QEvent::MouseButtonPress,
            ev->pos(),
            Qt::RightButton,
            Qt::RightButton,
            Qt::NoModifier);
        KSystemTray::mousePressEvent(myEv);
     } else
        KSystemTray::mousePressEvent(ev);
}

/*virtual*/ void KttsMgrTray::contextMenuAboutToShow(KMenu* menu)
{
    Q_UNUSED(menu);
    // Enable menu items based on current KTTSD state.
    bool kttsdRunning = isKttsdRunning();
    int jobState = -1;
    if (kttsdRunning)
    {
        uint job = getCurrentTextJob();
        if (job != 0)
        {
            // kDebug() << "KttsMgrTray::getStatus: job = " << job << endl;
            jobState = getTextJobState(job);
        }
    }
    actStop->setEnabled(jobState != -1);
    actPause->setEnabled(jobState == jsSpeaking);
    actResume->setEnabled(jobState == jsPaused);
    actRepeat->setEnabled(jobState != -1);
    actSpeakClipboard->setEnabled(kttsdRunning);
    DCOPClient *client = kapp->dcopClient();
    bool configActive (client->isApplicationRegistered("kcmshell_kcmkttsd"));
    actConfigure->setEnabled(!configActive);
}

void KttsMgrTray::exitWhenFinishedSpeaking()
{
    // kDebug() << "KttsMgrTray::exitWhenFinishedSpeaking: running" << endl;
    QString jobNums = getTextJobNumbers();
    QStringList jobNumsList = jobNums.split(",");
    uint jobNumsListCount = jobNumsList.count();
    // Since there can only be 2 Finished jobs at a time, more than 2 jobs means at least
    // one job is not Finished.
    if (jobNumsListCount > 2) return;
    // Exit if all jobs are Finished or there are no jobs.
    for (uint ndx=0; ndx < jobNumsListCount; ++ndx)
    {
        if (getTextJobState(jobNumsList[ndx].toInt()) != KSpeech::jsFinished) return;
    }
    kapp->quit();
}

void KttsMgrTray::textFinished(const QByteArray& /*appId*/, uint /*jobNum*/)
{
    // kDebug() << "KttsMgrTray::textFinished: running" << endl;
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
    uint jobCount = getTextJobCount();
    QString status = i18np("1 job", "%n jobs", jobCount);
    if (jobCount > 0)
    {
        uint job = getCurrentTextJob();
        int jobState = 0;
        if (job != 0)
        {
            // kDebug() << "KttsMgrTray::getStatus: job = " << job << endl;
            jobState = getTextJobState(job);
            int sentenceCount = getTextCount(job);
            uint seq = moveRelTextSentence(0, job);
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
        if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error) != 0)
            kError() << "Starting KTTSD failed with message " << error << endl;
    }
    speakClipboard();
}

void KttsMgrTray::aboutSelected()
{
    KAboutApplication aboutDlg(kapp->aboutData(), 0, true);
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
    {
        uint jobNum = getCurrentTextJob();
        removeText(jobNum);
    }
}

void KttsMgrTray::pauseSelected()
{
    if (isKttsdRunning())
    {
        uint jobNum = getCurrentTextJob();
        pauseText(jobNum);
    }
}

void KttsMgrTray::resumeSelected()
{
    if (isKttsdRunning())
    {
        uint jobNum = getCurrentTextJob();
        resumeText(jobNum);
    }
}

void KttsMgrTray::repeatSelected()
{
    if (isKttsdRunning())
    {
        uint jobNum = getCurrentTextJob();
        startText(jobNum);
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
    DCOPClient *client = kapp->dcopClient();
    return (client->isApplicationRegistered("kttsd"));
}

#include "kttsmgr.moc"
