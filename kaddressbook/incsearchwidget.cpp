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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

#include "incsearchwidget.h"

IncSearchWidget::IncSearchWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this, 2, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Search:" ), this );
  label->setAlignment( QLabel::AlignVCenter | QLabel::AlignRight );
  layout->addWidget( label );

  mSearchText = new KLineEdit( this );
  QWhatsThis::add( mSearchText, i18n( "The incremental search<p>Enter some text here will start the search for the contact, which matches the search pattern best. The part of the contact, which will be used for matching, depends on the field selection." ) );
  label->setBuddy( mSearchText );
  layout->addWidget( mSearchText );

  mFieldCombo = new QComboBox( false, this );
  layout->addWidget( mFieldCombo );

  QToolTip::add( mFieldCombo, i18n( "Select incremental search field" ) );
  QWhatsThis::add( mFieldCombo, i18n( "Here you can choose the field, which shall be used for incremental search." ) );

  connect( mSearchText, SIGNAL( textChanged( const QString& ) ),
           SLOT( announceDoSearch() ) );
  connect( mSearchText, SIGNAL( returnPressed() ),
           SLOT( announceDoSearch() ) );
  connect( mFieldCombo, SIGNAL( activated( const QString& ) ),
           SLOT( announceDoSearch() ) );

  initFields();

  setFocusProxy( mSearchText );
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::announceDoSearch()
{
  emit doSearch( mSearchText->text() );
}

void IncSearchWidget::initFields()
{
  mFieldList = KABC::Field::allFields();

  mFieldCombo->clear();
  mFieldCombo->insertItem( i18n( "All Fields" ) );

  KABC::Field::List::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
    mFieldCombo->insertItem( (*it)->label() );

  announceDoSearch();
}

KABC::Field *IncSearchWidget::currentField() const
{
  if ( mFieldCombo->currentItem() == -1 || mFieldCombo->currentItem() == 0 )
    return 0;  // for error or 'use all fields'
  else
    return mFieldList[ mFieldCombo->currentItem() - 1 ];
}

void IncSearchWidget::setCurrentItem( int pos )
{
  mFieldCombo->setCurrentItem( pos );
}

int IncSearchWidget::currentItem() const
{
  return mFieldCombo->currentItem();
}

#include "incsearchwidget.moc"
