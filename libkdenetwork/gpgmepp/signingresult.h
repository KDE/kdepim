/* signingresult.h - wraps a gpgme sign result
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

#ifndef __GPGMEPP_SIGNINGRESULT_H__
#define __GPGMEPP_SIGNINGRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>
#include <gpgmepp/context.h>

#include <time.h>

#include <vector>
#include <kdepimmacros.h>
namespace GpgME {

  class Error;
  class CreatedSignature;
  class InvalidSigningKey;

  class KDE_EXPORT SigningResult : public Result {
  public:
    SigningResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit SigningResult( const Error & err );
    SigningResult( const SigningResult & other );
    ~SigningResult();

    const SigningResult & operator=( const SigningResult & other );

    bool isNull() const;

    CreatedSignature createdSignature( unsigned int index ) const;
    std::vector<CreatedSignature> createdSignatures() const;

    InvalidSigningKey invalidSigningKey( unsigned int index ) const;
    std::vector<InvalidSigningKey> invalidSigningKeys() const;

    class Private;
  private:
    Private * d;
  };

  class KDE_EXPORT InvalidSigningKey {
    friend class SigningResult;
    InvalidSigningKey( SigningResult::Private * parent, unsigned int index );
  public:
    InvalidSigningKey();
    InvalidSigningKey( const InvalidSigningKey & other );
    ~InvalidSigningKey();

    const InvalidSigningKey & operator=( const InvalidSigningKey & other );

    bool isNull() const;

    const char * fingerprint() const;
    Error reason() const;

  private:
    SigningResult::Private * d;
    unsigned int idx;
  };

  class KDE_EXPORT CreatedSignature {
    friend class SigningResult;
    CreatedSignature( SigningResult::Private * parent, unsigned int index );
  public:
    class Notation;

    CreatedSignature();
    CreatedSignature( const CreatedSignature & other );
    ~CreatedSignature();

    const CreatedSignature & operator=( const CreatedSignature & other );

    bool isNull() const;

    const char * fingerprint() const;

    time_t creationTime() const;

    Context::SignatureMode mode() const;
    
    unsigned int publicKeyAlgorithm() const;
    const char * publicKeyAlgorithmAsString() const;

    unsigned int hashAlgorithm() const;
    const char * hashAlgorithmAsString() const;

    unsigned int signatureClass() const;

  private:
    SigningResult::Private * d;
    unsigned int idx;
  };

}

#endif // __GPGMEPP_SIGNINGRESULT_H__
