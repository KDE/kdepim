/*  -*- mode: C++; c-file-style: "gnu" -*-
    keylistview.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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
    if ( ( item->rtti() & Kleo::KeyListViewItem::RTTI_MASK ) == Kleo::KeyListViewItem::RTTI )
      tipStr = static_cast<const Kleo::KeyListViewItem*>( item )->toolTip( col );
    else
      tipStr = item->text( col ) ;

    if ( !tipStr.isEmpty() )
      tip( cellRect, tipStr );
  }

} // anon namespace

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
  { SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)),
    SLOT(slotEmitContextMenuRequested(QListViewItem*,const QPoint&,int)) },
};
static const int numSignalReplacements = sizeof signalReplacements / sizeof *signalReplacements;


Kleo::KeyListView::KeyListView( const ColumnStrategy * columnStrategy,
				QWidget * parent, const char * name, WFlags f )
  : KListView( parent, name ),
    mColumnStrategy( columnStrategy ),
    mItemToolTip( 0 )
{
  setWFlags( f );

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
  mItemToolTip = new ItemToolTip( this );
}

Kleo::KeyListView::~KeyListView() {
  delete mColumnStrategy; mColumnStrategy = 0;
  delete mItemToolTip; mItemToolTip = 0;
}

void Kleo::KeyListView::slotAddKey( const GpgME::Key & key ) {
  if ( !key.isNull() )
    (void)new KeyListViewItem( this, key );
}

// slots for the emission of covariant signals:

void Kleo::KeyListView::slotEmitDoubleClicked( QListViewItem * item, const QPoint & p, int col ) {
  if ( !item || ( item->rtti() & KeyListViewItem::RTTI_MASK ) == KeyListViewItem::RTTI )
    emit doubleClicked( static_cast<KeyListViewItem*>( item ), p, col );
}

void Kleo::KeyListView::slotEmitReturnPressed( QListViewItem * item ) {
  if ( !item || ( item->rtti() & KeyListViewItem::RTTI_MASK ) == KeyListViewItem::RTTI )
    emit returnPressed( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitSelectionChanged( QListViewItem * item ) {
  if ( !item || ( item->rtti() & KeyListViewItem::RTTI_MASK ) == KeyListViewItem::RTTI )
    emit selectionChanged( static_cast<KeyListViewItem*>( item ) );
}

void Kleo::KeyListView::slotEmitContextMenuRequested( QListViewItem * item, const QPoint & p, int col ) {
  if ( !item || ( item->rtti() & KeyListViewItem::RTTI_MASK ) == KeyListViewItem::RTTI )
    emit contextMenuRequested( static_cast<KeyListViewItem*>( item ), p, col );
}

//
//
// KeyListViewItem
//
//

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, const GpgME::Key & key )
  : KListViewItem( parent ), mKey( key )
{

}

Kleo::KeyListViewItem::KeyListViewItem( KeyListView * parent, KeyListViewItem * after, const GpgME::Key & key )
  : KListViewItem( parent, after ), mKey( key )
{

}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, const GpgME::Key & key )
  : KListViewItem( parent ), mKey( key )
{

}

Kleo::KeyListViewItem::KeyListViewItem( KeyListViewItem * parent, KeyListViewItem * after, const GpgME::Key & key )
  : KListViewItem( parent, after ), mKey( key )
{

}

void Kleo::KeyListViewItem::setKey( const GpgME::Key & key ) {
  mKey = key;
  repaint();
}

QString Kleo::KeyListViewItem::text( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->text( key(), col )
    : QString::null ;
}

QString Kleo::KeyListViewItem::toolTip( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->toolTip( key(), col )
    : QString::null ;
}

const QPixmap * Kleo::KeyListViewItem::pixmap( int col ) const {
  return listView() && listView()->columnStrategy()
    ? listView()->columnStrategy()->pixmap( key(), col ) : 0 ;
}

int Kleo::KeyListViewItem::compare( QListViewItem * item, int col, bool ascending ) const {
  if ( !item || item->rtti() != RTTI || !listView() || !listView()->columnStrategy() )
    return KListViewItem::compare( item, col, ascending );
  KeyListViewItem * that = static_cast<KeyListViewItem*>( item );
  return listView()->columnStrategy()->compare( this->key(), that->key(), col );
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
// Collection of covariant return reimplementations of QListView(Item)
// members:
//
//

Kleo::KeyListView * Kleo::KeyListViewItem::listView() const {
  return static_cast<Kleo::KeyListView*>( KListViewItem::listView() );
}

Kleo::KeyListViewItem * Kleo::KeyListViewItem::nextSibling() const {
  return static_cast<Kleo::KeyListViewItem*>( KListViewItem::nextSibling() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::firstChild() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::firstChild() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::selectedItem() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::selectedItem() );
}



#include "keylistview.moc"
