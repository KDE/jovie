/***************************************************** vim:set ts=4 sw=4 sts=4:
  Notification Action constants and utility functions.
  -------------------
  Copyright : (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: 2004 by Gary Cramblitt <garycramblitt@comcast.net>

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

#ifndef _NOTIFYACTION_H
#define _NOTIFYACTION_H

#include <kdemacros.h>

class QString;

class KDE_EXPORT NotifyAction
{

public:

    enum NotifyActions
    {
        SpeakEventName,
        SpeakMsg,
        DoNotSpeak,
        SpeakCustom
    };

    static int count();
    static QString actionName( const int action );
    static int action( const QString& actionName );
    static QString actionDisplayName( const int action );
    static QString actionDisplayName( const QString& actionName );
};

// --------------------------------------------------------------------

class KDE_EXPORT NotifyPresent
{

public:

    enum NotifyPresentations
    {
        None,
        Dialog,
        Passive,
        DialogAndPassive,
        All
    };

    static int count();
    static QString presentName( const int present );
    static int present( const QString& presentName );
    static QString presentDisplayName( const int present );
    static QString presentDisplayName( const QString& presentName );
};

// --------------------------------------------------------------------

class KDE_EXPORT NotifyEvent
{

public:
    /**
     * Retrieves the displayable name for an event source.
     */
    static QString getEventSrcName(const QString& eventSrc, QString& iconName);

    /**
     * Retrieves the displayable name for an event from an event source.
     */
    static QString getEventName(const QString& eventSrc, const QString& event);
};

#endif              // _NOTIFYACTION_H
