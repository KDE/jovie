/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD
  
  The KDE Text-to-Speech Daemon.
  -----------------------------
  Copyright:
  (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
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

// KDE Includes.
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

// KTTSD includes.
#include "kspeech.h"

int main (int argc, char *argv[]){
    KLocale::setMainCatalog("kttsd");
    KAboutData aboutdata("kttsd", 0, ki18n("kttsd"),
         "0.4.0", ki18n("Text-to-speech synthesis deamon"),
         KAboutData::License_GPL, ki18n("(C) 2002, José Pablo Ezequiel Fernández"));
    aboutdata.addAuthor(ki18n("José Pablo Ezequiel Fernández"),ki18n("Original Author"),"pupeno@pupeno.com");
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Maintainer"),"garycramblitt@comcast.net");
    aboutdata.addAuthor(ki18n("Gunnar Schmi Dt"), ki18n("Contributor"),"gunnar@schmi-dt.de");
    aboutdata.addAuthor(ki18n("Olaf Schmidt"), ki18n("Contributor"),"ojschmidt@kde.org");
    aboutdata.addAuthor(ki18n("Paul Giannaros"), ki18n("Contributor"), "ceruleanblaze@gmail.com");
    aboutdata.addCredit(ki18n("Jorge Luis Arzola"), ki18n("Testing"), "arzolacub@hotmail.com");
    aboutdata.addCredit(ki18n("David Powell"), ki18n("Testing"), "achiestdragon@gmail.com");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
    
    KUniqueApplication::setOrganizationDomain("kde.org");
    KUniqueApplication::setApplicationName("kttsd");
    KUniqueApplication app;

    if(!KUniqueApplication::start()){
        kDebug() << "KTTSD is already running";
        return (0);
    }

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    kDebug() << "main: Creating KTTSD Service";
    KSpeech* service = new KSpeech();

    // kDebug() << "Entering event loop.";
    return app.exec();
    delete service;
}
