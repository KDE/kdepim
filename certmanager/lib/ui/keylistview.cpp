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

// a list of signals where we want to repleace QListViewItem with
// Kleo:KeyListViewItem:
static const struct {
  const char * source;
  const char * target;
} signalReplacements[] = {
  { SIGNAL(doubleClicked(QListViewItem*,const QPoint&,int)),
    SLOT(slotEmitDoubleClicked(QListViewItem*,const QPoint&,int)) },
  { SIGNAL(returnPressed(QListViewItem*)),
    SLOT(slotEmitReturnPressed(QListViewItem*)) }
};
static const int numSignalReplacements = sizeof signalReplacements / sizeof *signalReplacements;


Kleo::KeyListView::KeyListView( const ColumnStrategy * columnStrategy,
				QWidget * parent, const char * name, WFlags f )
  //: KListView( parent, name, f ) // doesn't work - shitty KListView
  : KListView( parent, name ),
    mColumnStrategy( columnStrategy )
{
  setWFlags( f );

  if ( !columnStrategy ) {
    kdWarning() << "Kleo::KeyListView: need a column strategy to work with!" << endl;
    return;
  }

  const QFontMetrics fm = fontMetrics();

  for ( int col = 0 ; !columnStrategy->title( col ).isEmpty() ; ++col ) {
    addColumn( columnStrategy->title( col ), columnStrategy->width( col, fm ) );
    setColumnWidthMode( col, columnStrategy->widthMode( col ) );
  }

  setAllColumnsShowFocus( true );

  for ( int i = 0 ; i < numSignalReplacements ; ++i )
    connect( this, signalReplacements[i].source, signalReplacements[i].target );
}

Kleo::KeyListView::~KeyListView() {
  delete mColumnStrategy;
}

void Kleo::KeyListView::slotAddKey( const GpgME::Key & key ) {
  if ( !key.isNull() )
    (void)new KeyListViewItem( this, key );
}

void Kleo::KeyListView::slotEmitDoubleClicked( QListViewItem * item, const QPoint & p, int col ) {
  if ( !item || item->rtti() == KeyListViewItem::RTTI )
    emit doubleClicked( static_cast<KeyListViewItem*>( item ), p, col );
}

void Kleo::KeyListView::slotEmitReturnPressed( QListViewItem * item ) {
  if ( !item || item->rtti() == KeyListViewItem::RTTI )
    emit returnPressed( static_cast<KeyListViewItem*>( item ) );
}

//
//
// KListViewItem
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

//
//
// Collection of covariant return reimplementations of QListView(Item)
// members:
//
//

Kleo::KeyListView * Kleo::KeyListViewItem::listView() const {
  return static_cast<Kleo::KeyListView*>( KListViewItem::listView() );
}

Kleo::KeyListViewItem * Kleo::KeyListView::selectedItem() const {
  return static_cast<Kleo::KeyListViewItem*>( KListView::selectedItem() );
}

#include "keylistview.moc"
