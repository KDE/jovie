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

#ifndef _SBDFILTERPROC_H_
#define _SBDTERPROC_H_

// KTTS includes.
#include "filterproc.h"

class SbdFilterProc: public KttsFilterProc
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        SbdFilterProc( QObject *parent, const char *name );

        /**
         * Destructor.
         */
        virtual ~SbdFilterProc();

        /**
         * Set Sentence Boundary Regular Expression.
         * This method will only be called if the application overrode the default.
         *
         * @param re            The sentence delimiter regular expression.
         */
       virtual void setSbRegExp(const QString& re);
};

#endif      // _SBDFILTERPROC_H_
