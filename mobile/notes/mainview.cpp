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
#include <akonadi_next/notecreatorandselector.h>
#include <akonadi_next/note.h>

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

Collection MainView::suitableContainerCollection(const QModelIndex &parent)
{
  const int rowCount = entityTreeModel()->rowCount(parent);
  for (int row = 0; row < rowCount; ++row)
  {
    static const int column = 0;
    const QModelIndex idx = entityTreeModel()->index(row, column, parent);
    Q_ASSERT(idx.isValid());
    const Collection collection = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
    Q_ASSERT(collection.isValid());
    if (collection.contentMimeTypes().contains(Akonotes::Note::mimeType()))
      return collection;
    const Collection descendantCollection = suitableContainerCollection(idx);
    if (descendantCollection.isValid())
      return descendantCollection;
  }

}

void MainView::startComposer()
{
  // If a collection is currently selected, put the new note there.

  const int rowCount = selectedItemsModel()->rowCount();

  if (rowCount > 1)
    // Multiple items are selected. Find out how this should be handled.
    return;

  if (rowCount == 1)
  {
    const QModelIndex idx = selectedItemsModel()->index(0, 0);
    Q_ASSERT(idx.isValid());
    const Collection collection = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
    Q_ASSERT(collection.isValid());
    Akonotes::NoteCreatorAndSelector *noteCreator = new Akonotes::NoteCreatorAndSelector(itemSelectionModel(), itemSelectionModel(), this);
    noteCreator->createNote(collection);
    connect(itemSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
    return;
  }

  // otherwise nothing is selected, find a collection which can contain notes and put it there.

  Collection collection = suitableContainerCollection();

  if (!collection.isValid())
  {
    // No suitable collection found.
    // Create a resource with LocalResourceCreator,
    // then add a collection, then use the NoteCreatorAndSelector.
    return;
  }
  Akonotes::NoteCreatorAndSelector *noteCreator = new Akonotes::NoteCreatorAndSelector(regularSelectionModel(), itemSelectionModel(), this);
  noteCreator->createNote(collection);
}

void MainView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{

  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if (list.size() != 1)
    return;

  const QModelIndex idx = list.first();
  selectedItemChanged(idx.row(), idx.data(EntityTreeModel::ItemIdRole).toLongLong());
}

