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
#include <Akonadi/ItemFetchScope>

#include "notelistproxy.h"
#include <QDeclarativeContext>
#include <KMime/KMimeMessage>

using namespace Akonadi;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "notes", new NoteListProxy( Akonadi::EntityTreeModel::UserRole ), parent )
{
  addMimeType( "text/x-vnd.akonadi.note" );
  itemFetchScope().fetchFullPayload();
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

void MainView::saveNote(const QString& title, const QString& content)
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>(itemSelectionModel()->model());

  if (!model->hasChildren())
    return;

  static const int column = 0;
  static const int row = 0;
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if (list.size() != 1)
    return;

  const QModelIndex idx = list.first();

  Q_ASSERT( idx.isValid() );

  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if (!item.isValid())
    return;

  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  note->subject()->fromUnicodeString(title, "utf-8");
  KMime::Content *c = note->mainBodyPart();
  c->fromUnicodeString(content);

  note->assemble();

  model->setData(idx, QVariant::fromValue(item), EntityTreeModel::ItemRole);
}


void MainView::saveCurrentNoteTitle(const QString& title)
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>(itemSelectionModel()->model());

  if (!model->hasChildren())
    return;

  static const int column = 0;
  static const int row = 0;
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if (list.size() != 1)
    return;

  const QModelIndex idx = list.first();

  Q_ASSERT( idx.isValid() );

  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if (!item.isValid())
    return;

  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  note->subject()->fromUnicodeString(title, "utf-8");

  note->assemble();

  model->setData(idx, QVariant::fromValue(item), EntityTreeModel::ItemRole);
}

void MainView::saveCurrentNoteContent(const QString& content)
{
  QAbstractItemModel *model = const_cast<QAbstractItemModel*>(itemSelectionModel()->model());

  if (!model->hasChildren())
    return;

  static const int column = 0;
  static const int row = 0;
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if (list.size() != 1)
    return;

  const QModelIndex idx = list.first();

  Q_ASSERT( idx.isValid() );

  Item item = idx.data( EntityTreeModel::ItemRole ).value<Item>();

  if (!item.isValid())
    return;

  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();
  KMime::Content *c = note->mainBodyPart();
  c->fromUnicodeString(content);

  note->assemble();

  model->setData(idx, QVariant::fromValue(item), EntityTreeModel::ItemRole);
}
