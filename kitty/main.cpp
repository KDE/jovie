/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD

  The KDE Text-to-Speech Daemon.
  -----------------------------
  Copyright: (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Copyright: (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright: (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

// KDE Includes.
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kdebug.h>
#include <klocale.h>

// KTTSD includes.
#include "kitty.h"
#include "kittytrayicon.h"

int main (int argc, char *argv[]){
    KLocale::setMainCatalog("kttsd");
    KAboutData aboutdata("kitty", 0, ki18n("kitty"),
         "0.6.0", ki18n("Text-to-speech synthesis daemon"),
         KAboutData::License_GPL, ki18n("(C) 2002, José Pablo Ezequiel Fernández"));
    aboutdata.addAuthor(ki18n("José Pablo Ezequiel Fernández"),ki18n("Original Author"),"pupeno@pupeno.com");
    aboutdata.addAuthor(ki18n("Jeremy Whiting"), ki18n("Current Maintainer"), "jpwhiting@kde.org");
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Previous Maintainer"),"garycramblitt@comcast.net");
    aboutdata.addAuthor(ki18n("Gunnar Schmi Dt"), ki18n("Contributor"),"gunnar@schmi-dt.de");
    aboutdata.addAuthor(ki18n("Olaf Schmidt"), ki18n("Contributor"),"ojschmidt@kde.org");
    aboutdata.addAuthor(ki18n("Paul Giannaros"), ki18n("Contributor"), "ceruleanblaze@gmail.com");
    aboutdata.addCredit(ki18n("Jorge Luis Arzola"), ki18n("Testing"), "arzolacub@hotmail.com");
    aboutdata.addCredit(ki18n("David Powell"), ki18n("Testing"), "achiestdragon@gmail.com");
    aboutdata.setProgramIconName("preferences-desktop-text-to-speech");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    KUniqueApplication::setOrganizationDomain("kde.org");
    KUniqueApplication::setApplicationName("kitty");
    KUniqueApplication app;

    if (!KUniqueApplication::start()) {
        kDebug() << "KITTY is already running";
        return (0);
    }

    KCrash::setFlags(KCrash::AutoRestart);

    // This app is started automatically, no need for session management
    app.disableSessionManagement();

    kDebug() << "main: Creating Kitty Service";
    Kitty* service = Kitty::Instance();
    service->init();

    KittyTrayIcon* tray = new KittyTrayIcon();

    // kDebug() << "Entering event loop.";
    return app.exec();
    delete service;
}
