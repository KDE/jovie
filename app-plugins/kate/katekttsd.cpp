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
#include <tqmessagebox.h>
#include <dcopclient.h>
#include <tqtimer.h>

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

K_EXPORT_COMPONENT_FACTORY( ktexteditor_kttsd, KGenericFactory<KateKttsdPlugin>( "ktexteditor_kttsd" ) )

KateKttsdPlugin::KateKttsdPlugin( TQObject *parent, const char* name, const TQStringList& )
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
    for (uint z=0; z < m_views.count(); ++z)
        if (m_views.at(z)->parentClient() == view)
    {
        KateKttsdPluginView *nview = m_views.at(z);
        m_views.remove (nview);
        delete nview;
    }
    KGlobal::locale()->removeCatalogue("kttsd");
}


KateKttsdPluginView::KateKttsdPluginView( KTextEditor::View *view, const char *name )
    : TQObject( view, name ),
    KXMLGUIClient( view )
{
    view->insertChildClient( this );
    setInstance( KGenericFactory<KateKttsdPlugin>::instance() );
    KGlobal::locale()->insertCatalogue("kttsd");
    (void) new KAction( i18n("Speak Text"), "kttsd", 0, this, TQT_SLOT(slotReadOut()), actionCollection(), "tools_kttsd" );
    setXMLFile( "ktexteditor_kttsdui.rc" );
}

void KateKttsdPluginView::slotReadOut()
{
    KTextEditor::View *v = (KTextEditor::View*)parent();
    KTextEditor::SelectionInterface *si = KTextEditor::selectionInterface( v->document() );
    TQString text;

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
        TQString error;
        if (kapp->startServiceByDesktopName("kttsd", TQStringList(), &error))
            TQMessageBox::warning(0, i18n( "Starting KTTSD Failed"), error );
    }
    TQByteArray  data;
    TQByteArray  data2;
    TQCString    replyType;
    TQByteArray  replyData;
    TQDataStream arg(data, IO_WriteOnly);
    arg << text << "";
    if ( !client->call("kttsd", "KSpeech", "setText(TQString,TQString)",
                       data, replyType, replyData, true) )
       TQMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                 i18n( "The DCOP call setText failed." ));
    TQDataStream arg2(data2, IO_WriteOnly);

    arg2 << 0;
    if ( !client->call("kttsd", "KSpeech", "startText(uint)",
                       data2, replyType, replyData, true) )
       TQMessageBox::warning( 0, i18n( "DCOP Call Failed" ),
                                i18n( "The DCOP call startText failed." ));
}

