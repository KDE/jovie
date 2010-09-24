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

// Jovie includes.
#include "jovietrayicon.h"
#include "jovie.h"

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

/* ------------------  JovieTrayIcon class ----------------------- */

JovieTrayIcon::JovieTrayIcon(QWidget *parent)
    :KStatusNotifierItem("jovie", parent)
{
    setObjectName( QLatin1String("jovietrayicon" ));
    setStatus(KStatusNotifierItem::Active);
    setCategory(ApplicationStatus);

    QString status = "Jovie - ";
    status += i18n("KDE Text-to-Speech Manager");
    setToolTipTitle(status);
    setToolTipIconByName("preferences-desktop-text-to-speech");
    setIconByName("preferences-desktop-text-to-speech");

    // Set up menu.
    QAction *act;

    actStop = contextMenu()->addAction (
        i18n("&Stop/Delete"), this, SLOT(stopSelected()));
    actStop->setIcon(KIcon( QLatin1String( "media-playback-stop" )));
    actPause = contextMenu()->addAction (
        i18n("&Pause"), this, SLOT(pauseSelected()));
    actPause->setIcon(KIcon( QLatin1String( "media-playback-pause" )));
    actResume = contextMenu()->addAction (
        i18n("&Resume"), this, SLOT(resumeSelected()));
    actResume->setIcon(KIcon( QLatin1String( "media-playback-start" )));
    actRepeat = contextMenu()->addAction (
        i18n("R&epeat"), this, SLOT(repeatSelected()));
    actRepeat->setIcon(KIcon( QLatin1String( "view-refresh" )));
    act = contextMenu()->addSeparator();
    actSpeakClipboard = contextMenu()->addAction (
        i18n("Spea&k Clipboard Contents"), this, SLOT(speakClipboardSelected()));
    act->setIcon(KIcon( QLatin1String( "klipper" )));
    actConfigure = contextMenu()->addAction (
        i18n("&Configure"), this, SLOT(configureSelected()));
    actConfigure->setIcon(KIcon( QLatin1String( "configure" )));
    act = contextMenu()->addSeparator();
    act = contextMenu()->addAction (
        i18n("Jovie &Handbook"), this, SLOT(helpSelected()));
    act->setIcon(KIcon( QLatin1String( "help-contents" )));
    act = contextMenu()->addAction (
        i18n("&About Jovie"), this, SLOT(aboutSelected()));
    act->setIcon(KIcon( QLatin1String( "preferences-desktop-text-to-speech" )));

    connect(this, SIGNAL(activateRequested(bool, const QPoint &)),
                  SLOT(slotActivateRequested(bool, const QPoint &)));
    connect(contextMenu(), SIGNAL(aboutToShow()),
                  SLOT(contextMenuAboutToShow()));
}

JovieTrayIcon::~JovieTrayIcon()
{
}

void JovieTrayIcon::slotActivateRequested(bool active, const QPoint &pos)
{
    Q_UNUSED(active)
    Q_UNUSED(pos)
    // pause/resume icon/app
    if (overlayIconName() == QString("media-playback-pause")) {
        Jovie::Instance()->resume();
        setOverlayIconByName("");
    }
    else {
        Jovie::Instance()->pause();
        setOverlayIconByName("media-playback-pause");
    }
}

void JovieTrayIcon::contextMenuAboutToShow()
{
    const bool configActive = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kcmshell_kcmkttsd"));
    actConfigure->setEnabled(!configActive);
}

void JovieTrayIcon::speakClipboardSelected()
{
    Jovie::Instance()->sayClipboard();
}

void JovieTrayIcon::aboutSelected()
{
    KAboutApplicationDialog aboutDlg(KGlobal::mainComponent().aboutData());
    aboutDlg.exec();
}

void JovieTrayIcon::helpSelected()
{
    KToolInvocation::invokeHelp(QString(),"jovie");
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
    lst << "kcmkttsd" << "--caption" << i18n("KDE Text-to-Speech");
    QProcess::startDetached("kcmshell4",lst);
}

#include "jovietrayicon.moc"
