/***************************************************** vim:set ts=4 sw=4 sts=4:
  Manages all the Talker (synth) plugins.
  -------------------
  Copyright:
  (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <ktrader.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "pluginconf.h"
#include "talkermgr.h"
#include "threadedplugin.h"

/**
 * Constructor.
 */
TalkerMgr::TalkerMgr(QObject *parent, const char *name) :
    QObject( parent, name )
{
}

/**
 * Destructor.
 */
TalkerMgr::~TalkerMgr()
{
    while (!m_loadedPlugIns.isEmpty()) delete m_loadedPlugIns.takeFirst();
}

/**
 * Load all the configured synth plugins,  populating loadedPlugIns structure.
 */
int TalkerMgr::loadPlugIns(KConfig* config)
{
    // kDebug() << "Running: TalkerMgr::loadPlugIns()" << endl;
    int good = 0;
    int bad = 0;

    m_talkerToPlugInCache.clear();
    while (!m_loadedPlugIns.isEmpty()) delete m_loadedPlugIns.takeFirst();
    m_loadedTalkerCodes.clear();
    m_loadedTalkerIds.clear();

    config->setGroup("General");
    QStringList talkerIDsList = config->readEntry("TalkerIDs", QStringList(), ',');
    if (!talkerIDsList.isEmpty())
    {
        KLibFactory *factory;
        QStringList::ConstIterator itEnd(talkerIDsList.constEnd());
        for( QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it )
        {
            // kDebug() << "Loading plugInProc for Talker ID " << *it << endl;

            // Talker ID.
            QString talkerID = *it;

            // Set the group for the language we're loading
            config->setGroup("Talker_" + talkerID);

            // Get the DesktopEntryName of the plugin we will try to load.
            QString desktopEntryName = config->readEntry("DesktopEntryName", QString());

            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            if (desktopEntryName.isEmpty())
            {
                QString synthName = config->readEntry("PlugIn", QString());
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = TalkerCode::TalkerNameToDesktopEntryName(synthName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) config->writeEntry("DesktopEntryName", desktopEntryName);
            }

            // Get the talker code.
            QString talkerCode = config->readEntry("TalkerCode", QString());

            // Normalize the talker code.
            QString fullLanguageCode;
            talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, fullLanguageCode);

            // Find the KTTSD SynthPlugin.
            KTrader::OfferList offers = KTrader::self()->query(
                "KTTSD/SynthPlugin", QString("DesktopEntryName == '%1'").arg(desktopEntryName));

            if(offers.count() > 1){
                ++bad;
                kDebug() << "More than 1 plug in doesn't make any sense, well, let's use any" << endl;
            } else if(offers.count() < 1){
                ++bad;
                kDebug() << "Less than 1 plug in, nothing can be done" << endl;
            } else {
                kDebug() << "Loading " << offers[0]->library() << endl;
                factory = KLibLoader::self()->factory(offers[0]->library().latin1());
                if(factory){
                    PlugInProc *speech = 
                            KLibLoader::createInstance<PlugInProc>(
                            offers[0]->library().latin1(), this, QStringList(offers[0]->library().latin1()));
                    if(!speech){
                        kDebug() << "Couldn't create the speech object from " << offers[0]->library() << endl;
                        ++bad;
                    } else {
                        if (speech->supportsAsync())
                        {
                            speech->init(config, "Talker_" + talkerID);
                            // kDebug() << "Plug in " << desktopEntryName << " created successfully." << endl;
                            m_loadedPlugIns.append(speech);
                        } else {
                            // Synchronous plugins are run in a separate thread.
                            // Init will start the thread and it will immediately go to sleep.
                            QString threadedPlugInName = QString::fromLatin1("threaded") + desktopEntryName;
                            ThreadedPlugIn* speechThread = new ThreadedPlugIn(speech,
                                    this, threadedPlugInName.latin1());
                            speechThread->init(config, "Talker_" + talkerCode);
                            // kDebug() << "Threaded Plug in " << desktopEntryName << " for language " <<  (*it).right((*it).length()-5) << " created succesfully." << endl;
                            m_loadedPlugIns.append(speechThread);
                        }
                        ++good;
                        m_loadedTalkerCodes.append(TalkerCode(talkerCode));
                        m_loadedTalkerIds.append(talkerID);
                    }
                } else {
                    kDebug() << "Couldn't create the factory object from " << offers[0]->library() << endl;
                    ++bad;
                }
            }
        }
    }
    if(bad > 0){
        if(good == 0){
            // No plugin could be loaded.
            return -1;
        } else {
            // At least one plugin was loaded and one failed.
            return 0;
        }
    } else {
        if (good == 0)
            // No plugin could be loaded.
            return -1;
        else
            // All the plug in were loaded perfectly
            return 1;
    }
}

/**
 * Get a list of the talkers configured in KTTS.
 * @return               A QStringList of fully-specified talker codes, one
 *                       for each talker user has configured.
 */
QStringList TalkerMgr::getTalkers()
{
    QStringList talkerList;
    for (int ndx = 0; ndx < int(m_loadedPlugIns.count()); ++ndx)
    {
        talkerList.append(m_loadedTalkerCodes[ndx].getTalkerCode());
    }
    return talkerList;
}

/**
 * Returns a list of all the loaded plugins.
 */
PlugInList TalkerMgr::getLoadedPlugIns()
{
    return m_loadedPlugIns;
}

/**
 * Given a talker code, returns pointer to the closest matching plugin.
 * @param talker          The talker (language) code.
 * @return                Index to m_loadedPlugins array of Talkers.
 *
 * If a plugin has not been loaded to match the talker, returns the default
 * plugin.
 */
int TalkerMgr::talkerToPluginIndex(const QString& talker) const
{
    // kDebug() << "TalkerMgr::talkerToPluginIndex: matching talker " << talker << " to closest matching plugin." << endl;
    // If we have a cached match, return that.
    if (m_talkerToPlugInCache.contains(talker))
        return m_talkerToPlugInCache[talker];
    else
    {
        int winner = TalkerCode::findClosestMatchingTalker(m_loadedTalkerCodes, talker, true);
        m_talkerToPlugInCache[talker] = winner;
        return winner;
    }
}

/**
 * Given a talker code, returns pointer to the closest matching plugin.
 * @param talker          The talker (language) code.
 * @return                Pointer to closest matching plugin.
 *
 * If a plugin has not been loaded to match the talker, returns the default
 * plugin.
 *
 * TODO: When picking a talker, %KTTSD will automatically determine if text contains
 * markup and pick a talker that supports that markup, if available.  This
 * overrides all other attributes, i.e, it is treated as an automatic "top priority"
 * attribute.
 */
PlugInProc* TalkerMgr::talkerToPlugin(const QString& talker) const
{
    int talkerNdx = talkerToPluginIndex(talker);
    return const_cast< PlugInList* >(&m_loadedPlugIns)->at(talkerNdx);
}

/**
 * Given a talker code, returns the parsed TalkerCode of the closest matching Talker.
 * @param talker          The talker (language) code.
 * @return                Parsed TalkerCode structure.
 *
 * If a plugin has not been loaded to match the talker, returns the default
 * plugin.
 *
 * The returned TalkerCode is a copy and should be destroyed by caller.
 *
 * TODO: When picking a talker, %KTTSD will automatically determine if text contains
 * markup and pick a talker that supports that markup, if available.  This
 * overrides all other attributes, i.e, it is treated as an automatic "top priority"
 * attribute.
 */
TalkerCode* TalkerMgr::talkerToTalkerCode(const QString& talker)
{
    int talkerNdx = talkerToPluginIndex(talker);
    return new TalkerCode(&m_loadedTalkerCodes[talkerNdx]);
}

/**
 * Given a Talker Code, returns the Talker ID of the talker that would speak
 * a text job with that Talker Code.
 * @param talkerCode     Talker Code.
 * @return               Talker ID of the talker that would speak the text job.
 */
QString TalkerMgr::talkerCodeToTalkerId(const QString& talkerCode)
{
    int talkerNdx = talkerToPluginIndex(talkerCode);
    return m_loadedTalkerIds[talkerNdx];
}

/**
 * Get the user's default talker.
 * @return               A fully-specified talker code.
 *
 * @see talkers
 * @see getTalkers
 */
QString TalkerMgr::userDefaultTalker() const
{
    return m_loadedTalkerCodes[0].getTalkerCode();
}

/**
 * Determine whether the currently-configured speech plugin supports a speech markup language.
 * @param talker         Code for the talker to do the speaking.  Example "en".
 *                       If NULL, defaults to the user's default talker.
 * @param markupType     The kttsd code for the desired speech markup language.
 * @return               True if the plugin currently configured for the indicated
 *                       talker supports the indicated speech markup language.
 * @see kttsdMarkupType
 */
bool TalkerMgr::supportsMarkup(const QString& talker, const uint /*markupType*/) const
{
    kDebug() << "TalkerMgr::supportsMarkup: Testing talker " << talker << endl;
    QString matchingTalker = talker;
    if (matchingTalker.isEmpty()) matchingTalker = userDefaultTalker();
    PlugInProc* plugin = talkerToPlugin(matchingTalker);
    return ( plugin->getSsmlXsltFilename() !=
            KGlobal::dirs()->resourceDirs("data").last() + "kttsd/xslt/SSMLtoPlainText.xsl");
}

bool TalkerMgr::autoconfigureTalker(const QString& langCode, KConfig* config)
{
    // Not yet implemented.
    // return false;

    QString languageCode = langCode;

    // Get last TalkerID from config.
    QStringList talkerIDsList = config->readEntry("TalkerIDs", QStringList(), ',');
    int lastTalkerID = 0;
    for (int talkerIdNdx = 0; talkerIdNdx < talkerIDsList.count(); ++talkerIdNdx)
    {
        int id = talkerIDsList[talkerIdNdx].toInt();
        if (id > lastTalkerID) lastTalkerID = id;
    }

    // Assign a new Talker ID for the talker.  Wraps around to 1.
    QString talkerID = QString::number(lastTalkerID + 1);

    // Query for all the KTTSD SynthPlugins.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");

    // Iterate thru the possible plug ins.
    for(int i=0; i < offers.count() ; ++i)
    {
        // See if this plugin supports the desired language.
        QStringList languageCodes = offers[i]->property("X-KDE-Languages").toStringList();
        if (languageCodes.contains(languageCode))
        {
            QString desktopEntryName = offers[i]->desktopEntryName();

            // Load the plugin.
            KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
            if (factory)
            {
                // If the factory is created successfully, instantiate the PlugInConf class for the
                // specific plug in to get the plug in configuration object.
                PlugInConf* loadedTalkerPlugIn =
                    KLibLoader::createInstance<PlugInConf>(
                        offers[0]->library().latin1(), NULL, QStringList(offers[0]->library().latin1()));
                if (loadedTalkerPlugIn)
                {
                    // Give plugin the language code and permit plugin to autoconfigure itself.
                    loadedTalkerPlugIn->setDesiredLanguage(languageCode);
                    loadedTalkerPlugIn->load(config, QString("Talker_")+talkerID);

                    // If plugin was able to configure itself, it returns a full talker code.
                    QString talkerCode = loadedTalkerPlugIn->getTalkerCode();

                    if (!talkerCode.isEmpty())
                    {
                        // Erase extraneous Talker configuration entries that might be there.
                        config->deleteGroup(QString("Talker_")+talkerID);

                        // Let plugin save its configuration.
                        config->setGroup(QString("Talker_")+talkerID);
                        loadedTalkerPlugIn->save(config, QString("Talker_"+talkerID));

                        // Record configuration data.
                        config->setGroup(QString("Talker_")+talkerID);
                        config->writeEntry("DesktopEntryName", desktopEntryName);
                        talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, languageCode);
                        config->writeEntry("TalkerCode", talkerCode);

                        // Add TalkerID to configured list.
                        talkerIDsList.append(talkerID);
                        config->setGroup("General");
                        config->writeEntry("TalkerIDs", talkerIDsList.join(","));
                        config->sync();

                        // TODO: Now that we have modified the config, need a way to inform
                        // other apps, including KTTSMgr.  As this routine is likely called
                        // when KTTSMgr is not running, is not a serious problem.

                        // Success!
                        delete loadedTalkerPlugIn;
                        return true;
                    }

                    // Plugin no longer needed.
                    delete loadedTalkerPlugIn;
                }
            }
        }
    }

    return false;
}
