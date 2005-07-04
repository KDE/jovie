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
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <qcombobox.h>

// KDE includes
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <klistview.h>
#include <kiconloader.h>

// KTTS includes
#include "utils.h"
#include "selectevent.h"

SelectEvent::SelectEvent(QWidget* parent, const char* name, WFlags fl, const QString& initEventSrc)
    : SelectEventWidget(parent,name,fl)
{
    // Load list of event sources (applications).
    QStringList fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/eventsrc", false, true );
    QStringList::ConstIterator it = fullpaths.begin();
    QStringList relativePaths;
    for ( ; it != fullpaths.end(); ++it)
    {
        QString relativePath = *it;
        if ( relativePath.at(0) == '/' && KStandardDirs::exists( relativePath ) )
        {
            relativePath = makeRelative( relativePath );
            relativePaths.append(relativePath);
        }
    }
    relativePaths.sort();
    it = relativePaths.begin();
    for ( ; it != relativePaths.end(); ++it)
    {
        QString relativePath = *it;
        if ( !relativePath.isEmpty() )
        {
            KConfig* config = new KConfig(relativePath, true, false, "data");
            config->setGroup( QString::fromLatin1("!Global!") );
            QString icon = config->readEntry(QString::fromLatin1("IconName"),
                QString::fromLatin1("misc"));
            QString description = config->readEntry( QString::fromLatin1("Comment"),
                i18n("No description available") );
            delete config;
            int index = relativePath.find( '/' );
            QString appname;
            if ( index >= 0 )
                appname = relativePath.left( index );
            else
                kdDebug() << "Cannot determine application name from path: " << relativePath << endl;
            eventSrcComboBox->insertItem( SmallIcon( icon ), description );
            m_eventSrcNames.append( appname );
            if ( appname == initEventSrc ) KttsUtils::setCbItemFromText(eventSrcComboBox, description);
        }
    }
    slotEventSrcComboBox_activated(eventSrcComboBox->currentItem());
    connect (eventSrcComboBox, SIGNAL(activated(int)), this, SLOT(slotEventSrcComboBox_activated(int)));
}

SelectEvent::~SelectEvent() { }

void SelectEvent::slotEventSrcComboBox_activated(int index)
{
    eventsListView->clear();
    QListViewItem* item = 0;
    QString eventSrc = m_eventSrcNames[index];
    QString configFilename = eventSrc + QString::fromLatin1( "/eventsrc" );
    KConfig* config = new KConfig( configFilename, true, false, "data" );
    QStringList eventNames = config->groupList();
    uint eventNamesCount = eventNames.count();
    for (uint ndx = 0; ndx < eventNamesCount; ++ndx)
    {
        QString eventName = eventNames[ndx];
        if ( eventName != "!Global!" )
        {
            config->setGroup( eventName );
            QString eventDesc = config->readEntry( QString::fromLatin1( "Comment" ),
                config->readEntry( QString::fromLatin1( "Name" )));
            if ( !item )
                item = new KListViewItem( eventsListView, eventDesc, eventName );
            else
                item = new KListViewItem( eventsListView, item, eventDesc, eventName );
        }
    }
    delete config;
    eventsListView->sort();
    item = eventsListView->lastChild();
    QString eventDesc = i18n("All other %1 events").arg(eventSrcComboBox->currentText());
    if ( !item )
        item = new KListViewItem( eventsListView, eventDesc, "default" );
    else
        item = new KListViewItem( eventsListView, item, eventDesc, "default" );

}

QString SelectEvent::getEventSrc()
{
    return m_eventSrcNames[eventSrcComboBox->currentItem()];
}

QString SelectEvent::getEvent()
{
    QListViewItem* item = eventsListView->currentItem();
    if ( item )
        return item->text(1);
    else
        return QString::null;
}

// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde3/share/apps/kwin/eventsrc"
QString SelectEvent::makeRelative( const QString& fullPath )
{
    int slash = fullPath.findRev( '/' ) - 1;
    slash = fullPath.findRev( '/', slash );

    if ( slash < 0 )
        return QString::null;

    return fullPath.mid( slash+1 );
}


#include "selectevent.moc"
