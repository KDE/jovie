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

// kspeak includes.
#include "kspeak.h"

static const KCmdLineOptions options[] =
{
    { "e", 0, 0 },
    { "echo", I18N_NOOP("Echo commands. [off]"), 0},
    { "r", 0, 0 },
    { "replies", I18N_NOOP("Show KTTSD D-Bus replies. [off]"), 0},
    { "s", 0, 0 },
    { "signals", I18N_NOOP("Show KTTSD D-Bus signals. [off]"), 0},
    { "k", 0, 0 },
    { "startkttsd", I18N_NOOP("Start KTTSD if not already running. [off]"), 0},
    { "x", 0, 0 },
    { "nostoponerror", I18N_NOOP("Continue on error."), 0},
    { "+scriptfile", I18N_NOOP("Name of script to run.  Use '-' for stdin."), 0},
    { "!+[args...]", I18N_NOOP("Optional arguments passed to script."), 0},
    { "", I18N_NOOP("Type 'help' for kspeak commands."), 0},
    KCmdLineLastOption // End of options.
};

/**
 * Main routine.
 */
int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "kspeak", I18N_NOOP("kspeak"),
        "0.1.0", I18N_NOOP("A utility for sending speech commands to KTTSD service via D-Bus."),
         KAboutData::License_GPL, "(C) 2006, Gary Cramblitt <garycramblitt@comcast.net>");
    aboutdata.addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer"),"garycramblitt@comcast.net");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app(false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() == 0) KCmdLineArgs::usage(i18n("No script file specified"));

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
