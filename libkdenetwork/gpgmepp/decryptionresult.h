/* decryptionresult.h - wraps a gpgme keygen result
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

#ifndef __GPGMEPP_DECRYPTIONRESULT_H__
#define __GPGMEPP_DECRYPTIONRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>

namespace GpgME {

  class Error;

  class DecryptionResult : public Result {
  public:
    DecryptionResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit DecryptionResult( const Error & err );
    DecryptionResult( const DecryptionResult & other );
    ~DecryptionResult();

    const DecryptionResult & operator=( const DecryptionResult & other );

    bool isNull() const;

    const char * unsupportedAlgortihm() const;

    bool wrongKeyUsage() const;

  private:
    class Private;
    Private * d;
  };

}

#endif // __GPGMEPP_KEYGENERATIONRESULT_H__
