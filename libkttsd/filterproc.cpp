/***************************************************** vim:set ts=4 sw=4 sts=4:
  Filter Processing class.
  This is the interface definition for text filters.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; version 2 of the License.                 *
 *                                                                            *
 ******************************************************************************/

// FilterProc includes.
#include "filterproc.h"

/**
 * Constructor.
 */
KttsFilterProc::KttsFilterProc( QObject *parent, const char *name, const QStringList& /*args*/) :
        QObject(parent, name) { }

/**
 * Destructor.
 */
KttsFilterProc::~KttsFilterProc() { }

/*virtual*/ QString KttsFilterProc::convert(QString& inputText, TalkerCode* /*talkerCode*/)
{
    return inputText;
}
