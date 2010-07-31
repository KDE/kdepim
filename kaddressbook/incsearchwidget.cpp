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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqapplication.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

#include "incsearchwidget.h"

IncSearchWidget::IncSearchWidget( TQWidget *parent, const char *name )
    : TQWidget( parent, name )
{
  TQHBoxLayout *layout = new TQHBoxLayout( this, 2, KDialog::spacingHint() );

  TQToolButton *button = new TQToolButton( this );
  button->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Minimum );
  button->setPixmap( SmallIcon( TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase" ) );
  button->setAccel( TQKeySequence( CTRL+ALT+Key_S ) );
  button->setAutoRaise( true );
  TQToolTip::add( button, i18n( "Reset" ) );
  layout->addWidget( button );

  TQLabel *label = new TQLabel( i18n( "Search:" ), this, "kde toolbar widget" );
  label->setAlignment( TQLabel::AlignVCenter | TQLabel::AlignRight );
  layout->addWidget( label );

  mSearchText = new KLineEdit( this );
  mSearchText->setSizePolicy( TQSizePolicy::MinimumExpanding, TQSizePolicy::Preferred );
  TQWhatsThis::add( mSearchText, i18n( "The incremental search<p>Enter some text here will start the search for the contact, which matches the search pattern best. The part of the contact, which will be used for matching, depends on the field selection." ) );
  label->setBuddy( mSearchText );
  layout->addWidget( mSearchText );

  label = new TQLabel( i18n( "as in 'Search in:'", "&in:" ), this, "kde toolbar widget" );
  label->setAlignment( TQLabel::AlignVCenter | TQLabel::AlignRight );
  layout->addWidget( label );

  mFieldCombo = new TQComboBox( false, this );
  layout->addWidget( mFieldCombo );
  label->setBuddy(mFieldCombo);

  TQToolTip::add( mFieldCombo, i18n( "Select incremental search field" ) );
  TQWhatsThis::add( mFieldCombo, i18n( "Here you can choose the field, which shall be used for incremental search." ) );

  mInputTimer = new TQTimer( this );

  connect( mInputTimer, TQT_SIGNAL( timeout() ),
           TQT_SLOT( timeout() ) );
  connect( mSearchText, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( announceDoSearch() ) );
  connect( mSearchText, TQT_SIGNAL( returnPressed() ),
           TQT_SLOT( announceDoSearch() ) );
  connect( mFieldCombo, TQT_SIGNAL( activated( const TQString& ) ),
           TQT_SLOT( announceDoSearch() ) );
  connect( button, TQT_SIGNAL( clicked() ),
           mSearchText, TQT_SLOT( clear() ) );
  connect( button, TQT_SIGNAL( clicked() ),
           TQT_SLOT( announceDoSearch() ) );

  initFields();

  mSearchText->installEventFilter( this );

  setFocusProxy( mSearchText );
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::announceDoSearch()
{
  if ( mInputTimer->isActive() )
    mInputTimer->stop();

  mInputTimer->start( 0, true );
}

void IncSearchWidget::timeout()
{
  emit doSearch( mSearchText->text() );
}

void IncSearchWidget::initFields()
{
  mFieldList = KABC::Field::allFields();

  mFieldCombo->clear();
  mFieldCombo->insertItem( i18n( "Visible Fields" ) );
  mFieldCombo->insertItem( i18n( "All Fields" ) );

  KABC::Field::List::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
    mFieldCombo->insertItem( (*it)->label() );

  announceDoSearch();
}

KABC::Field::List IncSearchWidget::currentFields() const
{
  KABC::Field::List fieldList;

  if ( mFieldCombo->currentItem() == 0 )
    fieldList = mViewFields;
  else if ( mFieldCombo->currentItem() > 1 )
    fieldList.append( mFieldList[ mFieldCombo->currentItem() - 2 ] );

  return fieldList;
}

void IncSearchWidget::setCurrentItem( int pos )
{
  mFieldCombo->setCurrentItem( pos );
}

int IncSearchWidget::currentItem() const
{
  return mFieldCombo->currentItem();
}

void IncSearchWidget::setViewFields( const KABC::Field::List &fields )
{
  mViewFields = fields;
}

void IncSearchWidget::clear()
{
  mSearchText->clear();
}

void IncSearchWidget::keyPressEvent( TQKeyEvent *event )
{
  if ( event->key() == Qt::Key_Up ) {
    event->accept();
    emit scrollUp();
  } else if ( event->key() == Qt::Key_Down ) {
    event->accept();
    emit scrollDown();
  }
}

#include "incsearchwidget.moc"
