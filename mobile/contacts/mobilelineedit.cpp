/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mobilelineedit.h"

#include <libkdepim/addressline/addresseelineedit.h>

#include <kicon.h>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class MobileLineEdit::Private
{
  MobileLineEdit *const q;

  public:
    explicit Private( MobileLineEdit *parent )
      : q( parent ), mEdit( 0 ), mButton( 0 )
    {
    }

  public:
    KPIM::AddresseeLineEdit *mEdit;
    QPushButton *mButton;
};

MobileLineEdit::MobileLineEdit( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  QHBoxLayout *box = new QHBoxLayout( this );
  box->setMargin( 0 );
  box->setSpacing( 0 );

  d->mEdit = new KPIM::AddresseeLineEdit( this );
  box->addWidget( d->mEdit );

  d->mButton = new QPushButton( this );
  d->mButton->setIcon( KIcon( QLatin1String("edit-clear-locationbar-rtl") ) );
  d->mButton->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
  box->addWidget( d->mButton );

  connect( d->mButton, SIGNAL(clicked()), SLOT(clear()) );
  connect( d->mButton, SIGNAL(clicked()), SIGNAL(clearClicked()) );
}

MobileLineEdit::~MobileLineEdit()
{
  delete d;
}

void MobileLineEdit::setText( const QString &text )
{
  d->mEdit->setText( text );
}

QString MobileLineEdit::text() const
{
  return d->mEdit->text();
}

void MobileLineEdit::clear()
{
  d->mEdit->clear();
}


