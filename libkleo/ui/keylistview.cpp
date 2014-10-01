/*
    keylistview.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/


#include "keylistview.h"

#include "kleo_ui_debug.h"

#include <QFontMetrics>
#include <QToolTip>
#include <QPoint>
#include <QFont>
#include <QColor>
#include <QTimer>
#include <gpgme++/key.h>

#include <vector>
#include <map>

#include <assert.h>
#include <QKeyEvent>

static const int updateDelayMilliSecs = 500;


class Kleo::KeyListView::Private {
public:
  Private() : updateTimer( 0 ) {}

  std::vector<GpgME::Key> keyBuffer;
  QTimer * updateTimer;
  std::map<QByteArray,KeyListViewItem*> itemMap;
};

// a list of signals where we want to replace QListViewItem with
// Kleo:KeyListViewItem:
static const struct {
  const char * source;
  const char * target;
} signalReplacements[] = {
  { SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
    SLOT(slotEmitDoubleClicked(QTreeWidgetItem*,int)) },
  { SIGNAL(itemSelectionChanged()),
    SLOT(slotEmitSelectionChanged()) },
  { SIGNAL(customContextMenuRequested(QPoint)),
    SLOT(slotEmitContextMenu(QPoint)) },
};
static const int numSignalReplacements = sizeof signalReplacements / sizeof *signalReplacements;


Kleo::KeyListView::KeyListView( const ColumnStrategy * columnStrategy, const DisplayStrategy * displayStrategy, QWidget * parent, Qt::WindowFlags f )
  : QTreeWidget( parent ),
    mColumnStrategy( columnStrategy ),
    mDisplayStrategy ( displayStrategy  ),
    mHierarchical( false ),d(new Private())
{
  setWindowFlags( f );
  setContextMenuPolicy( Qt::CustomContextMenu );

  d->updateTimer = new QTimer( this );
  d->updateTimer->setSingleShot( true );
  connect(d->updateTimer, &QTimer::timeout, this, &KeyListView::slotUpdateTimeout);
  if ( !columnStrategy ) {
    qCWarning(KLEO_UI_LOG) <<"Kleo::KeyListView: need a column strategy to work with!";
    return;
  }

  const QFontMetrics fm = fontMetrics();

  for ( int col = 0 ; !columnStrategy->title( col ).isEmpty() ; ++col ) {
    headerItem()->setText( col, columnStrategy->title( col ) );
    header()->resizeSection( col, columnStrategy->width( col, fm ) );
    header()->setResizeMode( col, columnStrategy->resizeMode( col ) );
  }

  setAllColumnsShowFocus( true );

  for ( int i = 0 ; i < numSignalReplacements ; ++i )
    connect( this, signalReplacements[i].source, signalReplacements[i].target );

  this->setToolTip(QString());
  viewport()->setToolTip(QString()); // make double sure :)
}

Kleo::KeyListView::~KeyListView() {
  d->updateTimer->stop();
  // need to clear here, since in ~QListView, our children won't have
  // a valid listView() pointing to us anymore, and their dtors try to
  // unregister from us.
  clear();
  assert( d->itemMap.size() == 0 );
  // need to delete the tooltip ourselves, as ~QToolTip isn't virtual :o
  delete d;
  delete mColumnStrategy; mColumnStrategy = 0;
  delete mDisplayStrategy; mDisplayStrategy = 0;
}


void Kleo::KeyListView::takeItem( QTreeWidgetItem * qlvi ) {
  //qCDebug(KLEO_UI_LOG) <<"Kleo::KeyListView::takeItem(" << qlvi <<" )";
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    deregisterItem( item );
  takeTopLevelItem( indexOfTopLevelItem( qlvi ) );
}


void Kleo::KeyListView::setHierarchical( bool hier ) {
  if ( hier == mHierarchical )
    return;
  mHierarchical = hier;
  if ( hier )
    gatherScattered();
  else
    scatterGathered( firstChild() );
}

void Kleo::KeyListView::slotAddKey( const GpgME::Key & key ) {
  if ( key.isNull() )
    return;

  d->keyBuffer.push_back( key );
  if ( !d->updateTimer->isActive() )
    d->updateTimer->start( updateDelayMilliSecs );
}

void Kleo::KeyListView::slotUpdateTimeout() {
  if ( d->keyBuffer.empty() )
    return;

  const bool wasUpdatesEnabled = viewport()->updatesEnabled();
  if ( wasUpdatesEnabled )
    viewport()->setUpdatesEnabled( false );
  qCDebug(KLEO_UI_LOG) <<"Kleo::KeyListView::slotUpdateTimeout(): processing"
                  << d->keyBuffer.size() << "items en block";
  if ( hierarchical() ) {
    for ( std::vector<GpgME::Key>::const_iterator it = d->keyBuffer.begin() ; it != d->keyBuffer.end() ; ++it )
      doHierarchicalInsert( *it );
    gatherScattered();
  } else {
    for ( std::vector<GpgME::Key>::const_iterator it = d->keyBuffer.begin() ; it != d->keyBuffer.end() ; ++it )
      (void)new KeyListViewItem( this, *it );
  }
  if ( wasUpdatesEnabled )
    viewport()->setUpdatesEnabled( true );
  d->keyBuffer.clear();
}

void Kleo::KeyListView::clear() {
  d->updateTimer->stop();
  d->keyBuffer.clear();
  while ( QTreeWidgetItem *item = topLevelItem( 0 ) )
    delete item;
  QTreeWidget::clear();
}

void Kleo::KeyListView::registerItem( KeyListViewItem * item ) {
  //qCDebug(KLEO_UI_LOG) <<"registerItem(" << item <<" )";
  if ( !item )
    return;
  const QByteArray fpr = item->key().primaryFingerprint();
  if ( !fpr.isEmpty() )
    d->itemMap.insert( std::make_pair( fpr, item ) );
}

void Kleo::KeyListView::deregisterItem( const KeyListViewItem * item ) {
  //qCDebug(KLEO_UI_LOG) <<"deregisterItem( KeyLVI:" << item <<" )";
  if ( !item )
    return;
  std::map<QByteArray,KeyListViewItem*>::iterator it
    = d->itemMap.find( item->key().primaryFingerprint() );
  if ( it == d->itemMap.end() )
    return;
  // This assert triggers, though it shouldn't. Print some more
  // information when it happens.
  //Q_ASSERT( it->second == item );
  if ( it->second != item ) {
    qCWarning(KLEO_UI_LOG) << "deregisterItem:"
                   << "item      " << item->key().primaryFingerprint()
                   << "it->second" << (it->second ? it->second->key().primaryFingerprint() : "is null" );
    return;
  }
  d->itemMap.erase( it );
}

void Kleo::KeyListView::doHierarchicalInsert( const GpgME::Key & key ) {
  const QByteArray fpr = key.primaryFingerprint();
  if ( fpr.isEmpty() )
    return;
  KeyListViewItem * item = 0;
  if ( !key.isRoot() )
    if ( KeyListViewItem * parent = itemByFingerprint( key.chainID() ) ) {
      item = new KeyListViewItem( parent, key );
      parent->setExpanded( true );
    }
  if ( !item )
    item = new KeyListViewItem( this, key ); // top-level (for now)

  d->itemMap.insert( std::make_pair( fpr, item ) );
}

void Kleo::KeyListView::gatherScattered() {
  KeyListViewItem * item = firstChild();
  while ( item ) {
    KeyListViewItem * cur = item;
    item = item->nextSibling();
    if ( cur->key().isRoot() )
      continue;
    if ( KeyListViewItem * parent = itemByFingerprint( cur->key().chainID() ) ) {
      // found a new parent...
      // ### todo: optimize by suppressing removing/adding the item to the itemMap...
      takeTopLevelItem( indexOfTopLevelItem( cur ) );
      parent->addChild( cur );
      parent->setExpanded( true );
    }
  }
}

void Kleo::KeyListView::scatterGathered( KeyListViewItem * start ) {
  KeyListViewItem* item = start;
  while ( item ) {
    KeyListViewItem * cur = item;
    item = item->nextSibling();

    scatterGathered( Kleo::lvi_cast<Kleo::KeyListViewItem>( cur->child( 0 ) ) );
    assert( cur->childCount() == 0 );

    // ### todo: optimize by suppressing removing/adding the item to the itemMap...
    if ( cur->parent() )
      static_cast<Kleo::KeyListViewItem*>( cur->parent() )->takeItem( cur );
    else
      takeItem( cur );
    addTopLevelItem( cur );
  }
}

Kleo::KeyListViewItem * Kleo::KeyListView::itemByFingerprint( const QByteArray & s ) const {
  if ( s.isEmpty() )
    return 0;
  const std::map<QByteArray,KeyListViewItem*>::const_iterator it = d->itemMap.find( s );
  if ( it == d->itemMap.end() )
    return 0;
  return it->second;
}


void Kleo::KeyListView::slotRefreshKey( const GpgME::Key & key ) {
  const char * fpr = key.primaryFingerprint();
  if ( !fpr )
    return;
  if ( KeyListViewItem * item = itemByFingerprint( fpr ) )
    item->setKey ( key );
  else
    // none found -> add it
    slotAddKey( key );
}

// slots for the emission of covariant signals:

void Kleo::KeyListView::slotEmitDoubleClicked( QTreeWidgetItem * item, int col ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit doubleClicked( static_cast<KeyListViewItem*>( item ), col );
}

void Kleo::KeyListView::slotEmitReturnPressed( QTreeWidgetItem * item ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit returnPressed( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitSelectionChanged(  ) {
  emit selectionChanged( selectedItem() );
}

void Kleo::KeyListView::slotEmitContextMenu( const QPoint& pos )
{
  QTreeWidgetItem *item = itemAt( pos );
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit contextMenu( static_cast<KeyListViewItem*>( item ), viewport()->mapToGlobal( pos ) );
}

//
//
// KeyListViewItem
//
//

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, const GpgME::Key & key )
  : QTreeWidgetItem( parent, RTTI )
{
  Q_ASSERT( parent );
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key )
  : QTreeWidgetItem( parent, after, RTTI )
{
  Q_ASSERT( parent );
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key )
  : QTreeWidgetItem( parent, RTTI )
{
  Q_ASSERT( parent && parent->listView() );
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key )
  : QTreeWidgetItem( parent, after, RTTI )
{
  Q_ASSERT( parent && parent->listView() );
  setKey( key );
}

Kleo::KeyListViewItem::~KeyListViewItem() {
  // delete the children first... When children are deleted in the
  // QLVI dtor, they don't have listView() anymore, thus they don't
  // call deregister( this ), leading to stale entries in the
  // itemMap...
  while ( QTreeWidgetItem * item = child( 0 ) )
    delete item;
  // better do this here, too, since deletion is top-down and thus
  // we're deleted when our parent item is no longer a
  // KeyListViewItem, but a mere QListViewItem, so our takeItem()
  // overload is gone by that time...
  if ( KeyListView * lv = listView() )
    lv->deregisterItem( this );
}

void Kleo::KeyListViewItem::setKey( const GpgME::Key & key ) {
  KeyListView * lv = listView();
  if ( lv )
    lv->deregisterItem( this );
  mKey = key;
  if ( lv )
    lv->registerItem( this );

  // the ColumnStrategy operations might be very slow, so cache their
  // result here, where we're non-const :)
  const Kleo::KeyListView::ColumnStrategy * cs = lv ? lv->columnStrategy() : 0 ;
  if ( !cs )
    return;
  const KeyListView::DisplayStrategy * ds = lv ? lv->displayStrategy() : 0 ;
  const int numCols = lv ? lv->columnCount() : 0 ;
  for ( int i = 0 ; i < numCols ; ++i ) {
    setText( i, cs->text( key, i ) );
    setToolTip( i, cs->toolTip( key, i ) );
    const QIcon icon = cs->icon( key, i );
    if ( !icon.isNull() )
      setIcon( i, icon );
    if ( ds ) {
      setForeground( i, QBrush( ds->keyForeground( key, foreground( i ).color() ) ) );
      setBackgroundColor( i, ds->keyBackground( key, backgroundColor( i ) ) );
      setFont( i, ds->keyFont( key, font( i ) ) );
    }
  }
}

QString Kleo::KeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->toolTip( key(), col )
    : QString() ;
}

bool Kleo::KeyListViewItem::operator<(const QTreeWidgetItem& other) const
{
  if ( other.type() != RTTI || !listView() || !listView()->columnStrategy() )
    return QTreeWidgetItem::operator<(other);
  const KeyListViewItem * that = static_cast<const KeyListViewItem*>( &other );
  return listView()->columnStrategy()->compare( this->key(), that->key(), treeWidget()->sortColumn() ) < 0;
}


void Kleo::KeyListViewItem::takeItem( QTreeWidgetItem * qlvi ) {
  //qCDebug(KLEO_UI_LOG) <<"Kleo::KeyListViewItem::takeItem(" << qlvi <<" )";
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    listView()->deregisterItem( item );
  takeChild( indexOfChild( qlvi ) );
}


//
//
// ColumnStrategy
//
//

Kleo::KeyListView::ColumnStrategy::~ColumnStrategy() {}

int Kleo::KeyListView::ColumnStrategy::compare( const GpgME::Key & key1, const GpgME::Key & key2, const int col ) const {
  return QString::localeAwareCompare( text( key1, col ), text( key2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::width( int col, const QFontMetrics & fm ) const {
  return fm.width( title( col ) ) * 2;
}

QString Kleo::KeyListView::ColumnStrategy::toolTip( const GpgME::Key & key, int col ) const {
  return text( key, col );
}


//
//
// DisplayStrategy
//
//

Kleo::KeyListView::DisplayStrategy::~DisplayStrategy() {}


//font
QFont Kleo::KeyListView::DisplayStrategy::keyFont( const GpgME::Key &, const QFont & font ) const {
  return font;
}

//foreground
QColor Kleo::KeyListView::DisplayStrategy::keyForeground( const GpgME::Key &, const QColor & fg )const {
  return fg;
}

//background
QColor Kleo::KeyListView::DisplayStrategy::keyBackground( const GpgME::Key &, const QColor & bg )const {
  return bg;
}


//
//
// Collection of covariant return reimplementations of QListView(Item)
// members:
//
//

Kleo::KeyListView * Kleo::KeyListViewItem::listView() const {
  return static_cast<Kleo::KeyListView*>( QTreeWidgetItem::treeWidget() );
}

Kleo::KeyListViewItem * Kleo::KeyListViewItem::nextSibling() const
{
  if ( parent() ) {
    const int myIndex = parent()->indexOfChild( const_cast<KeyListViewItem*>( this ) );
    return static_cast<Kleo::KeyListViewItem*>( parent()->child( myIndex + 1 ) );
  }
  const int myIndex = treeWidget()->indexOfTopLevelItem( const_cast<KeyListViewItem*>( this ) );
  return static_cast<Kleo::KeyListViewItem*>( treeWidget()->topLevelItem( myIndex + 1 ) );
}

Kleo::KeyListViewItem * Kleo::KeyListView::firstChild() const {
  return static_cast<Kleo::KeyListViewItem*>( topLevelItem( 0 ) );
}

Kleo::KeyListViewItem * Kleo::KeyListView::selectedItem() const
{
  QList<KeyListViewItem*> selection = selectedItems();
  if ( selection.size() == 0 )
    return 0;
  return selection.first();
}

QList<Kleo::KeyListViewItem*> Kleo::KeyListView::selectedItems() const
{
  QList<KeyListViewItem*> result;
  foreach ( QTreeWidgetItem* selectedItem, QTreeWidget::selectedItems() ) {
    if ( Kleo::KeyListViewItem * i = Kleo::lvi_cast<Kleo::KeyListViewItem>( selectedItem ) )
      result.append( i );
  }
  return result;
}

bool Kleo::KeyListView::isMultiSelection() const
{
  return selectionMode() == ExtendedSelection || selectionMode() == MultiSelection;
}

void Kleo::KeyListView::keyPressEvent(QKeyEvent* event)
{
  if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) {
    if ( selectedItem() )
      slotEmitReturnPressed( selectedItem() );
  }
  QTreeView::keyPressEvent(event);
}


