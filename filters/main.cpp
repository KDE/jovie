/**********************
  #FILENAME: #FILEDESC
  #COPYRIGHTNOTICE
  #LICENSE
***********************/

#include <QString>
#include <iostream>
using namespace std;

#include <QTextStream>

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
        I18N_NOOP("Config file group name passed to filter"), "testfilter" },
    { "list", I18N_NOOP("Display list of available Filter PlugIns and exit"), 0 },
    { "b", 0, 0 },
    { "break", I18N_NOOP("Display tabs as \\t, otherwise they are removed"), 0 },
    { "list", I18N_NOOP("Display list of available filter plugins and exit"), 0 }
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
                            text.replace( "\t", "\\t" );
                        else
                            text.replace( "\t", "" );
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
