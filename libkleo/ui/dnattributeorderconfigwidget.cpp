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

#include "dnattributeorderconfigwidget.h"

#include "kleo/dn.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kconfig.h>

#include <QToolButton>
#include <QLayout>
#include <q3header.h>
#include <QLabel>
#include <q3listview.h>

//Added by qt3to4:
#include <QGridLayout>

#include <assert.h>

class Kleo::DNAttributeOrderConfigWidget::Private {
public:
  enum { UUp=0, Up=1, Left=2, Right=3, Down=4, DDown=5 };

  Q3ListView * availableLV;
  Q3ListView * currentLV;
  QToolButton * navTB[6];

  Q3ListViewItem * placeHolderItem;

  Kleo::DNAttributeMapper * mapper;
};

static void prepare( Q3ListView * lv ) {
  lv->setAllColumnsShowFocus( true );
  lv->setResizeMode( Q3ListView::LastColumn );
  lv->header()->setClickEnabled( false );
  lv->addColumn( QString() );
  lv->addColumn( i18n("Description") );
}

Kleo::DNAttributeOrderConfigWidget::DNAttributeOrderConfigWidget( DNAttributeMapper * mapper, QWidget * parent, const char * name, Qt::WFlags f )
  : QWidget( parent, f ), d( new Private() )
{
  setObjectName(name);
  assert( mapper );
  d->mapper = mapper;

  QGridLayout * glay = new QGridLayout( this );
  glay->setMargin( 0 );
  glay->setSpacing( KDialog::spacingHint() );
  glay->setColumnStretch( 0, 1 );
  glay->setColumnStretch( 2, 1 );

  int row = -1;

  ++row;
  glay->addWidget( new QLabel( i18n("Available attributes:"), this ), row, 0 );
  glay->addWidget( new QLabel( i18n("Current attribute order:"), this ), row, 2 );


  ++row;
  glay->setRowStretch( row, 1 );

  d->availableLV = new Q3ListView( this );
  prepare( d->availableLV );
  d->availableLV->setSorting( 0 );
  glay->addWidget( d->availableLV, row, 0 );

  d->currentLV = new Q3ListView( this );
  prepare( d->currentLV );
  d->currentLV->setSorting( -1 );
  glay->addWidget( d->currentLV, row, 2 );

  connect( d->availableLV, SIGNAL(clicked(Q3ListViewItem*)),
	   SLOT(slotAvailableSelectionChanged(Q3ListViewItem*)) );
  connect( d->currentLV, SIGNAL(clicked(Q3ListViewItem*)),
	   SLOT(slotCurrentOrderSelectionChanged(Q3ListViewItem*)) );

  d->placeHolderItem = new Q3ListViewItem( d->availableLV, "_X_", i18n("All others") );

  // the up/down/left/right arrow cross:

  QGridLayout * xlay = new QGridLayout();
  xlay->setSpacing( 0 );
  xlay->setObjectName( "xlay" );
  xlay->setAlignment( Qt::AlignCenter );

  static const struct {
    const char * icon;
    int row, col;
    const char * tooltip;
    const char * slot;
  } navButtons[] = {
    { "arrow-up-double",    0, 1, I18N_NOOP( "Move to top" ),    SLOT(slotDoubleUpButtonClicked()) },
    { "arrow-up",    1, 1, I18N_NOOP( "Move one up" ),    SLOT(slotUpButtonClicked()) },
    { "arrow-left",  2, 0, I18N_NOOP( "Remove from current attribute order" ), SLOT(slotLeftButtonClicked()) },
    { "arrow-right", 2, 2, I18N_NOOP( "Add to current attribute order" ), SLOT(slotRightButtonClicked()) },
    { "arrow-down",  3, 1, I18N_NOOP( "Move one down" ),  SLOT(slotDownButtonClicked()) },
    { "arrow-down-double",  4, 1, I18N_NOOP( "Move to bottom" ), SLOT(slotDoubleDownButtonClicked()) }
  };

  for ( unsigned int i = 0 ; i < sizeof navButtons / sizeof *navButtons ; ++i ) {
    QToolButton * tb = d->navTB[i] = new QToolButton( this );
    tb->setIcon( KIcon( navButtons[i].icon ) );
    tb->setEnabled( false );
    tb->setToolTip( i18n( navButtons[i].tooltip ) );
    xlay->addWidget( tb, navButtons[i].row, navButtons[i].col );
    connect( tb, SIGNAL(clicked()), navButtons[i].slot );
  }

  glay->addLayout( xlay, row, 1 );
}

Kleo::DNAttributeOrderConfigWidget::~DNAttributeOrderConfigWidget() {
  delete d;
}

void Kleo::DNAttributeOrderConfigWidget::load() {
  // save the _X_ item:
  takePlaceHolderItem();
  // clear the rest:
  d->availableLV->clear();
  d->currentLV->clear();

  const QStringList order = d->mapper->attributeOrder();

  // fill the RHS listview:
  Q3ListViewItem * last = 0;
  for ( QStringList::const_iterator it = order.begin() ; it != order.end() ; ++it ) {
    const QString attr = (*it).toUpper();
    if ( attr == "_X_" ) {
      takePlaceHolderItem();
      d->currentLV->insertItem( d->placeHolderItem );
      d->placeHolderItem->moveItem( last );
      last = d->placeHolderItem;
    } else
      last = new Q3ListViewItem( d->currentLV, last, attr, d->mapper->name2label( attr ) );
  }

  // fill the LHS listview with what's left:

  const QStringList all = Kleo::DNAttributeMapper::instance()->names();
  for ( QStringList::const_iterator it = all.begin() ; it != all.end() ; ++it )
    if ( !order.contains( *it )  )
      (void)new Q3ListViewItem( d->availableLV, *it, d->mapper->name2label( *it ) );

  if ( !d->placeHolderItem->listView() )
    d->availableLV->insertItem( d->placeHolderItem );
}

void Kleo::DNAttributeOrderConfigWidget::takePlaceHolderItem() {
  if ( Q3ListView * lv = d->placeHolderItem->listView() )
    lv->takeItem( d->placeHolderItem );
}

void Kleo::DNAttributeOrderConfigWidget::save() const {
  QStringList order;
  for ( Q3ListViewItemIterator it( d->currentLV ) ; it.current() ; ++it )
    order.push_back( it.current()->text( 0 ) );

  d->mapper->setAttributeOrder( order );
}

void Kleo::DNAttributeOrderConfigWidget::defaults() {
  kDebug() << "Sorry, not implemented: Kleo::DNAttributeOrderConfigWidget::defaults()" << endl;
}



void Kleo::DNAttributeOrderConfigWidget::slotAvailableSelectionChanged( Q3ListViewItem * item ) {
  d->navTB[Private::Right]->setEnabled( item );
}

void Kleo::DNAttributeOrderConfigWidget::slotCurrentOrderSelectionChanged( Q3ListViewItem * item ) {
  enableDisableButtons( item );
}

void Kleo::DNAttributeOrderConfigWidget::enableDisableButtons( Q3ListViewItem * item ) {
  d->navTB[Private::UUp  ]->setEnabled( item && item->itemAbove() );
  d->navTB[Private::Up   ]->setEnabled( item && item->itemAbove() );
  d->navTB[Private::Left ]->setEnabled( item );
  d->navTB[Private::Down ]->setEnabled( item && item->itemBelow() );
  d->navTB[Private::DDown]->setEnabled( item && item->itemBelow() );
}

void Kleo::DNAttributeOrderConfigWidget::slotUpButtonClicked() {
  Q3ListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  Q3ListViewItem * above = item->itemAbove();
  if ( !above )
    return;
  above->moveItem( item ); // moves "above" to after "item", ie. "item" one up
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleUpButtonClicked() {
  Q3ListViewItem * item = d->currentLV->selectedItem();
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
  Q3ListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  Q3ListViewItem * below = item->itemBelow();
  if ( !below )
    return;
  item->moveItem( below ); // moves "item" to after "below", ie. "item" one down
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleDownButtonClicked() {
  Q3ListViewItem * item = d->currentLV->selectedItem();
  if ( !item )
    return;
  Q3ListViewItem * last = d->currentLV->lastItem();
  assert( last );
  if ( item == last )
    return;
  item->moveItem( last ); // moves "item" to after "last", ie. to the bottom
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotLeftButtonClicked() {
  Q3ListViewItem * right = d->currentLV->selectedItem();
  if ( !right )
    return;
  Q3ListViewItem * next = right->itemBelow();
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
  Q3ListViewItem * left = d->availableLV->selectedItem();
  if ( !left )
    return;
  Q3ListViewItem * next = left->itemBelow();
  if ( !next )
    next = left->itemAbove();
  d->availableLV->takeItem( left );
  d->currentLV->insertItem( left );
  if ( Q3ListViewItem * right = d->currentLV->selectedItem() ) {
    if ( Q3ListViewItem * above = right->itemAbove() )
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
