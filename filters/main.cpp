
#include <qapplication.h>
#include <qfile.h>
#include <qxml.h>
#include <qmap.h>
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

