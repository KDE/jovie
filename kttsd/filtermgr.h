/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; either version 2 of the License.          *
 *                                                                            *
 ******************************************************************************/

#ifndef _FILTERMGR_H_
#define _FILTERMGR_H_

// Qt includes.
#include <qptrlist.h>

// KTTS includes.
#include "filterproc.h"

class KConfig;
class TalkerCode;

typedef QPtrList<KttsFilterProc> FilterList;

class FilterMgr : public KttsFilterProc
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        FilterMgr(QObject *parent = 0, const char *name = 0);

        /**
         * Destructor.
         */
        ~FilterMgr();

        /**
         * Initialize the filters.
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
         * Synchronously convert text.
         * @param inputText         Input text.
         * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
         *                          use for synthing the text.  Useful for extracting hints about
         *                          how to filter the text.  For example, languageCode.
         * @param appId             The DCOP appId of the application that queued the text.
         *                          Also useful for hints about how to do the filtering.
         * @return                  Converted text.
         */
        virtual QString convert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId);

        /**
         * Asynchronously convert input.
         * @param inputText         Input text.
         * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
         *                          use for synthing the text.  Useful for extracting hints about
         *                          how to filter the text.  For example, languageCode.
         * @param appId             The DCOP appId of the application that queued the text.
         *                          Also useful for hints about how to do the filtering.
         *
         * When the input text has been converted, filteringFinished signal will be emitted
         * and caller can retrieve using getOutput();
         */
        virtual bool asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QCString& appId);

        /**
         * Waits for filtering to finish.
         */
        virtual void waitForFinished();

        /**
         * Returns the state of the FilterMgr.
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
         * has in fact stopped.
         */
        virtual void stopFiltering();

        /**
         * Set Sentence Boundary Regular Expression.
         * This method will only be called if the application overrode the default.
         *
         * @param re            The sentence delimiter regular expression.
         */
        virtual void setSbRegExp(const QString& re);

    signals:

    private slots:
        void slotFilterStopped();
        void slotFilterFinished();

    private:
        // Loads the processing plug in for a named filter plug in.
        KttsFilterProc* loadFilterPlugin(const QString& plugInName);
        // Invokes the next filter.
        void nextFilter();

        // FilterMgr state.
        int m_state;
        // List of filters.
        FilterList m_filterList;
        // Index into filterlist for current filter.
        int m_filterIndex;
        // True if waiting for completion.
        bool m_waitingForFinish;
        // Text being filtered.
        QString m_text;
        // Talker Code.
        TalkerCode* m_talkerCode;
        // AppId.
        QCString m_appId;
        // Sentence Boundary regular expression (if app overrode the default).
        QString m_re;
};

#endif      // _FILTERMGR_H_
