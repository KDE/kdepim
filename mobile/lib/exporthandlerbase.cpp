/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "exporthandlerbase.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/recursiveitemfetchjob.h>
#include <kmessagebox.h>

#include <QtGui/QItemSelectionModel>

ExportHandlerBase::ExportHandlerBase( QObject *parent )
  : QObject( parent ),
    mSelectionModel( 0 )
{
}

ExportHandlerBase::~ExportHandlerBase()
{
}

void ExportHandlerBase::setSelectionModel( QItemSelectionModel *model )
{
  mSelectionModel = model;
}

void ExportHandlerBase::exec()
{
  Akonadi::Collection::List selectedCollections;

  const QModelIndexList indexes = mSelectionModel->selectedRows();
  foreach ( const QModelIndex &index, indexes ) {
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      selectedCollections << collection;
  }

  bool exportAllItems = false;
  if ( !selectedCollections.isEmpty() ) {
    switch ( KMessageBox::questionYesNo( 0, dialogText(), QString(),
                                         KGuiItem( dialogAllText() ),
                                         KGuiItem( dialogLocalOnlyText() ) ) ) {
      case KMessageBox::Yes:
        exportAllItems = true;
        break;
      case KMessageBox::No: // fall through
      default:
        exportAllItems = false;
    }
  } else {
    exportAllItems = true;
  }

  Akonadi::Item::List items;
  if ( exportAllItems ) {
    Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob( Akonadi::Collection::root(), mimeTypes() );
    job->fetchScope().fetchFullPayload();

    job->exec();

    items << job->items();
  } else {
    foreach ( const Akonadi::Collection &collection, selectedCollections ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection );
      job->fetchScope().fetchFullPayload();

      if ( job->exec() )
        items << job->items();
    }
  }

  exportItems( items );

  deleteLater();
}

#include "exporthandlerbase.moc"
