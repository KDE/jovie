/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
 ******************************************************************************/

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

// FilterMgr includes.
#include "filtermgr.h"
#include "filtermgr.moc"

/**
 * Constructor.
 */
FilterMgrThread::FilterMgrThread( QObject *parent, const char *name ) :
    QObject( parent, name ),
    QThread()
{
}

/**
 * Destructor.
 */
/*virtual*/ FilterMgrThread::~FilterMgrThread()
{
}

/**
 * Loads and initializes the filters.
 * @param config          Settings object.
 * @return                False if FilterMgr is not ready to filter.
 */
bool FilterMgrThread::init(KConfig *config)
{
    // Load each of the filters and initialize.
    config->setGroup("General");
    QStringList filterIDsList = config->readListEntry("FilterIDs", ',');
    // kdDebug() << "FilterMgr::init: FilterIDs = " << filterIDsList << endl;
    if ( !filterIDsList.isEmpty() )
    {
        QStringList::ConstIterator itEnd = filterIDsList.constEnd();
        for (QStringList::ConstIterator it = filterIDsList.constBegin(); it != itEnd; ++it)
        {
            QString filterID = *it;
            QString groupName = "Filter_" + filterID;
            config->setGroup( groupName );
            QString filterPlugInName = config->readEntry( "PlugInName" );
            if (config->readBoolEntry("Enabled"))
            {
                // kdDebug() << "FilterMgr::init: filterID = " << filterID << endl;
                KttsFilterProc* filterProc = loadFilterPlugin( filterPlugInName );
                if ( filterProc )
                {
                    filterProc->init( config, groupName );
                    m_filterList.append( filterProc );
                }
            }
        }
    }
    return true;
}

/**
 * Get/Set text being processed.
 */
void FilterMgrThread::setText( const QString& text ) { m_text = text; }
QString FilterMgrThread::text() { return m_text; }

/**
 * Set/Get TalkerCode.
 */
void FilterMgrThread::setTalkerCode( TalkerCode* talkerCode ) { m_talkerCode = talkerCode; }
TalkerCode* FilterMgrThread::talkerCode() { return m_talkerCode; }

/**
 * Set/Get AppId.
 */
void FilterMgrThread::setAppId( const QCString& appId ) { m_appId = appId; }
QCString FilterMgrThread::appId() { return m_appId; }

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
void FilterMgrThread::setSbRegExp( const QString& re ) { m_re = re; }

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
bool FilterMgrThread::wasModified() { return m_wasModified; }

// This is where the real work takes place.
/*virtual*/ void FilterMgrThread::run()
{
    // kdDebug() << "FilterMgrThread::run: processing text = " << m_text << endl;
    m_wasModified = false;

    for ( uint ndx=0; ndx < m_filterList.count(); ++ndx )
    {
        KttsFilterProc* filterProc = m_filterList.at( ndx );
        if ( filterProc->isSBD() ) filterProc->setSbRegExp( m_re );
        m_text = filterProc->convert(m_text, m_talkerCode, m_appId);
        m_wasModified = m_wasModified || filterProc->wasModified();
        if ( filterProc->wasModified() ) 
            kdDebug() << "FilterMgrThread::run: Filter " << ndx << " modified the text." << endl;
        // Stop if the filter was a SBD and it modified the text.
        if ( filterProc->isSBD() && filterProc->wasModified() ) break;
    }

    // Result is in m_text;

    // Post an event.  We need to emit filterFinished signal, but not from the
    // separate thread.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 301);
    QApplication::postEvent(this, ev);
}

bool FilterMgrThread::event ( QEvent * e )
{
    if ( e->type() == (QEvent::User + 301) )
    {
        // kdDebug() << "FilterMgrThread::event: emitting filteringFinished signal." << endl;
        emit filteringFinished();
        return TRUE;
    }
    else return FALSE;
}

// Loads the processing plug in for a named filter plug in.
KttsFilterProc* FilterMgrThread::loadFilterPlugin(const QString& plugInName)
{
    // kdDebug() << "FilterMgr::loadFilterPlugin: Running"<< endl;

    // Get list of plugins.
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/FilterPlugin");

    // Iterate thru the offers to find the plug in that matches the name.
    for(unsigned int i=0; i < offers.count() ; ++i){
        // Compare the plug in to be loaded with the entry in offers[i]
        // kdDebug() << "Comparing " << offers[i]->plugInName() << " to " << synthName << endl;
        if(offers[i]->name() == plugInName)
        {
            // When the entry is found, load the plug in
            // First create a factory for the library
            KLibFactory *factory = KLibLoader::self()->factory(offers[i]->library().latin1());
            if(factory){
                // If the factory is created successfully, instantiate the KttsFilterConf class for the
                // specific plug in to get the plug in configuration object.
                int errorNo;
                KttsFilterProc *plugIn =
                        KParts::ComponentFactory::createInstanceFromLibrary<KttsFilterProc>(
                        offers[i]->library().latin1(), NULL, offers[i]->library().latin1(),
                QStringList(), &errorNo);
                if(plugIn){
                    // If everything went ok, return the plug in pointer.
                    return plugIn;
                } else {
                    // Something went wrong, returning null.
                    kdDebug() << "FilterMgr::loadFilterPlugin: Unable to instantiate KttsFilterProc class for plugin " << plugInName << " error: " << errorNo << endl;
                    return NULL;
                }
            } else {
                // Something went wrong, returning null.
                kdDebug() << "FilterMgr::loadFilterPlugin: Unable to create Factory object for plugin " << plugInName << endl;
                return NULL;
            }
            break;
        }
    }
    // The plug in was not found (unexpected behaviour, returns null).
    kdDebug() << "FilterMgr::loadFilterPlugin: KTrader did not return an offer for plugin " << plugInName << endl;
    return NULL;
}

// ----------------------------------------------------------------------------

/**
 * Constructor.
 */
FilterMgr::FilterMgr( QObject *parent, const char *name) :
    KttsFilterProc(parent, name) 
{
    // kdDebug() << "FilterMgr::FilterMgr: Running" << endl;
    m_state = fsIdle;
    m_filterMgrThread = new FilterMgrThread( parent, "filtermgr_thread" );
    connect( m_filterMgrThread, SIGNAL(filteringFinished()), this, SLOT(slotThreadFilteringFinished()) );
}

/**
 * Destructor.
 */
FilterMgr::~FilterMgr()
{
    // kdDebug() << "FilterMgr::~FilterMgr: Running" << endl;
    if ( m_filterMgrThread )
    {
        if ( m_filterMgrThread->running() )
        {
            m_filterMgrThread->terminate();
            m_filterMgrThread->wait();
        }
        delete m_filterMgrThread;
    }
}

/**
 * Loads and initializes the filters.
 * @param config          Settings object.
 * @return                False if FilterMgr is not ready to filter.
 */
bool FilterMgr::init(KConfig *config, const QString& /*configGroup*/)
{
    m_config = config;
    return m_filterMgrThread->init( config );
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
QString FilterMgr::convert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId)
{
    if (asyncConvert(inputText, talkerCode, appId))
    {
        waitForFinished();
        return m_filterMgrThread->text();
    } else return inputText;
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
bool FilterMgr::asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId)
{
    m_filterMgrThread->setText( inputText );
    m_filterMgrThread->setTalkerCode( talkerCode );
    m_filterMgrThread->setAppId( appId );
    m_filterMgrThread->setSbRegExp( m_re );
    m_state = fsFiltering;
    m_filterMgrThread->start();
    return true;
}

/**
 * Waits for filtering to finish.
 */
void FilterMgr::waitForFinished()
{
    if ( m_filterMgrThread->running() )
    {
        // kdDebug() << "FilterMgr::waitForFinished: waiting" << endl;
        m_filterMgrThread->wait();
        // kdDebug() << "FilterMgr::waitForFinished: finished waiting" << endl;
        m_state = fsFinished;
        m_re = QString::null;
    }
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
    return m_filterMgrThread->text();
}

/**
 * Acknowledges the finished filtering.
 */
void FilterMgr::ackFinished()
{
    m_state = fsIdle;
}

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped.
 */
void FilterMgr::stopFiltering()
{
    if ( m_filterMgrThread->running() )
    {
        m_filterMgrThread->terminate();
        m_filterMgrThread->wait();
        // Delete and re-create FilterMgr thread.
        delete m_filterMgrThread;
        m_filterMgrThread = new FilterMgrThread();
        m_filterMgrThread->init( m_config );
        connect( m_filterMgrThread, SIGNAL(filteringFinished()), this,
             SLOT(slotThreadFilteringFinished()) );
        m_state = fsIdle;
        emit filteringStopped();
    }
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

void FilterMgr::slotThreadFilteringFinished()
{
    m_state = fsFinished;
    m_re = QString::null;
    // kdDebug() << "FilterMgr::slotThreadFilteringFinished: emitting filterFinished signal." << endl;
    emit filteringFinished();
}

