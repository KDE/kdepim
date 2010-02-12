/*  -*- c++ -*-
    dnattributeorderconfigwidget.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarävdalens Datakonsult AB

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
#include <QGridLayout>
#include <QLayout>
#include <QLabel>

#include <QTreeWidget>
#include <QHeaderView>

#include <cassert>

class Kleo::DNAttributeOrderConfigWidget::Private {
public:
  enum { UUp=0, Up=1, Left=2, Right=3, Down=4, DDown=5 };

  QTreeWidget * availableLV;
  QTreeWidget* currentLV;
  QToolButton * navTB[6];

  QTreeWidgetItem * placeHolderItem;

  Kleo::DNAttributeMapper * mapper;
};

static void prepare( QTreeWidget * lv ) {
  lv->setAllColumnsShowFocus( true );
  lv->header()->setStretchLastSection( true );
  lv->setHeaderLabels( QStringList() << QString() << i18n("Description") );
}

Kleo::DNAttributeOrderConfigWidget::DNAttributeOrderConfigWidget( DNAttributeMapper * mapper, QWidget * parent, Qt::WindowFlags f )
  : QWidget( parent, f ), d( new Private )
{
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

  d->availableLV = new QTreeWidget( this );
  prepare( d->availableLV );
  d->availableLV->sortItems( 0, Qt::AscendingOrder );
  glay->addWidget( d->availableLV, row, 0 );

  d->currentLV = new QTreeWidget( this );
  prepare( d->currentLV );
  glay->addWidget( d->currentLV, row, 2 );

  connect( d->availableLV, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           SLOT(slotAvailableSelectionChanged(QTreeWidgetItem*)) );
  connect( d->currentLV, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           SLOT(slotCurrentOrderSelectionChanged(QTreeWidgetItem*)) );

  d->placeHolderItem = new QTreeWidgetItem( d->availableLV );
  d->placeHolderItem->setText( 0, "_X_" );
  d->placeHolderItem->setText( 1, i18n("All others") );

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
    { "go-top",    0, 1, I18N_NOOP( "Move to top" ),    SLOT(slotDoubleUpButtonClicked()) },
    { "go-up",    1, 1, I18N_NOOP( "Move one up" ),    SLOT(slotUpButtonClicked()) },
    { "go-previous",  2, 0, I18N_NOOP( "Remove from current attribute order" ), SLOT(slotLeftButtonClicked()) },
    { "go-next", 2, 2, I18N_NOOP( "Add to current attribute order" ), SLOT(slotRightButtonClicked()) },
    { "go-down",  3, 1, I18N_NOOP( "Move one down" ),  SLOT(slotDownButtonClicked()) },
    { "go-bottom",  4, 1, I18N_NOOP( "Move to bottom" ), SLOT(slotDoubleDownButtonClicked()) }
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
  QTreeWidgetItem* last = 0;
  for ( QStringList::const_iterator it = order.begin() ; it != order.end() ; ++it ) {
    const QString attr = (*it).toUpper();
    if ( attr == "_X_" ) {
      takePlaceHolderItem();
      d->currentLV->insertTopLevelItem( d->currentLV->topLevelItemCount(), d->placeHolderItem );
      last = d->placeHolderItem;
    } else {
      last = new QTreeWidgetItem( d->currentLV, last );
      last->setText( 0, attr );
      last->setText( 1, d->mapper->name2label( attr ) );
    }
  }

  // fill the LHS listview with what's left:

  const QStringList all = Kleo::DNAttributeMapper::instance()->names();
  for ( QStringList::const_iterator it = all.begin() ; it != all.end() ; ++it ) {
    if ( !order.contains( *it )  ) {
      QTreeWidgetItem *item = new QTreeWidgetItem( d->availableLV );
      item->setText( 0, *it );
      item->setText( 1, d->mapper->name2label( *it ) );
    }
  }

  if ( !d->placeHolderItem->treeWidget() )
    d->availableLV->addTopLevelItem( d->placeHolderItem );
}

void Kleo::DNAttributeOrderConfigWidget::takePlaceHolderItem() {
  if ( QTreeWidget* lv = d->placeHolderItem->treeWidget() )
    lv->takeTopLevelItem( lv->indexOfTopLevelItem( d->placeHolderItem ) );
}

void Kleo::DNAttributeOrderConfigWidget::save() const {
  QStringList order;
  for ( QTreeWidgetItemIterator it( d->currentLV ) ; (*it) ; ++it )
    order.push_back( (*it)->text( 0 ) );

  d->mapper->setAttributeOrder( order );
}

void Kleo::DNAttributeOrderConfigWidget::defaults() {
  kDebug(5150) <<"Sorry, not implemented: Kleo::DNAttributeOrderConfigWidget::defaults()";
}



void Kleo::DNAttributeOrderConfigWidget::slotAvailableSelectionChanged( QTreeWidgetItem * item ) {
  d->navTB[Private::Right]->setEnabled( item );
}

void Kleo::DNAttributeOrderConfigWidget::slotCurrentOrderSelectionChanged( QTreeWidgetItem * item ) {
  enableDisableButtons( item );
}

void Kleo::DNAttributeOrderConfigWidget::enableDisableButtons( QTreeWidgetItem * item ) {
  d->navTB[Private::UUp  ]->setEnabled( item && d->currentLV->itemAbove( item ) );
  d->navTB[Private::Up   ]->setEnabled( item && d->currentLV->itemAbove( item ) );
  d->navTB[Private::Left ]->setEnabled( item );
  d->navTB[Private::Down ]->setEnabled( item && d->currentLV->itemBelow( item ) );
  d->navTB[Private::DDown]->setEnabled( item && d->currentLV->itemBelow( item ) );
}

void Kleo::DNAttributeOrderConfigWidget::slotUpButtonClicked() {
  if ( d->currentLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * item = d->currentLV->selectedItems().first();
  int itemIndex = d->currentLV->indexOfTopLevelItem( item );
  if ( itemIndex <= 0 )
    return;
  d->currentLV->takeTopLevelItem( itemIndex );
  d->currentLV->insertTopLevelItem( itemIndex - 1, item );
  d->currentLV->clearSelection();
  item->setSelected( true );
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleUpButtonClicked() {
  if ( d->currentLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * item = d->currentLV->selectedItems().first();
  int itemIndex = d->currentLV->indexOfTopLevelItem( item );
  if ( itemIndex == 0 )
    return;
  d->currentLV->takeTopLevelItem( itemIndex );
  d->currentLV->insertTopLevelItem( 0, item );
  d->currentLV->clearSelection();
  item->setSelected( true );
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDownButtonClicked() {
  if ( d->currentLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * item = d->currentLV->selectedItems().first();
  int itemIndex = d->currentLV->indexOfTopLevelItem( item );
  if ( itemIndex + 1 >= d->currentLV->topLevelItemCount() )
    return;
  d->currentLV->takeTopLevelItem( itemIndex );
  d->currentLV->insertTopLevelItem( itemIndex + 1, item );
  d->currentLV->clearSelection();
  item->setSelected( true );
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotDoubleDownButtonClicked() {
  if ( d->currentLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * item = d->currentLV->selectedItems().first();
  const int itemIndex = d->currentLV->indexOfTopLevelItem( item );
  if ( itemIndex + 1 >= d->currentLV->topLevelItemCount() )
    return;
  d->currentLV->takeTopLevelItem( itemIndex );
  d->currentLV->addTopLevelItem( item );
  d->currentLV->clearSelection();
  item->setSelected( true );
  enableDisableButtons( item );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotLeftButtonClicked() {
  if ( d->currentLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * right = d->currentLV->selectedItems().first();
  QTreeWidgetItem * next = d->currentLV->itemBelow( right );
  if ( !next )
    next = d->currentLV->itemAbove( right );
  d->currentLV->takeTopLevelItem( d->currentLV->indexOfTopLevelItem( right ) );
  d->availableLV->addTopLevelItem( right );
  d->availableLV->sortItems( 0, Qt::AscendingOrder );
  if ( next )
    next->setSelected( true );
  enableDisableButtons( next );
  emit changed();
}

void Kleo::DNAttributeOrderConfigWidget::slotRightButtonClicked() {
  if ( d->availableLV->selectedItems().size() <= 0 )
    return;
  QTreeWidgetItem * left = d->availableLV->selectedItems().first();
  QTreeWidgetItem* next = d->availableLV->itemBelow( left );
  if ( !next )
    next = d->availableLV->itemAbove( left );
  d->availableLV->takeTopLevelItem( d->availableLV->indexOfTopLevelItem( left ) );
  int newRightIndex = d->currentLV->topLevelItemCount();
  if ( d->currentLV->selectedItems().size() > 0 ) {
    QTreeWidgetItem * right = d->currentLV->selectedItems().first();
    newRightIndex = d->currentLV->indexOfTopLevelItem( right );
    right->setSelected( false );
  }
  d->currentLV->insertTopLevelItem( newRightIndex, left );
  left->setSelected( true );
  enableDisableButtons( left );
  d->navTB[Private::Right]->setEnabled( next );
  if ( next )
    next->setSelected( true );
  emit changed();
}



void Kleo::DNAttributeOrderConfigWidget::virtual_hook( int, void* ) {}

#include "dnattributeorderconfigwidget.moc"
