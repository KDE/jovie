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
#include <QtGui/QComboBox>
#include <QtGui/QHeaderView>

// KDE includes
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

// KTTS includes
#include "utils.h"
#include "selectevent.h"

SelectEvent::SelectEvent(QWidget* parent, const QString& initEventSrc) :
    QWidget(parent)
{
    setupUi(this);
    // Hide Event Name column.
    eventsListView->setColumnHidden(1, true);
    eventsListView->verticalHeader()->hide();
    eventsListView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    // Load list of event sources (applications).
    QStringList fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/*.notifyrc", KStandardDirs::NoDuplicates);
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
            KConfig* config = new KConfig(relativePath, KConfig::NoGlobals, "data" );
            KConfigGroup globalConfig( config, QString::fromLatin1("Global") );
            QString icon = globalConfig.readEntry(QString::fromLatin1("IconName"),
                QString::fromLatin1("misc"));
            QString description = globalConfig.readEntry( QString::fromLatin1("Comment"),
                i18n("No description available") );
            delete config;
            int index = relativePath.indexOf( '/' );
            QString appname;
            if ( index >= 0 )
                appname = relativePath.left( index );
            else
                kDebug() << "Cannot determine application name from path: " << relativePath;
            eventSrcComboBox->addItem( SmallIcon( icon ), description );
            m_eventSrcNames.append( appname );
            if ( appname == initEventSrc ) KttsUtils::setCbItemFromText(eventSrcComboBox, description);
        }
    }
    slotEventSrcComboBox_activated(eventSrcComboBox->currentIndex());
    connect (eventSrcComboBox, SIGNAL(activated(int)), this, SLOT(slotEventSrcComboBox_activated(int)));
}

SelectEvent::~SelectEvent() { }

void SelectEvent::slotEventSrcComboBox_activated(int index)
{
    eventsListView->setRowCount(0);
    QString eventSrc = m_eventSrcNames[index];
    QString configFilename = eventSrc + "/" + eventSrc + QString::fromLatin1( ".notifyrc" );
    KConfig* config = new KConfig( configFilename, KConfig::NoGlobals, "data" );
    QStringList eventNames = config->groupList();
    uint eventNamesCount = eventNames.count();
    for (uint ndx = 0; ndx < eventNamesCount; ++ndx)
    {
        QString eventName = eventNames[ndx];
        if ( eventName != "!Global!" )
        {
            KConfigGroup eventConfig(config, eventName );
            QString eventDesc = eventConfig.readEntry( QString::fromLatin1( "Comment" ),
                eventConfig.readEntry( QString::fromLatin1( "Name" ),QString()));
            int row = eventsListView->rowCount();
            eventsListView->setRowCount(row + 1);
            eventsListView->setItem(row, 0, new QTableWidgetItem(eventDesc));
            eventsListView->setItem(row, 1, new QTableWidgetItem(eventName));
       }
    }
    delete config;
    eventsListView->sortItems(0);
    QString eventDesc = i18n("All other %1 events", eventSrcComboBox->currentText());
    int row = eventsListView->rowCount();
    eventsListView->setRowCount(row + 1);
    eventsListView->setItem(row, 0, new QTableWidgetItem(eventDesc));
    eventsListView->setItem(row, 1, new QTableWidgetItem("default"));
}

QString SelectEvent::getEventSrc()
{
    return m_eventSrcNames[eventSrcComboBox->currentIndex()];
}

QString SelectEvent::getEvent()
{
    int row = eventsListView->currentRow();
    if (row < 0 || row >= eventsListView->rowCount()) return QString();
    return eventsListView->item(row, 1)->text();
}

// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde3/share/apps/kwin/eventsrc"
QString SelectEvent::makeRelative( const QString& fullPath )
{
    int slash = fullPath.lastIndexOf( '/' ) - 1;
    slash = fullPath.lastIndexOf( '/', slash );

    if ( slash < 0 )
        return QString();

    return fullPath.mid( slash+1 );
}

#include "selectevent.moc"

