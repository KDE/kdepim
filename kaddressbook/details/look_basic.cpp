/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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

#include <kdebug.h>

#include "look_basic.h"

KABBasicLook::KABBasicLook( QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly( false )
{
}

void KABBasicLook::setReadOnly( bool state )
{
  mReadOnly = state;
}

bool KABBasicLook::isReadOnly() const
{
  return mReadOnly;
}

void KABBasicLook::setAddressee( const KABC::Addressee &addr )
{
  if ( mAddressee == addr )
    return;

  mAddressee = addr;
  repaint( false );
}

KABC::Addressee KABBasicLook::addressee()
{
  return mAddressee;
}

void KABBasicLook::restoreSettings( KConfig* )
{
}

void KABBasicLook::saveSettings( KConfig* )
{
}

KABLookFactory::KABLookFactory( QWidget *parent, const char *name )
  : mParent( parent ), mName( name )
{
}

KABLookFactory::~KABLookFactory()
{
}

#include "look_basic.moc"
