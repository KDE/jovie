
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

#include <qxml.h>
#include <qmap.h>

typedef QMap<QString, QString> QStringMap;
class QString;

class XHTMLToSSMLParser : public QXmlDefaultHandler {

public:
    /// No need to reimplement constructor..
    /// The document parsing starts
    bool startDocument();
    /// start of an element encountered (<element foo="bar">)
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    /// end of an element encountered (</element>)
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    /// text encountered (blah bah blah)
    bool characters(const QString &);
    
    /// Get the output text that was generated during the parsing.
    /// @returns                    The converted text.
    QString convertedText();
    
    /// Parse a line from the configuration file which maps xhtml : ssml equivalent.
    /// It makes entries in the m_xhtml2ssml map accordingly.
    /// @param line               A line from a file to parse
    /// @returns                     true if the syntax of the line was okay and the parsing succeeded - false otherwise.
    bool readFileConfigEntry(const QString &line);

private:
    /// Dict of xhtml tags -> ssml tags
    QStringMap m_xhtml2ssml;
    /// The output of the conversion
    QString m_output;
};

#endif
