/***************************************************** vim:set ts=4 sw=4 sts=4:
  Jovie System Tray Application
  -------------------------------
  Copyright 2004-2006 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2010 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------

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

// Jovie includes.
#include "jovietrayicon.h"
#include "jovie.h"

// Qt includes.
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QPointer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

// KDE includes.
#include <kactioncollection.h>
#include <kaboutapplicationdialog.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kshortcutsdialog.h>
#include <ksystemtrayicon.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kuniqueapplication.h>

/* ------------------  JovieTrayIcon class ----------------------- */

JovieTrayIcon::JovieTrayIcon(QWidget *parent)
    :KStatusNotifierItem(QLatin1String( "jovie" ), parent)
{
    setObjectName( QLatin1String("jovietrayicon" ));
    setStatus(KStatusNotifierItem::Active);
    setCategory(ApplicationStatus);

    QString status = QLatin1String( "Jovie - " );
    status += i18n("KDE Text-to-Speech Manager");
    setToolTipTitle(status);
    setToolTipIconByName(QLatin1String( "preferences-desktop-text-to-speech" ));
    setIconByName(QLatin1String( "preferences-desktop-text-to-speech" ));

    // Set up menu.
    actStop = actionCollection()->addAction(QLatin1String("stop"), this, SLOT(stopSelected()));
    actStop->setIcon(KIcon(QLatin1String("media-playback-stop")));
    actStop->setText(i18n("&Stop/Delete"));
    actStop->setGlobalShortcut(KShortcut());
    contextMenu()->addAction(actStop);

    actPause = actionCollection()->addAction(QLatin1String("pause"), this, SLOT(pauseSelected()));
    actPause->setIcon(KIcon( QLatin1String( "media-playback-pause" )));
    actPause->setText(i18n("&Pause"));
    actPause->setGlobalShortcut(KShortcut());
    contextMenu()->addAction(actPause);

    actResume = actionCollection()->addAction(QLatin1String("resume"), this, SLOT(resumeSelected()));
    actResume->setIcon(KIcon(QLatin1String( "media-playback-start" )));
    actResume->setText(i18n("&Resume"));
    actResume->setGlobalShortcut(KShortcut());
    contextMenu()->addAction(actResume);

    actRepeat = actionCollection()->addAction(QLatin1String("repeat"), this, SLOT(repeatSelected()));
    actRepeat->setIcon(KIcon(QLatin1String( "view-refresh" )));
    actRepeat->setText(i18n("R&epeat"));
    actRepeat->setGlobalShortcut(KShortcut());
    contextMenu()->addAction(actRepeat);

    (void)contextMenu()->addSeparator();

    actSpeakClipboard = actionCollection()->addAction(QLatin1String("speakclipboard"),
                        this, SLOT(speakClipboardSelected()));
    actSpeakClipboard->setIcon(KIcon(QLatin1String( "klipper" )));
    actSpeakClipboard->setText(i18n("Spea&k Clipboard Contents"));
    actSpeakClipboard->setGlobalShortcut(KShortcut());
    contextMenu()->addAction (actSpeakClipboard);

    talkersMenu = new QMenu(i18n("Select Talker"));
    contextMenu()->addMenu(talkersMenu);

    slotUpdateTalkersMenu();

    actConfigure = KStandardAction::preferences(this, SLOT(configureSelected()), contextMenu());
    contextMenu()->addAction(actConfigure);

    contextMenu()->addAction(
        KStandardAction::keyBindings(this, SLOT(configureKeysSelected()), contextMenu()));

    (void)contextMenu()->addSeparator();
    (void)contextMenu()->addAction(
        KStandardAction::helpContents(this, SLOT(helpSelected()), contextMenu()));
    (void)contextMenu()->addAction (
        KStandardAction::aboutApp(this, SLOT(aboutSelected()), contextMenu()));

    connect(this, SIGNAL(activateRequested(bool,QPoint)),
            SLOT(slotActivateRequested(bool,QPoint)));
    connect(contextMenu(), SIGNAL(aboutToShow()),
            SLOT(contextMenuAboutToShow()));
}

JovieTrayIcon::~JovieTrayIcon()
{
}

void JovieTrayIcon::slotUpdateTalkersMenu(){
    talkersMenu->clear();
    talkersActions.clear();
    
    // Object for the KTTSD configuration.
    KConfig config(QLatin1String( "kttsdrc" ));

    // Load existing Talkers into Talker List.
    TalkerListModel talkerListModel;
    talkerListModel.loadTalkerCodesFromConfig(&config);

    TalkerCode::TalkerCodeList list = talkerListModel.datastore();

    for (int i=0;i<list.size();i++) {
       TalkerCode talkerCode=list.at(i);
       KAction* act=new KAction(this);
       act->setText(talkerCode.name());
       act->setProperty("talkercode",talkerCode.getTalkerCode());
       connect(act,SIGNAL(triggered()),this,SLOT(talkerSelected()));
       talkersActions.push_back(act);
       talkersMenu->addAction(act);
    }
}

void JovieTrayIcon::slotActivateRequested(bool active, const QPoint &pos)
{
    Q_UNUSED(active)
    Q_UNUSED(pos)
    // pause/resume icon/app
    if (overlayIconName() == QLatin1String("media-playback-pause")) {
        Jovie::Instance()->resume();
        setOverlayIconByName(QLatin1String( "" ));
    }
    else {
        Jovie::Instance()->pause();
        setOverlayIconByName(QLatin1String( "media-playback-pause" ));
    }
}

void JovieTrayIcon::contextMenuAboutToShow()
{
    const bool configActive = (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String( "org.kde.kcmshell_kcmkttsd" )));
    actConfigure->setEnabled(!configActive);
}

void JovieTrayIcon::speakClipboardSelected()
{
    Jovie::Instance()->sayClipboard();
}

void JovieTrayIcon::talkerSelected()
{
    KAction* act=(KAction*)QObject::sender ();
    TalkerCode talkerCode;
    talkerCode.setTalkerCode(act->property("talkercode").toString());
    Jovie::Instance()->setCurrentTalker(talkerCode);
}

void JovieTrayIcon::aboutSelected()
{
    QPointer<KAboutApplicationDialog> aboutDlg = 
      new KAboutApplicationDialog(KGlobal::mainComponent().aboutData());
    aboutDlg->exec();
    delete aboutDlg;
}

void JovieTrayIcon::helpSelected()
{
    KToolInvocation::invokeHelp(QString(),QLatin1String( "jovie" ));
}

void JovieTrayIcon::stopSelected()
{
    Jovie::Instance()->stop();
}

void JovieTrayIcon::pauseSelected()
{
    Jovie::Instance()->pause();
}

void JovieTrayIcon::resumeSelected()
{
    Jovie::Instance()->resume();
}

void JovieTrayIcon::repeatSelected()
{
    //int jobNum = m_kspeech->getCurrentJob();
    //int seq = m_kspeech->moveRelSentence(jobNum, 0);
    //m_kspeech->moveRelSentence(jobNum, -seq);
}

void JovieTrayIcon::configureSelected()
{
    QStringList lst;
    lst << QLatin1String( "kcmkttsd" ) << QLatin1String( "--caption" ) << i18n("KDE Text-to-Speech");
    QProcess::startDetached(QLatin1String( "kcmshell4" ),lst);
}

void JovieTrayIcon::configureKeysSelected()
{
    KShortcutsDialog::configure( actionCollection() );
}

#include "jovietrayicon.moc"
