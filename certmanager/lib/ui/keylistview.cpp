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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keylistview.h"

#include <kdebug.h>

#include <qfontmetrics.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qheader.h>
#include <qpoint.h>
#include <qptrlist.h>
#include <qpainter.h>
#include <qfont.h>
#include <qcolor.h>
#include <qtimer.h>
#include <qcstring.h>

#include <gpgmepp/key.h>

#include <vector>
#include <map>

#include <assert.h>

static const int updateDelayMilliSecs = 500;

namespace {

  class ItemToolTip : public QToolTip {
  public:
    ItemToolTip( Kleo::KeyListView * parent );
  protected:
    void maybeTip( const QPoint & p );
  private:
    Kleo::KeyListView * mKeyListView;
  };

  ItemToolTip::ItemToolTip( Kleo::KeyListView * parent )
    : QToolTip( parent->viewport() ), mKeyListView( parent ) {}

  void ItemToolTip::maybeTip( const QPoint & p ) {
    if ( !mKeyListView )
      return;

    const QListViewItem * item = mKeyListView->itemAt( p );
    if ( !item )
      return;

    const QRect itemRect = mKeyListView->itemRect( item );
    if ( !itemRect.isValid() )
      return;

    const int col = mKeyListView->header()->sectionAt( p.x() );
    if ( col == -1 )
      return;

    const QRect headerRect = mKeyListView->header()->sectionRect( col );
    if ( !headerRect.isValid() )
      return;

    const QRect cellRect( headerRect.left(), itemRect.top(),
			  headerRect.width(), itemRect.height() );

    QString tipStr;
    if ( const Kleo::KeyListViewItem * klvi = Kleo::lvi_cast<Kleo::KeyListViewItem>( item ) )
      tipStr = klvi->toolTip( col );
    else
      tipStr = item->text( col ) ;

    if ( !tipStr.isEmpty() )
      tip( cellRect, tipStr );
  }

} // anon namespace

struct Kleo::KeyListView::Private {
  Private() : updateTimer( 0 ), itemToolTip( 0 ) {}

  std::vector<GpgME::Key> keyBuffer;
  QTimer * updateTimer;
  QToolTip * itemToolTip;
  std::map<QCString,KeyListViewItem*> itemMap;
};

// a list of signals where we want to replace QListViewItem with
// Kleo:KeyListViewItem:
static const struct {
  const char * source;
  const char * target;
} signalReplacements[] = {
  { SIGNAL(doubleClicked(QListViewItem*,const QPoint&,int)),
    SLOT(slotEmitDoubleClicked(QListViewItem*,const QPoint&,int)) },
  { SIGNAL(returnPressed(QListViewItem*)),
    SLOT(slotEmitReturnPressed(QListViewItem*)) },
  { SIGNAL(selectionChanged(QListViewItem*)),
    SLOT(slotEmitSelectionChanged(QListViewItem*)) },
  { SIGNAL(contextMenu(KListView*, QListViewItem*,const QPoint&)),
    SLOT(slotEmitContextMenu(KListView*, QListViewItem*,const QPoint&)) },
};
static const int numSignalReplacements = sizeof signalReplacements / sizeof *signalReplacements;


Kleo::KeyListView::KeyListView( const ColumnStrategy * columnStrategy, const DisplayStrategy * displayStrategy, QWidget * parent, const char * name, WFlags f )
  : KListView( parent, name ),
    mColumnStrategy( columnStrategy ),
    mDisplayStrategy ( displayStrategy  ),
    mHierarchical( false )
{
  setWFlags( f );

  d = new Private();

  d->updateTimer = new QTimer( this );
  connect( d->updateTimer, SIGNAL(timeout()), SLOT(slotUpdateTimeout()) );

  if ( !columnStrategy ) {
    kdWarning(5150) << "Kleo::KeyListView: need a column strategy to work with!" << endl;
    return;
  }

  const QFontMetrics fm = fontMetrics();

  for ( int col = 0 ; !columnStrategy->title( col ).isEmpty() ; ++col ) {
    addColumn( columnStrategy->title( col ), columnStrategy->width( col, fm ) );
    setColumnWidthMode( col, columnStrategy->widthMode( col ) );
  }

  setAllColumnsShowFocus( true );
  setShowToolTips( false ); // we do it instead...

  for ( int i = 0 ; i < numSignalReplacements ; ++i )
    connect( this, signalReplacements[i].source, signalReplacements[i].target );

  QToolTip::remove( this );
  QToolTip::remove( viewport() ); // make double sure :)
  d->itemToolTip = new ItemToolTip( this );
}

Kleo::KeyListView::~KeyListView() {
  d->updateTimer->stop();
  // need to clear here, since in ~QListView, our children won't have
  // a valid listView() pointing to us anymore, and their dtors try to
  // unregister from us.
  clear();
  assert( d->itemMap.size() == 0 );
  // need to delete the tooltip ourselves, as ~QToolTip isn't virtual :o
  delete d->itemToolTip; d->itemToolTip = 0;
  delete d; d = 0;
  delete mColumnStrategy; mColumnStrategy = 0;
  delete mDisplayStrategy; mDisplayStrategy = 0;
}

void Kleo::KeyListView::insertItem( QListViewItem * qlvi ) {
  //qDebug( "Kleo::KeyListView::insertItem( %p )", qlvi );
  KListView::insertItem( qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    registerItem( item );
}

void Kleo::KeyListView::takeItem( QListViewItem * qlvi ) {
  //qDebug( "Kleo::KeyListView::takeItem( %p )", qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    deregisterItem( item );
  KListView::takeItem( qlvi );
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
    d->updateTimer->start( updateDelayMilliSecs, true /* single-shot */ );
}

void Kleo::KeyListView::slotUpdateTimeout() {
  if ( d->keyBuffer.empty() )
    return;

  const bool wasUpdatesEnabled = viewport()->isUpdatesEnabled();
  if ( wasUpdatesEnabled )
    viewport()->setUpdatesEnabled( false );
  kdDebug( 5150 ) << "Kleo::KeyListView::slotUpdateTimeout(): processing "
		  << d->keyBuffer.size() << " items en block" << endl;
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
  KListView::clear();
}

void Kleo::KeyListView::registerItem( KeyListViewItem * item ) {
  //qDebug( "registerItem( %p )", item );
  if ( !item )
    return;
  const QCString fpr = item->key().primaryFingerprint();
  if ( !fpr.isEmpty() )
    d->itemMap.insert( std::make_pair( fpr, item ) );
}

void Kleo::KeyListView::deregisterItem( const KeyListViewItem * item ) {
  //qDebug( "deregisterItem( KeyLVI: %p )", item );
  if ( !item )
    return;
  std::map<QCString,KeyListViewItem*>::iterator it
    = d->itemMap.find( item->key().primaryFingerprint() );
  if ( it == d->itemMap.end() )
    return;
  Q_ASSERT( it->second == item );
  if ( it->second != item )
    return;
  d->itemMap.erase( it );
}

void Kleo::KeyListView::doHierarchicalInsert( const GpgME::Key & key ) {
  const QCString fpr = key.primaryFingerprint();
  if ( fpr.isEmpty() )
    return;
  KeyListViewItem * item = 0;
  if ( !key.isRoot() )
    if ( KeyListViewItem * parent = itemByFingerprint( key.chainID() ) ) {
      item = new KeyListViewItem( parent, key );
      parent->setOpen( true );
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
      takeItem( cur );
      parent->insertItem( cur );
      parent->setOpen( true );
    }
  }
}

void Kleo::KeyListView::scatterGathered( QListViewItem * start ) {
  QListViewItem * item = start;
  while ( item ) {
    QListViewItem * cur = item;
    item = item->nextSibling();

    scatterGathered( cur->firstChild() );
    assert( cur->childCount() == 0 );

    // ### todo: optimize by suppressing removing/adding the item to the itemMap...
    if ( cur->parent() )
      cur->parent()->takeItem( cur );
    else
      takeItem( cur );
    insertItem( cur );
  }
}

Kleo::KeyListViewItem * Kleo::KeyListView::itemByFingerprint( const QCString & s ) const {
  if ( s.isEmpty() )
    return 0;
  const std::map<QCString,KeyListViewItem*>::const_iterator it = d->itemMap.find( s );
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

void Kleo::KeyListView::slotEmitDoubleClicked( QListViewItem * item, const QPoint & p, int col ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit doubleClicked( static_cast<KeyListViewItem*>( item ), p, col );
}

void Kleo::KeyListView::slotEmitReturnPressed( QListViewItem * item ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit returnPressed( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitSelectionChanged( QListViewItem * item ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit selectionChanged( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitContextMenu( KListView*, QListViewItem * item, const QPoint & p ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit contextMenu( static_cast<KeyListViewItem*>( item ), p );
}

//
//
// KeyListViewItem
//
//

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, const GpgME::Key & key )
  : QListViewItem( parent )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key )
  : QListViewItem( parent, after )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key )
  : QListViewItem( parent )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key )
  : QListViewItem( parent, after )
{
  setKey( key );
}

Kleo::KeyListViewItem::~KeyListViewItem() {
  // delete the children first... When children are deleted in the
  // QLVI dtor, they don't have listView() anymore, thus they don't
  // call deregister( this ), leading to stale entries in the
  // itemMap...
  while ( QListViewItem * item = firstChild() )
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
  const int numCols = lv ? lv->columns() : 0 ;
  for ( int i = 0 ; i < numCols ; ++i ) {
    setText( i, cs->text( key, i ) );
    if ( const QPixmap * pix = cs->pixmap( key, i ) )
      setPixmap( i, *pix );
  }
  repaint();
}

QString Kleo::KeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->toolTip( key(), col )
    : QString::null ;
}

int Kleo::KeyListViewItem::compare( QListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return QListViewItem::compare( item, col, ascending );
  KeyListViewItem * that = static_cast<KeyListViewItem*>( item );
  return listView()->columnStrategy()->compare( this->key(), that->key(), col );
}

void Kleo::KeyListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    QListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const QColor fg = ds->keyForeground( key(), cg.text() );
  const QColor bg = ds->keyBackground( key(), cg.base() );
  const QFont f = ds->keyFont( key(), p->font() );

  QColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( QColorGroup::Text, fg );
  _cg.setColor( QColorGroup::Base, bg );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
}

void Kleo::KeyListViewItem::insertItem( QListViewItem * qlvi ) {
  //qDebug( "Kleo::KeyListViewItem::insertItem( %p )", qlvi );
  QListViewItem::insertItem( qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    listView()->registerItem( item );
}

void Kleo::KeyListViewItem::takeItem( QListViewItem * qlvi ) {
  //qDebug( "Kleo::KeyListViewItem::takeItem( %p )", qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    listView()->deregisterItem( item );
  QListViewItem::takeItem( qlvi );
}


//
//
// SubkeyKeyListViewItem
//
//

Kleo::SubkeyKeyListViewItem::SubkeyKeyListViewItem( KeyListView * parent, const GpgME::Subkey & subkey )
  : KeyListViewItem( parent, subkey.parent() ), mSubkey( subkey )
{

}

Kleo::SubkeyKeyListViewItem::SubkeyKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Subkey & subkey )
  : KeyListViewItem( parent, after, subkey.parent() ), mSubkey( subkey )
{

}

Kleo::SubkeyKeyListViewItem::SubkeyKeyListViewItem( KeyListViewItem * parent, const GpgME::Subkey & subkey )
  : KeyListViewItem( parent, subkey.parent() ), mSubkey( subkey )
{

}

Kleo::SubkeyKeyListViewItem::SubkeyKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Subkey & subkey )
  : KeyListViewItem( parent, after, subkey.parent() ), mSubkey( subkey )
{

}

void Kleo::SubkeyKeyListViewItem::setSubkey( const GpgME::Subkey & subkey ) {
  mSubkey = subkey;
  setKey( subkey.parent() );
}

QString Kleo::SubkeyKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyText( subkey(), col )
    : QString::null ;
}

QString Kleo::SubkeyKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyToolTip( subkey(), col )
    : QString::null ;
}

const QPixmap * Kleo::SubkeyKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyPixmap( subkey(), col ) : 0 ;
}

int Kleo::SubkeyKeyListViewItem::compare( QListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  SubkeyKeyListViewItem * that = static_cast<SubkeyKeyListViewItem*>( item );
  return listView()->columnStrategy()->subkeyCompare( this->subkey(), that->subkey(), col );
}

void Kleo::SubkeyKeyListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    QListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const QColor fg = ds->subkeyForeground( subkey(), cg.text() );
  const QColor bg = ds->subkeyBackground( subkey(), cg.base() );
  const QFont f = ds->subkeyFont( subkey(), p->font() );

  QColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( QColorGroup::Text, fg );
  _cg.setColor( QColorGroup::Base, bg );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
}


//
//
// UserIDKeyListViewItem
//
//

Kleo::UserIDKeyListViewItem::UserIDKeyListViewItem( KeyListView * parent, const GpgME::UserID & userID )
  : KeyListViewItem( parent, userID.parent() ), mUserID( userID )
{

}

Kleo::UserIDKeyListViewItem::UserIDKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::UserID & userID )
  : KeyListViewItem( parent, after, userID.parent() ), mUserID( userID )
{

}

Kleo::UserIDKeyListViewItem::UserIDKeyListViewItem( KeyListViewItem * parent, const GpgME::UserID & userID )
  : KeyListViewItem( parent, userID.parent() ), mUserID( userID )
{

}

Kleo::UserIDKeyListViewItem::UserIDKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::UserID & userID )
  : KeyListViewItem( parent, after, userID.parent() ), mUserID( userID )
{

}

void Kleo::UserIDKeyListViewItem::setUserID( const GpgME::UserID & userID ) {
  mUserID = userID;
  setKey( userID.parent() );
}

QString Kleo::UserIDKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDText( userID(), col )
    : QString::null ;
}

QString Kleo::UserIDKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDToolTip( userID(), col )
    : QString::null ;
}

const QPixmap * Kleo::UserIDKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDPixmap( userID(), col ) : 0 ;
}

int Kleo::UserIDKeyListViewItem::compare( QListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  UserIDKeyListViewItem * that = static_cast<UserIDKeyListViewItem*>( item );
  return listView()->columnStrategy()->userIDCompare( this->userID(), that->userID(), col );
}


void Kleo::UserIDKeyListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    QListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const QColor fg = ds->useridForeground( userID(), cg.text() );
  const QColor bg = ds->useridBackground( userID(), cg.base() );
  const QFont f = ds->useridFont( userID(), p->font() );

  QColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( QColorGroup::Text, fg );
  _cg.setColor( QColorGroup::Base, bg );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
}


//
//
// SignatureKeyListViewItem
//
//

Kleo::SignatureKeyListViewItem::SignatureKeyListViewItem( KeyListView * parent, const GpgME::UserID::Signature & signature )
  : KeyListViewItem( parent, signature.parent().parent() ), mSignature( signature )
{

}

Kleo::SignatureKeyListViewItem::SignatureKeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::UserID::Signature & signature )
  : KeyListViewItem( parent, after, signature.parent().parent() ), mSignature( signature )
{

}

Kleo::SignatureKeyListViewItem::SignatureKeyListViewItem( KeyListViewItem * parent, const GpgME::UserID::Signature & signature )
  : KeyListViewItem( parent, signature.parent().parent() ), mSignature( signature )
{

}

Kleo::SignatureKeyListViewItem::SignatureKeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::UserID::Signature & signature )
  : KeyListViewItem( parent, after, signature.parent().parent() ), mSignature( signature )
{

}

void Kleo::SignatureKeyListViewItem::setSignature( const GpgME::UserID::Signature & signature ) {
  mSignature = signature;
  setKey( signature.parent().parent() );
}

QString Kleo::SignatureKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signatureText( signature(), col )
    : QString::null ;
}

QString Kleo::SignatureKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signatureToolTip( signature(), col )
    : QString::null ;
}

const QPixmap * Kleo::SignatureKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signaturePixmap( signature(), col ) : 0 ;
}

int Kleo::SignatureKeyListViewItem::compare( QListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  SignatureKeyListViewItem * that = static_cast<SignatureKeyListViewItem*>( item );
  return listView()->columnStrategy()->signatureCompare( this->signature(), that->signature(), col );
}

void Kleo::SignatureKeyListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    QListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const QColor fg = ds->signatureForeground( signature(), cg.text() );
  const QColor bg = ds->signatureBackground( signature(), cg.base() );
  const QFont f = ds->signatureFont( signature(), p->font() );

  QColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( QColorGroup::Text, fg );
  _cg.setColor( QColorGroup::Base, bg );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
}


//
//
// ColumnStrategy
//
//

Kleo::KeyListView::ColumnStrategy::~ColumnStrategy() {}

int Kleo::KeyListView::ColumnStrategy::compare( const GpgME::Key & key1, const GpgME::Key & key2, int col ) const {
  return QString::localeAwareCompare( text( key1, col ), text( key2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::width( int col, const QFontMetrics & fm ) const {
  return fm.width( title( col ) ) * 2;
}

int Kleo::KeyListView::ColumnStrategy::subkeyCompare( const GpgME::Subkey & sub1, const GpgME::Subkey & sub2, int col ) const {
  return QString::localeAwareCompare( subkeyText( sub1, col ), subkeyText( sub2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::userIDCompare( const GpgME::UserID & uid1, const GpgME::UserID & uid2, int col ) const {
  return QString::localeAwareCompare( userIDText( uid1, col ), userIDText( uid2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::signatureCompare( const GpgME::UserID::Signature & sig1, const GpgME::UserID::Signature & sig2, int col ) const {
  return QString::localeAwareCompare( signatureText( sig1, col ), signatureText( sig2, col ) );
}

QString Kleo::KeyListView::ColumnStrategy::toolTip( const GpgME::Key & key, int col ) const {
  return text( key, col );
}

QString Kleo::KeyListView::ColumnStrategy::subkeyToolTip( const GpgME::Subkey & sub, int col ) const {
  return subkeyText( sub, col );
}

QString Kleo::KeyListView::ColumnStrategy::userIDToolTip( const GpgME::UserID & uid, int col ) const {
  return userIDText( uid, col );
}

QString Kleo::KeyListView::ColumnStrategy::signatureToolTip( const GpgME::UserID::Signature & sig, int col ) const {
  return signatureText( sig, col );
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

QFont Kleo::KeyListView::DisplayStrategy::subkeyFont( const GpgME::Subkey &, const QFont & font ) const {
  return font;
}

QFont Kleo::KeyListView::DisplayStrategy::useridFont( const GpgME::UserID &, const QFont & font ) const {
  return font;
}

QFont Kleo::KeyListView::DisplayStrategy::signatureFont( const GpgME::UserID::Signature &, const QFont & font ) const {
  return font;
}

//foreground
QColor Kleo::KeyListView::DisplayStrategy::keyForeground( const GpgME::Key &, const QColor & fg )const {
  return fg;
}

QColor Kleo::KeyListView::DisplayStrategy::subkeyForeground( const GpgME::Subkey &, const QColor & fg ) const {
  return fg;
}

QColor Kleo::KeyListView::DisplayStrategy::useridForeground( const GpgME::UserID &, const QColor & fg ) const {
  return fg;
}

QColor Kleo::KeyListView::DisplayStrategy::signatureForeground( const GpgME::UserID::Signature &, const QColor & fg ) const {
  return fg;
}

//background
QColor Kleo::KeyListView::DisplayStrategy::keyBackground( const GpgME::Key &, const QColor & bg )const {
  return bg;
}

QColor Kleo::KeyListView::DisplayStrategy::subkeyBackground( const GpgME::Subkey &, const QColor & bg ) const {
  return bg;
}

QColor Kleo::KeyListView::DisplayStrategy::useridBackground( const GpgME::UserID &, const QColor & bg ) const {
  return bg;
}

QColor Kleo::KeyListView::DisplayStrategy::signatureBackground( const GpgME::UserID::Signature &, const QColor & bg ) const {
  return bg;
}


//
//
// Collection of covariant return reimplementations of QListView(Item)
// members:
//
//

Kleo::KeyListView * Kleo::KeyListViewItem::listView() const {
  return static_cast<Kleo::KeyListView*>( QListViewItem::listView() );
}

Kleo::KeyListViewItem * Kleo::KeyListViewItem::nextSibling() const {
  return static_cast<Kleo::KeyListViewItem*>( QListViewItem::nextSibling() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::firstChild() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::firstChild() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::selectedItem() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::selectedItem() );
}

static void selectedItems( QPtrList<Kleo::KeyListViewItem> & result, QListViewItem * start ) {
  for ( QListViewItem * item = start ; item ; item = item->nextSibling() ) {
    if ( item->isSelected() )
      if ( Kleo::KeyListViewItem * i = Kleo::lvi_cast<Kleo::KeyListViewItem>( item ) )
	result.append( i );
    selectedItems( result, item->firstChild() );
  }
}

QPtrList<Kleo::KeyListViewItem> Kleo::KeyListView::selectedItems() const {
  QPtrList<KeyListViewItem> result;
  ::selectedItems( result, firstChild() );
  return result;
}

static bool hasSelection( QListViewItem * start ) {
  for ( QListViewItem * item = start ; item ; item = item->nextSibling() )
    if ( item->isSelected() || hasSelection( item->firstChild() ) )
      return true;
  return false;
}

bool Kleo::KeyListView::hasSelection() const {
  return ::hasSelection( firstChild() );
}

#include "keylistview.moc"
