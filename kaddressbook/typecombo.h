/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef TYPECOMBO_H
#define TYPECOMBO_H

#include <kabc/phonenumber.h>
#include <kcombobox.h>

/**
  Combo box for type information of Addresses and Phone numbers.
*/
template <class T, typename U>
class TypeCombo : public KComboBox
{
  public:
    typedef typename T::List List;
    typedef typename T::List::Iterator Iterator;

    TypeCombo( List &list, QWidget *parent, const char *name = 0 );

    void setLineEdit( QLineEdit *edit ) { mLineEdit = edit; }
    QLineEdit *lineEdit() const { return mLineEdit; }

    void updateTypes();

    void selectType( U type );

    U selectedType();

    Iterator selectedElement();

    void insertType( const List &list, U type,
                     const T &defaultObject );
    void insertTypeList( const List &list );

    bool hasType( U type );

    bool isEmpty() const;

  private:
    List &mTypeList;
    QLineEdit *mLineEdit;
};

template <class T, typename U>
TypeCombo<T, U>::TypeCombo( TypeCombo::List &list, QWidget *parent,
                      const char *name )
  : KComboBox( parent ),
    mTypeList( list )
{
	setObjectName(name);
}

template <class T, typename U>
bool TypeCombo<T, U>::isEmpty() const
{
    return mTypeList.isEmpty();
}

template <class T, typename U>
void TypeCombo<T, U>::updateTypes()
{
  if(mTypeList.isEmpty())
	  return;
  // Remember current item
  QString currentId;
  int current = currentIndex();
  if ( current >= 0 ) currentId = mTypeList[ current ].id();

  clear();

  QMap<int,int> labelCount;

  int i;
  for ( i = 0; i < mTypeList.count(); ++i ) {
    U type = ( mTypeList[ i ].type() & ~( T::Pref ) );
    QString label = mTypeList[ i ].typeLabel( type );
    int count = 1;
    if ( labelCount.contains( type ) ) {
      count = labelCount[ type ] + 1;
    }
    labelCount[ type ] = count;
    if ( count > 1 ) {
      label = i18nc( "label (number)", "%1 (%2)", label, count );
    }
    addItem( label );
  }

  // Restore previous current item
  if ( !currentId.isEmpty() ) {
    for ( i = 0; i < mTypeList.count(); ++i ) {
      if ( mTypeList[ i ].id() == currentId ) {
        setCurrentIndex( i );
        break;
      }
    }
  }
}

template <class T, typename U>
void TypeCombo<T, U>::selectType( U type )
{
  int i;
  for ( i = 0; i < mTypeList.count(); ++i ) {
    if ( (mTypeList[ i ].type() & ~T::Pref) == type ) {
      setCurrentIndex( i );
      break;
    }
  }
}

template <class T, typename U>
U TypeCombo<T, U>::selectedType()
{
  return mTypeList[ currentIndex() ].type();
}

template <class T, typename U>
typename TypeCombo<T, U>::Iterator TypeCombo<T, U>::selectedElement()
{
#ifdef __GNUC__
#warning Ugly porting hack!
#endif
   typename TypeCombo<T, U>::Iterator it = mTypeList.begin();
  for ( int i = 0; i < currentIndex(); ++i, ++it );
  return it;
//  return mTypeList.at( currentItem() );
}

template <class T, typename U>
void TypeCombo<T, U>::insertType( const TypeCombo::List &list, U type,
                               const T &defaultObject )
{
  int i;
  for ( i = 0; i < list.count(); ++i ) {
    if ( list[ i ].type() == type ) {
      mTypeList.append( list[ i ] );
      break;
    }
  }
  if ( i == list.count() ) {
    mTypeList.append( defaultObject );
  }
}

template <class T, typename U>
void TypeCombo<T, U>::insertTypeList( const TypeCombo::List &list )
{
  int i;
  for ( i = 0; i < list.count(); ++i ) {
    int j;
    for ( j = 0; j < mTypeList.count(); ++j ) {
      if ( list[ i ].id() == mTypeList[ j ].id() ) break;
    }
    if ( j == mTypeList.count() ) {
      mTypeList.append( list[ i ] );
    }
  }
}

template <class T, typename U>
bool TypeCombo<T, U>::hasType( U type )
{
  for ( int i = 0; i < mTypeList.count(); ++i ) {
    if ( ( mTypeList[ i ].type() & ~T::Pref ) == type )
      return true;
  }

  return false;
}

#endif
