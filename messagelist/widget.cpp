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

#include "widget.h"

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemcopyjob.h>
#include <akonadi/itemmovejob.h>

#include "storagemodel.h"
#include "core/messageitem.h"
#include "core/view.h"
#include <core/settings.h>

#include <QtCore/QTimer>
#include <QAction>
#include <QApplication>
#include <QDrag>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <KDE/KActionCollection>
#include <KDE/KComboBox>
#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KIconLoader>
#include <KDE/KLocale>
#include <KDE/KMenu>
#include <KDE/KToggleAction>
#include <KDE/KXMLGUIClient>
#include <KDE/KXMLGUIFactory>

#include <Nepomuk2/Tag>
#include "core/groupheaderitem.h"

#include <Nepomuk2/ResourceWatcher>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/ResourceWatcher>
#include <soprano/nao.h>


namespace MessageList
{

class Widget::Private
{
public:
  Private( Widget *owner )
    : q( owner ), mLastSelectedMessage(-1), mXmlGuiClient( 0 ) { }

  Akonadi::Item::List selectionAsItems() const;
  Akonadi::Item itemForRow( int row ) const;
  KMime::Message::Ptr messageForRow( int row ) const;

  Widget * const q;

  int mLastSelectedMessage;
  KXMLGUIClient *mXmlGuiClient;
  QModelIndex mGroupHeaderItemIndex;

};

} // namespace MessageList

using namespace MessageList;
using namespace Akonadi;

Widget::Widget( QWidget *parent )
  : Core::Widget( parent ), d( new Private( this ) )
{
  populateStatusFilterCombo();

  Nepomuk2::ResourceWatcher *watcher = new Nepomuk2::ResourceWatcher(this);
  watcher->addType(Soprano::Vocabulary::NAO::Tag());
  connect(watcher, SIGNAL(resourceCreated(Nepomuk2::Resource,QList<QUrl>)),
          this, SLOT(populateStatusFilterCombo()));
  connect(watcher, SIGNAL(resourceRemoved(QUrl,QList<QUrl>)),
          this, SLOT(populateStatusFilterCombo()));
  connect(watcher, SIGNAL(propertyChanged(Nepomuk2::Resource,Nepomuk2::Types::Property,QVariantList,QVariantList)),
          this, SLOT(populateStatusFilterCombo()));
  watcher->start();
}

Widget::~Widget()
{
  delete d;
}

void Widget::setXmlGuiClient( KXMLGUIClient *xmlGuiClient )
{
  d->mXmlGuiClient = xmlGuiClient;
}

bool Widget::canAcceptDrag( const QDropEvent * e )
{
  if ( e->source() == view()->viewport() )
    return false;

  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();
  if ( collections.size()!=1 )
    return false; // no folder here or too many (in case we can't decide where the drop will end)

  const Collection target = collections.first();

  if ( ( target.rights() & Collection::CanCreateItem ) == 0 )
    return false; // no way to drag into

  const KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
  foreach ( const KUrl &url, urls ) {
    const Collection collection = Collection::fromUrl( url );
    if ( collection.isValid() ) { // You're not supposed to drop collections here
      return false;
    } else { // Yay, this is an item!
      const QString type = url.queryItems()[QLatin1String( "type" )]; // But does it have the right type?
      if ( !target.contentMimeTypes().contains( type ) ) {
        return false;
      }
    }
  }

  return true;
}

bool Widget::selectNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                    MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                    bool centerItem,
                                    bool loop )
{
  return view()->selectNextMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
}

bool Widget::selectPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                        MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                        bool centerItem,
                                        bool loop )
{
  return view()->selectPreviousMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
}

bool Widget::focusNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  return view()->focusNextMessageItem( messageTypeFilter, centerItem, loop );
}

bool Widget::focusPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  return view()->focusPreviousMessageItem( messageTypeFilter, centerItem, loop );
}

void Widget::selectFocusedMessageItem( bool centerItem )
{
  view()->selectFocusedMessageItem( centerItem );
}

bool Widget::selectFirstMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem )
{
  return view()->selectFirstMessageItem( messageTypeFilter, centerItem );
}

void Widget::selectAll()
{
  view()->setAllGroupsExpanded( true );
  view()->selectAll();
}

void Widget::setCurrentThreadExpanded( bool expand )
{
  view()->setCurrentThreadExpanded(expand );
}

void Widget::setAllThreadsExpanded( bool expand )
{
  view()->setAllThreadsExpanded( expand );
}

void Widget::setAllGroupsExpanded( bool expand )
{
  view()->setAllGroupsExpanded(expand);
}

void Widget::focusQuickSearch()
{
  view()->focusQuickSearch();
}


void Widget::fillMessageTagCombo( KComboBox * combo )
{
  KConfigGroup conf( MessageList::Core::Settings::self()->config(),"MessageListView");
  QString tagSelected= conf.readEntry(QLatin1String("TagSelected"));
  if(tagSelected.isEmpty()) {
    return;
  }
  const QStringList tagSelectedLst = tagSelected.split(QLatin1String(","));
  foreach( const Nepomuk2::Tag &nepomukTag, Nepomuk2::Tag::allTags() ) {
    const QString id = nepomukTag.uri().toString();
    if(tagSelectedLst.contains(id)) {
      QString iconName = nepomukTag.genericIcon(); 
      if(iconName.isEmpty()) 
        iconName = QLatin1String( "mail-tagged" ); 
      const QString label = nepomukTag.label();
      const QString id = nepomukTag.uri().toString();
      combo->addItem( SmallIcon( iconName ), label, QVariant( id ) );
    }
  }
}

void Widget::viewMessageSelected( MessageList::Core::MessageItem *msg )
{
  int row = -1;
  if ( msg ) {
    row = msg->currentModelIndexRow();
  }

  if ( !msg || !msg->isValid() || !storageModel() ) {
    d->mLastSelectedMessage = -1;
    emit messageSelected( Item() );
    return;
  }

  Q_ASSERT( row >= 0 );

  d->mLastSelectedMessage = row;

  emit messageSelected( d->itemForRow( row ) ); // this MAY be null
}

void Widget::viewMessageActivated( MessageList::Core::MessageItem *msg )
{
  Q_ASSERT( msg ); // must not be null
  Q_ASSERT( storageModel() );

  if ( !msg->isValid() ) {
    return;
  }

  int row = msg->currentModelIndexRow();
  Q_ASSERT( row >= 0 );

  // The assert below may fail when quickly opening and closing a non-selected thread.
  // This will actually activate the item without selecting it...
  //Q_ASSERT( d->mLastSelectedMessage == row );

  if ( d->mLastSelectedMessage != row ) {
    // Very ugly. We are activating a non selected message.
    // This is very likely a double click on the plus sign near a thread leader.
    // Dealing with mLastSelectedMessage here would be expensive: it would involve releasing the last selected,
    // emitting signals, handling recursion... ugly.
    // We choose a very simple solution: double clicking on the plus sign near a thread leader does
    // NOT activate the message (i.e open it in a toplevel window) if it isn't previously selected.
    return;
  }

  emit messageActivated( d->itemForRow( row ) ); // this MAY be null
}

void Widget::viewSelectionChanged()
{
  emit selectionChanged();
  if ( !currentMessageItem() ) {
    emit messageSelected( Item() );
  }
}

void Widget::viewMessageListContextPopupRequest( const QList< MessageList::Core::MessageItem * > &selectedItems,
                                                 const QPoint &globalPos )
{
  Q_UNUSED( selectedItems );

  if ( !d->mXmlGuiClient )
    return;

  QMenu *popup = static_cast<QMenu*>( d->mXmlGuiClient->factory()->container(
                                        QLatin1String( "akonadi_messagelist_contextmenu" ),
                                        d->mXmlGuiClient ) );
  if ( popup ) {
    popup->exec( globalPos );
  }
}

void Widget::viewMessageStatusChangeRequest( MessageList::Core::MessageItem *msg, const Akonadi::MessageStatus &set, const Akonadi::MessageStatus &clear )
{
  Q_ASSERT( msg ); // must not be null
  Q_ASSERT( storageModel() );

  if ( !msg->isValid() ) {
    return;
  }

  int row = msg->currentModelIndexRow();
  Q_ASSERT( row >= 0 );

  Item item = d->itemForRow( row );
  Q_ASSERT( item.isValid() );

  emit messageStatusChangeRequest( item, set, clear );
}

void Widget::viewGroupHeaderContextPopupRequest( MessageList::Core::GroupHeaderItem *ghi, const QPoint &globalPos )
{
  Q_UNUSED( ghi );

  KMenu menu( this );

  QAction *act;

  QModelIndex index = view()->model()->index( ghi, 0 );
  d->mGroupHeaderItemIndex = index;

  if ( view()->isExpanded( index ) ) {
    act = menu.addAction( i18n ( "Collapse Group" ) );
    connect( act, SIGNAL(triggered(bool)),
             this, SLOT(slotCollapseItem()) );
  } else {
    act = menu.addAction( i18n ( "Expand Group" ) );
    connect( act, SIGNAL(triggered(bool)),
             this, SLOT(slotExpandItem()) );
  }
  
  menu.addSeparator();

  act = menu.addAction( i18n( "Expand All Groups" ) );
  connect( act, SIGNAL(triggered(bool)),
           view(), SLOT(slotExpandAllGroups()) );

  act = menu.addAction( i18n( "Collapse All Groups" ) );
  connect( act, SIGNAL(triggered(bool)),
           view(), SLOT(slotCollapseAllGroups()) );

  menu.exec( globalPos );
}

void Widget::viewDragEnterEvent( QDragEnterEvent *e )
{
  if ( !canAcceptDrag( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
}

void Widget::viewDragMoveEvent( QDragMoveEvent *e )
{
  if ( !canAcceptDrag( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
}

enum DragMode
{
  DragCopy,
  DragMove,
  DragCancel
};

void Widget::viewDropEvent( QDropEvent *e )
{
  if ( !canAcceptDrag( e ) ) {
    e->ignore();
    return;
  }

  KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
  if ( urls.isEmpty() ) {
    kWarning() << "Could not decode drag data!";
    e->ignore();
    return;
  }

  e->accept();

  int action;
  if ( ( e->possibleActions() & Qt::MoveAction ) == 0 ) { // We can't move anyway
    action = DragCopy;
  } else {
    action = DragCancel;
    int keybstate = QApplication::keyboardModifiers();
    if ( keybstate & Qt::CTRL ) {
      action = DragCopy;

    } else if ( keybstate & Qt::SHIFT ) {
      action = DragMove;

    } else {
      KMenu menu;
      QAction *moveAction = menu.addAction( KIcon( QLatin1String( "go-jump" )), i18n( "&Move Here" ) );
      QAction *copyAction = menu.addAction( KIcon( QLatin1String( "edit-copy" ) ), i18n( "&Copy Here" ) );
      menu.addSeparator();
      menu.addAction( KIcon( QLatin1String( "dialog-cancel" ) ), i18n( "C&ancel" ) );

      QAction *menuChoice = menu.exec( QCursor::pos() );
      if ( menuChoice == moveAction ) {
        action = DragMove;
      } else if ( menuChoice == copyAction ) {
        action = DragCopy;
      } else {
        action = DragCancel;
      }
    }
  }
  if ( action == DragCancel )
    return;

  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();
  Collection target = collections.first();
  Item::List items;
  foreach ( const KUrl &url, urls ) {
    items << Item::fromUrl( url );
  }

  if ( action == DragCopy ) {
    new ItemCopyJob( items, target, this );
  } else if ( action == DragMove ) {
    new ItemMoveJob( items, target, this );
  }
}


void Widget::viewStartDragRequest()
{
  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();

  if ( collections.isEmpty() )
    return; // no folder here

  QList<Item> items = d->selectionAsItems();
  if ( items.isEmpty() )
    return;

  bool readOnly = false;

  foreach ( const Collection &c, collections ) {
    // We won't be able to remove items from this collection
    if ( ( c.rights() & Collection::CanDeleteItem ) == 0 ) {
      // So the drag will be read-only
      readOnly = true;
      break;
    }
  }

  KUrl::List urls;
  foreach ( const Item &i, items ) {
    urls << i.url( Item::UrlWithMimeType );
  }

  QMimeData *mimeData = new QMimeData;
  urls.populateMimeData( mimeData );

  QDrag *drag = new QDrag( view()->viewport() );
  drag->setMimeData( mimeData );

  // Set pixmap
  QPixmap pixmap;
  if( items.size() == 1 ) {
    pixmap = QPixmap( DesktopIcon(QLatin1String( "mail-message" ), KIconLoader::SizeSmall) );
  } else {
    pixmap = QPixmap( DesktopIcon(QLatin1String( "document-multiple" ), KIconLoader::SizeSmall) );
  }

  // Calculate hotspot (as in Konqueror)
  if( !pixmap.isNull() ) {
    drag->setHotSpot( QPoint( pixmap.width() / 2, pixmap.height() / 2 ) );
    drag->setPixmap( pixmap );
  }

  if ( readOnly )
    drag->exec( Qt::CopyAction );
  else
    drag->exec( Qt::CopyAction | Qt::MoveAction );
}

Item::List Widget::Private::selectionAsItems() const
{
  Item::List res;
  QList<Core::MessageItem *> selection = q->view()->selectionAsMessageItemList();

  foreach ( Core::MessageItem *mi, selection ) {
    Item i = itemForRow( mi->currentModelIndexRow() );
    Q_ASSERT( i.isValid() );
    res << i;
  }

  return res;
}

Item Widget::Private::itemForRow( int row ) const
{
  return static_cast<const StorageModel*>( q->storageModel() )->itemForRow( row );
}

KMime::Message::Ptr Widget::Private::messageForRow( int row ) const
{
  return static_cast<const StorageModel*>( q->storageModel() )->messageForRow( row );
}

Item Widget::currentItem() const
{
  Core::MessageItem *mi = view()->currentMessageItem();

  if ( mi == 0 ) {
    return Item();
  }

  return d->itemForRow( mi->currentModelIndexRow() );
}

KMime::Message::Ptr Widget::currentMessage() const
{
  Core::MessageItem *mi = view()->currentMessageItem();

  if ( mi == 0 ) {
    return KMime::Message::Ptr();
  }

  return d->messageForRow( mi->currentModelIndexRow() );
}


QList<KMime::Message::Ptr > Widget::selectionAsMessageList( bool includeCollapsedChildren ) const
{
  QList<KMime::Message::Ptr> lstMiPtr;
  QList<Core::MessageItem *> lstMi = view()->selectionAsMessageItemList( includeCollapsedChildren );
  if ( lstMi.isEmpty() ) {
     return lstMiPtr;
  }
  foreach( Core::MessageItem *it, lstMi ) {
    lstMiPtr.append( d->messageForRow( it->currentModelIndexRow() ) );
  }
  return lstMiPtr;
}

QList<Akonadi::Item> Widget::selectionAsMessageItemList( bool includeCollapsedChildren ) const
{
  QList<Item> lstMiPtr;
  QList<Core::MessageItem *> lstMi = view()->selectionAsMessageItemList( includeCollapsedChildren );
  if ( lstMi.isEmpty() ) {
     return lstMiPtr;
  }
  foreach( Core::MessageItem *it, lstMi ) {
    lstMiPtr.append( d->itemForRow( it->currentModelIndexRow() ) );
  }
  return lstMiPtr;
}

QVector<qlonglong> Widget::selectionAsMessageItemListId( bool includeCollapsedChildren ) const
{
  QVector<qlonglong> lstMiPtr;
  QList<Core::MessageItem *> lstMi = view()->selectionAsMessageItemList( includeCollapsedChildren );
  if ( lstMi.isEmpty() ) {
     return lstMiPtr;
  }
  foreach( Core::MessageItem *it, lstMi ) {
    lstMiPtr.append( d->itemForRow( it->currentModelIndexRow() ).id() );
  }
  return lstMiPtr;
}


QList<Akonadi::Item> Widget::currentThreadAsMessageList() const
{
  QList<Item> lstMiPtr;
  QList<Core::MessageItem *> lstMi = view()->currentThreadAsMessageItemList();
  if ( lstMi.isEmpty() ) {
    return lstMiPtr;
  }
  foreach( Core::MessageItem *it, lstMi ) {
    lstMiPtr.append( d->itemForRow( it->currentModelIndexRow() ) );
  }
  return lstMiPtr;
}


Akonadi::MessageStatus Widget::currentFilterStatus() const
{
  return view()->currentFilterStatus();
}

QString Widget::currentFilterSearchString() const
{
  return view()->currentFilterSearchString();
}


bool Widget::isThreaded() const
{
  return view()->isThreaded();
}

bool Widget::selectionEmpty() const
{
  return view()->selectionEmpty();
}

bool Widget::getSelectionStats(
  Akonadi::Item::List &selectedItems,
  Akonadi::Item::List &selectedVisibleItems,
  bool * allSelectedBelongToSameThread,
  bool includeCollapsedChildren ) const
{
  if ( !storageModel() )
    return false;

  selectedItems.clear();
  selectedVisibleItems.clear();

  QList< Core::MessageItem * > selected = view()->selectionAsMessageItemList( includeCollapsedChildren );

  Core::MessageItem * topmost = 0;

  *allSelectedBelongToSameThread = true;

  foreach( Core::MessageItem *it, selected ) {
    const Item item = d->itemForRow( it->currentModelIndexRow() );
    selectedItems.append( item );
    if ( view()->isDisplayedWithParentsExpanded( it ) )
      selectedVisibleItems.append( item );
    if ( topmost == 0 )
      topmost = ( *it ).topmostMessage();
    else {
      if ( topmost != ( *it ).topmostMessage() )
        *allSelectedBelongToSameThread = false;
    }
  }
  return true;
}

void Widget::deletePersistentSet( MessageList::Core::MessageItemSetReference ref )
{
  view()->deletePersistentSet( ref );
}

void Widget::markMessageItemsAsAboutToBeRemoved( MessageList::Core::MessageItemSetReference ref, bool bMark )
{
  QList< Core::MessageItem * > lstPersistent = view()->persistentSetCurrentMessageItemList( ref );
  if ( !lstPersistent.isEmpty() )
    view()->markMessageItemsAsAboutToBeRemoved( lstPersistent, bMark );
}

QList<Akonadi::Item> Widget::itemListFromPersistentSet( MessageList::Core::MessageItemSetReference ref )
{
  QList<Akonadi::Item> lstItem;
  QList< Core::MessageItem * > refList = view()->persistentSetCurrentMessageItemList( ref );
  if ( !refList.isEmpty() ) {
    foreach( Core::MessageItem *it, refList ) {
      lstItem.append( d->itemForRow( it->currentModelIndexRow() ) );
    }
  }
  return lstItem;
}


MessageList::Core::MessageItemSetReference Widget::selectionAsPersistentSet( bool includeCollapsedChildren ) const
{
  QList<Core::MessageItem *> lstMi = view()->selectionAsMessageItemList( includeCollapsedChildren );
  if ( lstMi.isEmpty() ) {
     return -1;
  }
  return view()->createPersistentSet( lstMi );
}

MessageList::Core::MessageItemSetReference Widget::currentThreadAsPersistentSet() const
{
  QList<Core::MessageItem *> lstMi = view()->currentThreadAsMessageItemList();
  if ( lstMi.isEmpty() ) {
    return -1;
  }
  return view()->createPersistentSet( lstMi );
}

Akonadi::Collection Widget::currentCollection() const
{
  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();
  if ( collections.size()!=1 )
    return Akonadi::Collection(); // no folder here or too many (in case we can't decide where the drop will end)
  return collections.first();
}

void Widget::slotCollapseItem()
{
  view()->setCollapseItem(d->mGroupHeaderItemIndex);
}

void Widget::slotExpandItem()
{
  view()->setExpandItem(d->mGroupHeaderItemIndex);
}

#include "widget.moc"
