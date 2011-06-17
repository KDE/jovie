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

#include <tqmap.h>

class TQString;

typedef TQMap<TQString, TQString> AttributeToValueMap;

class XMLElement {
public:
    XMLElement();
    XMLElement(const TQString &name);
    ~XMLElement();
    
    /// Copy constructor
    XMLElement(const XMLElement &);
    
    /// Assignment operator
    XMLElement operator=(const XMLElement &element);
    
    /// Get the name of the tag (the text between the greater than and less than symbols).
    /// @returns                the name of the tag.
    TQString name();
    
    /// set the name of the tag.
    /// @param name       the new name of the tag.
    void setName(const TQString &name);
    
    /// Get a textual representation of the starting of the tag with all attributes and their values set out.
    /// @verbatim
    ///   XMLElement element("elem");
    ///   element.addAttribute("foo", "bar");
    ///   element.startTag();   <- <elem foo="bar">
    /// @endverbatim
    /// @returns               A textual representation of the start of the element.
    TQString startTag();
                                   
    /// Get a textual representation of the closed tag that XMLElement represents.
    /// @returns               A textual representation of the closed tag represented by the XMLElement.
    TQString endTag();
    
    /// Create an attribute and set its value.
    /// @param attr            The attribute.
    /// @param value          The value of the attribute.
    void setAttribute(const TQString &attr, const TQString &value);
    
    /// Get the value of an attribute.
    /// @param attr             The attribute.
    /// @returns                   The value of @param attr 
    TQString attribute(const TQString &attr);
    
    /// Convert to a TQString.
    /// Had issues with TQMap and custom classes. For now you can just convert to/from TQString and use
    /// That as the key/value pair.
    /// @returns                    A TQString representation of the XMLAttribute.
    TQString toTQString();
    
    /// Create an XMLElement from a TQString.
    /// @param str               The TQString to convert from. Must be of the following syntax- "foo name=\"bar\""
    static XMLElement fromTQString(const TQString &str);

private:
    /// The name of the tag.
    TQString m_name;
    /// Attribute : value mappings.
    AttributeToValueMap m_attrmapper;
};

#endif
