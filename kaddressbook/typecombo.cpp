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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "typecombo.h"

void TypeCombo::TypeList::addType( int type, const QString &label )
{
  mTypes.append( type );
  mTypeLabels.append( label );
}

void TypeCombo::TypeList::clear()
{
  mTypes.clear();
  mTypeLabels.clear();
}

uint TypeCombo::TypeList::count()
{
  return mTypes.count();
}

int TypeCombo::TypeList::type( int index )
{
  return mTypes[ index ];
}

QString TypeCombo::TypeList::label( int index )
{
  return mTypeLabels[ index ];
}


TypeCombo::TypeCombo( QWidget *parent, const char *name )
  : KComboBox( parent, name ),
    mTypeList( 0 )
{
}

TypeCombo::TypeCombo( TypeCombo::TypeList *typeList, QWidget *parent,
                      const char *name )
  : KComboBox( parent, name ),
    mTypeList( typeList )
{
  updateTypes();
}

void TypeCombo::setTypeList( TypeCombo::TypeList *typeList )
{
  mTypeList = typeList;
  updateTypes();
}

void TypeCombo::updateTypes()
{
  clear();

  if ( !mTypeList ) return;

  uint i;
  for( i = 0; i< mTypeList->count(); ++i ) {
    insertItem( mTypeList->label( i ) );
  }
}

void TypeCombo::selectType( int type )
{
  uint i;
  for( i = 0; i < mTypeList->count(); ++i ) {
    if ( mTypeList->type( i ) == type ) {
      setCurrentItem( i );
      break;
    }
  }
}

int TypeCombo::selectedType()
{
  return mTypeList->type( currentItem() );
}

KABC::PhoneNumber::List::Iterator
TypeCombo::selectedElement( KABC::PhoneNumber::List &list )
{
  int selected = selectedType();

  KABC::PhoneNumber::List::Iterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it).type() == selected ) {
      return it;
    }
  }

  return list.end();
}

#include "typecombo.moc"
