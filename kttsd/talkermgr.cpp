/***************************************************** vim:set ts=4 sw=4 sts=4:
  Manages all the Talker (synth) plugins.
  -------------------
  Copyright:
  (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
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
    for (int ndx = 0; ndx < int(m_loadedPlugIns.count()); ++ndx)
        delete m_loadedPlugIns[ndx].plugIn;
    m_loadedPlugIns.clear();
}

/**
 * Load all the configured synth plugins,  populating loadedPlugIns structure.
 */
int TalkerMgr::loadPlugIns(KConfig* config)
{
    // kdDebug() << "Running: TalkerMgr::loadPlugIns()" << endl;
    int good = 0;
    int bad = 0;

    m_talkerToPlugInCache.clear();
    for (int ndx = 0; ndx < int(m_loadedPlugIns.count()); ++ndx)
        delete m_loadedPlugIns[ndx].plugIn;
    m_loadedPlugIns.clear();

    config->setGroup("General");
    QStringList talkerIDsList = config->readListEntry("TalkerIDs", ',');
    if (!talkerIDsList.isEmpty())
    {
        KLibFactory *factory;
        QStringList::ConstIterator itEnd(talkerIDsList.constEnd());
        for( QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it )
        {
            // kdDebug() << "Loading plugInProc for Talker ID " << *it << endl;

            // Talker ID.
            QString talkerID = *it;

            // Set the group for the language we're loading
            config->setGroup("Talker_" + talkerID);

            // Get the DesktopEntryName of the plugin we will try to load.
            QString desktopEntryName = config->readEntry("DesktopEntryName", QString::null);

            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            if (desktopEntryName.isEmpty())
            {
                QString synthName = config->readEntry("PlugIn", QString::null);
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = TalkerNameToDesktopEntryName(synthName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) config->writeEntry("DesktopEntryName", desktopEntryName);
            }

            // Get the talker code.
            QString talkerCode = config->readEntry("TalkerCode", QString::null);

            // Normalize the talker code.
            QString fullLanguageCode;
            talkerCode = TalkerCode::normalizeTalkerCode(talkerCode, fullLanguageCode);

            // Find the KTTSD SynthPlugin.
            KTrader::OfferList offers = KTrader::self()->query(
                "KTTSD/SynthPlugin", QString("DesktopEntryName == '%1'").arg(desktopEntryName));

            if(offers.count() > 1){
                bad++;
                kdDebug() << "More than 1 plug in doesn't make any sense, well, let's use any" << endl;
            } else if(offers.count() < 1){
                bad++;
                kdDebug() << "Less than 1 plug in, nothing can be done" << endl;
            } else {
                kdDebug() << "Loading " << offers[0]->library() << endl;
                factory = KLibLoader::self()->factory(offers[0]->library().latin1());
                if(factory){
                    PlugInProc *speech = 
                            KParts::ComponentFactory::createInstanceFromLibrary<PlugInProc>(
                            offers[0]->library().latin1(), this, offers[0]->library().latin1());
                    if(!speech){
                        kdDebug() << "Couldn't create the speech object from " << offers[0]->library() << endl;
                        bad++;
                    } else {
                        TalkerInfo talkerInfo;
                        talkerInfo.talkerID = talkerID;
                        talkerInfo.talkerCode = talkerCode;
                        talkerInfo.parsedTalkerCode = TalkerCode(talkerCode);
                        if (speech->supportsAsync())
                        {
                            speech->init(config, "Talker_" + talkerID);
                            // kdDebug() << "Plug in " << desktopEntryName << " created successfully." << endl;
                            talkerInfo.plugIn = speech;
                        } else {
                            // Synchronous plugins are run in a separate thread.
                            // Init will start the thread and it will immediately go to sleep.
                            QString threadedPlugInName = QString::fromLatin1("threaded") + desktopEntryName;
                            ThreadedPlugIn* speechThread = new ThreadedPlugIn(speech,
                                    this, threadedPlugInName.latin1());
                            speechThread->init(config, "Talker_" + talkerCode);
                            // kdDebug() << "Threaded Plug in " << desktopEntryName << " for language " <<  (*it).right((*it).length()-5) << " created succesfully." << endl;
                            talkerInfo.plugIn = speechThread;
                        }
                        good++;
                        m_loadedPlugIns.append(talkerInfo);
                    }
                } else {
                    kdDebug() << "Couldn't create the factory object from " << offers[0]->library() << endl;
                    bad++;
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
        talkerList.append(m_loadedPlugIns[ndx].talkerCode);
    }
    return talkerList;
}

/**
 * Returns a list of all the loaded plugins.
 */
QPtrList<PlugInProc> TalkerMgr::getLoadedPlugIns()
{
    QPtrList<PlugInProc> plugins;
    int loadedPlugInsCount = int(m_loadedPlugIns.count());
    for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
        plugins.append(m_loadedPlugIns[ndx].plugIn);
    return plugins;
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
    // kdDebug() << "TalkerMgr::talkerToPluginIndex: matching talker " << talker << " to closest matching plugin." << endl;
    // If we have a cached match, return that.
    if (m_talkerToPlugInCache.contains(talker))
        return m_talkerToPlugInCache[talker];
    else
    {
        // Parse the given talker.
        TalkerCode parsedTalkerCode(talker);
        // If no language code specified, use the language code of the default plugin.
        if (parsedTalkerCode.languageCode().isEmpty()) parsedTalkerCode.setLanguageCode(
                    m_loadedPlugIns[0].parsedTalkerCode.languageCode());
        // TODO: If there are no talkers configured in the language, %KTTSD will attempt
        //       to automatically configure one (see automatic configuraton discussion below)
        // The talker that matches on the most priority attributes wins.
        int loadedPlugInsCount = int(m_loadedPlugIns.count());
        QMemArray<int> priorityMatch(loadedPlugInsCount);
        for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
        {
            priorityMatch[ndx] = 0;
            // kdDebug() << "Comparing language code " << parsedTalkerCode.languageCode() << " to " << m_loadedPlugIns[ndx].parsedTalkerCode.languageCode() << endl;
            if (parsedTalkerCode.languageCode() == m_loadedPlugIns[ndx].parsedTalkerCode.languageCode())
            {
                priorityMatch[ndx]++;
                // kdDebug() << "TalkerMgr::talkerToPluginIndex: Match on language " << parsedTalkerCode.languageCode() << endl;
            }
            if (parsedTalkerCode.countryCode().left(1) == "*")
                if (parsedTalkerCode.countryCode().mid(1) ==
                    m_loadedPlugIns[ndx].parsedTalkerCode.countryCode())
                    priorityMatch[ndx]++;
            if (parsedTalkerCode.voice().left(1) == "*")
                if (parsedTalkerCode.voice().mid(1) == m_loadedPlugIns[ndx].parsedTalkerCode.voice())
                    priorityMatch[ndx]++;
            if (parsedTalkerCode.gender().left(1) == "*")
                if (parsedTalkerCode.gender().mid(1) == m_loadedPlugIns[ndx].parsedTalkerCode.gender())
                    priorityMatch[ndx]++;
            if (parsedTalkerCode.volume().left(1) == "*")
                if (parsedTalkerCode.volume().mid(1) == m_loadedPlugIns[ndx].parsedTalkerCode.volume())
                    priorityMatch[ndx]++;
            if (parsedTalkerCode.rate().left(1) == "*")
                if (parsedTalkerCode.rate().mid(1) == m_loadedPlugIns[ndx].parsedTalkerCode.rate())
                    priorityMatch[ndx]++;
            if (parsedTalkerCode.plugInName().left(1) == "*")
                if (parsedTalkerCode.plugInName().mid(1) ==
                    m_loadedPlugIns[ndx].parsedTalkerCode.plugInName())
                    priorityMatch[ndx]++;
        }
        int maxPriority = -1;
        for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
        {
            if (priorityMatch[ndx] > maxPriority) maxPriority = priorityMatch[ndx];
        }
        int winnerCount = 0;
        int winner = -1;
        for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
        {
            if (priorityMatch[ndx] == maxPriority)
            {
                winnerCount++;
                winner = ndx;
            }
        }
        // kdDebug() << "TalkerMgr::talkerToPluginIndex: winnerCount = " << winnerCount << " winner = " << winner << endl;
        // If a tie, the one that matches on the most preferred attributes wins.
        // If there is still a tie, the one nearest the top of the kttsmgr display
        // (first configured) will be chosen.
        if (winnerCount > 1)
        {
            QMemArray<int> preferredMatch(loadedPlugInsCount);
            for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
            {
                preferredMatch[ndx] = 0;
                if (priorityMatch[ndx] == maxPriority)
                {
                    if (parsedTalkerCode.countryCode().left(1) != "*")
                        if (parsedTalkerCode.countryCode() == m_loadedPlugIns[ndx].parsedTalkerCode.countryCode())
                            preferredMatch[ndx]++;
                    if (parsedTalkerCode.voice().left(1) != "*")
                        if (parsedTalkerCode.voice() == m_loadedPlugIns[ndx].parsedTalkerCode.voice())
                            preferredMatch[ndx]++;
                    if (parsedTalkerCode.gender().left(1) != "*")
                        if (parsedTalkerCode.gender() == m_loadedPlugIns[ndx].parsedTalkerCode.gender())
                            preferredMatch[ndx]++;
                    if (parsedTalkerCode.volume().left(1) != "*")
                        if (parsedTalkerCode.volume() == m_loadedPlugIns[ndx].parsedTalkerCode.volume())
                            preferredMatch[ndx]++;
                    if (parsedTalkerCode.rate().left(1) != "*")
                        if (parsedTalkerCode.rate() == m_loadedPlugIns[ndx].parsedTalkerCode.rate())
                            preferredMatch[ndx]++;
                    if (parsedTalkerCode.plugInName().left(1) != "*")
                        if (parsedTalkerCode.plugInName() ==
                            m_loadedPlugIns[ndx].parsedTalkerCode.plugInName())
                            preferredMatch[ndx]++;
                }
            }
            int maxPreferred = -1;
            for (int ndx = 0; ndx < loadedPlugInsCount; ++ndx)
            {
                if (preferredMatch[ndx] > maxPreferred) maxPreferred = preferredMatch[ndx];
            }
            winner = -1;
            for (int ndx = loadedPlugInsCount-1; ndx >= 0; ndx--)
            {
                if (priorityMatch[ndx] == maxPriority)
                {
                    if (preferredMatch[ndx] == maxPreferred)
                    {
                        winner = ndx;
                    }
                }
            }
        }
        // If no winner found, use the first plugin.
        if (winner < 0) winner = 0;
        // Cache the answer. 
        // FIXME this is really ugly but needed at the moment (because of const)
        // The best thing to do would be to emit a signal that lets us know to cache this talker.
        const_cast<TalkerMgr*>(this)->m_talkerToPlugInCache[talker] = winner;
        // kdDebug() << "TalkerMgr::talkerToPluginIndex: returning winner = " << winner << endl;
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
    return m_loadedPlugIns[talkerNdx].plugIn;
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
    return new TalkerCode(&m_loadedPlugIns[talkerNdx].parsedTalkerCode);
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
    return m_loadedPlugIns[talkerNdx].talkerID;
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
    return m_loadedPlugIns[0].talkerCode;
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
    kdDebug() << "TalkerMgr::supportsMarkup: Testing talker " << talker << endl;
    QString matchingTalker = talker;
    if (matchingTalker.isEmpty()) matchingTalker = userDefaultTalker();
    PlugInProc* plugin = talkerToPlugin(matchingTalker);
    return ( plugin->getSsmlXsltFilename() !=
            KGlobal::dirs()->resourceDirs("data").last() + "kttsd/xslt/SSMLtoPlainText.xsl");
}

/**
 * Uses KTrader to convert a translated Synth Plugin Name to DesktopEntryName.
 * @param name                   The translated plugin name.  From Name= line in .desktop file.
 * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
 *                               QString::null if not found.
 */
QString TalkerMgr::TalkerNameToDesktopEntryName(const QString& name)
{
    if (name.isEmpty()) return QString::null;
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin",
    QString("Name == '%1'").arg(name));

    if (offers.count() == 1)
        return offers[0]->desktopEntryName();
    else
        return QString::null;
}

bool TalkerMgr::autoconfigureTalker(const QString& langCode, KConfig* config)
{
    // Not yet implemented.
    // return false;

    QString languageCode = langCode;

    // Get last TalkerID from config.
    QStringList talkerIDsList = config->readListEntry("TalkerIDs", ',');
    int lastTalkerID = 0;
    for (uint talkerIdNdx = 0; talkerIdNdx < talkerIDsList.count(); ++talkerIdNdx)
    {
        int id = talkerIDsList[talkerIdNdx].toInt();
        if (id > lastTalkerID) lastTalkerID = id;
    }

    // Assign a new Talker ID for the talker.  Wraps around to 1.
    QString talkerID = QString::number(lastTalkerID + 1);

    // Query for all the KTTSD SynthPlugins.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");

    // Iterate thru the possible plug ins.
    for(unsigned int i=0; i < offers.count() ; ++i)
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
                    KParts::ComponentFactory::createInstanceFromLibrary<PlugInConf>(
                        offers[0]->library().latin1(), NULL, offers[0]->library().latin1());
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
