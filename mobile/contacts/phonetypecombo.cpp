/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>
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

#include "phonetypecombo.h"


class PhoneTypeCombo::Private
{
  PhoneTypeCombo *const q;

  public:
    Private( PhoneTypeCombo *parent ): q( parent ), mType( KContacts::PhoneNumber::Home ), mLastSelected( 0 )
    {
      for ( int i = 0; i < KContacts::PhoneNumber::typeList().count(); ++i )
        mTypeList.append( KContacts::PhoneNumber::typeList().at( i ) );

      update();
    }

    void update()
    {
      q->clear();

      for ( int i = 0; i < mTypeList.count(); ++i ) {
        q->addItem( KContacts::PhoneNumber::typeLabel( KContacts::PhoneNumber::Type( mTypeList.at( i ) ) ) );
      }

      q->setCurrentIndex( mLastSelected = mTypeList.indexOf( mType ) );
    }

  public:
    KContacts::PhoneNumber::Type mType;
    int mLastSelected;
    QList<int> mTypeList;

  public: // slots
    void selected( int index );
};

void PhoneTypeCombo::Private::selected( int pos )
{
  mType = KContacts::PhoneNumber::Type( mTypeList.at( pos ) );
  mLastSelected = pos;
}

PhoneTypeCombo::PhoneTypeCombo( QWidget *parent ) : QComboBox( parent ), d( new Private( this ) )
{
  connect( this, SIGNAL(activated(int)), SLOT(selected(int)) );
}

PhoneTypeCombo::~PhoneTypeCombo()
{
  delete d;
}

void PhoneTypeCombo::setType( KContacts::PhoneNumber::Type type )
{
  if ( !d->mTypeList.contains( type ) )
    d->mTypeList.insert( d->mTypeList.at( d->mTypeList.count() - 1 ), type );

  d->mType = type;
  d->update();
}

KContacts::PhoneNumber::Type PhoneTypeCombo::type() const
{
  return d->mType;
}

#include "moc_phonetypecombo.cpp"
