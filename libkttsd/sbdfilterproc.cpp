/***************************************************** vim:set ts=4 sw=4 sts=4:
  Sentence Boundary Detection (SBD) Filter Processing class.
  This is the interface definition for text filters that perform sentence
  boundary detection (SBD).
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  Notice that this class is a filter with some additional methods required for
  SBD.

 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; version 2 of the License.                 *
 *                                                                            *
 ******************************************************************************/

// Ktts includes.
#include "sbdfilterproc.h"

/**
 * Constructor.
 */
SbdFilterProc::SbdFilterProc( QObject *parent, const char *name) :
    KttsFilterProc( parent, name ) 
{
    // kdDebug() << "SbdFilterProc::SbdFilterProc: Running" << endl;
}

/**
 * Destructor.
 */
SbdFilterProc::~SbdFilterProc()
{
    // kdDebug() << "SbdFilterProc::~SdbFilterProc: Running" << endl;
}

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
/*virtual*/ void SbdFilterProc::setSbRegExp( const QString& /*re*/ ) { }

