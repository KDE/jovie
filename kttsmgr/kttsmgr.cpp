/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD main class
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License or later.      *
 *                                                                         *
 ***************************************************************************/

// Qt includes.
#include <qimage.h>
#include <qtooltip.h>

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
    { "systray", I18N_NOOP("Start minimized in system tray."), 0 },
    { "a", 0, 0 },
    { "autoexit", I18N_NOOP("Exit when speaking is finished and minimized in system tray."), 0 }
};

int main (int argc, char *argv[])
{
    KGlobal::locale()->setMainCatalogue("kttsd");

    KAboutData aboutdata("kttsmgr", I18N_NOOP("KTTSMgr"),
        "0.3.0", I18N_NOOP("Text-to-Speech Manager"),
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
    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Panel);
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

/*  KttsMgrTray class */

KttsMgrTray::KttsMgrTray(QWidget *parent):
    DCOPStub("kttsd", "KSpeech"),
    KSystemTray(parent, "kttsmgrsystemtray")
{
    QPixmap icon = KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small);
    setPixmap (icon);

    QToolTip::add(this, i18n("Text-to-Speech Manager"));

    int id;
    id = contextMenu()->idAt(0);
    if (id != -1) contextMenu()->changeTitle(id, icon, "KTTSMgr");

    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("klipper", KIcon::Small),
        i18n("&Speak Clipboard Contents"), this, SLOT(speakClipboardSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("stop", KIcon::Small),
        i18n("&Hold"), this, SLOT(holdSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("exec", KIcon::Small),
        i18n("Resume"), this, SLOT(resumeSelected()));
    id = contextMenu()->insertSeparator();
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("contents", KIcon::Small),
        i18n("KTTS &Handbook"), this, SLOT(helpSelected()));
    id = contextMenu()->insertItem (KGlobal::iconLoader()->loadIcon("kttsd", KIcon::Small),
        i18n("&About KTTSMgr"), this, SLOT(aboutSelected()));

    connect(this, SIGNAL(quitSelected()), this, SLOT(quitSelected()));
    // If --autoexit option given, exit when speaking stops.
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
/*    if (args->isSet("autoexit"))
    {
        connectDCOPSignal("kttsd", "KSpeech",
            "textFinished(QCString,uint)",
            "textFinished(QCString,uint)",
            false);
        // Install an event filter so we can check when KTTSMgr becomes inconified to the systray.
        parent->installEventFilter(this);
    }*/
}

KttsMgrTray::~KttsMgrTray() { }

void KttsMgrTray::textFinished(const QCString& /*appId*/, uint /*jobNum*/)
{
    kdDebug() << "KttsMgrTray::textFinished: running" << endl;
    exitWhenFinishedSpeaking();
}

/*virtual*/ bool KttsMgrTray::eventFilter( QObject* /*o*/, QEvent* e )
{
    if ( e->type() == QEvent::Hide ) exitWhenFinishedSpeaking();
    return false;
}

void KttsMgrTray::exitWhenFinishedSpeaking()
{
    // kdDebug() << "KttsMgrTray::exitWhenFinishedSpeaking: running" << endl;
    if ( parentWidget()->isShown() ) return;
    QString jobNums = getTextJobNumbers();
    QStringList jobNumsList = QStringList::split(jobNums, ",");
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

void KttsMgrTray::speakClipboardSelected()
{
    if (!isKttsdRunning())
    {
        QString error;
        if (KApplication::startServiceByDesktopName("kttsd", QStringList(), &error) != 0)
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
    kapp->invokeHelp(QString::null,"kttsd");
}

void KttsMgrTray::quitSelected()
{
    kdDebug() << "Running KttsMgrTray::quitSelected" << endl;
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
