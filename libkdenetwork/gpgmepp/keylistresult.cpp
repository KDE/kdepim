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
#include "result_p.h"

#include <gpgme.h>

#include <cstring>

class GpgME::KeyListResult::Private : public GpgME::Shared {
public:
  Private( const _gpgme_op_keylist_result & r ) : Shared(), res( r ) {}
  Private( const Private & other ) : Shared(), res( other.res ) {}

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

GpgME::KeyListResult::KeyListResult( const Error & error, const _gpgme_op_keylist_result & res )
  : GpgME::Result( error ), d( 0 )
{
  d = new Private( res );
  d->ref();
}

make_standard_stuff(KeyListResult)

void GpgME::KeyListResult::detach() {
  if ( isNull() || d->refCount() <= 1 )
    return;
  d->unref();
  d = new Private( *d );
}

void GpgME::KeyListResult::mergeWith( const KeyListResult & other ) {
  if ( other.isNull() )
    return;
  if ( isNull() ) { // just assign
    operator=( other );
    return;
  }
  // merge the truncated flag (try to keep detaching to a minimum):
  if ( other.d->res.truncated && !d->res.truncated ) {
    detach();
    d->res.truncated = true;
  }
  if ( !error() ) // only merge the error when there was none yet.
    Result::operator=( other );
}

bool GpgME::KeyListResult::isTruncated() const {
  return d && d->res.truncated;
}
