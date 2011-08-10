/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD Filter Test Program
  -------------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

#include <tqstring.h>
#include <iostream>
using namespace std;

#include <tqtextstream.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

#include "filterproc.h"
#include "talkercode.h"

static const KCmdLineOptions options[] =
{
    { "+pluginName", I18N_NOOP("Name of a KTTSD filter plugin (required)"), 0 },
    { "t", 0, 0 },
    { "talker <talker>", I18N_NOOP("Talker code passed to filter"), "en" }, 
    { "a", 0, 0 },
    { "appid <appID>", I18N_NOOP("DCOP application ID passed to filter"), "testfilter" },
    { "g", 0, 0 },
    { "group <filterID>",
        I18N_NOOP2("A string that appears in a single config file, not a group of config files",
            "Config file group name passed to filter"), "testfilter" },
    { "list", I18N_NOOP("Display list of available Filter PlugIns and exit"), 0 },
    { "b", 0, 0 },
    { "break", I18N_NOOP("Display tabs as \\t, otherwise they are removed"), 0 },
    { "list", I18N_NOOP("Display list of available filter plugins and exit"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "testfilter", I18N_NOOP("testfilter"),
        "0.1.0", I18N_NOOP("A utility for testing KTTSD filter plugins."),
         KAboutData::License_GPL, "(C) 2005, Gary Cramblitt <garycramblitt@comcast.net>");
    aboutdata.addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer"),"garycramblitt@comcast.net");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app( false, false );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin");

    if (args->isSet("list"))
    {
        // Iterate thru the offers to list the plugins.
        const int offersCount = offers.count();
        for(int ndx=0; ndx < offersCount ; ++ndx)
        {
            TQString name = offers[ndx]->name();
            cout << name.latin1() << endl;
        }
        return 0;
    }

    TQString filterName;
    if (args->count() > 0) filterName = args->arg(0);
    TQString talker = args->getOption("talker");
    TQCString appId = args->getOption("appid");
    TQString groupName = args->getOption("group");

    if (filterName.isEmpty()) kdError(1) << "No filter name given." << endl;

    const int offersCount = offers.count();
    for(int ndx=0; ndx < offersCount ; ++ndx)
    {
        if(offers[ndx]->name() == filterName)
        {
            // When the entry is found, load the plug in
            // First create a factory for the library
            KLibFactory *factory = KLibLoader::self()->factory(offers[ndx]->library().latin1());
            if(factory)
            {
                // If the factory is created successfully, instantiate the KttsFilterConf class for the
                // specific plug in to get the plug in configuration object.
                int errorNo;
                KttsFilterProc *plugIn =
                    KParts::ComponentFactory::createInstanceFromLibrary<KttsFilterProc>(
                    offers[ndx]->library().latin1(), NULL, offers[ndx]->library().latin1(),
                        TQStringList(), &errorNo);
                    if(plugIn)
                    {
                        KConfig* config = new KConfig("kttsdrc");
                        config->setGroup( "General" );
                        plugIn->init( config, groupName );
                        TQTextStream inp ( stdin,  IO_ReadOnly );
                        TQString text;
                        text = inp.read();
                        TalkerCode* talkerCode = new TalkerCode( talker );
                        text = plugIn->convert( text, talkerCode, appId );
                        if ( args->isSet("break") ) 
                            text.replace( "\t", "\\t" );
                        else
                            text.replace( "\t", "" );
                        cout << text.latin1() << endl;
                        delete config;
                        delete plugIn;
                        return 0;
                    } else
                        kdError(2) << "Unable to create instance from library." << endl;
            } else
                kdError(3) << "Unable to create factory." << endl;
        }
    }
    kdError(4) << "Unable to find a plugin named " << filterName << endl;
}
