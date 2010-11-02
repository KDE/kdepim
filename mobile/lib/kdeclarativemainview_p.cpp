/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#include "kdeclarativemainview_p.h"

#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KLineEdit>
#include <KDE/KSharedConfig>

#include <akonadi/etmviewstatesaver.h>

#include "favoriteslistmodel.h"
#include "guistatemanager.h"

KDeclarativeMainViewPrivate::KDeclarativeMainViewPrivate()
  : mChangeRecorder( 0 )
  , mCollectionFilter( 0 )
  , mItemFilterModel( 0 )
  , mFavsListModel( 0 )
  , mFilterLineEdit( 0 )
  , mBulkActionFilterLineEdit( 0 )
  , mAgentStatusMonitor( 0 )
  , mGuiStateManager( 0 )
  , mStateMachine( 0 )
{ }

void KDeclarativeMainViewPrivate::restoreState()
{
  Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
  saver->setSelectionModel( mBnf->selectionModel() );
  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver->restoreState( cfg );
}

void KDeclarativeMainViewPrivate::saveState()
{
  Akonadi::ETMViewStateSaver saver;
  saver.setSelectionModel( mBnf->selectionModel() );

  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver.saveState( cfg );
  cfg.sync();
}

QStringList KDeclarativeMainViewPrivate::getFavoritesList()
{
  QStringList names;
  foreach ( const QString &group, KGlobal::config()->groupList() )
    if ( group.startsWith( sFavoritePrefix ) )
      names.append( QString( group ).remove( 0, sFavoritePrefixLength ) );
  return names;
}

QAbstractItemModel* KDeclarativeMainViewPrivate::getFavoritesListModel()
{
  if (!mFavsListModel)
    mFavsListModel = new FavoritesListModel( KGlobal::config() );

  return mFavsListModel;
}

void KDeclarativeMainViewPrivate::filterLineEditChanged( const QString &text )
{
  if ( !text.isEmpty() && !mFilterLineEdit->isVisible() ) {
    mFilterLineEdit->setFixedHeight( 35 );
    mFilterLineEdit->show();
    mFilterLineEdit->setFocus();
  } else if ( text.isEmpty() && mFilterLineEdit->isVisible() ) {
    mFilterLineEdit->setFixedHeight( 0 );
    mFilterLineEdit->hide();
  }
}

void KDeclarativeMainViewPrivate::bulkActionFilterLineEditChanged( const QString &text )
{
  if ( !text.isEmpty() && !mBulkActionFilterLineEdit->isVisible() ) {
    mBulkActionFilterLineEdit->setFixedHeight( 35 );
    mBulkActionFilterLineEdit->show();
    mBulkActionFilterLineEdit->setFocus();
  } else if ( text.isEmpty() && mBulkActionFilterLineEdit->isVisible() ) {
    mBulkActionFilterLineEdit->setFixedHeight( 0 );
    mBulkActionFilterLineEdit->hide();
  }
}

void KDeclarativeMainViewPrivate::searchStarted( const Akonadi::Collection &searchCollection )
{
  const QStringList selection = QStringList() << QString::fromLatin1( "c%1" ).arg( searchCollection.id() );
  Akonadi::ETMViewStateSaver *restorer = new Akonadi::ETMViewStateSaver;

  mGuiStateManager->pushState( GuiStateManager::SearchResultScreenState );

  QItemSelectionModel *selectionModel = mBnf->selectionModel();
  selectionModel->clearSelection();

  restorer->setSelectionModel( selectionModel );
  restorer->restoreSelection( selection );
}

void KDeclarativeMainViewPrivate::searchStopped()
{
  mGuiStateManager->popState();
}

