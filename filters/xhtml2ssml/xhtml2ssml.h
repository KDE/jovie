
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


#ifndef _XHTML2SSML_H_
#define _XHTML2SSML_H_

#include <tqxml.h>
#include <tqmap.h>

typedef TQMap<TQString, TQString> QStringMap;
class TQString;

class XHTMLToSSMLParser : public TQXmlDefaultHandler {

public:
    /// No need to reimplement constructor..
    /// The document parsing starts
    bool startDocument();
    /// start of an element encountered (<element foo="bar">)
    bool startElement(const TQString &namespaceURI, const TQString &localName, const TQString &qName, const TQXmlAttributes &atts);
    /// end of an element encountered (</element>)
    bool endElement(const TQString &namespaceURI, const TQString &localName, const TQString &qName);
    /// text encountered (blah bah blah)
    bool characters(const TQString &);
    
    /// Get the output text that was generated during the parsing.
    /// @returns                    The converted text.
    TQString convertedText();
    
    /// Parse a line from the configuration file which maps xhtml : ssml equivalent.
    /// It makes entries in the m_xhtml2ssml map accordingly.
    /// @param line               A line from a file to parse
    /// @returns                     true if the syntax of the line was okay and the parsing succeeded - false otherwise.
    bool readFileConfigEntry(const TQString &line);

private:
    /// Dict of xhtml tags -> ssml tags
    QStringMap m_xhtml2ssml;
    /// The output of the conversion
    TQString m_output;
};

#endif
