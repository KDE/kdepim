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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>

#include <kabc/secrecy.h>
#include <kcombobox.h>
#include <kdialog.h>

#include "secrecywidget.h"

SecrecyWidget::SecrecyWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );
  mSecrecyCombo = new KComboBox( this );
  layout->addWidget( mSecrecyCombo );

  const KABC::Secrecy::TypeList list = KABC::Secrecy::typeList();
  KABC::Secrecy::TypeList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    mSecrecyCombo->insertItem( KABC::Secrecy::typeLabel( *it ), *it );

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
    mSecrecyCombo->setCurrentItem( secrecy.type() );
}

KABC::Secrecy SecrecyWidget::secrecy() const
{
  KABC::Secrecy secrecy;
  secrecy.setType( mSecrecyCombo->currentItem() );

  return secrecy;
}

#include "secrecywidget.moc"
