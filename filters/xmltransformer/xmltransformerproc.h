/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Processing class.
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

#ifndef _XMLTRANSFORMERPROC_H_
#define _XMLTRANSFORMERPROC_H_

// Qt includes.
#include <qobject.h>

// KTTS includes.
#include "filterproc.h"

class XmlTransformerProc : virtual public KttsFilterProc
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    XmlTransformerProc( QObject *parent, const char *name, const QStringList &args = QStringList() );

    /**
     * Destructor.
     */
    virtual ~XmlTransformerProc();

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
     */
    virtual QString convert(QString& inputText, TalkerCode* talkerCode);

private:

    // User's name for the filter.
    QString m_UserFilterName;
    // XSLT file.
    QString m_xsltFilePath;
    // Path to xsltproc processor.
    QString m_xsltprocPath;
};

#endif      // _XMLTRANSFORMERPROC_H_
