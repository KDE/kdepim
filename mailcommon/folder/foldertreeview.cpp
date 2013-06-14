/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2009 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "foldertreeview.h"
#include "kernel/mailkernel.h"

#include <Akonadi/CollectionStatistics>
#include <Akonadi/CollectionStatisticsDelegate>
#include <Akonadi/EntityTreeModel>

#include <KConfigGroup>
#include <KDebug>
#include <KGuiItem>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>

#include <QHeaderView>
#include <QMouseEvent>

namespace MailCommon {

FolderTreeView::FolderTreeView( QWidget *parent, bool showUnreadCount )
  : Akonadi::EntityTreeView( parent ),
    mbDisableContextMenuAndExtraColumn( false ),
    mbDisableSaveConfig( false )
{
  init(showUnreadCount);
}

FolderTreeView::FolderTreeView( KXMLGUIClient *xmlGuiClient, QWidget *parent, bool showUnreadCount )
  : Akonadi::EntityTreeView( xmlGuiClient, parent ),
    mbDisableContextMenuAndExtraColumn( false ),
    mbDisableSaveConfig( false )
{
  init( showUnreadCount );
}

FolderTreeView::~FolderTreeView()
{
}

void FolderTreeView::disableSaveConfig()
{
  mbDisableSaveConfig = true;
}

void FolderTreeView::setTooltipsPolicy( FolderTreeWidget::ToolTipDisplayPolicy policy )
{
  if ( mToolTipDisplayPolicy == policy ) {
    return;
  }

  mToolTipDisplayPolicy = policy;
  emit changeTooltipsPolicy( mToolTipDisplayPolicy );
  writeConfig();
}

void FolderTreeView::disableContextMenuAndExtraColumn()
{
  mbDisableContextMenuAndExtraColumn = true;
  const int nbColumn = header()->count();
  for ( int i = 1; i <nbColumn; ++i ) {
    setColumnHidden( i, true );
  }
}

void FolderTreeView::init( bool showUnreadCount )
{
  setIconSize( QSize( 22, 22 ) );
  setUniformRowHeights( true );
  mSortingPolicy = FolderTreeWidget::SortByCurrentColumn;
  mToolTipDisplayPolicy = FolderTreeWidget::DisplayAlways;

  header()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( header(), SIGNAL(customContextMenuRequested(QPoint)),
           SLOT(slotHeaderContextMenuRequested(QPoint)) );

  mCollectionStatisticsDelegate = new Akonadi::CollectionStatisticsDelegate( this );
  mCollectionStatisticsDelegate->setProgressAnimationEnabled( true );
  setItemDelegate(mCollectionStatisticsDelegate);
  mCollectionStatisticsDelegate->setUnreadCountShown(
    showUnreadCount && !header()->isSectionHidden( 1 ) );
}

void FolderTreeView::showStatisticAnimation( bool anim )
{
  mCollectionStatisticsDelegate->setProgressAnimationEnabled( anim );
}

void FolderTreeView::writeConfig()
{
  if ( mbDisableSaveConfig ) {
    return;
  }

  KConfigGroup myGroup( KernelIf->config(), "MainFolderView" );
  myGroup.writeEntry( "IconSize", iconSize().width() );
  myGroup.writeEntry( "ToolTipDisplayPolicy", (int)mToolTipDisplayPolicy );
  myGroup.writeEntry( "SortingPolicy", (int)mSortingPolicy );
}

void FolderTreeView::readConfig()
{
  KConfigGroup myGroup( KernelIf->config(), "MainFolderView" );
  int iIconSize = myGroup.readEntry( "IconSize", iconSize().width() );
  if ( iIconSize < 16 || iIconSize > 32 ) {
    iIconSize = 22;
  }
  setIconSize( QSize( iIconSize, iIconSize ) );
  mToolTipDisplayPolicy =
    static_cast<FolderTreeWidget::ToolTipDisplayPolicy>(
      myGroup.readEntry( "ToolTipDisplayPolicy",
                         static_cast<int>( FolderTreeWidget::DisplayAlways ) ) );

  emit changeTooltipsPolicy( mToolTipDisplayPolicy );

  setSortingPolicy(
    ( FolderTreeWidget::SortingPolicy )myGroup.readEntry(
      "SortingPolicy", (int)FolderTreeWidget::SortByCurrentColumn ), false );
}

void FolderTreeView::slotHeaderContextMenuRequested( const QPoint &pnt )
{
  if ( mbDisableContextMenuAndExtraColumn ) {
    readConfig();
    return;
  }

  // the menu for the columns
  KMenu menu;
  QAction *act;
  menu.addTitle( i18n( "View Columns" ) );
  const int nbColumn = header()->count();
  for ( int i = 1; i <nbColumn; ++i ) {
    act = menu.addAction( model()->headerData( i, Qt::Horizontal ).toString() );
    act->setCheckable( true );
    act->setChecked( !header()->isSectionHidden( i ) );
    act->setData( QVariant( i ) );
    connect( act,  SIGNAL(triggered(bool)),
             SLOT(slotHeaderContextMenuChangeHeader(bool)) );
  }

  menu.addTitle( i18n( "Icon Size" ) );

  static int icon_sizes[] = { 16, 22, 32 /*, 48, 64, 128 */ };

  QActionGroup *grp = new QActionGroup( &menu );
  const int nbElement( (int)( sizeof( icon_sizes ) / sizeof( int ) ) );
  for ( int i = 0; i < nbElement; ++i ) {
    act = menu.addAction( QString( "%1x%2" ).arg( icon_sizes[ i ] ).arg( icon_sizes[ i ] ) );
    act->setCheckable( true );
    grp->addAction( act );
    if ( iconSize().width() == icon_sizes[ i ] ) {
      act->setChecked( true );
    }
    act->setData( QVariant( icon_sizes[ i ] ) );

    connect( act, SIGNAL(triggered(bool)),
             SLOT(slotHeaderContextMenuChangeIconSize(bool)) );
  }
  menu.addTitle( i18n( "Display Tooltips" ) );

  grp = new QActionGroup( &menu );

  act = menu.addAction( i18nc( "@action:inmenu Always display tooltips", "Always" ) );
  act->setCheckable( true );
  grp->addAction( act );
  act->setChecked( mToolTipDisplayPolicy == FolderTreeWidget::DisplayAlways );
  act->setData( QVariant( (int)FolderTreeWidget::DisplayAlways ) );
  connect( act, SIGNAL(triggered(bool)),
           SLOT(slotHeaderContextMenuChangeToolTipDisplayPolicy(bool)) );

  act = menu.addAction( i18nc( "@action:inmenu Never display tooltips.", "Never" ) );
  act->setCheckable( true );
  grp->addAction( act );
  act->setChecked( mToolTipDisplayPolicy == FolderTreeWidget::DisplayNever );
  act->setData( QVariant( (int)FolderTreeWidget::DisplayNever ) );
  connect( act, SIGNAL(triggered(bool)),
           SLOT(slotHeaderContextMenuChangeToolTipDisplayPolicy(bool)) );

  menu.addTitle( i18nc( "@action:inmenu", "Sort Items" ) );

  grp = new QActionGroup( &menu );

  act = menu.addAction( i18nc( "@action:inmenu", "Automatically, by Current Column" ) );
  act->setCheckable( true );
  grp->addAction( act );
  act->setChecked( mSortingPolicy == FolderTreeWidget::SortByCurrentColumn );
  act->setData( QVariant( (int)FolderTreeWidget::SortByCurrentColumn ) );
  connect( act, SIGNAL(triggered(bool)),
           SLOT(slotHeaderContextMenuChangeSortingPolicy(bool)) );

  act = menu.addAction( i18nc( "@action:inmenu", "Manually, by Drag And Drop" ) );
  act->setCheckable( true );
  grp->addAction( act );
  act->setChecked( mSortingPolicy == FolderTreeWidget::SortByDragAndDropKey );
  act->setData( QVariant( (int)FolderTreeWidget::SortByDragAndDropKey ) );
  connect( act, SIGNAL(triggered(bool)),
           SLOT(slotHeaderContextMenuChangeSortingPolicy(bool)) );

  menu.exec( header()->mapToGlobal( pnt ) );
}

void FolderTreeView::slotHeaderContextMenuChangeSortingPolicy( bool )
{
  QAction *act = dynamic_cast< QAction * >( sender() );
  if ( !act ) {
    return;
  }

  QVariant data = act->data();

  bool ok;
  int policy = data.toInt( &ok );
  if ( !ok ) {
    return;
  }

  setSortingPolicy( ( FolderTreeWidget::SortingPolicy )policy, true );
}

void FolderTreeView::setSortingPolicy( FolderTreeWidget::SortingPolicy policy, bool writeInConfig )
{
  if ( mSortingPolicy == policy ) {
    return;
  }

  mSortingPolicy = policy;
  switch ( mSortingPolicy ) {
  case FolderTreeWidget::SortByCurrentColumn:
    header()->setClickable( true );
    header()->setSortIndicatorShown( true );
    setSortingEnabled( true );
    emit manualSortingChanged( false );
    break;

  case FolderTreeWidget::SortByDragAndDropKey:
    header()->setClickable( false );
    header()->setSortIndicatorShown( false );

#if 0
    //
    // Qt 4.5 introduced a nasty bug here:
    // Sorting must be enabled in order to sortByColumn() to work.
    // If sorting is disabled it disconnects some internal signal/slot pairs
    // and calling sortByColumn() silently has no effect.
    // This is a bug as we actually DON'T want automatic sorting to be
    // performed by the view whenever it wants. We want to control sorting.
    //
    setSortingEnabled( true ); // hack for qutie bug: the param here should be false
    header()->setSortIndicator( 0, Qt::AscendingOrder );
#endif
    setSortingEnabled( false ); // hack for qutie bug: this call shouldn't be here at all
    emit manualSortingChanged( true );

    break;
  default:
    // should never happen
    break;
  }
  if ( writeInConfig ) {
    writeConfig();
  }
}

void FolderTreeView::slotHeaderContextMenuChangeToolTipDisplayPolicy( bool )
{
  QAction *act = dynamic_cast< QAction * >( sender() );
  if ( !act ) {
    return;
  }

  QVariant data = act->data();

  bool ok;
  const int id = data.toInt( &ok );
  if ( !ok ) {
    return;
  }
  emit changeTooltipsPolicy( ( FolderTreeWidget::ToolTipDisplayPolicy )id );
}

void FolderTreeView::slotHeaderContextMenuChangeHeader( bool )
{
  QAction *act = dynamic_cast< QAction * >( sender() );
  if ( !act ) {
    return;
  }

  QVariant data = act->data();

  bool ok;
  const int id = data.toInt( &ok );
  if ( !ok ) {
    return;
  }

  if ( id > header()->count() ) {
    return;
  }

  if ( id == 1 ) {
    mCollectionStatisticsDelegate->setUnreadCountShown( !act->isChecked() );
  }

  setColumnHidden( id, !act->isChecked() );
}

void FolderTreeView::slotHeaderContextMenuChangeIconSize( bool )
{
  QAction *act = dynamic_cast< QAction * >( sender() );
  if ( !act ) {
    return;
  }

  QVariant data = act->data();

  bool ok;
  const int size = data.toInt( &ok );
  if ( !ok ) {
    return;
  }

  const QSize newIconSize( QSize( size, size ) );
  if ( newIconSize == iconSize() ) {
    return;
  }
  setIconSize( newIconSize );

  writeConfig();
}

void FolderTreeView::setCurrentModelIndex( const QModelIndex & index )
{
  if ( index.isValid() ) {
    clearSelection();
    scrollTo( index );
    selectionModel()->setCurrentIndex( index, QItemSelectionModel::Rows );
  }
}

void FolderTreeView::selectModelIndex( const QModelIndex & index )
{
  if ( index.isValid() ) {
    scrollTo( index );
    selectionModel()->select(
      index,
      QItemSelectionModel::Rows | QItemSelectionModel::Select |
      QItemSelectionModel::Current | QItemSelectionModel::Clear );
  }
}

void FolderTreeView::slotSelectFocusFolder()
{
  const QModelIndex index = currentIndex();
  if ( index.isValid() ) {
    setCurrentIndex( index );
  }
}

void FolderTreeView::slotFocusNextFolder()
{
  const QModelIndex nextFolder = selectNextFolder( currentIndex() );

  if ( nextFolder.isValid() ) {
    expand( nextFolder );
    setCurrentModelIndex( nextFolder );
  }
}

QModelIndex FolderTreeView::selectNextFolder( const QModelIndex & current )
{
  QModelIndex below;
  if ( current.isValid() ) {
    model()->fetchMore( current );
    if ( model()->hasChildren( current ) ) {
      expand( current );
      below = indexBelow( current );
    } else if ( current.row() < model()->rowCount( model()->parent( current ) ) -1 ) {
      below = model()->index( current.row()+1, current.column(), model()->parent( current ) );
    } else {
      below = indexBelow( current );
    }
  }
  return below;
}

void FolderTreeView::slotFocusPrevFolder()
{
  const QModelIndex current = currentIndex();
  if ( current.isValid() ) {
    QModelIndex above = indexAbove( current );
    setCurrentModelIndex( above );
  }
}

void FolderTreeView::slotFocusFirstFolder()
{
  const QModelIndex first = moveCursor( QAbstractItemView::MoveHome, 0 );
  if ( first.isValid() ) {
    setCurrentModelIndex( first );
  }
}

void FolderTreeView::slotFocusLastFolder()
{
  const QModelIndex last = moveCursor( QAbstractItemView::MoveEnd, 0 );
  if ( last.isValid() ) {
    setCurrentModelIndex( last );
  }
}


void FolderTreeView::selectNextUnreadFolder( bool confirm )
{
  // find next unread collection starting from current position
  if ( !trySelectNextUnreadFolder( currentIndex(), MailCommon::Util::ForwardSearch, confirm ) ) {
    // if there is none, jump to the last collection and try again
    trySelectNextUnreadFolder( model()->index( 0, 0 ), MailCommon::Util::ForwardSearch, confirm );
  }
}

// helper method to find last item in the model tree
static QModelIndex lastChildOf( QAbstractItemModel *model, const QModelIndex &current )
{
  if ( model->rowCount( current ) == 0 ) {
    return current;
  }

  return lastChildOf( model, model->index( model->rowCount( current ) - 1, 0, current ) );
}

void FolderTreeView::selectPrevUnreadFolder( bool confirm )
{
  // find next unread collection starting from current position
  if ( !trySelectNextUnreadFolder( currentIndex(), MailCommon::Util::BackwardSearch, confirm ) ) {
    // if there is none, jump to top and try again
    const QModelIndex index = lastChildOf( model(), QModelIndex() );
    trySelectNextUnreadFolder( index, MailCommon::Util::BackwardSearch, confirm );
  }
}

bool FolderTreeView::trySelectNextUnreadFolder( const QModelIndex &current,
                                                MailCommon::Util::SearchDirection direction,
                                                bool confirm )
{
  QModelIndex index = current;
  while ( true ) {
    index = MailCommon::Util::nextUnreadCollection( model(), index, direction );

    if ( !index.isValid() ) {
      return false;
    }

    const Akonadi::Collection collection =
      index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection == Kernel::self()->trashCollectionFolder() ||
        collection == Kernel::self()->outboxCollectionFolder() )
      continue;

    if ( ignoreUnreadFolder( collection, confirm ) ) {
      continue;
    }

    if ( allowedToEnterFolder( collection, confirm ) ) {
      expand( index );
      setCurrentIndex( index );
      selectModelIndex( index );
      return true;
    } else {
      return false;
    }
  }

  return false;
}

bool FolderTreeView::ignoreUnreadFolder( const Akonadi::Collection &collection, bool confirm ) const
{
  if ( !confirm ) {
    return false;
  }

  // Skip drafts, sent mail and templates as well, when reading mail with the
  // space bar - but not when changing into the next folder with unread mail
  // via ctrl+ or ctrl- so we do this only if (confirm == true), which means
  // we are doing readOn.

  return ( collection == Kernel::self()->draftsCollectionFolder() ||
           collection == Kernel::self()->templatesCollectionFolder() ||
           collection == Kernel::self()->sentCollectionFolder() );
}

bool FolderTreeView::allowedToEnterFolder( const Akonadi::Collection &collection,
                                           bool confirm ) const
{
  if ( !confirm ) {
    return true;
  }

  // warn user that going to next folder - but keep track of
  // whether he wishes to be notified again in "AskNextFolder"
  // parameter (kept in the config file for kmail)
  const int result =
    KMessageBox::questionYesNo(
      const_cast<FolderTreeView*>( this ),
      i18n( "<qt>Go to the next unread message in folder <b>%1</b>?</qt>", collection.name() ),
      i18n( "Go to Next Unread Message" ),
      KGuiItem( i18n( "Go To" ) ),
      KGuiItem( i18n( "Do Not Go To" ) ), // defaults
      ":kmail_AskNextFolder", 0 );

  return ( result == KMessageBox::Yes );
}

bool FolderTreeView::isUnreadFolder( const QModelIndex &current,
                                     QModelIndex &index, FolderTreeView::Move move,
                                     bool confirm )
{
  if ( current.isValid() ) {

    if ( move == FolderTreeView::Next ) {
      index = selectNextFolder( current );
    } else if ( move == FolderTreeView::Previous ) {
      index = indexAbove( current );
    }

    if ( index.isValid() ) {
      const Akonadi::Collection collection =
        index.model()->data(
          current, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();

      if ( collection.isValid() ) {
        if ( collection.statistics().unreadCount() > 0 ) {
          if ( !confirm ) {
            selectModelIndex( current );
            return true;
          } else {
            // Skip drafts, sent mail and templates as well, when reading mail with the
            // space bar - but not when changing into the next folder with unread mail
            // via ctrl+ or ctrl- so we do this only if (confirm == true), which means
            // we are doing readOn.

            if ( collection == Kernel::self()->draftsCollectionFolder() ||
                 collection == Kernel::self()->templatesCollectionFolder() ||
                 collection == Kernel::self()->sentCollectionFolder() ) {
              return false;
            }

            // warn user that going to next folder - but keep track of
            // whether he wishes to be notified again in "AskNextFolder"
            // parameter (kept in the config file for kmail)
            if ( KMessageBox::questionYesNo(
                   this,
                   i18n( "<qt>Go to the next unread message in folder <b>%1</b>?</qt>",
                         collection.name() ),
                   i18n( "Go to Next Unread Message" ),
                   KGuiItem( i18n( "Go To" ) ),
                   KGuiItem( i18n( "Do Not Go To" ) ), // defaults
                   ":kmail_AskNextFolder",
                   0 ) == KMessageBox::No ) {
              return true; // assume selected (do not continue looping)
            }

            selectModelIndex( current );
            return true;
          }
        }
      }
    }
  }
  return false;
}

Akonadi::Collection FolderTreeView::currentFolder() const
{
  const QModelIndex current = currentIndex();
  if ( current.isValid() ) {
    const Akonadi::Collection collection =
      current.model()->data(
        current,
        Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    return collection;
  }
  return Akonadi::Collection();
}

void FolderTreeView::mousePressEvent( QMouseEvent *e )
{
  const bool buttonPressedIsMiddle = ( e->button() == Qt::MidButton );
  emit prefereCreateNewTab( buttonPressedIsMiddle );
  EntityTreeView::mousePressEvent( e );
}

void FolderTreeView::restoreHeaderState( const QByteArray &data )
{
  if (data.isEmpty()) {
    const int nbColumn = header()->count();
    for ( int i = 1; i <nbColumn; ++i ) {
      setColumnHidden( i, true );
    }
  }
  else
    header()->restoreState( data );
  mCollectionStatisticsDelegate->setUnreadCountShown( header()->isSectionHidden( 1 ) );
}

void FolderTreeView::updatePalette()
{
  mCollectionStatisticsDelegate->updatePalette();
}

}

#include "foldertreeview.moc"
