/***************************************************** vim:set ts=4 sw=4 sts=4:
  kspeak
  
  Command-line utility for sending commands to KTTSD service via D-Bus.
  --------------------------------------------------------------------
  Copyright:
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

// Qt includes.

// KDE includes.
#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <stdlib.h>

// kspeak includes.
#include "kspeak.h"

/**
 * Main routine.
 */
int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "kspeak", 0, ki18n("kspeak"),
        "0.1.0", ki18n("A utility for sending speech commands to KTTSD service via D-Bus."),
         KAboutData::License_GPL, ki18n("Copyright 2006, Gary Cramblitt <email>garycramblitt@comcast.net</email>"));
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Maintainer"),"garycramblitt@comcast.net");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    // Tell which options are supported

    KCmdLineOptions options;
    options.add("e");
    options.add("echo", ki18n("Echo commands. [off]"));
    options.add("r");
    options.add("replies", ki18n("Show KTTSD D-Bus replies. [off]"));
    options.add("s");
    options.add("signals", ki18n("Show KTTSD D-Bus signals. [off]"));
    options.add("k");
    options.add("startkttsd", ki18n("Start KTTSD if not already running. [off]"));
    options.add("x");
    options.add("nostoponerror", ki18n("Continue on error."));
    options.add("+scriptfile", ki18n("Name of script to run.  Use '-' for stdin."));
    options.add("!+[args...]", ki18n("Optional arguments passed to script."));
    options.add("", ki18n("Type 'help' for kspeak commands."));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app(false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0) KCmdLineArgs::usageError(i18n("No script file specified"));

    // Create KSpeak object.
    KSpeak kspeak(args, 0);
    kspeak.setEcho(args->isSet("echo"));
    kspeak.setStopOnError(args->isSet("stoponerror"));
    kspeak.setShowReply(args->isSet("replies"));
    kspeak.setShowSignals(args->isSet("signals"));
    if (!kspeak.isKttsdRunning(args->isSet("startkttsd"))) exit (1);

    // Main event loop.
    int result = app.exec();
    return result;
}
