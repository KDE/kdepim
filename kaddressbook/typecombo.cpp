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

#include <klocale.h>
#include <kdebug.h>

#include "typecombo.h"

TypeCombo::TypeCombo( KABC::PhoneNumber::List &list, QWidget *parent,
                      const char *name )
  : KComboBox( parent, name ),
    mTypeList( list )
{
}

void TypeCombo::updateTypes()
{
  clear();

  QMap<QString,int> labelCount;

  uint i;
  for( i = 0; i < mTypeList.count(); ++i ) {
    QString label = mTypeList[ i ].label();
    int count = 1;
    if ( labelCount.contains( label ) ) {
      count = labelCount[ label ] + 1;
    }
    labelCount[ label ] = count;
    if ( count > 1 ) {
      label = i18n("label (number)", "%1 (%2)").arg( label )
                                           .arg( QString::number( count ) );
    }
    insertItem( label );
  }
}

void TypeCombo::selectType( int type )
{
  uint i;
  for( i = 0; i < mTypeList.count(); ++i ) {
    if ( mTypeList[ i ].type() == type ) {
      setCurrentItem( i );
      break;
    }
  }
}

KABC::PhoneNumber::List::Iterator TypeCombo::selectedElement()
{
  return mTypeList.at( currentItem() );
}

#include "typecombo.moc"
