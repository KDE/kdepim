/*
    This file is part of KJots.

    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kjotstreeview.h"

#include <QMenu>
#include <QContextMenuEvent>

#include <KActionCollection>
#include <KXMLGUIClient>

#include "kjotsmodel.h"

using namespace Akonadi;


KJotsTreeView::KJotsTreeView( KXMLGUIClient* xmlGuiClient, QWidget* parent )
    : EntityTreeView( xmlGuiClient, parent ), m_xmlGuiClient( xmlGuiClient )
{

}

void KJotsTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *popup = new QMenu(this);

    QModelIndexList rows = selectionModel()->selectedRows();

    const bool noselection = rows.isEmpty();
    const bool singleselection = rows.size() == 1;
    const bool multiselection = rows.size() > 1;

    popup->addAction(m_xmlGuiClient->actionCollection()->action("new_book"));
    if ( singleselection )
    {
      popup->addAction(m_xmlGuiClient->actionCollection()->action("new_page"));
      popup->addAction(m_xmlGuiClient->actionCollection()->action("rename_entry"));

      popup->addAction(m_xmlGuiClient->actionCollection()->action("copy_link_address"));
      popup->addAction(m_xmlGuiClient->actionCollection()->action("change_color"));
    }
    if ( !noselection )
      popup->addAction(m_xmlGuiClient->actionCollection()->action("save_to"));
    popup->addSeparator();

    if ( singleselection )
    {
      Item item = rows.at( 0 ).data( KJotsModel::ItemRole ).value<Item>();
      if ( item.isValid() ) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action("del_page"));
      } else {
        popup->addAction(m_xmlGuiClient->actionCollection()->action("del_folder"));
      }
    }

    if ( multiselection )
      popup->addAction(m_xmlGuiClient->actionCollection()->action("del_mult"));

    popup->exec( event->globalPos() );

    delete popup;
}

