/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include "secrecywidget.h"

#include <QtGui/QVBoxLayout>

#include <kabc/secrecy.h>
#include <kcombobox.h>
#include <kdialog.h>

SecrecyWidget::SecrecyWidget( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setObjectName( name );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );
  mSecrecyCombo = new KComboBox( this );
  layout->addWidget( mSecrecyCombo );

  const KABC::Secrecy::TypeList list = KABC::Secrecy::typeList();
  KABC::Secrecy::TypeList::ConstIterator it;
  /* (*it) is the type enum, which is also used as the index in the combo */
  for ( it = list.begin(); it != list.end(); ++it )
    mSecrecyCombo->insertItem( *it, KABC::Secrecy::typeLabel( *it ) );

  connect( mSecrecyCombo, SIGNAL( activated( const QString& ) ),
           SIGNAL( changed() ) );
}

SecrecyWidget::~SecrecyWidget()
{
}

void SecrecyWidget::setReadOnly( bool readOnly )
{
  mSecrecyCombo->setEnabled( !readOnly );
}

void SecrecyWidget::setSecrecy( const KABC::Secrecy &secrecy )
{
  if ( secrecy.type() != KABC::Secrecy::Invalid )
    mSecrecyCombo->setCurrentIndex( secrecy.type() );
}

KABC::Secrecy SecrecyWidget::secrecy() const
{
  static QMap<int, KABC::Secrecy::Type> typeMap;
  if ( typeMap.isEmpty() ) {
    typeMap.insert( 0, KABC::Secrecy::Public );
    typeMap.insert( 1, KABC::Secrecy::Private );
    typeMap.insert( 2, KABC::Secrecy::Confidential );
    typeMap.insert( 3, KABC::Secrecy::Invalid );
  }

  KABC::Secrecy secrecy;
  secrecy.setType( typeMap.value( mSecrecyCombo->currentIndex() ) );

  return secrecy;
}

#include "secrecywidget.moc"
