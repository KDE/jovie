/****************************************************************************
  XMLElement class

  Representation of an XML element with methods for getting/setting
  attributes and generating "opening" and "closing" tags.
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


#ifndef XMLELEMENT_H
#define XMLELEMENT_H

#include <qmap.h>

class QString;

typedef QMap<QString, QString> AttributeToValueMap;

class XMLElement {
public:
    XMLElement();
    XMLElement(const QString &name);
    ~XMLElement();
    
    /// Copy constructor
    XMLElement(const XMLElement &);
    
    /// Assignment operator
    XMLElement operator=(const XMLElement &element);
    
    /// Get the name of the tag (the text between the greater than and less than symbols).
    /// @returns                the name of the tag.
    QString name();
    
    /// set the name of the tag.
    /// @param name       the new name of the tag.
    void setName(const QString &name);
    
    /// Get a textual representation of the starting of the tag with all attributes and their values set out.
    /// @verbatim
    ///   XMLElement element("elem");
    ///   element.addAttribute("foo", "bar");
    ///   element.startTag();   <- <elem foo="bar">
    /// @endverbatim
    /// @returns               A textual representation of the start of the element.
    QString startTag();
                                   
    /// Get a textual representation of the closed tag that XMLElement represents.
    /// @returns               A textual representation of the closed tag represented by the XMLElement.
    QString endTag();
    
    /// Create an attribute and set its value.
    /// @param attr            The attribute.
    /// @param value          The value of the attribute.
    void setAttribute(const QString &attr, const QString &value);
    
    /// Get the value of an attribute.
    /// @param attr             The attribute.
    /// @returns                   The value of @param attr 
    QString attribute(const QString &attr);
    
    /// Convert to a QString.
    /// Had issues with QMap and custom classes. For now you can just convert to/from QString and use
    /// That as the key/value pair.
    /// @returns                    A QString representation of the XMLAttribute.
    QString toQString();
    
    /// Create an XMLElement from a QString.
    /// @param str               The QString to convert from. Must be of the following syntax- "foo name=\"bar\""
    static XMLElement fromQString(const QString &str);

private:
    /// The name of the tag.
    QString m_name;
    /// Attribute : value mappings.
    AttributeToValueMap m_attrmapper;
};

#endif
