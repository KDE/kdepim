/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kabc/field.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

#include "incsearchwidget.h"

IncSearchWidget::IncSearchWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
  setCaption( i18n( "Incremental Search" ) );

  QHBoxLayout *layout = new QHBoxLayout( this, 2, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Incremental search:" ), this );
  label->setAlignment( QLabel::AlignVCenter | QLabel::AlignRight );
  layout->addWidget( label );

  mSearchText = new KLineEdit( this );
  layout->addWidget( mSearchText );

  mFieldCombo = new QComboBox( false, this );
  layout->addWidget( mFieldCombo );

  QToolTip::add( mFieldCombo, i18n( "Select Incremental Search Field" ) );

  resize( QSize(420, 50).expandedTo( sizeHint() ) );

  connect( mSearchText, SIGNAL( textChanged( const QString& ) ),
           SLOT( announceDoSearch() ) );
  connect( mSearchText, SIGNAL( returnPressed() ),
           SLOT( announceDoSearch() ) );
  connect( mFieldCombo, SIGNAL( activated( const QString& ) ),
           SLOT( announceDoSearch() ) );
  connect( mFieldCombo, SIGNAL( activated( const QString& ) ),
           SLOT( announceFieldChanged() ) );
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::announceDoSearch()
{
  emit doSearch( mSearchText->text() );
}

void IncSearchWidget::announceFieldChanged()
{
  emit fieldChanged();
}

void IncSearchWidget::setFields( const KABC::Field::List &list )
{
  mFieldCombo->clear();

  KABC::Field::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    mFieldCombo->insertItem( (*it)->label() );

  mFieldList = list;

  announceDoSearch();
  announceFieldChanged();
}

KABC::Field *IncSearchWidget::currentField()const
{
  if ( mFieldCombo->currentItem() >= (int)mFieldList.count() ||
       mFieldCombo->currentItem() == -1 )
    return 0;
  else
    return mFieldList[ mFieldCombo->currentItem() ];
}

#include "incsearchwidget.moc"
