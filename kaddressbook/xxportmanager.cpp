/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "xxportmanager.h"
#include <kmessagebox.h>
#include <klocale.h>

#include "collectionselectiondialog.h"

#include <akonadi/collection.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entityfilterproxymodel.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>

#include <QtCore/QSignalMapper>
#include <QtGui/QAction>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QWidget>

#include "akonadi_next/entitytreeview.h"


XXPortManager::XXPortManager( QWidget *parent )
  : QObject( parent ), mCollectionModel( 0 ), mSelectionModel( 0 ), mParentWidget( parent )
{
  mImportMapper = new QSignalMapper( this );
  mExportMapper = new QSignalMapper( this );

  connect( mImportMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( slotImport( const QString& ) ) );
  connect( mExportMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( slotExport( const QString& ) ) );
}

XXPortManager::~XXPortManager()
{
}

void XXPortManager::addImportAction( QAction *action, const QString &identifier )
{
  mImportMapper->setMapping( action, identifier );
  connect( action, SIGNAL( triggered( bool ) ), mImportMapper, SLOT( map() ) );
}

void XXPortManager::addExportAction( QAction *action, const QString &identifier )
{
  mExportMapper->setMapping( action, identifier );
  connect( action, SIGNAL( triggered( bool ) ), mExportMapper, SLOT( map() ) );
}

void XXPortManager::setCollectionModel( QAbstractItemModel *collectionModel )
{
  mCollectionModel = collectionModel;
}

void XXPortManager::setSelectionModel( QItemSelectionModel *selectionModel )
{
  mSelectionModel = selectionModel;
}

void XXPortManager::slotImport( const QString &identifier )
{
  if ( !mCollectionModel )
    return;

  const XXPort* xxport = mFactory.createXXPort( identifier, mParentWidget );
  if( !xxport )
    return;

  const KABC::Addressee::List contacts = xxport->importContacts();

  delete xxport;

  if ( contacts.isEmpty() ) // nothing to import
    return;

  CollectionSelectionDialog dlg( mCollectionModel, mParentWidget );
  if ( !dlg.exec() )
    return;

  const Akonadi::Collection collection = dlg.selectedCollection();

  for ( int i = 0; i < contacts.count(); ++i ) {
    Akonadi::Item item;
    item.setPayload<KABC::Addressee>( contacts.at( i ) );
    item.setMimeType( KABC::Addressee::mimeType() );

    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
    job->exec();
  }
}

void XXPortManager::slotExport( const QString &identifier )
{
  if ( !mSelectionModel )
    return;

  const XXPort* xxport = mFactory.createXXPort( identifier, mParentWidget );
  if ( !xxport )
    return;

  KABC::AddresseeList contacts;

  foreach ( const QModelIndex &index, mSelectionModel->selectedRows() ) {
    const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    Q_ASSERT( item.isValid() );
    const KABC::Addressee contact = item.payload<KABC::Addressee>();
    contacts.append( contact );
  }

  if ( !contacts.isEmpty() )
    xxport->exportContacts( contacts );
  else
    KMessageBox::sorry( 0, i18n( "Any contact selected" ) );

  delete xxport;
}

#include "xxportmanager.moc"
