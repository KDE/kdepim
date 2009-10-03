/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "pane.h"

#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KMenu>

#include <QtCore/QAbstractItemModel>
#include <QtGui/QAbstractProxyModel>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QTabBar>
#include <QtGui/QToolButton>

#include "storagemodel.h"
#include "widget.h"
#include "core/settings.h"

namespace MessageList
{

class Pane::Private
{
public:
  Private( Pane *owner )
    : q( owner ) { }

  void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
  void onNewTabClicked();
  void onCloseTabClicked();
  void onCurrentTabChanged();
  void onTabContextMenuRequest( const QPoint &pos );

  QItemSelection mapSelectionToSource( const QItemSelection &selection ) const;
  QItemSelection mapSelectionFromSource( const QItemSelection &selection ) const;
  void updateTabControls();

  Pane * const q;

  QAbstractItemModel *mModel;
  QItemSelectionModel *mSelectionModel;

  QHash<Widget*, QItemSelectionModel*> mWidgetSelectionHash;
  QList<const QAbstractProxyModel*> mProxyStack;

  QToolButton *mNewTabButton;
  QToolButton *mCloseTabButton;

};

} // namespace MessageList

using namespace MessageList;


Pane::Pane( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent )
  : QTabWidget( parent ), d( new Private( this ) )
{
  d->mModel = model;
  d->mSelectionModel = selectionModel;

  // Build the proxy stack
  const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel*>( d->mSelectionModel->model() );

  while (proxyModel) {
    if (static_cast<const QAbstractItemModel*>(proxyModel) == d->mModel) {
      break;
    }

    d->mProxyStack << proxyModel;
    const QAbstractProxyModel *nextProxyModel = qobject_cast<const QAbstractProxyModel*>(proxyModel->sourceModel());

    if (!nextProxyModel) {
      // It's the final model in the chain, so it is necessarily the sourceModel.
      Q_ASSERT(qobject_cast<const QAbstractItemModel*>(proxyModel->sourceModel()) == d->mModel);
      break;
    }
    proxyModel = nextProxyModel;
  } // Proxy stack done

  d->mNewTabButton = new QToolButton( this );
  d->mNewTabButton->setIcon( KIcon( "tab-new" ) );
  d->mNewTabButton->adjustSize();
  d->mNewTabButton->setToolTip( i18nc("@info:tooltip", "Open a new tab"));
  setCornerWidget( d->mNewTabButton, Qt::TopLeftCorner );
  connect( d->mNewTabButton, SIGNAL( clicked() ),
           SLOT( onNewTabClicked() ) );

  d->mCloseTabButton = new QToolButton( this );
  d->mCloseTabButton->setIcon( KIcon( "tab-close" ) );
  d->mCloseTabButton->adjustSize();
  d->mCloseTabButton->setToolTip( i18nc("@info:tooltip", "Close the current tab"));
  setCornerWidget( d->mCloseTabButton, Qt::TopRightCorner );
  connect( d->mCloseTabButton, SIGNAL( clicked() ),
           SLOT( onCloseTabClicked() ) );

  createNewTab();
  setMovable( true );

  connect( d->mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
           this, SLOT(onSelectionChanged(QItemSelection, QItemSelection)) );
  connect( this, SIGNAL(currentChanged(int)),
           this, SLOT(onCurrentTabChanged()) );

  setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL(customContextMenuRequested(QPoint)),
           this, SLOT(onTabContextMenuRequest(QPoint)) );

  connect( Core::Settings::self(), SIGNAL(configChanged()),
           this, SLOT(updateTabControls()) );
}

Pane::~Pane()
{
  delete d;
}

bool Pane::selectNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                  MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                  bool centerItem,
                                  bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    return w->selectNextMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
  } else {
    return false;
  }
}

bool Pane::selectPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                      MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                      bool centerItem,
                                      bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    return w->selectPreviousMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
  } else {
    return false;
  }
}

bool Pane::focusNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    return w->focusNextMessageItem( messageTypeFilter, centerItem, loop );
  } else {
    return false;
  }
}

bool Pane::focusPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    return w->focusNextMessageItem( messageTypeFilter, centerItem, loop );
  } else {
    return false;
  }
}

void Pane::selectFocusedMessageItem( bool centerItem )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->selectFocusedMessageItem( centerItem );
  }
}

bool Pane::selectFirstMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    return w->selectFirstMessageItem( messageTypeFilter, centerItem );
  } else {
    return false;
  }
}

void Pane::selectAll()
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->selectAll();
  }
}


void Pane::setCurrentThreadExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->setCurrentThreadExpanded(expand );
  }
}

void Pane::setAllThreadsExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->setAllThreadsExpanded( expand );
  }
}

void Pane::setAllGroupsExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->setAllGroupsExpanded(expand);
  }
}


void Pane::Private::onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Widget *w = static_cast<Widget*>( q->currentWidget() );
  QItemSelectionModel *s = mWidgetSelectionHash[w];

  s->select( mapSelectionToSource( selected ), QItemSelectionModel::Select );
  s->select( mapSelectionToSource( deselected ), QItemSelectionModel::Deselect );

  QString label;
  QIcon icon = KIcon( "folder" );
  foreach ( const QModelIndex &index, s->selectedRows() ) {
    label+= index.data( Qt::DisplayRole ).toString()+", ";
  }
  label.chop( 2 );

  if ( label.isEmpty() ) {
    label = i18nc( "@title:tab Empty messagelist", "Empty" );
    icon = QIcon();
  } else if ( s->selectedRows().size()==1 ) {
    icon = s->selectedRows().first().data( Qt::DecorationRole ).value<QIcon>();
  }

  int index = q->indexOf( w );
  q->setTabText( index, label );
  q->setTabIcon( index, icon );
}

void Pane::Private::onNewTabClicked()
{
  q->createNewTab();
  updateTabControls();
}

void Pane::Private::onCloseTabClicked()
{
  Widget *w = static_cast<Widget*>( q->currentWidget() );
  if ( !w || (q->count() < 2) ) {
    return;
  }

  delete w;
  updateTabControls();
}

void Pane::Private::onCurrentTabChanged()
{
  Widget *w = static_cast<Widget*>( q->currentWidget() );
  QItemSelectionModel *s = mWidgetSelectionHash[w];

  disconnect( mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
              q, SLOT(onSelectionChanged(QItemSelection, QItemSelection)) );

  mSelectionModel->select( mapSelectionFromSource( s->selection() ),
                           QItemSelectionModel::ClearAndSelect );

  connect( mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
           q, SLOT(onSelectionChanged(QItemSelection, QItemSelection)) );
}

void Pane::Private::onTabContextMenuRequest( const QPoint &pos )
{
  QTabBar *bar = q->tabBar();
  int index = bar->tabAt( bar->mapFrom( q, pos ) );
  if ( index == -1 ) return;

  Widget *w = qobject_cast<Widget *>( q->widget( index ) );
  if ( !w ) return;

  KMenu menu( q );
  QAction *action;

  action = menu.addAction( i18nc( "@action:inmenu", "Close Tab" ) );
  action->setEnabled( q->count() > 1 );
  action->setIcon( KIcon( "tab-close" ) );
  connect( action, SIGNAL(triggered(bool)),
           q, SLOT(onCloseTabClicked()) ); // Reuse the logic...

  QAction *allOther = menu.addAction( i18nc("@action:inmenu", "Close All Other Tabs" ) );
  action->setEnabled( q->count() > 1 );
  action->setIcon( KIcon( "tab-close-other" ) );

  action = menu.exec( q->mapToGlobal( pos ) );

  if ( action == allOther ) { // Close all other tabs
    QList<Widget *> widgets;
    int index = q->indexOf( w );

    for ( int i=0; i<q->count(); i++ ) {
      if ( i==index) continue; // Skip the current one

      Widget *other = qobject_cast<Widget *>( q->widget( i ) );
      widgets << other;
    }

    foreach ( Widget *other, widgets ) {
      delete other;
    }

    updateTabControls();
  }
}

void Pane::createNewTab()
{
  Widget * w = new Widget( this );
  addTab( w, i18nc( "@title:tab Empty messagelist", "Empty" ) );

  QItemSelectionModel *s = new QItemSelectionModel( d->mModel, w );
  MessageList::StorageModel *m = new MessageList::StorageModel( d->mModel, s, w );
  w->setStorageModel( m );

  d->mWidgetSelectionHash[w] = s;

  connect( w, SIGNAL(messageSelected(Akonadi::Item)),
           this, SIGNAL(messageSelected(Akonadi::Item)) );
  connect( w, SIGNAL(messageActivated(Akonadi::Item)),
           this, SIGNAL(messageActivated(Akonadi::Item)) );
  connect( w, SIGNAL(selectionChanged()),
           this, SIGNAL(selectionChanged()) );
  connect( w, SIGNAL(messageStatusChangeRequest(Akonadi::Item, KPIM::MessageStatus, KPIM::MessageStatus)),
           this, SIGNAL(messageStatusChangeRequest(Akonadi::Item, KPIM::MessageStatus, KPIM::MessageStatus)) );

  connect( w, SIGNAL( fullSearchRequest() ), this, SIGNAL( fullSearchRequest() ) );
  d->updateTabControls();
  setCurrentWidget( w );
}

QItemSelection Pane::Private::mapSelectionToSource( const QItemSelection &selection ) const
{
  QItemSelection result = selection;

  foreach ( const QAbstractProxyModel *proxy, mProxyStack ) {
    result = proxy->mapSelectionToSource( result );
  }

  return result;
}

QItemSelection Pane::Private::mapSelectionFromSource( const QItemSelection &selection ) const
{
  QItemSelection result = selection;

  typedef QList<const QAbstractProxyModel*>::ConstIterator Iterator;

  for ( Iterator it = mProxyStack.end()-1; it!=mProxyStack.begin(); --it ) {
    result = (*it)->mapSelectionFromSource( result );
  }
  result = mProxyStack.first()->mapSelectionFromSource( result );

  return result;
}

void Pane::Private::updateTabControls()
{
  mCloseTabButton->setEnabled( q->count()>1 );

  if ( Core::Settings::self()->autoHideTabBarWithSingleTab() ) {
    q->tabBar()->setVisible( q->count()>1 );
  } else {
    q->tabBar()->setVisible( true );
  }
}

#include "pane.moc"
