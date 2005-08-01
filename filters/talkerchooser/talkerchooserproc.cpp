/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic Talker Chooser Filter Configuration class.
  -------------------
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
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>

// KTTS includes.
#include "talkercode.h"

// Talker Chooser includes.
#include "talkerchooserproc.h"
#include "talkerchooserproc.moc"

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
    m_chosenTalkerCode = TalkerCode(config->readEntry("TalkerCode"), false);
    // Legacy settings.
    QString s = config->readEntry( "LanguageCode" );
    if (!s.isEmpty()) m_chosenTalkerCode.setFullLanguageCode(s);
    s = config->readEntry( "SynthInName" );
    if (!s.isEmpty()) m_chosenTalkerCode.setPlugInName(s);
    s = config->readEntry( "Gender" );
    if (!s.isEmpty()) m_chosenTalkerCode.setGender(s);
    s = config->readEntry( "Volume" );
    if (!s.isEmpty()) m_chosenTalkerCode.setVolume(s);
    s = config->readEntry( "Rate" );
    if (!s.isEmpty()) m_chosenTalkerCode.setRate(s);
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
    const Q3CString& appId)
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
    // Only override the language if user specified a language code.
    if (!m_chosenTalkerCode.fullLanguageCode().isEmpty())
        talkerCode->setFullLanguageCode(m_chosenTalkerCode.fullLanguageCode());
    talkerCode->setVoice(m_chosenTalkerCode.voice());
    talkerCode->setGender(m_chosenTalkerCode.gender());
    talkerCode->setPlugInName(m_chosenTalkerCode.plugInName());
    talkerCode->setVolume(m_chosenTalkerCode.volume());
    talkerCode->setRate(m_chosenTalkerCode.rate());
    return inputText;
}
