/* encryptionresult.h - wraps a gpgme sign result
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

#ifndef __GPGMEPP_ENCRYPTIONRESULT_H__
#define __GPGMEPP_ENCRYPTIONRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>

#include <vector>
#include <kdepimmacros.h>

namespace GpgME {

  class Error;
  class InvalidRecipient;

  class KDE_EXPORT EncryptionResult : public Result {
  public:
    EncryptionResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit EncryptionResult( const Error & err );
    EncryptionResult( const EncryptionResult & other );
    ~EncryptionResult();

    const EncryptionResult & operator=( const EncryptionResult & other );

    bool isNull() const;

    unsigned int numInvalidRecipients() const;

    InvalidRecipient invalidEncryptionKey( unsigned int index ) const;
    std::vector<InvalidRecipient> invalidEncryptionKeys() const;

    class Private;
  private:
    Private * d;
  };

  class KDE_EXPORT InvalidRecipient {
    friend class EncryptionResult;
    InvalidRecipient( EncryptionResult::Private * parent, unsigned int index );
  public:
    InvalidRecipient();
    InvalidRecipient( const InvalidRecipient & other );
    ~InvalidRecipient();

    const InvalidRecipient & operator=( const InvalidRecipient & other );

    bool isNull() const;

    const char * fingerprint() const;
    Error reason() const;

  private:
    EncryptionResult::Private * d;
    unsigned int idx;
  };

}

#endif // __GPGMEPP_ENCRYPTIONRESULT_H__
