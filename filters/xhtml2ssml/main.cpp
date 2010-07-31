
#include <tqapplication.h>
#include <tqfile.h>
#include <tqxml.h>
#include <tqmap.h>
#include <iostream>
#include "xhtml2ssml.h"
#include "xmlelement.h"

int main(int argc, char *argv[]) {
    TQApplication a(argc, argv);
    TQFile f("demonstration.html");
    TQXmlInputSource input(&f);
    TQXmlSimpleReader reader;
    XHTMLToSSMLParser *parser = new XHTMLToSSMLParser();
    reader.setContentHandler(parser);
    reader.parse(input);
    std::cout << parser->convertedText() << "\n";
    delete parser;
    return 0;
}

