/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to select a KNotify application and event.
  -------------------
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

#ifndef _SELECTEVENT_H_
#define _SELECTEVENT_H_

#include "selecteventwidget.h"

class SelectEvent : public SelectEventWidget
{
    Q_OBJECT

public:
    /**
    * Constructor.
    * @param parent             Inherited KDialog parameter.
    * @param name               Inherited KDialog parameter.
    * @param initEventSrc       Event source to start with.
    */
    SelectEvent(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0,
        const QString& initEventSrc = QString() );

    /**
    * Destructor.
    */
    ~SelectEvent();

    /**
     * Returns the chosen event source (app name).
     */
    QString getEventSrc();

    /**
     * Returns the chosen event.
     */
    QString getEvent();

private slots:
    void slotEventSrcComboBox_activated(int index);

private:
    // returns e.g. "kwin/eventsrc" from a given path
    // "/opt/kde3/share/apps/kwin/eventsrc"
    QString makeRelative( const QString& fullPath );

    QStringList m_eventSrcNames;
};

#endif              // _SELECTEVENT_H_
