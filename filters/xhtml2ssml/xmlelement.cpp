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

#include "xmlelement.h"
#include <tqstringlist.h>
#include <iostream>

/// Constructors
XMLElement::XMLElement() {
    m_name = "";
    m_attrmapper = AttributeToValueMap();
}
XMLElement::XMLElement(const TQString &name) {
    m_name = name;
    m_attrmapper = AttributeToValueMap();
}
/// Destructor
XMLElement::~XMLElement() {
    return;
}

/// Copy constructor
XMLElement::XMLElement(const XMLElement &element) {
    m_attrmapper = element.m_attrmapper;
    m_name = element.m_name;
}

/// Assignement operator
XMLElement XMLElement::operator=(const XMLElement &element) {
    m_attrmapper = element.m_attrmapper;
    m_name = element.m_name;
    return *this;
}

TQString XMLElement::name() {
    return m_name;
}
TQString XMLElement::startTag() {
    TQString output = "<" + m_name + " ";
    for(AttributeToValueMap::Iterator it = m_attrmapper.begin(); it != m_attrmapper.end(); ++it) {
        output.append(it.key() + "=\"" + it.data() + "\" ");
    }
    output = output.left(output.length() - 1);
    // Get rid of the space at the end and then append a '>'
    output.append(">");
    return output;
}

TQString XMLElement::endTag() {
    return "</" + m_name + ">";
}

void XMLElement::setAttribute(const TQString &attr, const TQString &value) {
    m_attrmapper[attr] = value;
}
TQString XMLElement::attribute(const TQString &attr) {
    return m_attrmapper[attr];
}

TQString XMLElement::toQString() {
    TQString tag = startTag();
    return tag.left(tag.length() - 1).right(tag.length() - 2);
}

XMLElement XMLElement::fromQString(const TQString &str) {
    TQStringList sections = TQStringList::split(" ", str);
    TQString tagname = sections[0];
    XMLElement e(tagname.latin1());
    
    sections.pop_front();
    // Loop over the remaining strings which are attributes="values"
    if(sections.count()) {
        const int sectionsCount = sections.count();
        for(int i = 0; i < sectionsCount; ++i) {
            TQStringList list = TQStringList::split("=", sections[i]);
            if(list.count() != 2) {
                std::cerr << "XMLElement::fromQString: Cannot convert list: " << list.join("|") << ". `" << str << "' is not in valid format.\n";
                return XMLElement(" ");
            }
            e.setAttribute(list[0], list[1].left(list[1].length() - 1).right(list[1].length() -2));
        }
    }
    return e;
}

