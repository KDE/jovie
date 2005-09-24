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
#include <qstringlist.h>
#include <iostream>

/// Constructors
XMLElement::XMLElement() {
    m_name = "";
    m_attrmapper = AttributeToValueMap();
}
XMLElement::XMLElement(const QString &name) {
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

QString XMLElement::name() {
    return m_name;
}
QString XMLElement::startTag() {
    QString output = "<" + m_name + " ";
    for(AttributeToValueMap::Iterator it = m_attrmapper.begin(); it != m_attrmapper.end(); ++it) {
        output.append(it.key() + "=\"" + it.data() + "\" ");
    }
    output = output.left(output.length() - 1);
    // Get rid of the space at the end and then append a '>'
    output.append(">");
    return output;
}

QString XMLElement::endTag() {
    return "</" + m_name + ">";
}

void XMLElement::setAttribute(const QString &attr, const QString &value) {
    m_attrmapper[attr] = value;
}
QString XMLElement::attribute(const QString &attr) {
    return m_attrmapper[attr];
}

QString XMLElement::toQString() {
    QString tag = startTag();
    return tag.left(tag.length() - 1).right(tag.length() - 2);
}

XMLElement XMLElement::fromQString(const QString &str) {
    QStringList sections = str.split( " ");
    QString tagname = sections[0];
    XMLElement e(tagname.latin1());
    
    sections.pop_front();
    // Loop over the remaining strings which are attributes="values"
    if(sections.count()) {
        const int sectionsCount = sections.count();
        for(int i = 0; i < sectionsCount; ++i) {
            QStringList list = sections[i].split( "=");
            if(list.count() != 2) {
                std::cerr << "XMLElement::fromQString: Cannot convert list: " << list.join("|") << ". `" << str << "' is not in valid format.\n";
                return XMLElement(" ");
            }
            e.setAttribute(list[0], list[1].left(list[1].length() - 1).right(list[1].length() -2));
        }
    }
    return e;
}

