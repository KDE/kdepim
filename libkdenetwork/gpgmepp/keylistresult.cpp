/* keylistresult.cpp - wraps a gpgme keylist result
   Copyright (C) 2004 Klarälvdalens Datakonsult AB

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
   along with GPGME++; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/keylistresult.h>
#include "shared.h"

#include <gpgme.h>

#include <cstring>

class GpgME::KeyListResult::Private : public GpgME::Shared {
public:
  Private( const _gpgme_op_keylist_result & r ) : Shared(), res( r ) {}

  _gpgme_op_keylist_result res;
};

GpgME::KeyListResult::KeyListResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_keylist_result_t res = gpgme_op_keylist_result( ctx );
  if ( !res )
    return;
  d = new Private( *res );
  d->ref();
}

GpgME::KeyListResult::KeyListResult( const KeyListResult & other )
  : GpgME::Result( other ), d( other.d )
{
  if ( d )
    d->ref();
}

const GpgME::KeyListResult & GpgME::KeyListResult::operator=( const KeyListResult & other ) {
  if ( this->d == other.d ) return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->ref();

  return *this;
}

GpgME::KeyListResult::~KeyListResult() {
  if ( d )
    d->unref();
  d = 0;
}

bool GpgME::KeyListResult::isNull() const {
  return !d;
}

bool GpgME::KeyListResult::isTruncated() const {
  return d && d->res.truncated;
}
