

/****************************************************************************
  XHTMLToSSMLParser class

  Parses a piece of XHTML markup and converts into SSML.
  -------------------
  Copyright:
  (C) 2004 by Paul Giannaros <ceruleanblaze@gmail.com>
  -------------------
  Original author: Paul Giannaros <ceruleanblaze@gmail.com>
******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include <q3dict.h>
#include <QtXml>
#include <QFile>
//Added by qt3to4:
#include <QTextStream>
#include <iostream>

#include "xmlelement.h"
#include "xhtml2ssml.h"


/// Document parsing begin. Init stuff here.
bool XHTMLToSSMLParser::startDocument() {
    /// Read the file which maps xhtml tags -> ssml tags. Look at the file for more information.
    QFile file("tagmappingrc");
    if(!file.open(QIODevice::ReadOnly)) {
        std::cerr << "Could not read config file 'tagmappingrc'. Please check that it exists and is readable.\n";
        // Kill further parsing
        return false;
    }
    QTextStream stream(&file);
    // File parsing.
    bool linestatus = true;
    while(!stream.atEnd()) {
        linestatus = readFileConfigEntry(stream.readLine());
        // If there's some syntactical error in the file then return false.
        if(!linestatus) 
            return false;
        /// Maybe call processEvents() to prevent GUI blockages?
    }
    return true;
}

bool XHTMLToSSMLParser::startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &atts) {
    QString attributes = "";
    if(atts.length() > 0) {
        const int attsLength = atts.length();
        for(int i = 0; i < attsLength; ++i)
            attributes += " " + atts.qName(i) + "=\"" + atts.value(i) + "\"";
    }
    QString fromelement = qName + attributes;
    // If this element is one of the keys that was specified in the configuration file, get what it should be converted to and 
    // append to the output string.
    QString toelement = m_xhtml2ssml[fromelement];
    if(toelement)
        m_output.append(XMLElement::fromQString(toelement).startTag());
    return true;
}

bool XHTMLToSSMLParser::endElement(const QString &, const QString &, const QString &qName) {
    QString fromelement = qName;
    QString toelement = m_xhtml2ssml[fromelement];
    if(toelement)
        m_output.append(XMLElement::fromQString(toelement).endTag());
    return true;
}

bool XHTMLToSSMLParser::characters(const QString &characters) {
    m_output.append(characters);
    return true;
}


QString XHTMLToSSMLParser::convertedText() {
    return m_output.simplified();
}

/// Parse a line from the configuration file which maps xhtml : ssml equivalent.
/// It makes entries in the m_xhtml2ssml map accordingly.
/// @param line               A line from a file to parse
/// @returns                     true if the syntax of the line was okay and the parsing succeeded - false otherwise.
bool XHTMLToSSMLParser::readFileConfigEntry(const QString &line) {
    // comments
    if(line.trimmed().startsWith("#")) {
        return true;
    }
    // break into QStringList
    // the second parameter to split is the string, with all space simplified and all space around the : removed, i.e
    //  "something     :      somethingelse"   ->  "something:somethingelse"
    QStringList keyvalue = QString( ":").replace(":").split( ":", line.simplified().replace(" :", ":"));
    if(keyvalue.count() != 2)
        return false;
    m_xhtml2ssml[keyvalue[0]] = keyvalue[1];
    return true;
}
