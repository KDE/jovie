/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

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

#ifndef _FILTERMGR_H_
#define _FILTERMGR_H_

// TQt includes.
#include <tqptrlist.h>

// KTTS includes.
#include "filterproc.h"

class KConfig;
class TalkerCode;

typedef TQPtrList<KttsFilterProc> FilterList;

class FilterMgr : public KttsFilterProc
{
    Q_OBJECT
  TQ_OBJECT

    public:
        /**
         * Constructor.
         */
        FilterMgr(TQObject *tqparent = 0, const char *name = 0);

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
        virtual bool init(KConfig *config, const TQString &configGroup);

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
        virtual TQString convert(const TQString& inputText, TalkerCode* talkerCode, const TQCString& appId);

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
        virtual bool asyncConvert(const TQString& inputText, TalkerCode* talkerCode, const TQCString& appId);

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
        virtual TQString getOutput();

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
        virtual void setSbRegExp(const TQString& re);

        /**
         * Do not call SBD filters.
         */
        void setNoSBD(bool noSBD);
        bool noSBD();

        /**
         * True if there is at least one XML Transformer filter for html.
         */
        bool supportsHTML() { return m_supportsHTML; }

    protected:
        bool event ( TQEvent * e );

    private slots:
        void slotFilteringFinished();

    private:
        // Loads the processing plug in for a named filter plug in.
        KttsFilterProc* loadFilterPlugin(const TQString& plugInName);
        // Finishes up with current filter (if any) and goes on to the next filter.
        void nextFilter();
        // Uses KTrader to convert a translated Filter Plugin Name to DesktopEntryName.
        // @param name                   The translated plugin name.  From Name= line in .desktop file.
        // @return                       DesktopEntryName.  The name of the .desktop file (less .desktop).
        //                               TQString() if not found.
        TQString FilterNameToDesktopEntryName(const TQString& name);

        // List of filters.
        FilterList m_filterList;
        // Text being filtered.
        TQString m_text;
        // Index to list of filters.
        int m_filterIndex;
        // Current filter.
        KttsFilterProc* m_filterProc;
        // True if calling filters asynchronously.
        bool m_async;
        // Talker Code.
        TalkerCode* m_talkerCode;
        // AppId.
        TQCString m_appId;
        // Sentence Boundary regular expression (if app overrode the default).
        TQString m_re;
        // True if any of the filters modified the text.
        bool m_wasModified;
        // FilterMgr state.
        int m_state;
        // True if SBD Filters should not be called.
        bool m_noSBD;
        // True if at least one XML Transformer for html is enabled.
        bool m_supportsHTML;
};

#endif      // _FILTERMGR_H_
