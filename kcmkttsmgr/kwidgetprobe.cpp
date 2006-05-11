/*
 * kwidgetprobe.cpp  -  description
 * 
 *   Created              : Fri Jan 21 2000
 *   Copyright            : (C) 2000 by Richard Moore
 *   Email                : rich@kde.org
 */

/*
 *                                                                         
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.                                   
 *                                                                         
 */

#include "kwidgetprobe.h"
#include <kapplication.h>
#include <QTimer>
#include <QCursor>
#include <QMenu>
#include <qmenudata.h>
#include <QLabel>
#include <QAbstractButton>
#include "kwidgetprobe.moc"

KWidgetProbe::KWidgetProbe( QObject *parent, const char *name )
 : DCOPStub("kttsd", "KSpeech"), QObject(parent)
{
    prevWidget = 0;
    Q_UNUSED(name);
    timeout = 200;
    timer = new QTimer( this );
    queryFlags = QueryFocusWidget | QueryPointerWidget | QueryWidgetItem;

    connect( timer, SIGNAL(timeout()), this, SLOT(probe()) );
    timer->start( timeout );
}

KWidgetProbe::KWidgetProbe( int millis, uint flags, QObject *parent, const char *name )
  : DCOPStub("kttsd", "KSpeech"), QObject(parent)
{
    Q_UNUSED(name);
    queryFlags = flags;
    timeout = millis;
    prevWidget = 0;

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(probe()) );
    timer->start( timeout );
}

KWidgetProbe::~KWidgetProbe()
{
}

/**
 * Tells the class to do it's stuff - ie. figure out
 * which widget is under the mouse pointer and
 * report it.
 */
void KWidgetProbe::probe()
{
    if ( queryFlags & QueryFocusWidget ) {
        QWidget *focus = kapp->focusWidget();
        if ( focus ) {
            //      warning( "Focus: %s", focus->className() );
        }

        emit focusWidget( focus );
    }

    QPoint pos = QCursor::pos();

    if ( queryFlags & QueryPointerWidget ) {
        QWidget *pointed = kapp->widgetAt( pos, true );
        if ( pointed ) {
            if ( pointed != prevWidget ) {
                prevWidget = pointed;
                //      warning( "Pointer: %s", pointed->className() );
                // FIXME This should be a method
                if ( queryFlags & QueryWidgetItem ) {
                    if ( pointed->inherits("QMenu") ) {
                        QMenu *menuItem = qobject_cast<QMenu *>(pointed);
                        /*  int id = menuItem->idAt( menuItem->mapFromGlobal( pos ) );
                        if ( id != -1 ) {
                            sayScreenReaderOutput(menuItem->text(id), 0);
                            warning( "MenuItem: %s", menuItem->text( id ).data() );
                        }*/
                    }
                    else if ( pointed->inherits("QAbstractButton") ) {
                        QAbstractButton *button = qobject_cast<QAbstractButton *>(pointed);
                        sayScreenReaderOutput(button->text(), 0);
                    }
                    else if ( pointed->inherits("QLabel") ) {
                        QLabel *label = (QLabel *) pointed;
                        sayScreenReaderOutput(label->text(), 0);
                    }
                }
                emit pointerWidget( pointed );
            }
        }
    }
}

void KWidgetProbe::setQuery( uint flags )
{
    queryFlags = flags;
}

uint KWidgetProbe::query()
{
    return queryFlags;
}

void KWidgetProbe::setRefreshInterval( int millis )
{
    timeout = millis;
    timer->start( timeout );
}

int KWidgetProbe::refreshInterval()
{
    return timeout;
}
