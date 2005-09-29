/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Processing class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef _XMLTRANSFORMERPROC_H_
#define _XMLTRANSFORMERPROC_H_

// Qt includes.
#include <qobject.h>
#include <qstringlist.h>

// KTTS includes.
#include "filterproc.h"

class KProcess;

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
     * Returns True if the plugin supports asynchronous processing,
     * i.e., supports asyncConvert method.
     * @return                        True if this plugin supports asynchronous processing.
     *
     * If the plugin returns True, it must also implement @ref getState .
     * It must also emit @ref filteringFinished when filtering is completed.
     * If the plugin returns True, it must also implement @ref stopFiltering .
     * It must also emit @ref filteringStopped when filtering has been stopped.
     */
    virtual bool supportsAsync();

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
     * Convert input.  Runs asynchronously.
     * @param inputText         Input text.
     * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
     *                          use for synthing the text.  Useful for extracting hints about
     *                          how to filter the text.  For example, languageCode.
     * @param appId             The DCOP appId of the application that queued the text.
     *                          Also useful for hints about how to do the filtering.
     * @return                  False if the filter cannot perform the conversion.
     *
     * When conversion is completed, emits signal @ref filteringFinished.  Calling
     * program may then call @ref getOutput to retrieve converted text.  Calling
     * program must call @ref ackFinished to acknowledge the conversion.
     */
    virtual bool asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId);

    /**
     * Waits for a previous call to asyncConvert to finish.
     */
    virtual void waitForFinished();

    /**
     * Returns the state of the Filter.
     */
    virtual int getState();

    /**
     * Returns the filtered output.
     */
    virtual QString getOutput();

    /**
     * Acknowledges the finished filtering.
     */
    virtual void ackFinished();

    /**
     * Stops filtering.  The filteringStopped signal will emit when filtering
     * has in fact stopped and state returns to fsIdle;
     */
    virtual void stopFiltering();

    /**
     * Did this filter do anything?  If the filter returns the input as output
     * unmolested, it should return False when this method is called.
     */
    virtual bool wasModified();

private slots:
    void slotProcessExited(KProcess*);
    void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
    void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);

private:
    // Process output when xsltproc exits.
    void processOutput();

    // If not empty, only apply to text queued by an applications containing one of these strings.
    QStringList m_appIdList;
    // If not empty, only apply to XML that has the specified root element.
    QStringList m_rootElementList;
    // If not empty, only apply to XML that has the specified DOCTYPE spec.
    QStringList m_doctypeList;
    // The text that is being filtered.
    QString m_text;
    // Processing state.
    int m_state;
    // xsltproc process.
    KProcess* m_xsltProc;
    // Input and Output filenames.
    QString m_inFilename;
    QString m_outFilename;
    // User's name for the filter.
    QString m_UserFilterName;
    // XSLT file.
    QString m_xsltFilePath;
    // Path to xsltproc processor.
    QString m_xsltprocPath;
    // Did this filter modify the text?
    bool m_wasModified;
};

#endif      // _XMLTRANSFORMERPROC_H_
