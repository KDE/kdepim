/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "favoritescontroller.h"

#include "favoriteslistmodel.h"

#include <AkonadiWidgets/etmviewstatesaver.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <kmessagebox.h>

#include <QtCore/QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>

static const QString sFavoritePrefix = QLatin1String("Favorite_");
static const int sFavoritePrefixLength = 9;

class FavoritesController::Private
{
  public:
    QStringList rereadFavoritesList() const;

    void selectionChanged();
    void removeFavorite();
    void moveUpFavorite();
    void moveDownFavorite();

    KSharedConfig::Ptr mConfig;
    FavoritesListModel *mModel;
    QItemSelectionModel *mSelectionModel;
    QItemSelectionModel *mCollectionSelectionModel;
    QAction *mRemoveAction;
    QAction *mMoveUpAction;
    QAction *mMoveDownAction;
};

QStringList FavoritesController::Private::rereadFavoritesList() const
{
  QStringList names;
  foreach ( const QString &group, mConfig->groupList() ) {
    if ( group.startsWith( sFavoritePrefix ) )
      names.append( QString( group ).remove( 0, sFavoritePrefixLength ) );
  }

  return names;
}

void FavoritesController::Private::selectionChanged()
{
  const bool favoriteSelected = mSelectionModel->hasSelection();

  if ( favoriteSelected ) {
    mRemoveAction->setEnabled( true );

    const QModelIndex index = mSelectionModel->selectedRows().first();
    mMoveUpAction->setEnabled( index.row() != 0 );
    mMoveDownAction->setEnabled( index.row() != (mModel->rowCount() - 1) );
  } else {
    mRemoveAction->setEnabled( false );
    mMoveUpAction->setEnabled( false );
    mMoveDownAction->setEnabled( false );
  }
}

void FavoritesController::Private::removeFavorite()
{
  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mSelectionModel->selectedRows().first();

  const int result = KMessageBox::questionYesNo( 0, i18n( "Do you really want to remove favorite <b>%1</b>?",
                                                          index.data( Qt::DisplayRole ).toString() ),
                                                    i18n( "Remove Favorite" ) );
  if ( result == KMessageBox::No )
    return;

  mModel->removeItem( index.row() );
}

void FavoritesController::Private::moveUpFavorite()
{
  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mSelectionModel->selectedRows().first();
  mModel->moveUp( index.row() );
}

void FavoritesController::Private::moveDownFavorite()
{
  if ( !mSelectionModel->hasSelection() )
    return;

  const QModelIndex index = mSelectionModel->selectedRows().first();
  mModel->moveDown( index.row() );
}


FavoritesController::FavoritesController( const KSharedConfig::Ptr &config, QObject *parent )
  : QObject( parent ), d( new Private )
{
  d->mConfig = config;
  d->mModel = new FavoritesListModel( config, this );
  d->mSelectionModel = new QItemSelectionModel( d->mModel );
  d->mCollectionSelectionModel = 0;

  d->mRemoveAction = new QAction( i18n( "Remove" ), this );
  d->mMoveUpAction = new QAction( i18n( "Move Up" ), this );
  d->mMoveDownAction = new QAction( i18n( "Move Down" ), this );

  connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(selectionChanged()) );

  connect( d->mRemoveAction, SIGNAL(triggered(bool)), SLOT(removeFavorite()) );
  connect( d->mMoveUpAction, SIGNAL(triggered(bool)), SLOT(moveUpFavorite()) );
  connect( d->mMoveDownAction, SIGNAL(triggered(bool)), SLOT(moveDownFavorite()) );

  d->selectionChanged();
}

FavoritesController::~FavoritesController()
{
  delete d;
}

QAbstractItemModel* FavoritesController::model() const
{
  return d->mModel;
}

QItemSelectionModel* FavoritesController::selectionModel() const
{
  return d->mSelectionModel;
}

void FavoritesController::setCollectionSelectionModel( QItemSelectionModel *model )
{
  d->mCollectionSelectionModel = model;
}

QAction* FavoritesController::removeAction() const
{
  return d->mRemoveAction;
}

QAction* FavoritesController::moveUpAction() const
{
  return d->mMoveUpAction;
}

QAction* FavoritesController::moveDownAction() const
{
  return d->mMoveDownAction;
}

void FavoritesController::loadFavorite( const QString &name ) const
{
  Q_ASSERT( d->mCollectionSelectionModel );

  Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
  saver->setSelectionModel( d->mCollectionSelectionModel );

  KConfigGroup group( d->mConfig, sFavoritePrefix + name );
  if ( !group.isValid() ) {
    delete saver;
    return;
  }

  saver->restoreState( group );
}

void FavoritesController::saveFavorite( const QString &name )
{
  Q_ASSERT( d->mCollectionSelectionModel );

  Akonadi::ETMViewStateSaver saver;
  saver.setSelectionModel( d->mCollectionSelectionModel );

  KConfigGroup group( d->mConfig, sFavoritePrefix + name );
  saver.saveState( group );
  group.sync();
  d->mModel->setStringList( d->rereadFavoritesList() );
}

#include "moc_favoritescontroller.cpp"
