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

// Qt includes.

// Qt includes.
#include <QString>
#include <QStringList>

// KDE includes.
#include <kconfig.h>
#include <klocale.h>
#include <kstaticdeleter.h>

// KTTS includes.
#include "notify.h"

static QStringList* s_actionNames = 0;
static KStaticDeleter<QStringList> s_actionNames_sd;

static QStringList* s_actionDisplayNames = 0;
static KStaticDeleter<QStringList> s_actionDisplayNames_sd;

static void notifyaction_init()
{
    if ( !s_actionNames )
    {
        s_actionNames_sd.setObject(s_actionNames, new QStringList);
        s_actionNames->append( "SpeakEventName" );
        s_actionNames->append( "SpeakMsg" );
        s_actionNames->append( "DoNotSpeak" );
        s_actionNames->append( "SpeakCustom" );

        s_actionDisplayNames_sd.setObject(s_actionDisplayNames, new QStringList);
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

/*static*/ QString NotifyAction::actionName( const int action )
{
    notifyaction_init();
    return (*s_actionNames)[ action ];
}

/*static*/ int NotifyAction::action( const QString& actionName )
{
    notifyaction_init();
    return s_actionNames->indexOf( actionName );
}

/*static*/ QString NotifyAction::actionDisplayName( const int action )
{
    notifyaction_init();
    return (*s_actionDisplayNames)[ action ];
}

/*static*/ QString NotifyAction::actionDisplayName( const QString& actionName )
{
    notifyaction_init();
    return (*s_actionDisplayNames)[ action( actionName ) ];
}

// --------------------------------------------------------------------

static QStringList* s_presentNames = 0;
static KStaticDeleter<QStringList> s_presentNames_sd;

static QStringList* s_presentDisplayNames = 0;
static KStaticDeleter<QStringList> s_presentDisplayNames_sd;

static void notifypresent_init()
{
    if ( !s_presentNames )
    {
        s_presentNames_sd.setObject( s_presentNames, new QStringList );
        s_presentNames->append( "None" );
        s_presentNames->append( "Dialog" );
        s_presentNames->append( "Passive" );
        s_presentNames->append( "DialogAndPassive" );
        s_presentNames->append( "All" );

        s_presentDisplayNames_sd.setObject( s_presentDisplayNames, new QStringList );
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

/*static*/ QString NotifyPresent::presentName( const int present )
{
    notifypresent_init();
    return (*s_presentNames)[ present ];
}

/*static*/ int NotifyPresent::present( const QString& presentName )
{
    notifypresent_init();
    return s_presentNames->indexOf( presentName );
}

/*static*/ QString NotifyPresent::presentDisplayName( const int present )
{
    notifypresent_init();
    return (*s_presentDisplayNames)[ present ];
}

/*static*/ QString NotifyPresent::presentDisplayName( const QString& presentName )
{
    notifypresent_init();
    return (*s_presentDisplayNames)[ present( presentName ) ];
}

// --------------------------------------------------------------------

/**
 * Retrieves the displayable name for an event source.
 */
/*static*/ QString NotifyEvent::getEventSrcName(const QString& eventSrc, QString& iconName)
{
    QString configFilename = eventSrc + QString::fromLatin1( "/eventsrc" );
    KConfig config( "data", configFilename, KConfig::NoGlobals );
    KConfigGroup group( &config, QString::fromLatin1( "!Global!" ) );
    QString appDesc = group.readEntry( "Comment", i18n("No description available") );
    iconName = group.readEntry( "IconName" );
    return appDesc;
}

/**
 * Retrieves the displayable name for an event from an event source.
 */
/*static*/ QString NotifyEvent::getEventName(const QString& eventSrc, const QString& event)
{
    QString eventName;
    QString configFilename = eventSrc + QString::fromLatin1( "/eventsrc" );
    KConfig config( "data", configFilename, KConfig::NoGlobals );
    if ( config.hasGroup( event ) )
    {
        KConfigGroup group( &config,  event );
        eventName = group.readEntry( QString::fromLatin1( "Comment" ),
                    group.readEntry( QString::fromLatin1( "Name" ),QString()));
    }
    return eventName;
}

