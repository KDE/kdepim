/* trustitem.cpp - wraps a gpgme trust item
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/trustitem.h>

#include <gpgme.h>

#include <cassert>

namespace GpgME {

  struct TrustItem::Private  {
    Private( gpgme_trust_item_t aItem )
      : item( aItem )
    {
    }

    gpgme_trust_item_t item;
  };

  TrustItem::TrustItem( gpgme_trust_item_t item ) {
    d = new Private( item );
    if ( d->item )
      gpgme_trust_item_ref( d->item );
  }

  TrustItem::TrustItem( const TrustItem & other ) {
    d = new Private( other.d->item );
    if ( d->item )
      gpgme_trust_item_ref( d->item );
  }

  const TrustItem & TrustItem::operator=( const TrustItem & other ) {
    if ( &other == this ) return *this;

    if ( other.d->item )
      gpgme_trust_item_ref( other.d->item );
    if ( d->item )
      gpgme_trust_item_unref( d->item );
    *d = *other.d;
    return *this;
  }

  TrustItem::~TrustItem() {
    if ( d->item )
      gpgme_trust_item_unref( d->item );
    delete d; d = 0;
  }

  bool TrustItem::isNull() const {
    return !d || !d->item;
  }

  gpgme_trust_item_t TrustItem::impl() const {
    return d->item;
  }


  const char * TrustItem::keyID() const {
    return d->item ? d->item->keyid : 0 ;
  }

  const char * TrustItem::userID() const {
    return d->item ? d->item->name : 0 ;
  }

  const char * TrustItem::ownerTrustAsString() const {
    return d->item ? d->item->owner_trust : 0 ;
  }

  const char * TrustItem::validityAsString() const {
    return d->item ? d->item->validity : 0 ;
  }

  int TrustItem::trustLevel() const {
    return d->item ? d->item->level : 0 ;
  }

  TrustItem::Type TrustItem::type() const {
    if ( !d->item )
      return Unknown;
    else 
      return
	d->item->type == 1 ? Key :
	d->item->type == 2 ? UserID :
	Unknown ;
  }

} // namespace GpgME
