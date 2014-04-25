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
#include "guistatemanager.h"
#include "stylesheetloader.h"

#include <AkonadiCore/agentinstance.h>
#include <akonadi/agentinstancemodel.h>
#include <akonadi/etmviewstatesaver.h>

#include <KDE/KCmdLineArgs>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KLineEdit>
#include <KDE/KSharedConfig>
#include <KDE/KProcess>

#include <QtDBus/QDBusInterface>
#include <qplatformdefs.h>

KDeclarativeMainViewPrivate::KDeclarativeMainViewPrivate( KDeclarativeMainView *qq )
  : q( qq )
  , mChangeRecorder( 0 )
  , mCollectionFilter( 0 )
  , mItemFilterModel( 0 )
  , mBnf( 0 )
  , mAgentStatusMonitor( 0 )
  , mGuiStateManager( 0 )
  , mStateMachine( 0 )
  , mFavoritesEditor( 0 )
{ }

void KDeclarativeMainViewPrivate::initializeStateSaver()
{
  restoreState();
  connect( mEtm, SIGNAL(modelAboutToBeReset()), this, SLOT(saveState()) );
  connect( mEtm, SIGNAL(modelReset()), this, SLOT(restoreState()) );
}

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

void KDeclarativeMainViewPrivate::filterLineEditChanged( const QString &text )
{
  if ( !text.isEmpty() ) {
    mFilterLineEdit->setFixedHeight( 40 );
    mFilterLineEdit->show();
    mFilterLineEdit->setFocus();
  } else if ( text.isEmpty() ) {
    mFilterLineEdit->setFixedHeight( 0 );
    mFilterLineEdit->hide();
  }
}

void KDeclarativeMainViewPrivate::bulkActionFilterLineEditChanged( const QString &text )
{
  if ( !text.isEmpty() ) {
    mBulkActionFilterLineEdit->setFixedHeight( 40 );
    mBulkActionFilterLineEdit->show();
    mBulkActionFilterLineEdit->setFocus();
  } else if ( text.isEmpty() ) {
    mBulkActionFilterLineEdit->setFixedHeight( 0 );
    mBulkActionFilterLineEdit->hide();
  }
}

void KDeclarativeMainViewPrivate::searchStarted( const Akonadi::Collection &searchCollection )
{
  q->persistCurrentSelection( QLatin1String("SelectionBeforeSearchStarted") );

  const QStringList selection = QStringList() << QLatin1String( "c1" ) // the 'Search' collection
                                              << QString::fromLatin1( "c%1" ).arg( searchCollection.id() );
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

  q->restorePersistedSelection( QLatin1String("SelectionBeforeSearchStarted") );
  q->clearPersistedSelection( QLatin1String("SelectionBeforeSearchStarted") );
}

void KDeclarativeMainViewPrivate::guiStateChanged( int oldState, int newState )
{
  /**
   * If we come back from the BulkActionScreen and we had a filter string
   * entered before we entered the BulkActionScreen, we'll refresh this
   * filter string now.
   */
  if ( oldState == GuiStateManager::BulkActionScreenState ) {
    if ( newState == GuiStateManager::AccountScreenState ||
         newState == GuiStateManager::SingleFolderScreenState ||
         newState == GuiStateManager::MultipleFolderScreenState ) {

      KLineEdit *lineEdit = mFilterLineEdit.data();
      if ( lineEdit && mItemFilterModel ) {
        const QString text = lineEdit->text();
        if ( text.isEmpty() ) {
          // just trigger a refresh of the item view
          QMetaObject::invokeMethod( mItemFilterModel, "setFilterString", Qt::DirectConnection, Q_ARG( QString, text ) );
        } else {
          // trigger a refresh of the line edit and item view
          lineEdit->clear();
          lineEdit->setText( text );
        }
      }
    }
  }
}

void KDeclarativeMainViewPrivate::openHtml( const QString &path )
{
  q->openAttachment( path, QLatin1String( "text/html" ) );
}

DeclarativeBulkActionFilterLineEdit::DeclarativeBulkActionFilterLineEdit( QGraphicsItem *parent )
  : DeclarativeWidgetBase<KLineEdit, KDeclarativeMainView, &KDeclarativeMainView::setBulkActionFilterLineEdit>( parent )
{
}

DeclarativeBulkActionFilterLineEdit::~DeclarativeBulkActionFilterLineEdit()
{
}

void DeclarativeBulkActionFilterLineEdit::clear()
{
  widget()->clear();
}

void KDeclarativeMainViewPrivate::configureAgentInstance()
{
  if (mAgentInstanceSelectionModel->selectedRows().isEmpty())
    return;
  Akonadi::AgentInstance instance = mAgentInstanceSelectionModel->selectedRows().first().data( Akonadi::AgentInstanceModel::InstanceRole ).value<Akonadi::AgentInstance>();

  instance.configure( q );
}
