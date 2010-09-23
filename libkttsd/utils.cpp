/***************************************************************************
  Class of utility functions.
  -------------------
  Copyright : (C) 2004 Paul Giannaros <ceruleanblaze@gmail.com>
  -------------------
  Original author: Paul Giannaros <ceruleanblaze@gmail.com>
  Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License or             *
 *   (at your option) version 3.                                           *
 *                                                                         *
 ***************************************************************************/

#include "utils.h"

#include <kdebug.h>
#include <kcombobox.h>

KttsUtils::KttsUtils() {
}


KttsUtils::~KttsUtils() {
}

/**
 * Check if an XML document has a certain root element.
 * @param xmldoc             The document to check for the element.
 * @param elementName        The element to check for in the document.
 * @returns                  True if the root element exists in the document, false otherwise.
*/
bool KttsUtils::hasRootElement(const QString &xmldoc, const QString &elementName) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.simplified();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith(QLatin1String("<?xml"))) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.indexOf(QLatin1String( "?>" ));
        if(xmlStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
    }
    // Take off leading comments, if they exist.
    while(doc.startsWith(QLatin1String("<!--")) || doc.startsWith(QLatin1String(" <!--"))) {
        int commentStatementEnd = doc.indexOf(QLatin1String( "-->" ));
        if(commentStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        commentStatementEnd += 3; // len '>' == 2
        doc = doc.right(doc.length() - commentStatementEnd);
    }
    // Take off the doctype statement if it exists.
    while(doc.startsWith(QLatin1String("<!DOCTYPE")) || doc.startsWith(QLatin1String(" <!DOCTYPE"))) {
        int doctypeStatementEnd = doc.indexOf(QLatin1Char( '>' ));
        if(doctypeStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        doctypeStatementEnd += 1; // len '>' == 2
        doc = doc.right(doc.length() - doctypeStatementEnd);
    }
    // We should (hopefully) be left with the root element.
    return (doc.startsWith(QString(QLatin1Char( '<' ) + elementName)) || doc.startsWith(QString(QLatin1String( " <" ) + elementName)));
}

bool KttsUtils::hasDoctype(const QString &xmldoc, const QString &name) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.trimmed();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith(QLatin1String("<?xml"))) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.indexOf(QLatin1String( "?>" ));
        if(xmlStatementEnd == -1) {
            kDebug() << "KttsUtils::hasDoctype: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
        doc = doc.trimmed();
    }
    // Take off leading comments, if they exist.
    while(doc.startsWith(QLatin1String("<!--"))) {
        int commentStatementEnd = doc.indexOf(QLatin1String( "-->" ));
        if(commentStatementEnd == -1) {
            kDebug() << "KttsUtils::hasDoctype: Bad XML file syntax\n";
            return false;
        }
        commentStatementEnd += 3; // len '>' == 2
        doc = doc.right(doc.length() - commentStatementEnd);
        doc = doc.trimmed();
    }
    // Match the doctype statement if it exists.
    // kDebug() << "KttsUtils::hasDoctype: searching " << doc.left(20) << "... for " << "<!DOCTYPE " << name;
    return (doc.startsWith(QString(QLatin1String( "<!DOCTYPE " ) + name)));
}

/**
 * Sets the current item in the given combobox to the item with the given text.
 * If item with the text not found, does nothing.
 */
/*static*/ void KttsUtils::setCbItemFromText(KComboBox* cb, const QString& text)
{
    const int itemCount = cb->count();
    for (int ndx = 0; ndx < itemCount; ++ndx)
    {
        if (cb->itemText(ndx) == text)
        {
            cb->setCurrentIndex(ndx);
            return;
        }
    }
}

