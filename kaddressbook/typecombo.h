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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
template <class T>
class TypeCombo : public KComboBox
{
  public:
    typedef typename T::List List;
    typedef typename T::List::Iterator Iterator;

    TypeCombo( List &list, QWidget *parent, const char *name = 0 );

    void setLineEdit( QLineEdit *edit ) { mLineEdit = edit; }
    QLineEdit *lineEdit() const { return mLineEdit; }

    void updateTypes();

    void selectType( int type );

    int selectedType();

    Iterator selectedElement();

    void insertType( const List &list, int type,
                     const T &defaultObject );
    void insertTypeList( const List &list );

    bool hasType( int type );

  private:
    List &mTypeList;
    QLineEdit *mLineEdit;
};

template <class T>
TypeCombo<T>::TypeCombo( TypeCombo::List &list, QWidget *parent,
                      const char *name )
  : KComboBox( parent, name ),
    mTypeList( list )
{
}

template <class T>
void TypeCombo<T>::updateTypes()
{
  // Remember current item
  QString currentId;
  int current = currentItem();
  if ( current >= 0 ) currentId = mTypeList[ current ].id();

  clear();

  QMap<int,int> labelCount;

  uint i;
  for ( i = 0; i < mTypeList.count(); ++i ) {
    int type = ( mTypeList[ i ].type() & ~( T::Pref ) );
    QString label = mTypeList[ i ].typeLabel( type );
    int count = 1;
    if ( labelCount.contains( type ) ) {
      count = labelCount[ type ] + 1;
    }
    labelCount[ type ] = count;
    if ( count > 1 ) {
      label = i18n("label (number)", "%1 (%2)").arg( label )
                                           .arg( QString::number( count ) );
    }
    insertItem( label );
  }

  // Restore previous current item
  if ( !currentId.isEmpty() ) {
    for ( i = 0; i < mTypeList.count(); ++i ) {
      if ( mTypeList[ i ].id() == currentId ) {
        setCurrentItem( i );
        break;
      }
    }
  }
}

template <class T>
void TypeCombo<T>::selectType( int type )
{
  uint i;
  for ( i = 0; i < mTypeList.count(); ++i ) {
    if ( (mTypeList[ i ].type() & ~T::Pref) == type ) {
      setCurrentItem( i );
      break;
    }
  }
}

template <class T>
int TypeCombo<T>::selectedType()
{
  return mTypeList[ currentItem() ].type();
}

template <class T>
typename TypeCombo<T>::Iterator TypeCombo<T>::selectedElement()
{
  return mTypeList.at( currentItem() );
}

template <class T>
void TypeCombo<T>::insertType( const TypeCombo::List &list, int type,
                               const T &defaultObject )
{
  uint i;
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

template <class T>
void TypeCombo<T>::insertTypeList( const TypeCombo::List &list )
{
  uint i;
  for ( i = 0; i < list.count(); ++i ) {
    uint j;
    for ( j = 0; j < mTypeList.count(); ++j ) {
      if ( list[ i ].id() == mTypeList[ j ].id() ) break;
    }
    if ( j == mTypeList.count() ) {
      mTypeList.append( list[ i ] );
    }
  }
}

template <class T>
bool TypeCombo<T>::hasType( int type )
{
  for ( uint i = 0; i < mTypeList.count(); ++i ) {
    if ( ( mTypeList[ i ].type() & ~T::Pref ) == type )
      return true;
  }

  return false;
}

#endif
