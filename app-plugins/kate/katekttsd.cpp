/***************************************************************************
  A KTextEditor (Kate Part) plugin for speaking text.

  Copyright:
  (C) 2003-2004 by Olaf Schmidt <ojschmidt@kde.org>
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>

  Original Author: Olaf Schmidt <ojschmidt@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Qt includes.
#include <qmessagebox.h>
#include <dcopclient.h>
#include <qtimer.h>

// KDE includes.
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kgenericfactory.h>

// KateKttsdPlugin includes.
#include "katekttsd.h"
#include "katekttsd.moc"

K_EXPORT_COMPONENT_FACTORY( ktexteditor_kttsd, KGenericFactory<KateKttsdPlugin>( "kttsd" ) )

KateKttsdPlugin::KateKttsdPlugin( QObject *parent, const char* name, const QStringList& )
    : KTextEditor::Plugin ( (KTextEditor::Document*) parent, name )
{
}

KateKttsdPlugin::~KateKttsdPlugin()
{
}


void KateKttsdPlugin::addView(KTextEditor::View *view)
{
    KateKttsdPluginView *nview = new KateKttsdPluginView (view, "KTTSD Plugin");
    m_views.append (nview);
}

void KateKttsdPlugin::removeView(KTextEditor::View *view)
{
    for (uint z=0; z < m_views.count(); z++)
        if (m_views.at(z)->parentClient() == view)
    {
        KateKttsdPluginView *nview = m_views.at(z);
        m_views.remove (nview);
        delete nview;
    }
}


KateKttsdPluginView::KateKttsdPluginView( KTextEditor::View *view, const char *name )
    : QObject( view, name ),
    KXMLGUIClient( view )
{
    view->insertChildClient( this );
    setInstance( KGenericFactory<KateKttsdPlugin>::instance() );
    (void) new KAction( i18n("Speak Text"), "kttsd", 0, this, SLOT(slotReadOut()), actionCollection(), "tools_kttsd" );
    setXMLFile( "ktexteditor_kttsdui.rc" );
}

void KateKttsdPluginView::slotReadOut()
{
    KTextEditor::View *v = (KTextEditor::View*)parent();
    KTextEditor::SelectionInterface *si = KTextEditor::selectionInterface( v->document() );
    QString text;

    if ( si->hasSelection() )
      text = si->selection();
    else {
        KTextEditor::EditInterface *ei = KTextEditor::editInterface( v->document() );
        text = ei->text();
    }

    DCOPClient *client = kapp->dcopClient();
    // If KTTSD not running, start it.
    if (!client->isApplicationRegistered("kttsd"))
    {
        QString error;
        if (kapp->startServiceByDesktopName("kttsd", QStringList(), &error))
            QMessageBox::warning(0, i18n( "Starting KTTSD Failed"), error );
    }
    QByteArray  data;
    QByteArray  data2;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << text << "";
    if ( !client->call("kttsd", "KSpeech", "setText(QString,QString)",
                       data, replyType, replyData, true) )
       QMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                 i18n( "The DCOP call setText failed." ));
    QDataStream arg2(data2, IO_WriteOnly);

    arg2 << 0;
    if ( !client->call("kttsd", "KSpeech", "startText(uint)",
                       data2, replyType, replyData, true) )
       QMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                i18n( "The DCOP call startText failed." ));
}

