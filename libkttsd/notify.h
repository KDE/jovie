/***************************************************** vim:set ts=4 sw=4 sts=4:
  Notification Action constants and utility functions.
  -------------------
  Copyright : (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: 2004 by Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

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
    static TQString actionName( const int action );
    static int action( const TQString& actionName );
    static TQString actionDisplayName( const int action );
    static TQString actionDisplayName( const TQString& actionName );
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
    static TQString presentName( const int present );
    static int present( const TQString& presentName );
    static TQString presentDisplayName( const int present );
    static TQString presentDisplayName( const TQString& presentName );
};

// --------------------------------------------------------------------

class KDE_EXPORT NotifyEvent
{

public:
    /**
     * Retrieves the displayable name for an event source.
     */
    static TQString getEventSrcName(const TQString& eventSrc, TQString& iconName);

    /**
     * Retrieves the displayable name for an event from an event source.
     */
    static TQString getEventName(const TQString& eventSrc, const TQString& event);
};

#endif              // _NOTIFYACTION_H
