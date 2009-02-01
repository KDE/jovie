#include <QtCore/QTextStream>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kparts/componentfactory.h>

#include "filterproc.h"
#include "talkercode.h"

int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "testfilter", 0, ki18n("testfilter"),
        "0.1.0", ki18n("A utility for testing KTTSD filter plugins."),
         KAboutData::License_GPL, ki18n("Copyright 2005, Gary Cramblitt &lt;garycramblitt@comcast.net&gt;"));
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

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KService::List offers = KServiceTypeTrader::self()->query("KTTSD/FilterPlugin");

    if (args->isSet("list"))
    {
        // Iterate thru the offers to list the plugins.
        foreach (const KService::Ptr service, offers)
        {
            kDebug() << service->name();
        }
        return 0;
    }

    QString filterName;
    if (args->count() > 0) filterName = args->arg(0);
    QString talker = args->getOption("talker");
    QString appId = args->getOption("appid");
    QString groupName = args->getOption("group");

    if (filterName.isEmpty()) kError(1) << "No filter name given." << endl;

    QVariantList list;
    foreach (const KService::Ptr service, offers)
    {
        if (service->name() == filterName)
        {
            // When the entry is found, load the plug in
            // First create a factory for the library
            KPluginLoader loader(service->library());
            KPluginFactory *factory = loader.factory();
            if (factory)
            {
                // If the factory is created successfully, instantiate the KttsFilterConf class for the
                // specific plug in to get the plug in configuration object.
                KttsFilterProc *plugIn = factory->create<KttsFilterProc>(NULL, list);
                    //KLibLoader::createInstance<KttsFilterProc>(
                    //service->library(), NULL, QStringList(service->library()),
                    //    &errorNo);
                    if (plugIn)
                    {
                        KConfig* config = new KConfig("kttsdrc");
                        plugIn->init( config, groupName);
                        QTextStream inp ( stdin,  QIODevice::ReadOnly );
                        QString text;
                        text = inp.readLine();
                        TalkerCode* talkerCode = new TalkerCode( talker );
                        text = plugIn->convert( text, talkerCode, appId );
                        if ( args->isSet("break") ) 
                            text.replace( '\t', "\\t" );
                        else
                            text.remove( '\t');
                        kDebug() << text;
                        delete config;
                        delete plugIn;
                        return 0;
                    }
                    else
                    {
                        kError(2) << "Unable to create instance from library." << endl;
                    }
            }
            else
            {
                kError(3) << "Unable to create factory." << endl;
            }
        }
    }
    kError(4) << "Unable to find a plugin named " << filterName << endl;
}
