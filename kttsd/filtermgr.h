/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jeremy@scitools.com>
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

#ifndef FILTERMGR_H
#define FILTERMGR_H

// Qt includes.
#include <QtCore/QList>

// KTTS includes.
#include "filterproc.h"

class TalkerCode;

typedef QList<KttsFilterProc*> FilterList;

class FilterMgr : public KttsFilterProc
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        FilterMgr(QObject *parent = 0);

        /**
         * Destructor.
         */
        ~FilterMgr();

        /**
         * Initialize the filters.
         * @return                False if filter is not ready to filter.
         *
         * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
         * separate configuration files of their own.
         */
        virtual bool init();

        /**
         * Returns True if this filter is a Sentence Boundary Detector.
         * If so, the filter should implement @ref setSbRegExp() .
         * @return          True if this filter is a SBD.
         */
        //virtual bool isSBD();

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
        //virtual bool supportsAsync();

        /** 
         * Synchronously convert text.
         * @param inputText         Input text.
         * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
         *                          use for synthing the text.  Useful for extracting hints about
         *                          how to filter the text.  For example, languageCode.
         * @param appId             The DBUS appId of the application that queued the text.
         *                          Also useful for hints about how to do the filtering.
         * @return                  Converted text.
         */
        virtual QString convert(const QString& inputText, TalkerCode* talkerCode, const QString& appId);

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
        virtual bool asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QString& appId);

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

        /**
         * Do not call SBD filters.
         */
        //void setNoSBD(bool noSBD);
        //bool noSBD();

        /**
         * True if there is at least one XML Transformer filter for html.
         */
        bool supportsHTML() { return m_supportsHTML; }

    protected:
        bool event ( QEvent * e );

    private slots:
        void slotFilteringFinished();

    private:
        // Loads the processing plug in for a named filter plug in.
        KttsFilterProc* loadFilterPlugin(const QString& plugInName);
        // Finishes up with current filter (if any) and goes on to the next filter.
        void nextFilter();
        // Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
        // @param name                   The translated plugin name.  From Name= line in .desktop file.
        // @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
        //                               QString() if not found.
        QString FilterNameToDesktopEntryName(const QString& name);

        // List of filters.
        FilterList m_filterList;
        // Text being filtered.
        QString m_text;
        // Index to list of filters.
        int m_filterIndex;
        // Current filter.
        KttsFilterProc* m_filterProc;
        // True if calling filters asynchronously.
        //bool m_async;
        // Talker Code.
        TalkerCode* m_talkerCode;
        // AppId.
        QString m_appId;
        // Sentence Boundary regular expression (if app overrode the default).
        QString m_re;
        // True if any of the filters modified the text.
        bool m_wasModified;
        // FilterMgr state.
        int m_state;
        // True if SBD Filters should not be called.
        //bool m_noSBD;
        // True if at least one XML Transformer for html is enabled.
        bool m_supportsHTML;
};

#endif      // FILTERMGR_H
