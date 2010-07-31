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

#include <tqvaluelist.h>

namespace KCal {

/**
  This class provides a template for lists of pointers.
  It extends TQValueList<T *> by auto delete funtionality known from TQPtrList.
*/
template<class T>
class ListBase : public TQValueList<T *>
{
  public:
    ListBase()
      : TQValueList<T *>(), mAutoDelete( false )
    {
    }

    ListBase( const ListBase &l )
      : TQValueList<T *>( l ), mAutoDelete( false )
    {
    }

    ~ListBase()
    {
      if ( mAutoDelete ) {
        TQValueListIterator<T *> it;
        for( it = TQValueList<T*>::begin(); it != TQValueList<T*>::end(); ++it ) {
          delete *it;
        }
      }
    }

    ListBase &operator=( const ListBase &l )
    {
      if ( this == &l ) return *this;
      TQValueList<T *>::operator=( l );
      return *this;
    }

    void setAutoDelete( bool autoDelete )
    {
      mAutoDelete = autoDelete;
    }

    bool removeRef( T *t )
    {
      TQValueListIterator<T *> it = find( t );
      if ( it == TQValueList<T*>::end() ) {
        return false;
      } else {
        if ( mAutoDelete ) delete t;
        this->remove( it );
        return true;
      }
    }

    void clearAll()
    {
      if ( mAutoDelete ) {
        for ( TQValueListIterator<T*> it = TQValueList<T*>::begin();
              it != TQValueList<T*>::end(); ++it ) {
          delete *it;
        }
      }
      TQValueList<T*>::clear();
    }

  private:
    bool mAutoDelete;
};

}

#endif
