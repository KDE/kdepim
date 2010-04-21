/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "mainview.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

#include <akonadi/entitytreemodel.h>

#include "notelistproxy.h"
#include <QDeclarativeContext>
#include <KMime/KMimeMessage>

using namespace Akonadi;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "notes", new NoteListProxy( Akonadi::EntityTreeModel::UserRole ), parent )
{
  addMimeType( "text/x-vnd.akonadi.note" );
}

QString MainView::noteTitle(int row)
{
  if ( row < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( "itemModel").value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );

  if ( !itemModel )
    return QString();

  static const int column = 0;
  QModelIndex idx = itemModel->index(row, column);

  if ( !idx.isValid() )
    return QString();

  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  return note->subject()->asUnicodeString();
}

QString MainView::noteContent(int row)
{
  if ( row < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( "itemModel").value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );

  if ( !itemModel )
    return QString();

  static const int column = 0;
  QModelIndex idx = itemModel->index(row, column);

  if ( !idx.isValid() )
    return QString();

  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  // TODO: Rich mimetype.
  return note->mainBodyPart()->decodedText();
}

