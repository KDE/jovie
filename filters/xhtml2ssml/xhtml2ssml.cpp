

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

#include <tqstring.h>
#include <tqdict.h>
#include <tqxml.h>
#include <tqfile.h>
#include <iostream>

#include "xmlelement.h"
#include "xhtml2ssml.h"


/// Document parsing begin. Init stuff here.
bool XHTMLToSSMLParser::startDocument() {
    /// Read the file which maps xhtml tags -> ssml tags. Look at the file for more information.
    TQFile file("tagmappingrc");
    if(!file.open(IO_ReadOnly)) {
        std::cerr << "Could not read config file 'tagmappingrc'. Please check that it exists and is readable.\n";
        // Kill further parsing
        return false;
    }
    TQTextStream stream(&file);
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

bool XHTMLToSSMLParser::startElement(const TQString &, const TQString &, const TQString &qName, const TQXmlAttributes &atts) {
    TQString attributes = "";
    if(atts.length() > 0) {
        const int attsLength = atts.lenght();
        for(int i = 0; i < attsLength; ++i)
            attributes += " " + atts.qName(i) + "=\"" + atts.value(i) + "\"";
    }
    TQString fromelement = qName + attributes;
    // If this element is one of the keys that was specified in the configuration file, get what it should be converted to and 
    // append to the output string.
    TQString toelement = m_xhtml2ssml[fromelement];
    if(toelement)
        m_output.append(XMLElement::fromTQString(toelement).startTag());
    return true;
}

bool XHTMLToSSMLParser::endElement(const TQString &, const TQString &, const TQString &qName) {
    TQString fromelement = qName;
    TQString toelement = m_xhtml2ssml[fromelement];
    if(toelement)
        m_output.append(XMLElement::fromTQString(toelement).endTag());
    return true;
}

bool XHTMLToSSMLParser::characters(const TQString &characters) {
    m_output.append(characters);
    return true;
}


TQString XHTMLToSSMLParser::convertedText() {
    return m_output.simplifyWhiteSpace();
}

/// Parse a line from the configuration file which maps xhtml : ssml equivalent.
/// It makes entries in the m_xhtml2ssml map accordingly.
/// @param line               A line from a file to parse
/// @returns                     true if the syntax of the line was okay and the parsing succeeded - false otherwise.
bool XHTMLToSSMLParser::readFileConfigEntry(const TQString &line) {
    // comments
    if(line.stripWhiteSpace().startsWith("#")) {
        return true;
    }
    // break into TQStringList
    // the second parameter to split is the string, with all space simplified and all space around the : removed, i.e
    //  "something     :      somethingelse"   ->  "something:somethingelse"
    TQStringList keyvalue = TQStringList::split(":", line.simplifyWhiteSpace().tqreplace(" :", ":").tqreplace(": ", ":"));
    if(keyvalue.count() != 2)
        return false;
    m_xhtml2ssml[keyvalue[0]] = keyvalue[1];
    return true;
}
