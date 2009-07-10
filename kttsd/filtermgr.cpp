/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description:t, applying each configured Filter in turn.
    Runs synchronously via call to convert()

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

// FilterMgr includes.
#include "filtermgr.h"
#include "filtermgr.moc"

// Qt includes

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kservicetypetrader.h>

/**
 * Constructor.
 */
FilterMgr::FilterMgr( QObject *parent) :
    KttsFilterProc(parent, QVariantList())
{
    // kDebug() << "FilterMgr::FilterMgr: Running";
    m_state = fsIdle;
    m_talkerCode = 0;
}

/**
 * Destructor.
 */
FilterMgr::~FilterMgr()
{
    // kDebug() << "FilterMgr::~FilterMgr: Running";
    qDeleteAll(m_filterList);
    m_filterList.clear();
}

/**
 * Loads and initializes the filters.
 * @param config          Settings object.
 * @return                False if FilterMgr is not ready to filter.
 */
bool FilterMgr::init()
{
    // Load each of the filters and initialize.
    KSharedConfig::Ptr pConfig = KSharedConfig::openConfig( "kttsdrc" );
    KConfigGroup config( pConfig, "General");
    KConfig* rawconfig = new KConfig("kttsdrc");
    QStringList filterIDsList = config.readEntry("FilterIDs", QStringList());
    kDebug() << "FilterMgr::init: FilterIDs = " << filterIDsList;

    if ( !filterIDsList.isEmpty() )
    {
        QStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (QStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it)
        {
            QString filterID = *it;
            QString groupName = "Filter_" + filterID;
            KConfigGroup thisgroup = pConfig->group(groupName);
            QString desktopEntryName = thisgroup.readEntry( "DesktopEntryName" );
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            if (desktopEntryName.isEmpty())
            {
                QString filterPlugInName = thisgroup.readEntry("PlugInName", QString());
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty())
                    thisgroup.writeEntry("DesktopEntryName", desktopEntryName);
            }
            if (thisgroup.readEntry("Enabled",false) || thisgroup.readEntry("IsSBD",false))
            {
                kDebug() << "FilterMgr::init: filterID = " << filterID;
                KttsFilterProc* filterProc = loadFilterPlugin( desktopEntryName );
                if ( filterProc )
                {
                    filterProc->init( rawconfig, groupName );
                    m_filterList.append( filterProc );
                }
                //if (thisgroup.readEntry("DocType").contains("html") ||
                //    thisgroup.readEntry("RootElement").contains("html"))
                    //m_supportsHTML = true;
            }
        }
    }
    delete rawconfig;
    return true;
}

/**
 * Synchronously convert text.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 * @return                  Converted text.
 */
QString FilterMgr::convert(const QString& inputText, TalkerCode* talkerCode, const QString& appId)
{
    m_text = inputText;
    m_talkerCode = talkerCode;
    m_appId = appId;
    m_filterIndex = -1;
    m_filterProc = 0;
    m_state = fsFiltering;
    //m_async = false;
    while ( m_state == fsFiltering )
        nextFilter();
    return m_text;
}

// Finishes up with current filter (if any) and goes on to the next filter.
void FilterMgr::nextFilter()
{
    ++m_filterIndex;
    if (m_filterIndex == m_filterList.count())
    {
        m_state = fsFinished;
        return;
    }
    m_filterProc = m_filterList.at(m_filterIndex);
    m_text = m_filterProc->convert( m_text, m_talkerCode, m_appId );
    if (m_filterProc->wasModified())
        kDebug() << "FilterMgr::nextFilter: Filter# " << m_filterIndex << " modified the text.";
}

// Loads the processing plug in for a filter plug in given its DesktopEntryName.
KttsFilterProc* FilterMgr::loadFilterPlugin(const QString& desktopEntryName)
{
    // kDebug() << "FilterMgr::loadFilterPlugin: Running";

    // Find the plugin.
	KService::List offers = KServiceTypeTrader::self()->query("KTTSD/FilterPlugin",
        QString("DesktopEntryName == '%1'").arg(desktopEntryName));

    if (offers.count() == 1)
    {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().toLatin1());
        if(factory){
            // If the factory is created successfully, instantiate the KttsFilterConf class for the
            // specific plug in to get the plug in configuration object.
            int errorNo;
            KttsFilterProc *plugIn =
                    KLibLoader::createInstance<KttsFilterProc>(
                    offers[0]->library().toLatin1(), NULL, QStringList(offers[0]->library().toLatin1()),
             &errorNo);
            if(plugIn){
                // If everything went ok, return the plug in pointer.
                // kDebug() << "FilterMgr::loadFilterPlugin: plugin " << offers[0]->library().toLatin1() << " loaded successfully.";
                return plugIn;
            } else {
                // Something went wrong, returning null.
                kDebug() << "FilterMgr::loadFilterPlugin: Unable to instantiate KttsFilterProc class for plugin " << desktopEntryName << " error: " << errorNo;
                return NULL;
            }
        } else {
            // Something went wrong, returning null.
            kDebug() << "FilterMgr::loadFilterPlugin: Unable to create Factory object for plugin "
                << desktopEntryName << endl;
            return NULL;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    kDebug() << "FilterMgr::loadFilterPlugin: KTrader did not return an offer for plugin "
         << desktopEntryName << endl;
    return NULL;
}

/**
 * Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
 * @param name                   The translated plugin name.  From Name= line in .desktop file.
 * @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
 *                               QString() if not found.
 */
QString FilterMgr::FilterNameToDesktopEntryName(const QString& name)
{
    if (name.isEmpty()) return QString();
    KService::List offers = KServiceTypeTrader::self()->query("KTTSD/FilterPlugin",
    QString("Name == '%1'").arg(name));

    if (offers.count() == 1)
        return offers[0]->desktopEntryName();
    else
        return QString();
}

