/* trustitem.h - wraps a gpgme trust item
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

   This file is part of GPGME.
 
   GPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

// -*- c++ -*-
#ifndef __GPGMEPP_TRUSTITEM_H__
#define __GPGMEPP_TRUSTITEM_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/key.h>

namespace GpgME {

  class Context;

  class TrustItem {
    friend class Context;
  public:
    TrustItem( gpgme_trust_item_t item=0 );
    TrustItem( const TrustItem & other );
    virtual ~TrustItem();

    const TrustItem & operator=( const TrustItem & other );

    bool isNull() const;

    const char * keyID() const;
    const char * userID() const;

    const char * ownerTrustAsString() const;
    const char * validityAsString() const;

    int trustLevel() const;

    enum Type { Unknown=0, Key=1, UserID=2 };
    Type type() const;

  private:
    gpgme_trust_item_t impl() const;
    class Private;
    Private * d;
  };

} // namepace GpgME

#endif // __GPGMEPP_TRUSTITEM_H__
