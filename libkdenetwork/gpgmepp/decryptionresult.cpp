/* decryptionresult.cpp - wraps a gpgme keygen result
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

#include <gpgmepp/decryptionresult.h>
#include "shared.h"
#include "result_p.h"

#include <gpgme.h>

#include <cstring>
#include <cstdlib>

class GpgME::DecryptionResult::Private : public GpgME::Shared {
public:
  Private( const _gpgme_op_decrypt_result & r ) : Shared(), res( r ) {
    if ( res.unsupported_algorithm )
      res.unsupported_algorithm = strdup( res.unsupported_algorithm );
  }
  ~Private() {
    if ( res.unsupported_algorithm )
      std::free( res.unsupported_algorithm );
    res.unsupported_algorithm = 0;
  }

  _gpgme_op_decrypt_result res;
};

GpgME::DecryptionResult::DecryptionResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_decrypt_result_t res = gpgme_op_decrypt_result( ctx );
  if ( !res )
    return;
  d = new Private( *res );
  d->ref();
}

make_standard_stuff(DecryptionResult)

const char * GpgME::DecryptionResult::unsupportedAlgortihm() const {
  return d ? d->res.unsupported_algorithm : 0 ;
}

bool GpgME::DecryptionResult::wrongKeyUsage() const {
#ifdef HAVE_GPGME_WRONG_KEY_USAGE
  if ( d )
    return d->res.wrong_key_usage;
#endif
  return false;
}
