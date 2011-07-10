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

#include <KDE/KActionCollection>
#include <KDE/KActionMenu>
#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KMenu>
#include <KDE/KXMLGUIClient>

#include <QtCore/QAbstractItemModel>
#include <QtGui/QAbstractProxyModel>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QTabBar>
#include <QtGui/QToolButton>
#include <QtGui/QMouseEvent>

#include "storagemodel.h"
#include "widget.h"
#include "core/settings.h"
#include "core/manager.h"
#include <akonadi/kmime/messagestatus.h>
#include "core/model.h"

namespace MessageList
{

class Pane::Private
{
public:
  Private( Pane *owner )
    : q( owner ), mXmlGuiClient( 0 ), mActionMenu( 0 ), mPreferEmptyTab( false ) { }

  void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
  void onNewTabClicked();
  void onCloseTabClicked();
  void closeTab( QWidget * );
  void onCurrentTabChanged();
  void onTabContextMenuRequest( const QPoint &pos );

  QItemSelection mapSelectionToSource( const QItemSelection &selection ) const;
  QItemSelection mapSelectionFromSource( const QItemSelection &selection ) const;
  void updateTabControls();

  Pane * const q;

  KXMLGUIClient *mXmlGuiClient;
  KActionMenu *mActionMenu;

  QAbstractItemModel *mModel;
  QItemSelectionModel *mSelectionModel;

  QHash<Widget*, QItemSelectionModel*> mWidgetSelectionHash;
  QList<const QAbstractProxyModel*> mProxyStack;

  QToolButton *mNewTabButton;
  QToolButton *mCloseTabButton;
  bool mPreferEmptyTab;
};

} // namespace MessageList

using namespace Akonadi;
using namespace MessageList;


Pane::Pane( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent )
  : KTabWidget( parent ), d( new Private( this ) )
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
  d->mNewTabButton->setIcon( KIcon( QLatin1String( "tab-new" ) ) );
  d->mNewTabButton->adjustSize();
  d->mNewTabButton->setToolTip( i18nc("@info:tooltip", "Open a new tab"));
  setCornerWidget( d->mNewTabButton, Qt::TopLeftCorner );
  connect( d->mNewTabButton, SIGNAL( clicked() ),
           SLOT( onNewTabClicked() ) );

  d->mCloseTabButton = new QToolButton( this );
  d->mCloseTabButton->setIcon( KIcon( QLatin1String( "tab-close" ) ) );
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

  connect( this, SIGNAL(mouseMiddleClick( QWidget * )),
           this, SLOT(closeTab( QWidget * )) );
  tabBar()->installEventFilter( this );
}

Pane::~Pane()
{
  delete d;
}

void Pane::setXmlGuiClient( KXMLGUIClient *xmlGuiClient )
{
  d->mXmlGuiClient = xmlGuiClient;

  for ( int i=0; i<count(); i++ ) {
    Widget *w = qobject_cast<Widget *>( widget( i ) );
    w->setXmlGuiClient( d->mXmlGuiClient );
  }

  // Setup "View->Message List" actions.
  if ( xmlGuiClient ) {
    if ( d->mActionMenu ) {
      d->mXmlGuiClient->actionCollection()->removeAction( d->mActionMenu );
    }
    d->mActionMenu = new KActionMenu( KIcon(), i18n( "Message List" ), this );
    d->mXmlGuiClient->actionCollection()->addAction( QLatin1String( "view_message_list" ), d->mActionMenu );
    const Widget * const w = static_cast<Widget*>( currentWidget() );
    w->view()->fillViewMenu( d->mActionMenu->menu() );

    KAction *action = new KAction( i18n("Create new tab"), this );
    action->setShortcut( QKeySequence( Qt::ALT + Qt::Key_T ) );
    d->mXmlGuiClient->actionCollection()->addAction( QLatin1String( "create_new_tab" ), action );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( onNewTabClicked() ) );
    d->mActionMenu->addAction( action );
  }
}

bool Pane::selectNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                  MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                  bool centerItem,
                                  bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return true;

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
    if ( w->view()->model()->isLoading() )
      return true;

    return w->selectPreviousMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
  } else {
    return false;
  }
}

bool Pane::focusNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return true;

    return w->focusNextMessageItem( messageTypeFilter, centerItem, loop );
  } else {
    return false;
  }
}

bool Pane::focusPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return true;

    return w->focusPreviousMessageItem( messageTypeFilter, centerItem, loop );
  } else {
    return false;
  }
}

void Pane::selectFocusedMessageItem( bool centerItem )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return;

    w->selectFocusedMessageItem( centerItem );
  }
}

bool Pane::selectFirstMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return true;

    return w->selectFirstMessageItem( messageTypeFilter, centerItem );
  } else {
    return false;
  }
}

void Pane::selectAll()
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return;

    w->selectAll();
  }
}


void Pane::setCurrentThreadExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return;

    w->setCurrentThreadExpanded(expand );
  }
}

void Pane::setAllThreadsExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return;

    w->setAllThreadsExpanded( expand );
  }
}

void Pane::setAllGroupsExpanded( bool expand )
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    if ( w->view()->model()->isLoading() )
      return;

    w->setAllGroupsExpanded(expand);
  }
}

void Pane::focusQuickSearch()
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w ) {
    w->focusQuickSearch();
  }
}

void Pane::Private::onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Widget *w = static_cast<Widget*>( q->currentWidget() );
  QItemSelectionModel *s = mWidgetSelectionHash[w];

  s->select( mapSelectionToSource( selected ), QItemSelectionModel::Select );
  s->select( mapSelectionToSource( deselected ), QItemSelectionModel::Deselect );

  if ( mPreferEmptyTab ) {
    q->createNewTab();
    w = static_cast<Widget*>( q->currentWidget() );
    s = mWidgetSelectionHash[w];
  }

  QString label;
  QIcon icon = KIcon( QLatin1String( "folder" ) );
  QString toolTip;
  foreach ( const QModelIndex &index, s->selectedRows() ) {
    label+= index.data( Qt::DisplayRole ).toString()+QLatin1String( ", " );
  }
  label.chop( 2 );

  if ( label.isEmpty() ) {
    label = i18nc( "@title:tab Empty messagelist", "Empty" );
    icon = QIcon();
  } else if ( s->selectedRows().size()==1 ) {
    icon = s->selectedRows().first().data( Qt::DecorationRole ).value<QIcon>();
    QModelIndex idx = s->selectedRows().first().parent();
    toolTip = label;
    while ( idx != QModelIndex() ) {
      toolTip = idx.data().toString() + QLatin1Char( '/' ) + toolTip;
      idx = idx.parent();
    }
  }

  int index = q->indexOf( w );
  q->setTabText( index, label );
  q->setTabIcon( index, icon );
  q->setTabToolTip( index, toolTip);
}

void Pane::Private::onNewTabClicked()
{
  q->createNewTab();
}

void Pane::Private::onCloseTabClicked()
{
  closeTab( q->currentWidget() );
}

void Pane::Private::closeTab( QWidget *w )
{
  if ( !w || (q->count() < 2) ) {
    return;
  }

  delete w;
  updateTabControls();
}

bool Pane::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::MouseButtonPress ) {
    QMouseEvent * const mouseEvent = static_cast<QMouseEvent *>( event );
    if ( mouseEvent->button() == Qt::MidButton ) {
      return true;
    }
  }
  return KTabWidget::eventFilter( object, event );
}

void Pane::Private::onCurrentTabChanged()
{
  emit q->currentTabChanged();

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
  action->setIcon( KIcon( QLatin1String( "tab-close" ) ) );
  connect( action, SIGNAL(triggered(bool)),
           q, SLOT(onCloseTabClicked()) ); // Reuse the logic...

  QAction *allOther = menu.addAction( i18nc("@action:inmenu", "Close All Other Tabs" ) );
  allOther->setEnabled( q->count() > 1 );
  allOther->setIcon( KIcon( QLatin1String( "tab-close-other" ) ) );

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

MessageList::StorageModel *Pane::createStorageModel( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent )
{
  return new MessageList::StorageModel( model, selectionModel, parent );
}

void Pane::setCurrentFolder( const Akonadi::Collection &, bool, Core::PreSelectionMode preSelectionMode, const QString &overrideLabel )
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    QItemSelectionModel *s = d->mWidgetSelectionHash[w];
    MessageList::StorageModel *m = createStorageModel( d->mModel, s, w );
    w->setStorageModel( m, preSelectionMode );
    if ( !overrideLabel.isEmpty() ) {
       int index = indexOf( w );
       setTabText( index, overrideLabel );
    }
  }
}

void Pane::createNewTab()
{
  Widget * w = new Widget( this );
  w->setXmlGuiClient( d->mXmlGuiClient );
  addTab( w, i18nc( "@title:tab Empty messagelist", "Empty" ) );

  QItemSelectionModel *s = new QItemSelectionModel( d->mModel, w );
  MessageList::StorageModel *m = createStorageModel( d->mModel, s, w );
  w->setStorageModel( m );

  d->mWidgetSelectionHash[w] = s;

  connect( w, SIGNAL(messageSelected(Akonadi::Item)),
           this, SIGNAL(messageSelected(Akonadi::Item)) );
  connect( w, SIGNAL(messageActivated(Akonadi::Item)),
           this, SIGNAL(messageActivated(Akonadi::Item)) );
  connect( w, SIGNAL(selectionChanged()),
           this, SIGNAL(selectionChanged()) );
  connect( w, SIGNAL(messageStatusChangeRequest(Akonadi::Item, Akonadi::MessageStatus, Akonadi::MessageStatus)),
           this, SIGNAL(messageStatusChangeRequest(Akonadi::Item, Akonadi::MessageStatus, Akonadi::MessageStatus)) );

  connect( w, SIGNAL(statusMessage(QString)),
           this, SIGNAL(statusMessage(QString)) );

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

Item Pane::currentItem() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w == 0 ) {
    return Item();
  }

  return w->currentItem();
}

KMime::Message::Ptr Pane::currentMessage() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );

  if ( w == 0 ) {
    return KMime::Message::Ptr();
  }

  return w->currentMessage();
}

QList<KMime::Message::Ptr > Pane::selectionAsMessageList( bool includeCollapsedChildren ) const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w == 0 ) {
    return QList<KMime::Message::Ptr>();
  }
  return w->selectionAsMessageList( includeCollapsedChildren );
}

QList<Akonadi::Item> Pane::selectionAsMessageItemList( bool includeCollapsedChildren ) const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w == 0 ) {
    return QList<Akonadi::Item>();
  }
  return w->selectionAsMessageItemList( includeCollapsedChildren );
}


QList<Akonadi::Item> Pane::currentThreadAsMessageList() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w == 0 ) {
    return QList<Akonadi::Item>();
  }
  return w->currentThreadAsMessageList();
}

QList<Akonadi::Item> Pane::itemListFromPersistentSet( MessageList::Core::MessageItemSetReference ref )
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    return w->itemListFromPersistentSet(ref);
  }
  return QList<Akonadi::Item>();
}

void Pane::deletePersistentSet( MessageList::Core::MessageItemSetReference ref )
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    w->deletePersistentSet( ref );
  }
}

void Pane::markMessageItemsAsAboutToBeRemoved( MessageList::Core::MessageItemSetReference ref, bool bMark )
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    w->markMessageItemsAsAboutToBeRemoved( ref, bMark );
  }
}

Akonadi::MessageStatus Pane::currentFilterStatus() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w == 0 ) {
    return Akonadi::MessageStatus();
  }
  return w->currentFilterStatus();
}

QString Pane::currentFilterSearchString() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    return w->currentFilterSearchString();
  }
  return QString();
}

bool Pane::isThreaded() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    return w->isThreaded();
  }
  return false;
}

bool Pane::selectionEmpty() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    return w->selectionEmpty();
  }
  return false;
}

bool Pane::getSelectionStats( Akonadi::Item::List &selectedItems,
                              Akonadi::Item::List &selectedVisibleItems,
                              bool * allSelectedBelongToSameThread,
                              bool includeCollapsedChildren ) const
{
  Widget * w = static_cast<Widget*>( currentWidget() );
  if ( w == 0 ) {
    return false;
  }

  return w->getSelectionStats(
      selectedItems, selectedVisibleItems,
      allSelectedBelongToSameThread, includeCollapsedChildren
    );
}

MessageList::Core::MessageItemSetReference Pane::selectionAsPersistentSet( bool includeCollapsedChildren ) const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w )
    return w->selectionAsPersistentSet( includeCollapsedChildren );
  return -1;
}

MessageList::Core::MessageItemSetReference Pane::currentThreadAsPersistentSet() const
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w )
    return w->currentThreadAsPersistentSet();
  return -1;
}

void Pane::focusView()
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    QWidget *view = w->view();
    if ( view )
      view->setFocus();
  }
}

void Pane::reloadGlobalConfiguration()
{
  d->updateTabControls();

  Core::Settings::self()->writeConfig();
}

QItemSelectionModel* Pane::currentItemSelectionModel()
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w )
    return w->view()->selectionModel();
  return 0;
}

void Pane::resetModelStorage()
{
  Widget *w = static_cast<Widget*>( currentWidget() );
  if ( w ) {
    MessageList::StorageModel *m = static_cast<MessageList::StorageModel*>( w->storageModel() );
    if ( m )
      m->resetModelStorage();
  }
}

void Pane::setPreferEmptyTab( bool emptyTab )
{
  d->mPreferEmptyTab = emptyTab;
}

#include "pane.moc"
