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

#ifndef _FILTERPROC_H_
#define _FILTERPROC_H_

// Qt includes.
#include <qobject.h>
#include <qstringlist.h>

class TalkerCode;
class KConfig;

class KttsFilterProc : virtual public QObject
{
    Q_OBJECT

public:
    enum FilterState {
        fsIdle = 0,              // Not doing anything.  Ready to filter.
        fsFiltering = 1,         // Filtering.
        fsStopping = 2,          // Stop of filtering is in progress.
        fsFinished = 3           // Filtering finished.
    };

    /**
     * Constructor.
     */
    KttsFilterProc( QObject *parent, const char *name );

    /**
     * Destructor.
     */
    virtual ~KttsFilterProc();

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
     * Returns True if this filter is a Sentence Boundary Detector.
     * If so, the filter should implement @ref setSbRegExp() .
     * @return          True if this filter is a SBD.
     */
    virtual bool isSBD();

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
     * Convert input, returning output.  Runs synchronously.
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

    /**
     * Set Sentence Boundary Regular Expression.
     * This method will only be called if the application overrode the default.
     *
     * @param re            The sentence delimiter regular expression.
     */
    virtual void setSbRegExp(const QString& re);

signals:
    /**
     * Emitted when asynchronous filtering has completed.
     */
    void filteringFinished();

    /**
     * Emitted when stopFiltering has been called and filtering has in fact stopped.
     */
    void filteringStopped();

    /**
     * If an error occurs, Filter should signal the error and return input as output in
     * convert method.  If Filter should not be called in the future, perhaps because
     * it could not find its configuration file, return False for keepGoing.
     * @param keepGoing         False if the filter should not be called in the future.
     * @param msg               Error message.
     */
    void error(bool keepGoing, const QString &msg);
};

#endif      // _FILTERPROC_H_
