/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Processing class.
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

// TQt includes.
#include <tqdom.h>
#include <tqfile.h>
#include <tqlistview.h>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "filterproc.h"
#include "talkercode.h"

// StringReplacer includes.
#include "stringreplacerproc.h"
#include "stringreplacerproc.moc"

/**
 * Constructor.
 */
StringReplacerProc::StringReplacerProc( TQObject *tqparent, const char *name, const TQStringList& ) :
    KttsFilterProc(tqparent, name)
{
}

/**
 * Destructor.
 */
/*virtual*/ StringReplacerProc::~StringReplacerProc()
{
    m_matchList.clear();
    m_caseList.clear();
    m_substList.clear();
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
bool StringReplacerProc::init(KConfig* config, const TQString& configGroup){
    // kdDebug() << "StringReplacerProc::init: Running" << endl;
    TQString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", false );
    if ( wordsFilename.isEmpty() ) return false;
    wordsFilename += configGroup;
    config->setGroup( configGroup );
    wordsFilename = config->readEntry( "WordListFile", wordsFilename );

    // Open existing word list.
    TQFile file( wordsFilename );
    if ( !file.open( IO_ReadOnly ) )
        return false;
    TQDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        return false;
    }
    file.close();

    // Clear list.
    m_matchList.clear();
    m_caseList.clear();
    m_substList.clear();

    // Name setting.
    // TQDomNodeList nameList = doc.elementsByTagName( "name" );
    // TQDomNode nameNode = nameList.item( 0 );
    // m_widget->nameLineEdit->setText( nameNode.toElement().text() );

    // Language Codes setting.  List may be single element of comma-separated values,
    // or multiple elements.
    m_languageCodeList.clear();
    TQDomNodeList languageList = doc.elementsByTagName( "language-code" );
    for ( uint ndx=0; ndx < languageList.count(); ++ndx )
    {
        TQDomNode languageNode = languageList.item( ndx );
        m_languageCodeList += TQStringList::split(',', languageNode.toElement().text(), false);
    }

    // AppId.  Apply this filter only if DCOP appId of application that queued
    // the text contains this string.  List may be single element of comma-separated values,
    // or multiple elements.
    m_appIdList.clear();
    TQDomNodeList appIdList = doc.elementsByTagName( "appid" );
    for ( uint ndx=0; ndx < appIdList.count(); ++ndx )
    {
        TQDomNode appIdNode = appIdList.item( ndx );
        m_appIdList += TQStringList::split(',', appIdNode.toElement().text(), false);
    }

    // Word list.
    TQDomNodeList wordList = doc.elementsByTagName("word");
    const int wordListCount = wordList.count();
    for (int wordIndex = 0; wordIndex < wordListCount; ++wordIndex)
    {
        TQDomNode wordNode = wordList.item(wordIndex);
        TQDomNodeList propList = wordNode.childNodes();
        TQString wordType;
        TQString matchCase = "No"; // Default for old (v<=3.5.3) config files with no <case/>.
        TQString match;
        TQString subst;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex)
        {
            TQDomNode propNode = propList.item(propIndex);
            TQDomElement prop = propNode.toElement();
            if (prop.tagName() == "type") wordType = prop.text();
	    if (prop.tagName() == "case") matchCase = prop.text();
            if (prop.tagName() == "match") match = prop.text();
            if (prop.tagName() == "subst") subst = prop.text();
        }
        // Build Regular Expression for each word's match string.
        TQRegExp rx;
        rx.setCaseSensitive(matchCase == "Yes");
        if ( wordType == "Word" )
        {
                // TODO: Does \b honor strange non-Latin1 encodings?
            rx.setPattern( "\\b" + match + "\\b" );
        }
        else
        {
            rx.setPattern( match );
        }
            // Add Regular Expression to list (if valid).
        if ( rx.isValid() )
        {
            m_matchList.append( rx );
            m_substList.append( subst );
        }
    }
    return true;
}

/**
 * Convert input, returning output.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ TQString StringReplacerProc::convert(const TQString& inputText, TalkerCode* talkerCode, const TQCString& appId)
{
    m_wasModified = false;
    // If language doesn't match, return input unmolested.
    if ( !m_languageCodeList.isEmpty() )
    {
        TQString languageCode = talkerCode->languageCode();
        // kdDebug() << "StringReplacerProc::convert: converting " << inputText << 
        //    " if language code " << languageCode << " matches " << m_languageCodeList << endl;
        if ( !m_languageCodeList.tqcontains( languageCode ) )
        {
            if ( !talkerCode->countryCode().isEmpty() )
            {
                languageCode += '_' + talkerCode->countryCode();
                // kdDebug() << "StringReplacerProc::convert: converting " << inputText << 
                //    " if language code " << languageCode << " matches " << m_languageCodeList << endl;
                if ( !m_languageCodeList.tqcontains( languageCode ) ) return inputText;
            } else return inputText;
        }
    }
    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        // kdDebug() << "StringReplacerProc::convert: converting " << inputText << " if appId "
        //     << appId << " matches " << m_appIdList << endl;
        bool found = false;
        TQString appIdStr = appId;
        for ( uint ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.tqcontains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kdDebug() << "StringReplacerProc::convert: appId not found" << endl;
            return inputText;
        }
    }
    TQString newText = inputText;
    const int listCount = m_matchList.count();
    for ( int index = 0; index < listCount; ++index )
    {
        //kdDebug() << "newtext = " << newText << " matching " << m_matchList[index].pattern() << " replacing with " << m_substList[index] << endl;
        newText.tqreplace( m_matchList[index], m_substList[index] );
    }
    m_wasModified = true;
    return newText;
}

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool StringReplacerProc::wasModified() { return m_wasModified; }

