/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Processing class.
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
#include <qdom.h>
#include <qfile.h>
#include <qlistview.h>

// KDE includes.
// #include <kdebug.h>
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
StringReplacerProc::StringReplacerProc( QObject *parent, const char *name, const QStringList& ) :
    KttsFilterProc(parent, name)
{
}

/**
 * Destructor.
 */
/*virtual*/ StringReplacerProc::~StringReplacerProc()
{
    m_matchList.clear();
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
bool StringReplacerProc::init(KConfig* /*config*/, const QString& configGroup){
    // kdDebug() << "StringReplacerProc::init: Running" << endl;
    QString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,"kttsd/stringreplacer/", false );
    if ( wordsFilename.isEmpty() ) return false;
    wordsFilename += configGroup;

    // Open existing word list.
    QFile file( wordsFilename );
    if ( !file.open( IO_ReadOnly ) )
        return false;
    QDomDocument doc( "" );
    if ( !doc.setContent( &file ) ) {
        file.close();
        return false;
    }
    file.close();

    // Clear list.
    m_matchList.clear();
    m_substList.clear();

    // Name setting.
    // QDomNodeList nameList = doc.elementsByTagName( "name" );
    // QDomNode nameNode = nameList.item( 0 );
    // m_widget->nameLineEdit->setText( nameNode.toElement().text() );

    // Language Code setting.
    QDomNodeList languageList = doc.elementsByTagName( "language-code" );
    QDomNode languageNode = languageList.item( 0 );
    m_languageCode = languageNode.toElement().text();

    // Word list.
    QDomNodeList wordList = doc.elementsByTagName("word");
    int wordListCount = wordList.count();
    for (int wordIndex = 0; wordIndex < wordListCount; wordIndex++)
    {
        QDomNode wordNode = wordList.item(wordIndex);
        QDomNodeList propList = wordNode.childNodes();
        QString wordType;
        QString match;
        QString subst;
        int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; propIndex++)
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == "type") wordType = prop.text();
            if (prop.tagName() == "match") match = prop.text();
            if (prop.tagName() == "subst") subst = prop.text();
        }
        // Build Regular Expression for each word's match string.
        QRegExp rx;
        if ( wordType == i18n("Word") )
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
 */
/*virtual*/ QString StringReplacerProc::convert(const QString& inputText, TalkerCode* talkerCode)
{
    // If language doesn't match, return input unmolested.
    // kdDebug() << "StringReplacerProc::convert: converting " << inputText << " if language code "
    //    << talkerCode->languageCode() << " matches " << m_languageCode << endl;
    if (m_languageCode != talkerCode->languageCode()) return inputText;
    QString newText = inputText;
    const int listCount = m_matchList.count();
    for ( int index = 0; index < listCount; index++ )
    {
        newText.replace( m_matchList[index], m_substList[index] );
    }
    return newText;
}

