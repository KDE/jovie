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

// Qt includes.
#ifndef _FILTERPROC_H_
#define _FILTERPROC_H_

#include <qobject.h>
#include <qstringlist.h>

class TalkerCode;

class KttsFilterProc : virtual public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    KttsFilterProc( QObject *parent, const char *name, const QStringList &args = QStringList() );

    /**
     * Destructor.
     */
    virtual ~KttsFilterProc();

    /**
     * Convert input, returning output.
     * @param inputText         Input text.
     * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
     *                          use for synthing the text.  Useful for extracting hints about
     *                          how to filter the text.  For example, languageCode.
     */
    virtual QString convert(QString& inputText, TalkerCode* talkerCode);

signals:
    /**
     * If an error occurs, Filter should signal the error and return input as output in
     * convert method.  If Filter should not be called in the future, perhaps because
     * it could not find its configuration file, return False for keepGoing.
     * @param keepGoing         False if the filter should not be called in the future.
     * @param msg               Error message.
     */
    void error(const bool keepGoing, const QString &msg);
};

#endif      // _FILTERPROC_H_
