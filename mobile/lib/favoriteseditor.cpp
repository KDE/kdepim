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

#include "favoriteseditor.h"

#include "favoritescontroller.h"

#include <kactioncollection.h>

#include <QtCore/QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>

FavoritesEditor::FavoritesEditor( KActionCollection *actionCollection, const KSharedConfig::Ptr &config, QObject *parent )
  : QObject( parent )
{
  mFavoritesController = new FavoritesController( config, this );

  actionCollection->addAction( QLatin1String("favoriteseditor_moveup"), mFavoritesController->moveUpAction() );
  actionCollection->addAction( QLatin1String("favoriteseditor_movedown"), mFavoritesController->moveDownAction() );
  actionCollection->addAction( QLatin1String("favoriteseditor_remove"), mFavoritesController->removeAction() );
}

QAbstractItemModel* FavoritesEditor::model() const
{
  return mFavoritesController->model();
}

void FavoritesEditor::setCollectionSelectionModel( QItemSelectionModel *model )
{
  mFavoritesController->setCollectionSelectionModel( model );
}

void FavoritesEditor::loadFavorite( const QString &name ) const
{
  mFavoritesController->loadFavorite( name );
}

void FavoritesEditor::saveFavorite( const QString &name )
{
  mFavoritesController->saveFavorite( name );
}

void FavoritesEditor::setRowSelected( int row )
{
  Q_ASSERT( row >= 0 && row < mFavoritesController->model()->rowCount() );

  QAbstractItemModel *model = mFavoritesController->model();
  QItemSelectionModel *selectionModel = mFavoritesController->selectionModel();

  selectionModel->select( model->index( row, 0, QModelIndex() ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
}

#include "favoriteseditor.moc"
