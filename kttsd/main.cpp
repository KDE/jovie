/***************************************************** vim:set ts=4 sw=4 sts=4:
  main.cpp
  Where the main function for KTTSD resides.
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
 
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>

#include "kttsd.h"

int main (int argc, char *argv[]){
   KLocale::setMainCatalogue("kdelibs");
   KAboutData aboutdata("kttsd", I18N_NOOP("KDE"),
         "0.1.0", I18N_NOOP("Speech synthesis"),
         KAboutData::License_GPL, "(C) 2002, José Pablo Ezequiel Fernández");
   aboutdata.addAuthor("José Pablo Ezequiel Fernández",I18N_NOOP("Developer"),"pupeno@pupeno.com");

   KCmdLineArgs::init( argc, argv, &aboutdata );
   // KCmdLineArgs::addCmdLineOptions( options );
   KUniqueApplication::addCmdLineOptions();

   if(!KUniqueApplication::start()){
      kdDebug() << "KTTSD is already running" << endl;
      return (0);
   }

   KUniqueApplication app;
   // This app is started automatically, no need for session management
   app.disableSessionManagement();
   KTTSD *service = new KTTSD;
   
   // Are we ok to go ?
   if(!service->ok){
      return(1);
   }
   
   kdDebug() << "Starting KTTSD." << endl;
   return app.exec();
}
