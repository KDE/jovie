/*
 * kwidgetprobe.h  -  description
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

#ifndef KWIDGETPROBE_H
#define KWIDGETPROBE_H

#include <qwidget.h>
#include <qobject.h>

#include "kspeech_stub.h"

class QTimer;

/**
  * @author Richard Moore
  */
class KWidgetProbe : public QObject, public KSpeech_stub
{
   Q_OBJECT
public: 
	KWidgetProbe(QObject *parent=0, const char *name=0);
	KWidgetProbe( int millis, uint flags, QObject *parent=0, const char *name=0);
	~KWidgetProbe();

	enum QueryFlags {
		QueryFocusWidget = 1,
		QueryPointerWidget = 2,
		QueryWhatsThis = 4,
		QueryTooltip = 8,
		QueryWidgetItem = 16
	};
	
	void setQuery( uint flags );
	uint query();
	
	void setRefreshInterval( int millis );
	int refreshInterval();
	
public slots: // Public slots
	/**
	 * Tells the class to do it's stuff - ie. figure out
	 * which widget is under the mouse pointer and
	 *report it.
	 */
	void probe();

signals:
	void focusWidget( QWidget * );	
	void pointerWidget( QWidget * );

private:
	uint queryFlags;
	int timeout;
	QTimer *timer;
};

#endif
