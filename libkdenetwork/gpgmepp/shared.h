/* shared.h - internal tool for refcounting -*- c++ -*-
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

   This file is part of GPGME++.
 
   GPGME++ is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME++ is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.
*/

#ifndef __GPGMEPP_SHARED_H__
#define __GPGMEPP_SHARED_H__

#include <cassert>

namespace GpgME {

  class Shared {
  protected:
    Shared() : mRefCount( 0 ) {}
    virtual ~Shared() {
      assert( mRefCount <= 0 );
    }

  public:
    int ref() { return ++mRefCount; }
    int deref() { return unref(); }
    int unref() {
      if ( --mRefCount <= 0 ) {
	delete this;
	return 0;
      }
      return mRefCount;
    }
    int refCount() { return mRefCount; }

  private:
    int mRefCount;
  };

}


#endif // __GPGMEPP_SHARED_H__
