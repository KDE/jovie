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

// Qt includes.
#include <tqcombobox.h>

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

SelectEvent::SelectEvent(TQWidget* parent, const char* name, WFlags fl, const TQString& initEventSrc)
    : SelectEventWidget(parent,name,fl)
{
    // Load list of event sources (applications).
    TQStringList fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/eventsrc", false, true );
    TQStringList::ConstIterator it = fullpaths.begin();
    TQStringList relativePaths;
    for ( ; it != fullpaths.end(); ++it)
    {
        TQString relativePath = *it;
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
        TQString relativePath = *it;
        if ( !relativePath.isEmpty() )
        {
            KConfig* config = new KConfig(relativePath, true, false, "data");
            config->setGroup( TQString::fromLatin1("!Global!") );
            TQString icon = config->readEntry(TQString::fromLatin1("IconName"),
                TQString::fromLatin1("misc"));
            TQString description = config->readEntry( TQString::fromLatin1("Comment"),
                i18n("No description available") );
            delete config;
            int index = relativePath.find( '/' );
            TQString appname;
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
    connect (eventSrcComboBox, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotEventSrcComboBox_activated(int)));
}

SelectEvent::~SelectEvent() { }

void SelectEvent::slotEventSrcComboBox_activated(int index)
{
    eventsListView->clear();
    TQListViewItem* item = 0;
    TQString eventSrc = m_eventSrcNames[index];
    TQString configFilename = eventSrc + TQString::fromLatin1( "/eventsrc" );
    KConfig* config = new KConfig( configFilename, true, false, "data" );
    TQStringList eventNames = config->groupList();
    uint eventNamesCount = eventNames.count();
    for (uint ndx = 0; ndx < eventNamesCount; ++ndx)
    {
        TQString eventName = eventNames[ndx];
        if ( eventName != "!Global!" )
        {
            config->setGroup( eventName );
            TQString eventDesc = config->readEntry( TQString::fromLatin1( "Comment" ),
                config->readEntry( TQString::fromLatin1( "Name" )));
            if ( !item )
                item = new KListViewItem( eventsListView, eventDesc, eventName );
            else
                item = new KListViewItem( eventsListView, item, eventDesc, eventName );
        }
    }
    delete config;
    eventsListView->sort();
    item = eventsListView->lastChild();
    TQString eventDesc = i18n("All other %1 events").arg(eventSrcComboBox->currentText());
    if ( !item )
        item = new KListViewItem( eventsListView, eventDesc, "default" );
    else
        item = new KListViewItem( eventsListView, item, eventDesc, "default" );

}

TQString SelectEvent::getEventSrc()
{
    return m_eventSrcNames[eventSrcComboBox->currentItem()];
}

TQString SelectEvent::getEvent()
{
    TQListViewItem* item = eventsListView->currentItem();
    if ( item )
        return item->text(1);
    else
        return TQString::null;
}

// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde3/share/apps/kwin/eventsrc"
TQString SelectEvent::makeRelative( const TQString& fullPath )
{
    int slash = fullPath.findRev( '/' ) - 1;
    slash = fullPath.findRev( '/', slash );

    if ( slash < 0 )
        return TQString::null;

    return fullPath.mid( slash+1 );
}


#include "selectevent.moc"
