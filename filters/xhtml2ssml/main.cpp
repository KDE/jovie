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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QApplication>
#include <QtCore/QFile>
#include <QtXml/QXmlInputSource>
#include <QtXml/QXmlSimpleReader>
#include <QtCore/QMap>
#include <iostream>
#include "xhtml2ssml.h"
#include "xmlelement.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QFile f("demonstration.html");
    QXmlInputSource input(&f);
    QXmlSimpleReader reader;
    XHTMLToSSMLParser *parser = new XHTMLToSSMLParser();
    reader.setContentHandler(parser);
    reader.parse(input);
    std::cout << parser->convertedText() << "\n";
    delete parser;
    return 0;
}

