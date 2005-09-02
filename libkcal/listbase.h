/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_LISTBASE_H
#define KCAL_LISTBASE_H

#include <q3valuelist.h>

namespace KCal {

/**
  This class provides a template for lists of pointers.
  It extends QValueList<T *> by auto delete funtionality known from QPtrList.
*/
template<class T>
class ListBase : public Q3ValueList<T *>
{
  public:
    ListBase()
      : Q3ValueList<T *>(), mAutoDelete( false )
    {
    }

    ListBase( const ListBase &l )
      : Q3ValueList<T *>( l ), mAutoDelete( false )
    {
    }

    ~ListBase()
    {
      if ( mAutoDelete ) {
        Q3ValueListIterator<T *> it;
        for( it = Q3ValueList<T*>::begin(); it != Q3ValueList<T*>::end(); ++it ) {
          delete *it;
        }
      }
    }

    ListBase &operator=( const ListBase &l )
    {
      if ( this == &l ) return *this;
      Q3ValueList<T *>::operator=( l );
      return *this;
    }

    void setAutoDelete( bool autoDelete )
    {
      mAutoDelete = autoDelete;
    }

    bool removeRef( T *t )
    {
      Q3ValueListIterator<T *> it = find( t );
      if ( it == Q3ValueList<T*>::end() ) {
        return false;
      } else {
        if ( mAutoDelete ) delete t;
        remove( it );
        return true;
      }
    }

  private:
    bool mAutoDelete;
};

}

#endif
