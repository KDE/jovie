/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic Talker Chooser Filter Configuration class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; version 2 of the License.                 *
 *                                                                            *
 ******************************************************************************/

// Qt includes.
#include <qregexp.h>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>

// KTTS includes.
#include "talkercode.h"

// Talker Chooser includes.
#include "talkerchooserproc.h"

/**
 * Constructor.
 */
TalkerChooserProc::TalkerChooserProc( QObject *parent, const char *name, const QStringList& /*args*/ ) :
    KttsFilterProc(parent, name) 
{
    // kdDebug() << "TalkerChooserProc::TalkerChooserProc: Running" << endl;
}

/**
 * Destructor.
 */
TalkerChooserProc::~TalkerChooserProc()
{
    // kdDebug() << "TalkerChooserProc::~TalkerChooserProc: Running" << endl;
}

/**
 * Initialize the filter.
 * @param config          Settings object.
 * @param configGroup     Settings Group.
 * @return                False if filter is not ready to filter.
 *
 * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
 * separate configuration files of their own.
 */
bool TalkerChooserProc::init(KConfig* config, const QString& configGroup){
    // kdDebug() << "PlugInProc::init: Running" << endl;
    config->setGroup( configGroup );
    m_re = config->readEntry( "MatchRegExp" );
    m_appIdList = config->readListEntry( "AppIDs" );
    m_languageCode = config->readEntry( "LanguageCode" );
    m_synth = config->readEntry( "SynthInName" );
    m_gender = config->readEntry( "Gender" );
    m_volume = config->readEntry( "Volume" );
    m_rate = config->readEntry( "Rate" );
    return true;
}

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
/*virtual*/ bool TalkerChooserProc::supportsAsync() { return false; }

/**
 * Convert input, returning output.  Runs synchronously.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString TalkerChooserProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QCString& appId)
{
    if ( !m_re.isEmpty() )
    {
        int pos = inputText.find( QRegExp(m_re) );
        if ( pos < 0 ) return inputText;
    }
    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        // kdDebug() << "TalkerChooserProc::convert: converting " << inputText << " if appId "
        //      << appId << " matches " << m_appIdList << endl;
        bool found = false;
        QString appIdStr = appId;
        for ( uint ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.contains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kdDebug() << "TalkerChooserProc::convert: appId not found" << endl;
            return inputText;
        }
    }

    // Set the talker.
    // kdDebug() << "TalkerChooserProc::convert: setting lang " << m_languageCode <<
    //         " gender " << m_gender << " synth " << m_synth <<
    //         " volume " << m_volume << " rate " << m_rate << endl;
    talkerCode->setFullLanguageCode( m_languageCode );
    talkerCode->setVoice( QString::null );
    talkerCode->setGender( m_gender );
    talkerCode->setPlugInName( m_synth );
    talkerCode->setVolume( m_volume );
    talkerCode->setRate( m_rate );
    return inputText;
}

