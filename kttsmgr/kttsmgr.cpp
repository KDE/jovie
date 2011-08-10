/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTS Manager Program
  --------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

// TQt includes.
#include <tqimage.h>

// KDE includes.
#include <kconfig.h>
#include <kuniqueapplication.h>
#include <kcmultidialog.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <ksystemtray.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kaboutapplication.h>
#include <dcopclient.h>
#include <kdeversion.h>

// KTTSMgr includes.
#include "kspeech.h"
#include "kttsmgr.h"

static const KCmdLineOptions options[] =
{
    { "s", 0, 0 },
    { "systray", I18N_NOOP("Start minimized in system tray"), 0 },
    { "a", 0, 0 },
    { "autoexit", I18N_NOOP("Exit when speaking is finished and minimized in system tray"), 0 },
    KCmdLineLastOption
};

int main (int argc, char *argv[])
{
    KGlobal::locale()->setMainCatalogue("kttsd");

    KAboutData aboutdata("kttsmgr", I18N_NOOP("KTTSMgr"),
        "0.3.5.2", I18N_NOOP("Text-to-Speech Manager"),
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
        kdDebug() << "kttsmgr is already running" << endl;
        return (0);
    }

    KUniqueApplication app;

#if KDE_VERSION >= KDE_MAKE_VERSION (3,3,90)
    TQPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Panel);
    aboutdata.setProgramLogo(icon.convertToImage());
#endif

    // The real work of KTTS Manager is done in the KControl Module kcmkttsd.
    KCMultiDialog dlg(KCMultiDialog::Plain, i18n("KDE Text-to-Speech Manager"), 0, "kttsmgrdlg", false);
    dlg.addModule("kcmkttsd");

    dlg.setIcon(KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small));

    // Get SysTray and ShowMainWindow options.
    KConfig* config = new KConfig("kttsdrc");
    config->setGroup("General");
    bool embedInSysTray = config->readBoolEntry("EmbedInSysTray", true);
    // Can only hide main window if we are in the system tray, otherwise, no way
    // for user to do anything.
    bool showMainWindowOnStartup = true;
    if (embedInSysTray)
        showMainWindowOnStartup = config->readBoolEntry("ShowMainWindowOnStartup", true);

    // If --systray option specified, start minimized in system tray.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("systray"))
    {
        embedInSysTray = true;
        showMainWindowOnStartup = false;
    }

    KttsMgrTray* tray = 0;
    if (embedInSysTray)
    {
        tray = new KttsMgrTray(&dlg);
        tray->show();
    }
    else app.setMainWidget(&dlg);

    if (showMainWindowOnStartup)
#if KDE_VERSION < KDE_MAKE_VERSION (3,3,0)
        dlg.show();
#else
    {
        if (embedInSysTray)
            tray->setActive();
        else
            dlg.show();
    }
#endif
    int result = app.exec();
    delete tray;
    return result;
}

/*  KttsToolTip class */

KttsToolTip::KttsToolTip ( TQWidget* parent ) : TQToolTip(parent)
{
}

/*virtual*/ void KttsToolTip::maybeTip ( const TQPoint & p )
{
    Q_UNUSED(p);

    if (!parentWidget()->inherits("KttsMgrTray"))
        return;

    KttsMgrTray* kttsMgrTray = dynamic_cast<KttsMgrTray*>(parentWidget());

    TQRect r(kttsMgrTray->tqgeometry());
    if ( !r.isValid() )
        return;

    TQString status = "<qt><b>KTTSMgr</b> - ";
    status += i18n("<qt>Text-to-Speech Manager");
    status += "<br/><br/>";
    status += kttsMgrTray->gettqStatus();
    status += "</qt>";

    tip(r, status);
}

/*  KttsMgrTray class */

KttsMgrTray::KttsMgrTray(TQWidget *parent):
    DCOPStub("kttsd", "KSpeech"),
    DCOPObject("kkttsmgr_kspeechsink"),
    KSystemTray(parent, "kttsmgrsystemtray")
{
    TQPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small);
    setPixmap (icon);

    // TQToolTip::add(this, i18n("Text-to-speech manager"));
    m_toolTip = new KttsToolTip(this);

    int id;
    id = contextMenu()->idAt(0);
    if (id != -1) contextMenu()->changeTitle(id, icon, "KTTSMgr");

    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("klipper", KIcon::Small),
        i18n("&Speak Clipboard Contents"), this, TQT_SLOT(speakClipboardSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("stop", KIcon::Small),
        i18n("&Hold"), this, TQT_SLOT(holdSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("exec", KIcon::Small),
        i18n("Resume"), this, TQT_SLOT(resumeSelected()));
    id = contextMenu()->insertSeparator();
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("contents", KIcon::Small),
        i18n("KTTS &Handbook"), this, TQT_SLOT(helpSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small),
        i18n("&About KTTSMgr"), this, TQT_SLOT(aboutSelected()));

    connect(this, TQT_SIGNAL(quitSelected()), this, TQT_SLOT(quitSelected()));
    // If --autoexit option given, exit when speaking stops.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("autoexit"))
    {
        connectDCOPSignal("kttsd", "KSpeech",
            "textFinished(TQCString,uint)",
            "textFinished(TQCString,uint)",
            false);
        // Install an event filter so we can check when KTTSMgr becomes inconified to the systray.
        parent->installEventFilter(this);
    }
}

KttsMgrTray::~KttsMgrTray()
{
    delete m_toolTip;
}

void KttsMgrTray::textFinished(const TQCString& /*appId*/, uint /*jobNum*/)
{
    // kdDebug() << "KttsMgrTray::textFinished: running" << endl;
    exitWhenFinishedSpeaking();
}

/*virtual*/ bool KttsMgrTray::eventFilter( TQObject* /*o*/, TQEvent* e )
{
    if ( e->type() == TQEvent::Hide ) exitWhenFinishedSpeaking();
    return false;
}

void KttsMgrTray::exitWhenFinishedSpeaking()
{
    // kdDebug() << "KttsMgrTray::exitWhenFinishedSpeaking: running" << endl;
    if ( parentWidget()->isShown() ) return;
    TQString jobNums = getTextJobNumbers();
    TQStringList jobNumsList = TQStringList::split(jobNums, ",");
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

/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
TQString KttsMgrTray::stateToStr(int state)
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

TQString KttsMgrTray::gettqStatus()
{
    if (!isKttsdRunning()) return i18n("Text-to-Speech System is not running");
    uint jobCount = getTextJobCount();
    TQString status = i18n("1 job", "%n jobs", jobCount);
    if (jobCount > 0)
    {
        uint job = getCurrentTextJob();
        int jobState = 0;
        if (job != 0)
        {
            // kdDebug() << "KttsMgrTray::gettqStatus: job = " << job << endl;
            jobState = getTextJobState(job);
            int sentenceCount = getTextCount(job);
            uint seq = moveRelTextSentence(0, job);
            status += i18n(", current job %1 at sentence %2 of %3 sentences"
                ).tqarg(stateToStr(jobState)).tqarg(seq).tqarg(sentenceCount);
        }
    }
    return status;
}

void KttsMgrTray::speakClipboardSelected()
{
    if (!isKttsdRunning())
    {
        TQString error;
        if (KApplication::startServiceByDesktopName("kttsd", TQStringList(), &error) != 0)
            kdError() << "Starting KTTSD failed with message " << error << endl;
    }
    speakClipboard();
}

void KttsMgrTray::aboutSelected()
{
    KAboutApplication aboutDlg(kapp->aboutData(), 0, "kttsmgraboutdlg", true);
    aboutDlg.exec();
}

void KttsMgrTray::helpSelected()
{
    kapp->invokeHelp(TQString(),"kttsd");
}

void KttsMgrTray::quitSelected()
{
    // kdDebug() << "Running KttsMgrTray::quitSelected" << endl;
    kapp->quit();
}

void KttsMgrTray::holdSelected()
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

bool KttsMgrTray::isKttsdRunning()
{
    DCOPClient *client = kapp->dcopClient();
    return (client->isApplicationRegistered("kttsd"));
}

#include "kttsmgr.moc"
