/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

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

// Qt includes
#include <QCoreApplication>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <klocale.h>

// FilterMgr includes.
#include "filtermgr.h"
#include "filtermgr.moc"

/**
 * Constructor.
 */
FilterMgr::FilterMgr( QObject *parent, const char *name) :
    KttsFilterProc(parent, name) 
{
    // kDebug() << "FilterMgr::FilterMgr: Running" << endl;
    m_state = fsIdle;
    m_noSBD = false;
    m_supportsHTML = false;
    m_talkerCode = 0;
}

/**
 * Destructor.
 */
FilterMgr::~FilterMgr()
{
    // kDebug() << "FilterMgr::~FilterMgr: Running" << endl;
    if ( m_state == fsFiltering )
        stopFiltering();
    qDeleteAll(m_filterList);
    m_filterList.clear();
}

/**
 * Loads and initializes the filters.
 * @param config          Settings object.
 * @return                False if FilterMgr is not ready to filter.
 */
bool FilterMgr::init(KConfig *config, const QString& /*configGroup*/)
{
    // Load each of the filters and initialize.
    config->setGroup("General");
    QStringList filterIDsList = config->readEntry("FilterIDs", QStringList(), ',');
    // kDebug() << "FilterMgr::init: FilterIDs = " << filterIDsList << endl;
    // If no filters have been configured, automatically configure the standard SBD.
    if (filterIDsList.isEmpty())
    {
        config->setGroup("Filter_1");
        config->writeEntry("DesktopEntryName", "kttsd_sbdplugin");
        config->writeEntry("Enabled", true);
        config->writeEntry("IsSBD", true);
        config->writeEntry("MultiInstance", true);
        config->writeEntry("SentenceBoundary", "\\1\\t");
        config->writeEntry("SentenceDelimiterRegExp", "([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))");
        config->writeEntry("UserFilterName", i18n("Standard Sentence Boundary Detector"));
        config->setGroup("General");
        config->writeEntry("FilterIDs", "1");
        filterIDsList = config->readEntry("FilterIDs", QStringList(), ',');
    }
    if ( !filterIDsList.isEmpty() )
    {
        QStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (QStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it)
        {
            QString filterID = *it;
            QString groupName = "Filter_" + filterID;
            config->setGroup( groupName );
            QString desktopEntryName = config->readEntry( "DesktopEntryName" );
            // If a DesktopEntryName is not in the config file, it was configured before
            // we started using them, when we stored translated plugin names instead.
            // Try to convert the translated plugin name to a DesktopEntryName.
            // DesktopEntryNames are better because user can change their desktop language
            // and DesktopEntryName won't change.
            if (desktopEntryName.isEmpty())
            {
                QString filterPlugInName = config->readEntry("PlugInName", QString());
                // See if the translated name will untranslate.  If not, well, sorry.
                desktopEntryName = FilterNameToDesktopEntryName(filterPlugInName);
                // Record the DesktopEntryName from now on.
                if (!desktopEntryName.isEmpty()) config->writeEntry("DesktopEntryName", desktopEntryName);
            }
            if (config->readEntry("Enabled",QVariant(false)).toBool()  || config->readEntry("IsSBD",QVariant(false)).toBool())
            {
                // kDebug() << "FilterMgr::init: filterID = " << filterID << endl;
                KttsFilterProc* filterProc = loadFilterPlugin( desktopEntryName );
                if ( filterProc )
                {
                    filterProc->init( config, groupName );
                    m_filterList.append( filterProc );
                }
                if (config->readEntry("DocType").contains("html") ||
                    config->readEntry("RootElement").contains("html"))
                    m_supportsHTML = true;
            }
        }
    }
    return true;
}

/**
 * Returns True if this filter is a Sentence Boundary Detector.
 * If so, the filter should implement @ref setSbRegExp() .
 * @return          True if this filter is a SBD.
 */
/*virtual*/ bool FilterMgr::isSBD() { return true; }

/**
 * Returns True if the plugin supports asynchronous processing,
 * i.e., supports asyncConvert method.
 * @return                        True if this plugin supports asynchronous processing.
 *
 * If the plugin returns True, it must also implement @ref getState .
 * It must also emit @ref filteringFinished when filtering is completed.
 * If the plugin returns True, it must also implement @ref stopFiltering .
 * It must also emit @ref filteringStopped when filtering has been stopped.
 */
/*virtual*/ bool FilterMgr::supportsAsync() { return true; }

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
QString FilterMgr::convert(const QString& inputText, TalkerCode* talkerCode, const QByteArray& appId)
{
    m_text = inputText;
    m_talkerCode = talkerCode;
    m_appId = appId;
    m_filterIndex = -1;
    m_filterProc = 0;
    m_state = fsFiltering;
    m_async = false;
    while ( m_state == fsFiltering )
        nextFilter();
    return m_text;
}

/**
 * Aynchronously convert input.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 *
 * When the input text has been converted, filteringFinished signal will be emitted
 * and caller can retrieve using getOutput();
*/
bool FilterMgr::asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QByteArray& appId)
{
    m_text = inputText;
    m_talkerCode = talkerCode;
    m_appId = appId;
    m_filterIndex = -1;
    m_filterProc = 0;
    m_state = fsFiltering;
    m_async = true;
    nextFilter();
    return true;
}

// Finishes up with current filter (if any) and goes on to the next filter.
void FilterMgr::nextFilter()
{
    if ( m_filterProc )
    {
        if ( m_filterProc->supportsAsync() )
        {
            m_text = m_filterProc->getOutput();
            m_filterProc->ackFinished();
            disconnect( m_filterProc, SIGNAL(filteringFinished()), this, SLOT(slotFilteringFinished()) );
        }
        // if ( m_filterProc->wasModified() )
        //     kDebug() << "FilterMgr::nextFilter: Filter# " << m_filterIndex << " modified the text." << endl;
        if ( m_filterProc->wasModified() && m_filterProc->isSBD() )
        {
            m_state = fsFinished;
            // Post an event which will be later emitted as a signal.
            QCustomEvent* ev = new QCustomEvent(QEvent::User + 301);
            QCoreApplication::postEvent(this, ev);
            return;
        }
    }
    ++m_filterIndex;
    if ( m_filterIndex == static_cast<int>(m_filterList.count()) )
    {
        m_state = fsFinished;
        // Post an event which will be later emitted as a signal.
        QCustomEvent* ev = new QCustomEvent(QEvent::User + 301);
        QCoreApplication::postEvent(this, ev);
        return;
    }
    m_filterProc = m_filterList.at(m_filterIndex);
    if ( m_noSBD && m_filterProc->isSBD() )
    {
        m_state = fsFinished;
        // Post an event which will be later emitted as a signal.
        QCustomEvent* ev = new QCustomEvent(QEvent::User + 301);
        QCoreApplication::postEvent(this, ev);
        return;
    }
    m_filterProc->setSbRegExp( m_re );
    if ( m_async )
    {
        if ( m_filterProc->supportsAsync() )
        {
            // kDebug() << "FilterMgr::nextFilter: calling asyncConvert on filter " << m_filterIndex << endl;
            connect( m_filterProc, SIGNAL(filteringFinished()), this, SLOT(slotFilteringFinished()) );
            if ( !m_filterProc->asyncConvert( m_text, m_talkerCode, m_appId ) )
            {
                disconnect( m_filterProc, SIGNAL(filteringFinished()), this, SLOT(slotFilteringFinished()) );
                m_filterProc = 0;
                nextFilter();
            }
        } else {
            m_text = m_filterProc->convert( m_text, m_talkerCode, m_appId );
            nextFilter();
        }
    } else
        m_text = m_filterProc->convert( m_text, m_talkerCode, m_appId );
}

// Received when each filter finishes.
void FilterMgr::slotFilteringFinished()
{
    // kDebug() << "FilterMgr::slotFilteringFinished: received signal from filter " << m_filterIndex << endl;
    nextFilter();
}

bool FilterMgr::event ( QEvent * e )
{
    if ( e->type() == (QEvent::User + 301) )
    {
        // kDebug() << "FilterMgr::event: emitting filteringFinished signal." << endl;
        emit filteringFinished();
        return true;
    }
    if ( e->type() == (QEvent::User + 302) )
    {
        // kDebug() << "FilterMgr::event: emitting filteringStopped signal." << endl;
        emit filteringStopped();
        return true;
    }
    else return false;
}

/**
 * Waits for filtering to finish.
 */
void FilterMgr::waitForFinished()
{
    if ( m_state != fsFiltering ) return;
    disconnect(m_filterProc, SIGNAL(filteringFinished()), this, SLOT(slotFilteringFinished()) );
    m_async = false;
    m_filterProc->waitForFinished();
    while ( m_state == fsFiltering )
        nextFilter();
}

/**
 * Returns the state of the FilterMgr.
 */
int FilterMgr::getState() { return m_state; }

/**
 * Returns the filtered output.
 */
QString FilterMgr::getOutput()
{
    return m_text;
}

/**
 * Acknowledges the finished filtering.
 */
void FilterMgr::ackFinished()
{
    m_state = fsIdle;
    m_text.clear();
}

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped.
 */
void FilterMgr::stopFiltering()
{
    if ( m_state != fsFiltering ) return;
    if ( m_async )
        disconnect( m_filterProc, SIGNAL(filteringFinished()), this, SLOT(slotFilteringFinished()) );
    m_filterProc->stopFiltering();
    m_state = fsIdle;
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 302);
    QCoreApplication::postEvent(this, ev);
}

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
/*virtual*/ void FilterMgr::setSbRegExp(const QString& re)
{
    m_re = re;
}

/**
 * Do not call SBD filters.
 */
void FilterMgr::setNoSBD(bool noSBD) { m_noSBD = noSBD; }
bool FilterMgr::noSBD() { return m_noSBD; }

// Loads the processing plug in for a filter plug in given its DesktopEntryName.
KttsFilterProc* FilterMgr::loadFilterPlugin(const QString& desktopEntryName)
{
    // kDebug() << "FilterMgr::loadFilterPlugin: Running"<< endl;

    // Find the plugin.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
        QString("DesktopEntryName == '%1'").arg(desktopEntryName));

    if (offers.count() == 1)
    {
        // When the entry is found, load the plug in
        // First create a factory for the library
        KLibFactory *factory = KLibLoader::self()->factory(offers[0]->library().latin1());
        if(factory){
            // If the factory is created successfully, instantiate the KttsFilterConf class for the
            // specific plug in to get the plug in configuration object.
            int errorNo;
            KttsFilterProc *plugIn =
                    KLibLoader::createInstance<KttsFilterProc>(
                    offers[0]->library().latin1(), NULL, offers[0]->library().latin1(),
            QStringList(), &errorNo);
            if(plugIn){
                // If everything went ok, return the plug in pointer.
                return plugIn;
            } else {
                // Something went wrong, returning null.
                kDebug() << "FilterMgr::loadFilterPlugin: Unable to instantiate KttsFilterProc class for plugin " << desktopEntryName << " error: " << errorNo << endl;
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
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin",
    QString("Name == '%1'").arg(name));

    if (offers.count() == 1)
        return offers[0]->desktopEntryName();
    else
        return QString();
}

