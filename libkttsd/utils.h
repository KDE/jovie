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
 
#ifndef UTILS_H
#define UTILS_H

#include <kdemacros.h>
#include "kdeexportfix.h"

class QString;
class QComboBox;

class KDE_EXPORT KttsUtils {

public:
    /// Constructor
    KttsUtils();
    /// Destructor
    ~KttsUtils();

    /** 
     * Check if an XML document has a certain root element.
     * @param xmldoc                 The document to check for the element.
     * @param elementName      The element to check for in the document.
     * @returns                             true if the root element exists in the document, false otherwise.
     */
    static bool hasRootElement(const TQString &xmldoc, const TQString &elementName);

    /** 
     * Check if an XML document has a certain DOCTYPE.
     * @param xmldoc             The document to check for the doctype.
     * @param name                The doctype name to check for. Pass TQString::null to not check the name.
     * @param publicId           The public ID to check for. Pass TQString::null to not check the ID.
     * @param systemId          The system ID to check for. Pass TQString::null to not check the ID.
     * @returns                         true if the parameters match the doctype, false otherwise.
     */
    static bool hasDoctype(const TQString &xmldoc, const TQString &name/*, const TQString &publicId, const TQString &systemId*/);

    /**
     * Sets the current item in the given combobox to the item with the given text.
     * If item with the text not found, does nothing.
     */
    static void setCbItemFromText(TQComboBox* cb, const TQString& text);

};

#endif
