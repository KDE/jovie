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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Talker Chooser includes.
#include "talkerchooserproc.h"
#include "talkerchooserproc.moc"

// Qt includes.
#include <QtCore/QRegExp>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// KTTS includes.
#include "talkercode.h"

TalkerChooserProc::TalkerChooserProc( QObject *parent, const QVariantList& args ) :
    KttsFilterProc(parent, args) 
{
    Q_UNUSED(args);
    // kDebug() << "TalkerChooserProc::TalkerChooserProc: Running";
}

TalkerChooserProc::~TalkerChooserProc()
{
    // kDebug() << "TalkerChooserProc::~TalkerChooserProc: Running";
}

bool TalkerChooserProc::init(KConfig* c, const QString& configGroup){
    // kDebug() << "PlugInProc::init: Running";
    KConfigGroup config( c, configGroup );
    m_re = config.readEntry( "MatchRegExp" );
    m_appIdList = config.readEntry( "AppIDs", QStringList() );
    m_chosenTalkerCode = TalkerCode(config.readEntry("TalkerCode"), false);
    // Legacy settings.
    QString s = config.readEntry( "LanguageCode" );
    if (!s.isEmpty()) m_chosenTalkerCode.setLanguage(s);
    s = config.readEntry( "SynthInName" );
    //if (!s.isEmpty()) m_chosenTalkerCode.setPlugInName(s);
    s = config.readEntry( "Gender" );
    //if (!s.isEmpty()) m_chosenTalkerCode.setGender(s);
    s = config.readEntry( "Volume" );
    //if (!s.isEmpty()) m_chosenTalkerCode.setVolume(s);
    s = config.readEntry( "Rate" );
    //if (!s.isEmpty()) m_chosenTalkerCode.setRate(s);
    return true;
}

/*virtual*/ bool TalkerChooserProc::supportsAsync() { return false; }

/*virtual*/ QString TalkerChooserProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QString& appId)
{
    if ( !m_re.isEmpty() )
    {
        int pos = inputText.indexOf( QRegExp(m_re) );
        if ( pos < 0 ) return inputText;
    }
    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        // kDebug() << "TalkerChooserProc::convert: converting " << inputText << " if appId "
        //      << appId << " matches " << m_appIdList << endl;
        bool found = false;
        QString appIdStr = appId;
        for (int ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.contains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kDebug() << "TalkerChooserProc::convert: appId not found";
            return inputText;
        }
    }

    // Set the talker.
    // kDebug() << "TalkerChooserProc::convert: setting lang " << m_languageCode <<
    //         " gender " << m_gender << " synth " << m_synth <<
    //         " volume " << m_volume << " rate " << m_rate << endl;
    // Only override the language if user specified a language code.
    if (!m_chosenTalkerCode.language().isEmpty())
        talkerCode->setLanguage(m_chosenTalkerCode.language());
    //talkerCode->setVoice(m_chosenTalkerCode.voice());
    //talkerCode->setGender(m_chosenTalkerCode.gender());
    //talkerCode->setPlugInName(m_chosenTalkerCode.plugInName());
    //talkerCode->setVolume(m_chosenTalkerCode.volume());
    //talkerCode->setRate(m_chosenTalkerCode.rate());
    return inputText;
}
