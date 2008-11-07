/**********************
  #FILENAME: #FILEDESC
  #COPYRIGHTNOTICE
  #LICENSE
***********************/

#include <iostream>
using namespace std;

#include <QtCore/QTextStream>

#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

#include "filterproc.h"
#include "talkercode.h"

int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "testfilter", 0, ki18n("testfilter"),
        "0.1.0", ki18n("A utility for testing KTTSD filter plugins."),
         KAboutData::License_GPL, ki18n("Copyright 2005, Gary Cramblitt <garycramblitt@comcast.net>"));
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Maintainer"),"garycramblitt@comcast.net");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    // Tell which options are supported

    KCmdLineOptions options;
    options.add("+pluginName", ki18n("Name of a KTTSD filter plugin (required)"));
    options.add("t");
    options.add("talker <talker>", ki18n("Talker code passed to filter"), "en");
    options.add("a");
    options.add("appid <appID>", ki18n("DCOP application ID passed to filter"), "testfilter");
    options.add("g");
    options.add("group <filterID>", ki18n("Config file group name passed to filter"), "testfilter");
    options.add("list", ki18n("Display list of available Filter PlugIns and exit"));
    options.add("b");
    options.add("break", ki18n("Display tabs as \\t, otherwise they are removed"));
    options.add("list", ki18n("Display list of available filter plugins and exit"));
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
            QString name = offers[ndx]->name();
            cout << name.latin1() << endl;
        }
        return 0;
    }

    QString filterName;
    if (args->count() > 0) filterName = args->arg(0);
    QString talker = args->getOption("talker");
    QCString appId = args->getOption("appid");
    QString groupName = args->getOption("group");

    if (filterName.isEmpty()) kError(1) << "No filter name given." << endl;

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
                    KLibLoader::createInstance<KttsFilterProc>(
                    offers[ndx]->library().latin1(), NULL, offers[ndx]->library().latin1(),
                        QStringList(), &errorNo);
                    if(plugIn)
                    {
                        KConfig* config = new KConfig("kttsdrc");
                        config->setGroup( "General" );
                        plugIn->init( config, groupName );
                        QTextStream inp ( stdin,  QIODevice::ReadOnly );
                        QString text;
                        text = inp.read();
                        TalkerCode* talkerCode = new TalkerCode( talker );
                        text = plugIn->convert( text, talkerCode, appId );
                        if ( args->isSet("break") ) 
                            text.replace( '\t', "\\t" );
                        else
                            text.remove( '\t');
                        cout << text.latin1() << endl;
                        delete config;
                        delete plugIn;
                        return 0;
                    } else
                        kError(2) << "Unable to create instance from library." << endl;
            } else
                kError(3) << "Unable to create factory." << endl;
        }
    }
    kError(4) << "Unable to find a plugin named " << filterName << endl;
}
