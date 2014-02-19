/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
    Filters text, applying each configured Filter in turn.
    Runs asynchronously, emitting Finished() signal when all Filters have run.

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
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

/**
 * @class FilterMgr
 *
 * Manager for filter objects. Loads and configures filters that have been
 * set up by the user as per the config file. Also filters text to bee spoken
 * by running it through all the configured filters.
 */
class FilterMgr : public KttsFilterProc
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        explicit FilterMgr(QObject *parent = 0);

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
        // Talker Code.
        TalkerCode* m_talkerCode;
        // AppId.
        QString m_appId;
        // FilterMgr state.
        int m_state;
};

#endif      // FILTERMGR_H
