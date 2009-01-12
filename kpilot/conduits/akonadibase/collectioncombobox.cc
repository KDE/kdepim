/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "collectioncombobox.h"
#include "options.h"

#include <QtCore/QAbstractItemModel>
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>

#include <akonadi/control.h>
#include <akonadi/collectionmodel.h>

using namespace Akonadi;

class CollectionComboBox::Private
{
  public:
    Private( CollectionComboBox *parent )
      : mParent( parent ), mCurrentId( -1 )
    {
    }

    void activated( int index );

    CollectionComboBox *mParent;

    void checkCurrentSelectedCollection();

    KComboBox *mComboBox;

    Entity::Id mCurrentId;
};

void CollectionComboBox::Private::activated( int index )
{
  FUNCTIONSETUPL(5);
  if ( !mComboBox->model() )
    return;

  const QModelIndex modelIndex = mComboBox->model()->index( index, 0 );
  DEBUGKPILOT << "current index: " << index;
  if ( modelIndex.isValid() ) {
    DEBUGKPILOT << "modelIndex is valid.";
    emit mParent->selectionChanged( Collection( modelIndex.data( CollectionModel::CollectionIdRole ).toLongLong() ) );
  }
}

void CollectionComboBox::Private::checkCurrentSelectedCollection()
{
  FUNCTIONSETUPL(5);
  DEBUGKPILOT << "mcurrentId: " << mCurrentId;
  if( mCurrentId != -1 )
    mParent->setSelectedCollection( mCurrentId );
}

CollectionComboBox::CollectionComboBox( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );
  layout->setSpacing( 0 );

  d->mComboBox = new KComboBox( this );
  layout->addWidget( d->mComboBox );

  connect( d->mComboBox, SIGNAL( activated( int ) ), SLOT( activated( int ) ) );
}

CollectionComboBox::~CollectionComboBox()
{
  delete d;
}

void CollectionComboBox::setModel( QAbstractItemModel *model )
{
  d->mComboBox->setModel( model );

  connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) )
    , SLOT( checkCurrentSelectedCollection() ) );
}

Akonadi::Collection CollectionComboBox::selectedCollection() const
{
  FUNCTIONSETUPL(5);
  Q_ASSERT_X( d->mComboBox->model() != 0, "CollectionComboBox::selectionChanged", "No model set!" );

  int index = d->mComboBox->currentIndex();
  DEBUGKPILOT << "current index: " << index;

  const QModelIndex modelIndex = d->mComboBox->model()->index( index, 0 );

  if ( modelIndex.isValid() ) {
    Akonadi::Collection col( modelIndex.data(
       Akonadi::CollectionModel::CollectionIdRole ).toLongLong() );
    DEBUGKPILOT << "modelIndex is valid. returning: " << col.id();
    return col;
  } else {
    DEBUGKPILOT << "modelIndex is invalid.";
    return Akonadi::Collection();
  }
}

void CollectionComboBox::setSelectedCollection( const Entity::Id id )
{
  FUNCTIONSETUPL(5);
  DEBUGKPILOT << "requested id to set: " << id;
  Q_ASSERT_X( d->mComboBox->model() != 0, "CollectionComboBox::setSelectedCollection", "No model set!" );

  QAbstractItemModel* model = d->mComboBox->model();
  QModelIndexList result = model->match( model->index( 0, 0 ), CollectionModel::CollectionIdRole, id );
  d->mCurrentId = id;

  if( !result.isEmpty() ) {
    d->mComboBox->setCurrentIndex( result.first().row() );
  } else {
    DEBUGKPILOT << "invalid id requested.";
    if ( d->mComboBox->count() > 0 ) {
      DEBUGKPILOT << "selecting first available.";
      d->mComboBox->setCurrentIndex( 0 );
      d->mCurrentId = selectedCollection().id();
      DEBUGKPILOT << "current id now: " << d->mCurrentId;
    }
  }
}

#include "collectioncombobox.moc"
