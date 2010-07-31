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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "keylistview.h"

#include <kdebug.h>

#include <tqfontmetrics.h>
#include <tqtooltip.h>
#include <tqrect.h>
#include <tqheader.h>
#include <tqpoint.h>
#include <tqptrlist.h>
#include <tqpainter.h>
#include <tqfont.h>
#include <tqcolor.h>
#include <tqtimer.h>
#include <tqcstring.h>

#include <gpgmepp/key.h>

#include <vector>
#include <map>

#include <assert.h>

static const int updateDelayMilliSecs = 500;

namespace {

  class ItemToolTip : public TQToolTip {
  public:
    ItemToolTip( Kleo::KeyListView * parent );
  protected:
    void maybeTip( const TQPoint & p );
  private:
    Kleo::KeyListView * mKeyListView;
  };

  ItemToolTip::ItemToolTip( Kleo::KeyListView * parent )
    : TQToolTip( parent->viewport() ), mKeyListView( parent ) {}

  void ItemToolTip::maybeTip( const TQPoint & p ) {
    if ( !mKeyListView )
      return;

    const TQListViewItem * item = mKeyListView->itemAt( p );
    if ( !item )
      return;

    const TQRect itemRect = mKeyListView->itemRect( item );
    if ( !itemRect.isValid() )
      return;

    const int col = mKeyListView->header()->sectionAt( p.x() );
    if ( col == -1 )
      return;

    const TQRect headerRect = mKeyListView->header()->sectionRect( col );
    if ( !headerRect.isValid() )
      return;

    const TQRect cellRect( headerRect.left(), itemRect.top(),
			  headerRect.width(), itemRect.height() );

    TQString tipStr;
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
  TQTimer * updateTimer;
  TQToolTip * itemToolTip;
  std::map<TQCString,KeyListViewItem*> itemMap;
};

// a list of signals where we want to replace TQListViewItem with
// Kleo:KeyListViewItem:
static const struct {
  const char * source;
  const char * target;
} signalReplacements[] = {
  { TQT_SIGNAL(doubleClicked(TQListViewItem*,const TQPoint&,int)),
    TQT_SLOT(slotEmitDoubleClicked(TQListViewItem*,const TQPoint&,int)) },
  { TQT_SIGNAL(returnPressed(TQListViewItem*)),
    TQT_SLOT(slotEmitReturnPressed(TQListViewItem*)) },
  { TQT_SIGNAL(selectionChanged(TQListViewItem*)),
    TQT_SLOT(slotEmitSelectionChanged(TQListViewItem*)) },
  { TQT_SIGNAL(contextMenu(KListView*, TQListViewItem*,const TQPoint&)),
    TQT_SLOT(slotEmitContextMenu(KListView*, TQListViewItem*,const TQPoint&)) },
};
static const int numSignalReplacements = sizeof signalReplacements / sizeof *signalReplacements;


Kleo::KeyListView::KeyListView( const ColumnStrategy * columnStrategy, const DisplayStrategy * displayStrategy, TQWidget * parent, const char * name, WFlags f )
  : KListView( parent, name ),
    mColumnStrategy( columnStrategy ),
    mDisplayStrategy ( displayStrategy  ),
    mHierarchical( false )
{
  setWFlags( f );

  d = new Private();

  d->updateTimer = new TQTimer( this );
  connect( d->updateTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotUpdateTimeout()) );

  if ( !columnStrategy ) {
    kdWarning(5150) << "Kleo::KeyListView: need a column strategy to work with!" << endl;
    return;
  }

  const TQFontMetrics fm = fontMetrics();

  for ( int col = 0 ; !columnStrategy->title( col ).isEmpty() ; ++col ) {
    addColumn( columnStrategy->title( col ), columnStrategy->width( col, fm ) );
    setColumnWidthMode( col, columnStrategy->widthMode( col ) );
  }

  setAllColumnsShowFocus( true );
  setShowToolTips( false ); // we do it instead...

  for ( int i = 0 ; i < numSignalReplacements ; ++i )
    connect( this, signalReplacements[i].source, signalReplacements[i].target );

  TQToolTip::remove( this );
  TQToolTip::remove( viewport() ); // make double sure :)
  d->itemToolTip = new ItemToolTip( this );
}

Kleo::KeyListView::~KeyListView() {
  d->updateTimer->stop();
  // need to clear here, since in ~TQListView, our children won't have
  // a valid listView() pointing to us anymore, and their dtors try to
  // unregister from us.
  clear();
  assert( d->itemMap.size() == 0 );
  // need to delete the tooltip ourselves, as ~TQToolTip isn't virtual :o
  delete d->itemToolTip; d->itemToolTip = 0;
  delete d; d = 0;
  delete mColumnStrategy; mColumnStrategy = 0;
  delete mDisplayStrategy; mDisplayStrategy = 0;
}

void Kleo::KeyListView::insertItem( TQListViewItem * qlvi ) {
  //kdDebug() << "Kleo::KeyListView::insertItem( " << qlvi << " )" << endl;
  KListView::insertItem( qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    registerItem( item );
}

void Kleo::KeyListView::takeItem( TQListViewItem * qlvi ) {
  //kdDebug() << "Kleo::KeyListView::takeItem( " << qlvi << " )" << endl;
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
  //kdDebug() << "registerItem( " << item << " )" << endl;
  if ( !item )
    return;
  const TQCString fpr = item->key().primaryFingerprint();
  if ( !fpr.isEmpty() )
    d->itemMap.insert( std::make_pair( fpr, item ) );
}

void Kleo::KeyListView::deregisterItem( const KeyListViewItem * item ) {
  //kdDebug() << "deregisterItem( KeyLVI: " << item << " )" << endl;
  if ( !item )
    return;
  std::map<TQCString,KeyListViewItem*>::iterator it
    = d->itemMap.find( item->key().primaryFingerprint() );
  if ( it == d->itemMap.end() )
    return;
  Q_ASSERT( it->second == item );
  if ( it->second != item )
    return;
  d->itemMap.erase( it );
}

void Kleo::KeyListView::doHierarchicalInsert( const GpgME::Key & key ) {
  const TQCString fpr = key.primaryFingerprint();
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

void Kleo::KeyListView::scatterGathered( TQListViewItem * start ) {
  TQListViewItem * item = start;
  while ( item ) {
    TQListViewItem * cur = item;
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

Kleo::KeyListViewItem * Kleo::KeyListView::itemByFingerprint( const TQCString & s ) const {
  if ( s.isEmpty() )
    return 0;
  const std::map<TQCString,KeyListViewItem*>::const_iterator it = d->itemMap.find( s );
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

void Kleo::KeyListView::slotEmitDoubleClicked( TQListViewItem * item, const TQPoint & p, int col ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit doubleClicked( static_cast<KeyListViewItem*>( item ), p, col );
}

void Kleo::KeyListView::slotEmitReturnPressed( TQListViewItem * item ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit returnPressed( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitSelectionChanged( TQListViewItem * item ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit selectionChanged( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitContextMenu( KListView*, TQListViewItem * item, const TQPoint & p ) {
  if ( !item || lvi_cast<KeyListViewItem>( item ) )
    emit contextMenu( static_cast<KeyListViewItem*>( item ), p );
}

//
//
// KeyListViewItem
//
//

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, const GpgME::Key & key )
  : TQListViewItem( parent )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key )
  : TQListViewItem( parent, after )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key )
  : TQListViewItem( parent )
{
  setKey( key );
}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key )
  : TQListViewItem( parent, after )
{
  setKey( key );
}

Kleo::KeyListViewItem::~KeyListViewItem() {
  // delete the children first... When children are deleted in the
  // QLVI dtor, they don't have listView() anymore, thus they don't
  // call deregister( this ), leading to stale entries in the
  // itemMap...
  while ( TQListViewItem * item = firstChild() )
    delete item;
  // better do this here, too, since deletion is top-down and thus
  // we're deleted when our parent item is no longer a
  // KeyListViewItem, but a mere TQListViewItem, so our takeItem()
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
    if ( const TQPixmap * pix = cs->pixmap( key, i ) )
      setPixmap( i, *pix );
  }
  repaint();
}

TQString Kleo::KeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->toolTip( key(), col )
    : TQString::null ;
}

int Kleo::KeyListViewItem::compare( TQListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return TQListViewItem::compare( item, col, ascending );
  KeyListViewItem * that = static_cast<KeyListViewItem*>( item );
  return listView()->columnStrategy()->compare( this->key(), that->key(), col );
}

void Kleo::KeyListViewItem::paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    TQListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const TQColor fg = ds->keyForeground( key(), cg.text() );
  const TQColor bg = ds->keyBackground( key(), cg.base() );
  const TQFont f = ds->keyFont( key(), p->font() );

  TQColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( TQColorGroup::Text, fg );
  _cg.setColor( TQColorGroup::Base, bg );

  TQListViewItem::paintCell( p, _cg, column, width, alignment );
}

void Kleo::KeyListViewItem::insertItem( TQListViewItem * qlvi ) {
  //kdDebug() << "Kleo::KeyListViewItem::insertItem( " << qlvi << " )" << endl;
  TQListViewItem::insertItem( qlvi );
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    listView()->registerItem( item );
}

void Kleo::KeyListViewItem::takeItem( TQListViewItem * qlvi ) {
  //kdDebug() << "Kleo::KeyListViewItem::takeItem( " << qlvi << " )" << endl;
  if ( KeyListViewItem * item = lvi_cast<KeyListViewItem>( qlvi ) )
    listView()->deregisterItem( item );
  TQListViewItem::takeItem( qlvi );
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

TQString Kleo::SubkeyKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyText( subkey(), col )
    : TQString::null ;
}

TQString Kleo::SubkeyKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyToolTip( subkey(), col )
    : TQString::null ;
}

const TQPixmap * Kleo::SubkeyKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->subkeyPixmap( subkey(), col ) : 0 ;
}

int Kleo::SubkeyKeyListViewItem::compare( TQListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  SubkeyKeyListViewItem * that = static_cast<SubkeyKeyListViewItem*>( item );
  return listView()->columnStrategy()->subkeyCompare( this->subkey(), that->subkey(), col );
}

void Kleo::SubkeyKeyListViewItem::paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    TQListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const TQColor fg = ds->subkeyForeground( subkey(), cg.text() );
  const TQColor bg = ds->subkeyBackground( subkey(), cg.base() );
  const TQFont f = ds->subkeyFont( subkey(), p->font() );

  TQColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( TQColorGroup::Text, fg );
  _cg.setColor( TQColorGroup::Base, bg );

  TQListViewItem::paintCell( p, _cg, column, width, alignment );
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

TQString Kleo::UserIDKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDText( userID(), col )
    : TQString::null ;
}

TQString Kleo::UserIDKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDToolTip( userID(), col )
    : TQString::null ;
}

const TQPixmap * Kleo::UserIDKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->userIDPixmap( userID(), col ) : 0 ;
}

int Kleo::UserIDKeyListViewItem::compare( TQListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  UserIDKeyListViewItem * that = static_cast<UserIDKeyListViewItem*>( item );
  return listView()->columnStrategy()->userIDCompare( this->userID(), that->userID(), col );
}


void Kleo::UserIDKeyListViewItem::paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    TQListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const TQColor fg = ds->useridForeground( userID(), cg.text() );
  const TQColor bg = ds->useridBackground( userID(), cg.base() );
  const TQFont f = ds->useridFont( userID(), p->font() );

  TQColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( TQColorGroup::Text, fg );
  _cg.setColor( TQColorGroup::Base, bg );

  TQListViewItem::paintCell( p, _cg, column, width, alignment );
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

TQString Kleo::SignatureKeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signatureText( signature(), col )
    : TQString::null ;
}

TQString Kleo::SignatureKeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signatureToolTip( signature(), col )
    : TQString::null ;
}

const TQPixmap * Kleo::SignatureKeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->signaturePixmap( signature(), col ) : 0 ;
}

int Kleo::SignatureKeyListViewItem::compare( TQListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KeyListViewItem::compare( item, col, ascending );
  SignatureKeyListViewItem * that = static_cast<SignatureKeyListViewItem*>( item );
  return listView()->columnStrategy()->signatureCompare( this->signature(), that->signature(), col );
}

void Kleo::SignatureKeyListViewItem::paintCell( TQPainter * p, const TQColorGroup & cg, int column, int width, int alignment ) {
  const KeyListView::DisplayStrategy * ds = listView() ? listView()->displayStrategy() : 0 ;
  if ( !ds ) {
    TQListViewItem::paintCell( p, cg, column, width, alignment );
    return;
  }
  const TQColor fg = ds->signatureForeground( signature(), cg.text() );
  const TQColor bg = ds->signatureBackground( signature(), cg.base() );
  const TQFont f = ds->signatureFont( signature(), p->font() );

  TQColorGroup _cg = cg;
  p->setFont( f );
  _cg.setColor( TQColorGroup::Text, fg );
  _cg.setColor( TQColorGroup::Base, bg );

  TQListViewItem::paintCell( p, _cg, column, width, alignment );
}


//
//
// ColumnStrategy
//
//

Kleo::KeyListView::ColumnStrategy::~ColumnStrategy() {}

int Kleo::KeyListView::ColumnStrategy::compare( const GpgME::Key & key1, const GpgME::Key & key2, int col ) const {
  return TQString::localeAwareCompare( text( key1, col ), text( key2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::width( int col, const TQFontMetrics & fm ) const {
  return fm.width( title( col ) ) * 2;
}

int Kleo::KeyListView::ColumnStrategy::subkeyCompare( const GpgME::Subkey & sub1, const GpgME::Subkey & sub2, int col ) const {
  return TQString::localeAwareCompare( subkeyText( sub1, col ), subkeyText( sub2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::userIDCompare( const GpgME::UserID & uid1, const GpgME::UserID & uid2, int col ) const {
  return TQString::localeAwareCompare( userIDText( uid1, col ), userIDText( uid2, col ) );
}

int Kleo::KeyListView::ColumnStrategy::signatureCompare( const GpgME::UserID::Signature & sig1, const GpgME::UserID::Signature & sig2, int col ) const {
  return TQString::localeAwareCompare( signatureText( sig1, col ), signatureText( sig2, col ) );
}

TQString Kleo::KeyListView::ColumnStrategy::toolTip( const GpgME::Key & key, int col ) const {
  return text( key, col );
}

TQString Kleo::KeyListView::ColumnStrategy::subkeyToolTip( const GpgME::Subkey & sub, int col ) const {
  return subkeyText( sub, col );
}

TQString Kleo::KeyListView::ColumnStrategy::userIDToolTip( const GpgME::UserID & uid, int col ) const {
  return userIDText( uid, col );
}

TQString Kleo::KeyListView::ColumnStrategy::signatureToolTip( const GpgME::UserID::Signature & sig, int col ) const {
  return signatureText( sig, col );
}

//
//
// DisplayStrategy
//
//

Kleo::KeyListView::DisplayStrategy::~DisplayStrategy() {}


//font
TQFont Kleo::KeyListView::DisplayStrategy::keyFont( const GpgME::Key &, const TQFont & font ) const {
  return font;
}

TQFont Kleo::KeyListView::DisplayStrategy::subkeyFont( const GpgME::Subkey &, const TQFont & font ) const {
  return font;
}

TQFont Kleo::KeyListView::DisplayStrategy::useridFont( const GpgME::UserID &, const TQFont & font ) const {
  return font;
}

TQFont Kleo::KeyListView::DisplayStrategy::signatureFont( const GpgME::UserID::Signature &, const TQFont & font ) const {
  return font;
}

//foreground
TQColor Kleo::KeyListView::DisplayStrategy::keyForeground( const GpgME::Key &, const TQColor & fg )const {
  return fg;
}

TQColor Kleo::KeyListView::DisplayStrategy::subkeyForeground( const GpgME::Subkey &, const TQColor & fg ) const {
  return fg;
}

TQColor Kleo::KeyListView::DisplayStrategy::useridForeground( const GpgME::UserID &, const TQColor & fg ) const {
  return fg;
}

TQColor Kleo::KeyListView::DisplayStrategy::signatureForeground( const GpgME::UserID::Signature &, const TQColor & fg ) const {
  return fg;
}

//background
TQColor Kleo::KeyListView::DisplayStrategy::keyBackground( const GpgME::Key &, const TQColor & bg )const {
  return bg;
}

TQColor Kleo::KeyListView::DisplayStrategy::subkeyBackground( const GpgME::Subkey &, const TQColor & bg ) const {
  return bg;
}

TQColor Kleo::KeyListView::DisplayStrategy::useridBackground( const GpgME::UserID &, const TQColor & bg ) const {
  return bg;
}

TQColor Kleo::KeyListView::DisplayStrategy::signatureBackground( const GpgME::UserID::Signature &, const TQColor & bg ) const {
  return bg;
}


//
//
// Collection of covariant return reimplementations of TQListView(Item)
// members:
//
//

Kleo::KeyListView * Kleo::KeyListViewItem::listView() const {
  return static_cast<Kleo::KeyListView*>( TQListViewItem::listView() );
}

Kleo::KeyListViewItem * Kleo::KeyListViewItem::nextSibling() const {
  return static_cast<Kleo::KeyListViewItem*>( TQListViewItem::nextSibling() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::firstChild() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::firstChild() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::selectedItem() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::selectedItem() );
}

static void selectedItems( TQPtrList<Kleo::KeyListViewItem> & result, TQListViewItem * start ) {
  for ( TQListViewItem * item = start ; item ; item = item->nextSibling() ) {
    if ( item->isSelected() )
      if ( Kleo::KeyListViewItem * i = Kleo::lvi_cast<Kleo::KeyListViewItem>( item ) )
	result.append( i );
    selectedItems( result, item->firstChild() );
  }
}

TQPtrList<Kleo::KeyListViewItem> Kleo::KeyListView::selectedItems() const {
  TQPtrList<KeyListViewItem> result;
  ::selectedItems( result, firstChild() );
  return result;
}

static bool hasSelection( TQListViewItem * start ) {
  for ( TQListViewItem * item = start ; item ; item = item->nextSibling() )
    if ( item->isSelected() || hasSelection( item->firstChild() ) )
      return true;
  return false;
}

bool Kleo::KeyListView::hasSelection() const {
  return ::hasSelection( firstChild() );
}

#include "keylistview.moc"
