/***************************************************************************
  Class of utility functions.
  -------------------
  Copyright : (C) 2004 Paul Giannaros
  -------------------
  Original author: Paul Giannaros <ceruleanblaze@gmail.com>
  Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
#include <qstring.h>
#include <kdebug.h>

#include "utils.h"

KttsUtils::KttsUtils() {
}


KttsUtils::~KttsUtils() {
}

/** 
 * Check if an XML document has a certain root element.
 * @param xmldoc                 The document to check for the element.
 * @param elementName      The element to check for in the document.
 * @returns                             true if the root element exists in the document, false otherwise.
*/
bool KttsUtils::hasRootElement(const QString &xmldoc, const QString &elementName) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.simplifyWhiteSpace();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith("<?xml")) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.find("?>");
        if(xmlStatementEnd == -1) {
            kdDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
    }
    // Take off the doctype statement if it exists.
    if(doc.startsWith("<!DOCTYPE") || doc.startsWith(" <!DOCTYPE")) {
        int doctypeStatementEnd = doc.find(">");
        if(doctypeStatementEnd == -1) {
            kdDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        doctypeStatementEnd += 1; // len '>' == 2
        doc = doc.right(doc.length() - doctypeStatementEnd);
    }
    // We should (hopefully) be left with the root element.
    return (doc.startsWith("<" + elementName) || doc.startsWith(" <" + elementName));
}

/** 
 * Check if an XML document has a certain DOCTYPE.
 * @param xmldoc             The document to check for the doctype.
 * @param name                The doctype name to check for. Pass QString::null to not check the name.
 * @param publicId           The public ID to check for. Pass QString::null to not check the ID.
 * @param systemId          The system ID to check for. Pass QString::null to not check the ID.
 * @returns                         true if the parameters match the doctype, false otherwise.
*/
bool KttsUtils::hasDoctype(const QString &xmldoc, const QString &name/*, const QString &publicId, const QString &systemId*/) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.simplifyWhiteSpace();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith("<?xml")) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.find("?>");
        if(xmlStatementEnd == -1) {
            kdDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
    }
        // Take off the doctype statement if it exists.
    return (doc.startsWith("<!DOCTYPE" + name) || doc.startsWith(" <!DOCTYPE" + name));
}
