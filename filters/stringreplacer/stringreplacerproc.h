/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Processing class.
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

#ifndef _STRINGREPLACERPROC_H_
#define _STRINGREPLACERPROC_H_

// Qt includes.
#include <qobject.h>
#include <qtextstream.h>
#include <qvaluelist.h>
#include <qregexp.h>
#include <qstringlist.h>

// KTTS includes.
#include "filterproc.h"

class StringReplacerProc : virtual public KttsFilterProc
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    StringReplacerProc( QObject *parent, const char *name, const QStringList &args = QStringList() );

    /**
     * Destructor.
     */
    virtual ~StringReplacerProc();

    /**
     * Initialize the filter.
     * @param config          Settings object.
     * @param configGroup     Settings Group.
     * @return                False if filter is not ready to filter.
     *
     * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
     * separate configuration files of their own.
     */
    virtual bool init(KConfig *config, const QString &configGroup);

    /**
     * Convert input, returning output.
     * @param inputText         Input text.
     * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
     *                          use for synthing the text.  Useful for extracting hints about
     *                          how to filter the text.  For example, languageCode.
     * @param appId             The DCOP appId of the application that queued the text.
     *                          Also useful for hints about how to do the filtering.
     */
    virtual QString convert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId);

    /**
     * Did this filter do anything?  If the filter returns the input as output
     * unmolested, it should return False when this method is called.
     */
    virtual bool wasModified();

private:
    // Language codes supported by the filter.
    QStringList m_languageCodeList;
    // If not empty, apply filter only to apps containing one or more of these strings.
    QStringList m_appIdList;

    // List of regular expressions to match.
    QValueList<QRegExp> m_matchList;
    // List of substitutions to replace matches.
    QValueList<QString> m_substList;
    // True if this filter did anything to the text.
    bool m_wasModified;
};

#endif      // _STRINGREPLACERPROC_H_
