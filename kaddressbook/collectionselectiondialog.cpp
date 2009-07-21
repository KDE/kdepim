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

#include "collectionselectiondialog.h"

#include "collectioncombobox.h"

#include <akonadi_next/descendantsproxymodel.h>
#include <akonadi_next/entityfilterproxymodel.h>
#include <akonadi/item.h>

#include <kabc/addressee.h>
#include <klocale.h>

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

CollectionSelectionDialog::CollectionSelectionDialog( QAbstractItemModel *collectionModel, QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Select Address Book" ) );
  setButtons( Ok | Cancel );

  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  QVBoxLayout *layout = new QVBoxLayout( mainWidget );

  // flatten the collection tree structure to a collection list
  Akonadi::DescendantsProxyModel *descendantModel = new Akonadi::DescendantsProxyModel( this );
  descendantModel->setSourceModel( collectionModel );

/*
  Akonadi::CollectionFilterProxyModel *filterModel = new Akonadi::CollectionFilterProxyModel( this );

  filterModel->addMimeTypeFilter( KABC::Addressee::mimeType() );
  filterModel->setSourceModel( collectionModel );
*/

  mCollectionCombo = new KABC::CollectionComboBox( mainWidget );
  mCollectionCombo->setModel( descendantModel );

  layout->addWidget( new QLabel( i18n( "Select the address book" ) ) );
  layout->addWidget( mCollectionCombo );
}

CollectionSelectionDialog::~CollectionSelectionDialog()
{
}

Akonadi::Collection CollectionSelectionDialog::selectedCollection() const
{
  return mCollectionCombo->selectedCollection();
}
