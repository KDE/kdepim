/*  -*- c++ -*-
    dnattributeorderconfigwidget.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

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

#include "dnattributeorderconfigwidget.h"

#include "kleo/dn.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kapplication.h>

#include <qtoolbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qtooltip.h>

#include <assert.h>

struct Kleo::DNAttributeOrderConfigWidget::Private {
  enum { UUp=0, Up=1, Left=2, Right=3, Down=4, DDown=5 };

  QListView * availableLV;
  QListView * currentLV;
  QToolButton * navTB[6];

  QListViewItem * placeHolderItem;

  Kleo::DNAttributeMapper * mapper;
};

static void prepare( QListView * lv ) {
  lv->setAllColumnsShowFocus( true );
  lv->setResizeMode( QListView::LastColumn );
  lv->header()->setClickEnabled( false );
  lv->addColumn( QString::null );
  lv->addColumn( i18n("Description") );
}

Kleo::DNAttributeOrderConfigWidget::DNAttributeOrderConfigWidget( DNAttributeMapper * mapper, QWidget * parent, const char * name, WFlags f )
  : QWidget( parent, name, f ), d( 0 )
{
  assert( mapper );
  d = new Private();
  d->mapper = mapper;

  QGridLayout * glay = new QGridLayout( this, 2, 3, 0, KDialog::spacingHint() );
  glay->setColStretch( 0, 1 );
  glay->setColStretch( 2, 1 );

  int row = -1;

  ++row;
  glay->addWidget( new QLabel( i18n("Available attributes:"), this ), row, 0 );
  glay->addWidget( new QLabel( i18n("Current attribute order:"), this ), row, 2 );


  ++row;
  glay->setRowStretch( row, 1 );

  d->availableLV = new QListView( this );
  prepare( d->availableLV );
  d->availableLV->setSorting( 0 );
  glay->addWidget( d->availableLV, row, 0 );

  d->currentLV = new QListView( this );
  prepare( d->currentLV );
  d->currentLV->setSorting( -1 );
  glay->addWidget( d->currentLV, row, 2 );

  connect( d->availableLV, SIGNAL(selectionChanged(QListViewItem*)),
	   SLOT(slotAvailableSelectionChanged(QListViewItem*)) );
  connect( d->currentLV, SIGNAL(selectionChanged(QListViewItem*)),
	   SLOT(slotCurrentOrderSelectionChanged(QListViewItem*)) );

  d->placeHolderItem = new QListViewItem( d->availableLV, "_X_", i18n("All others") );

  // the up/down/left/right arrow cross:

  QGridLayout * xlay = new QGridLayout( 5, 3, 0, "xlay" );
  xlay->setAlignment( AlignCenter );

  static const struct {
    const char * icon;
    int row, col;
    const char * tooltip;
    const char * slot;
  } navButtons[] = {
    { "2uparrow",    0, 1, I18N_NOOP( "Move to top" ),    SLOT(slotDoubleUpButtonClicked()) },
    { "1uparrow",    1, 1, I18N_NOOP( "Move one up" ),    SLOT(slotUpButtonClicked()) },
    { "1leftarrow",  2, 0, I18N_NOOP( "Remove from current attribute order" ), SLOT(slotLeftButtonClicked()) },
    { "1rightarrow", 2, 2, I18N_NOOP( "Add to current attribute order" ), SLOT(slotRightButtonClicked()) },
    { "1downarrow",  3, 1, I18N_NOOP( "Move one down" ),  SLOT(slotDownButtonClicked()) },
    { "2downarrow",  4, 1, I18N_NOOP( "Move to bottom" ), SLOT(slotDoubleDownButtonClicked()) }
  };

  for ( unsigned int i = 0 ; i < sizeof navButtons / sizeof *navButtons ; ++i ) {
    QToolButton * tb = d->navTB[i] = new QToolButton( this );
    tb->setIconSet( SmallIconSet( navButtons[i].icon ) );
    tb->setEnabled( false );
    QToolTip::add( tb, i18n( navButtons[i].tooltip ) );
    xlay->addWidget( tb, navButtons[i].row, navButtons[i].col );
    connect( tb, SIGNAL(clicked()), navButtons[i].slot );
  }

  glay->addLayout( xlay, row, 1 );
}

Kleo::DNAttributeOrderConfigWidget::~DNAttributeOrderConfigWidget() {
  delete d; d = 0;
}

void Kleo::DNAttributeOrderConfigWidget::load() {
  // save the _X_ item:
  takePlaceHolderItem();
  // clear the rest:
  d->availableLV->clear();
  d->currentLV->clear();

  const QStringList order = d->mapper->attributeOrder();

  // fill the RHS listview:
  QListViewItem * last = 0;
  for ( QStringList::const_iterator it = order.begin() ; it != order.end() ; ++it ) {
    const QString attr = (*it).upper();
    if ( attr == "_X_" ) {
      takePlaceHolderItem();
      d->currentLV->insertItem( d->placeHolderItem );
      d->placeHolderItem->moveItem( last );
      last = d->placeHolderItem;
    } else
      last = new QListViewItem( d->currentLV, last, attr, d->mapper->name2label( attr ) );
  }

  // fill the LHS listview with what's left:

  const QStringList all = Kleo::DNAttributeMapper::instance()->names();
  for ( QStringList::const_iterator it = all.begin() ; it != all.end() ; ++it )
    if ( order.find( *it ) == order.end() )
      (void)new QListViewItem( d->availableLV, *it, d->mapper->name2label( *it ) );

  if ( !d->placeHolderItem->listView() )
    d->availableLV->insertItem( d->placeHolderItem );
}

void Kleo::DNAttributeOrderConfigWidget::takePlaceHolderItem() {
  if ( QListView * lv = d->placeHolderItem->listView() )
    lv->takeItem( d->placeHolderItem );
}

void Kleo::DNAttributeOrderConfigWidget::save() const {
  QStringList order;
  for ( QListViewItemIterator it( d->currentLV ) ; it.current() ; ++it )
    order.push_back( it.current()->text( 0 ) );

  d->mapper->setAttributeOrder( order );
}

void Kleo::DNAttributeOrderConfigWidget::defaults() {
  kdDebug() << "Sorry, not implemented: Kleo::DNAttributeOrderConfigWidget::defaults()" << endl;
}



void Kleo::DNAttributeOrderConfigWidget::slotAvailableSelectionChanged( QListViewItem * item ) {
  d->navTB[Private::Right]->setEnabled( item );
}

void Kleo::DNAttributeOrderConfigWidget::slotCurrentOrderSelectionChanged( QListViewItem * item ) {
  enableDisableButtons( item );
}

void Kleo::DNAttributeOrderConfigWidget::enableDisableButtons( QListViewItem * item ) {
  d->navTB[Private::UUp  ]->setEnabled( item && item->itemAbove() );
  d->navTB[Private::Up   ]->setEnabled( item && item->itemAbove() );
  d->navTB[Private::Left ]->setEnabled( item );
  d->navTB[Private::Down ]->setEnabled( item && item->itemBelow() );
  d->navTB[Private::DDown]->setEnabled( item && item->itemBelow() );
}

void Kleo::DNAttributeOrderConfigWidget::slotUpButtonClicked() {
  QListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  QListViewItem * above = item->itemAbove();
  if ( !above )
    return;
  above->moveItem( item ); // moves "above" to after "item", ie. "item" one up
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleUpButtonClicked() {
  QListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  if ( item == d->currentLV->firstChild() )
    return;
  d->currentLV->takeItem( item );
  d->currentLV->insertItem( item );
  d->currentLV->setSelected( item, true );
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDownButtonClicked() {
  QListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  QListViewItem * below = item->itemBelow();
  if ( !below )
    return;
  item->moveItem( below ); // moves "item" to after "below", ie. "item" one down
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleDownButtonClicked() {
  QListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  QListViewItem * last = d->currentLV->lastItem();
  assert( last );
  if ( item == last )
    return;
  item->moveItem( last ); // moves "item" to after "last", ie. to the bottom
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotLeftButtonClicked() {
  QListViewItem * right = d->currentLV->selectedItem();
  if ( !right )
    return;
  QListViewItem * next = right->itemBelow();
  if ( !next )
    next = right->itemAbove();
  d->currentLV->takeItem( right );
  d->availableLV->insertItem( right );
  if ( next )
    d->currentLV->setSelected( next, true );
  enableDisableButtons( next );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotRightButtonClicked() {
  QListViewItem * left = d->availableLV->selectedItem();
  if ( !left )
    return;
  QListViewItem * next = left->itemBelow();
  if ( !next )
    next = left->itemAbove();
  d->availableLV->takeItem( left );
  d->currentLV->insertItem( left );
  if ( QListViewItem * right = d->currentLV->selectedItem() ) {
    if ( QListViewItem * above = right->itemAbove() )
      left->moveItem( above ); // move new item immediately before old selected
    d->currentLV->setSelected( right, false );
  }
  d->currentLV->setSelected( left, true );
  enableDisableButtons( left );
  d->navTB[Private::Right]->setEnabled( next );
  if ( next )
    d->availableLV->setSelected( next, true );
  emit changed();
}



void Kleo::DNAttributeOrderConfigWidget::virtual_hook( int, void* ) {}

#include "dnattributeorderconfigwidget.moc"
