/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSMgr System Tray Application
  -------------------------------
  Copyright: (C) 2004-2006 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright: (C) 2010 by Jeremy Whiting <jpwhiting@kde.org>
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

// Kitty includes.
#include "kittytrayicon.h"
#include "kitty.h"

// Qt includes.
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

// KDE includes.
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <ksystemtrayicon.h>
#include <kmenu.h>
#include <kaboutapplicationdialog.h>
#include <ktoolinvocation.h>
#include <klocale.h>
#include <kicon.h>
#include <kconfig.h>

/* ------------------  KittyTrayIcon class ----------------------- */

KittyTrayIcon::KittyTrayIcon(QWidget *parent)
    :KStatusNotifierItem("kitty", parent)
{
    setObjectName("kittytrayicon");
    setIconByName("preferences-desktop-text-to-speech");
    setStatus(KStatusNotifierItem::Active);
    setCategory(KStatusNotifierItem::SystemServices);

    QString status = "Kitty - ";
    status += i18n("KDE Text-to-Speech Manager");
    setToolTipTitle(status);
    setToolTipIconByName("preferences-desktop-text-to-speech");

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
    connect(this, SIGNAL(activateRequested(bool, const QPoint &)),
                  SLOT(slotActivateRequested(bool, const QPoint &)));
    connect(contextMenu(), SIGNAL(aboutToShow()),
                  SLOT(contextMenuAboutToShow()));
}

KittyTrayIcon::~KittyTrayIcon()
{
}

void KittyTrayIcon::slotActivateRequested(bool active, const QPoint &pos)
{
    Q_UNUSED(active)
    Q_UNUSED(pos)
    // pause/resume icon/app
    if (overlayIconName() == QString("media-playback-pause")) {
        Kitty::Instance()->resume();
        setOverlayIconByName("");
    }
    else {
        Kitty::Instance()->pause();
        setOverlayIconByName("media-playback-pause");
    }
}

void KittyTrayIcon::contextMenuAboutToShow()
{
    const bool configActive = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kcmshell_kcmkttsd"));
    actConfigure->setEnabled(!configActive);
}

void KittyTrayIcon::speakClipboardSelected()
{
    Kitty::Instance()->sayClipboard();
}

void KittyTrayIcon::aboutSelected()
{
    KAboutApplicationDialog aboutDlg(KGlobal::mainComponent().aboutData());
    aboutDlg.exec();
}

void KittyTrayIcon::helpSelected()
{
    KToolInvocation::invokeHelp(QString(),"kttsd");
}

void KittyTrayIcon::quitSelected()
{
    // kDebug() << "Running KittyTrayIcon::quitSelected";
    kapp->quit();
}

void KittyTrayIcon::stopSelected()
{
    Kitty::Instance()->removeAllJobs();
}

void KittyTrayIcon::pauseSelected()
{
    Kitty::Instance()->pause();
}

void KittyTrayIcon::resumeSelected()
{
    Kitty::Instance()->resume();
}

void KittyTrayIcon::repeatSelected()
{
    //int jobNum = m_kspeech->getCurrentJob();
    //int seq = m_kspeech->moveRelSentence(jobNum, 0);
    //m_kspeech->moveRelSentence(jobNum, -seq);
}

void KittyTrayIcon::configureSelected()
{
    QStringList lst;
	lst << "kcmkttsd" << "--caption" << i18n("KDE Text-to-Speech");
	QProcess::startDetached("kcmshell4",lst);
}

#include "kittytrayicon.moc"
