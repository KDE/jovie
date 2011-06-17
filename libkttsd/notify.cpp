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

// TQt includes.

// TQt includes.
#include <tqstring.h>
#include <tqstringlist.h>

// KDE includes.
#include <kconfig.h>
#include <klocale.h>
#include <kstaticdeleter.h>

// KTTS includes.
#include "notify.h"

static TQStringList* s_actionNames = 0;
static KStaticDeleter<TQStringList> s_actionNames_sd;

static TQStringList* s_actionDisplayNames = 0;
static KStaticDeleter<TQStringList> s_actionDisplayNames_sd;

static void notifyaction_init()
{
    if ( !s_actionNames )
    {
        s_actionNames_sd.setObject(s_actionNames, new TQStringList);
        s_actionNames->append( "SpeakEventName" );
        s_actionNames->append( "SpeakMsg" );
        s_actionNames->append( "DoNotSpeak" );
        s_actionNames->append( "SpeakCustom" );

        s_actionDisplayNames_sd.setObject(s_actionDisplayNames, new TQStringList);
        s_actionDisplayNames->append( i18n("Speak event name") );
        s_actionDisplayNames->append( i18n("Speak the notification message") );
        s_actionDisplayNames->append( i18n("Do not speak the notification") );
        s_actionDisplayNames->append( i18n("Speak custom text:") );
    }
}

/*static*/ int NotifyAction::count()
{
    notifyaction_init();
    return s_actionNames->count();
}

/*static*/ TQString NotifyAction::actionName( const int action )
{
    notifyaction_init();
    return (*s_actionNames)[ action ];
}

/*static*/ int NotifyAction::action( const TQString& actionName )
{
    notifyaction_init();
    return s_actionNames->tqfindIndex( actionName );
}

/*static*/ TQString NotifyAction::actionDisplayName( const int action )
{
    notifyaction_init();
    return (*s_actionDisplayNames)[ action ];
}

/*static*/ TQString NotifyAction::actionDisplayName( const TQString& actionName )
{
    notifyaction_init();
    return (*s_actionDisplayNames)[ action( actionName ) ];
}

// --------------------------------------------------------------------

static TQStringList* s_presentNames = 0;
static KStaticDeleter<TQStringList> s_presentNames_sd;

static TQStringList* s_presentDisplayNames = 0;
static KStaticDeleter<TQStringList> s_presentDisplayNames_sd;

static void notifypresent_init()
{
    if ( !s_presentNames )
    {
        s_presentNames_sd.setObject( s_presentNames, new TQStringList );
        s_presentNames->append( "None" );
        s_presentNames->append( "Dialog" );
        s_presentNames->append( "Passive" );
        s_presentNames->append( "DialogAndPassive" );
        s_presentNames->append( "All" );

        s_presentDisplayNames_sd.setObject( s_presentDisplayNames, new TQStringList );
        s_presentDisplayNames->append( i18n("none") );
        s_presentDisplayNames->append( i18n("notification dialogs") );
        s_presentDisplayNames->append( i18n("passive popups") );
        s_presentDisplayNames->append( i18n("notification dialogs and passive popups") );
        s_presentDisplayNames->append( i18n("all notifications" ) );
    }
}

/*static*/ int NotifyPresent::count()
{
    notifypresent_init();
    return s_presentNames->count();
}

/*static*/ TQString NotifyPresent::presentName( const int present )
{
    notifypresent_init();
    return (*s_presentNames)[ present ];
}

/*static*/ int NotifyPresent::present( const TQString& presentName )
{
    notifypresent_init();
    return s_presentNames->tqfindIndex( presentName );
}

/*static*/ TQString NotifyPresent::presentDisplayName( const int present )
{
    notifypresent_init();
    return (*s_presentDisplayNames)[ present ];
}

/*static*/ TQString NotifyPresent::presentDisplayName( const TQString& presentName )
{
    notifypresent_init();
    return (*s_presentDisplayNames)[ present( presentName ) ];
}

// --------------------------------------------------------------------

/**
 * Retrieves the displayable name for an event source.
 */
/*static*/ TQString NotifyEvent::getEventSrcName(const TQString& eventSrc, TQString& iconName)
{
    TQString configFilename = eventSrc + TQString::tqfromLatin1( "/eventsrc" );
    KConfig* config = new KConfig( configFilename, true, false, "data" );
    config->setGroup( TQString::tqfromLatin1( "!Global!" ) );
    TQString appDesc = config->readEntry( "Comment", i18n("No description available") );
    iconName = config->readEntry( "IconName" );
    delete config;
    return appDesc;
}

/**
 * Retrieves the displayable name for an event from an event source.
 */
/*static*/ TQString NotifyEvent::getEventName(const TQString& eventSrc, const TQString& event)
{
    TQString eventName;
    TQString configFilename = eventSrc + TQString::tqfromLatin1( "/eventsrc" );
    KConfig* config = new KConfig( configFilename, true, false, "data" );
    if ( config->hasGroup( event ) )
    {
        config->setGroup( event );
        eventName = config->readEntry( TQString::tqfromLatin1( "Comment" ),
            config->readEntry( TQString::tqfromLatin1( "Name" )));
    }
    delete config;
    return eventName;
}

