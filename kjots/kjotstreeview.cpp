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

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>

#include <KActionCollection>
#include <KXMLGUIClient>
#include <KColorDialog>
#include <KInputDialog>
#include <KLocale>

#include <KMime/KMimeMessage>

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

void KJotsTreeView::delayedInitialization()
{
  connect(m_xmlGuiClient->actionCollection()->action("rename_entry"), SIGNAL(triggered()),
      this, SLOT(renameEntry()));
  connect(m_xmlGuiClient->actionCollection()->action("copy_link_address"), SIGNAL(triggered()),
      this, SLOT(copyLinkAddress()));
  connect(m_xmlGuiClient->actionCollection()->action("change_color"), SIGNAL(triggered()),
      this, SLOT(changeColor()));
}

void KJotsTreeView::renameEntry()
{
  QModelIndexList rows = selectionModel()->selectedRows();

  if ( rows.size() != 1 )
    return;

  QModelIndex idx = rows.at( 0 );

  Item item = idx.data( KJotsModel::ItemRole ).value<Item>();

  KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

  QString title = msg->subject()->asUnicodeString();

  bool ok;
  QString name = KInputDialog::getText(i18n( "Rename Book" ),
      i18n( "Book name:" ), title, &ok, this);

  if (ok)
    model()->setData( idx, name, Qt::EditRole );
}

void KJotsTreeView::copyLinkAddress()
{
  QModelIndexList rows = selectionModel()->selectedRows();

  if ( rows.size() != 1 )
    return;

  Item item = rows.at( 0 ).data( KJotsModel::ItemRole ).value<Item>();

  KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

  QString title = msg->subject()->asUnicodeString();

  QMimeData *mimeData = new QMimeData();

  QString link = QString( "<a href=\"%1\">%2</a>" ).arg( item.url().url() ).arg( title );

  mimeData->setData( "kjots/internal_link", link.toUtf8() );
  mimeData->setText( title );
  QApplication::clipboard()->setMimeData( mimeData );
  return;
}

void KJotsTreeView::changeColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );

  if ( result != KColorDialog::Accepted )
    return;

  QModelIndexList rows = selectionModel()->selectedRows();

  foreach ( const QModelIndex &idx, rows )
  {
    model()->setData(idx, myColor, Qt::BackgroundColorRole );
  }
}






