/* verificationresult.h - wraps a gpgme verify result
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

#ifndef __GPGMEPP_VERIFICATIONRESULT_H__
#define __GPGMEPP_VERIFICATIONRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>

#include <time.h>

#include <vector>

#include <kdepimmacros.h>

namespace GpgME {

  class Error;
  class Signature;

  class KDE_EXPORT VerificationResult : public Result {
  public:
    VerificationResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit VerificationResult( const Error & err );
    VerificationResult( const VerificationResult & other );
    ~VerificationResult();

    const VerificationResult & operator=( const VerificationResult & other );

    bool isNull() const;

    Signature signature( unsigned int index ) const;
    std::vector<Signature> signatures() const;

    class Private;
  private:
    Private * d;
  };

  class KDE_EXPORT Signature {
    friend class VerificationResult;
    Signature( VerificationResult::Private * parent, unsigned int index );
  public:
    class Notation;

    Signature();
    Signature( const Signature & other );
    ~Signature();

    const Signature & operator=( const Signature & other );

    bool isNull() const;


    enum Summary {
      None       = 0x000,
      Valid      = 0x001,
      Green      = 0x002,
      Red        = 0x004,
      KeyRevoked = 0x008,
      KeyExpired = 0x010,
      SigExpired = 0x020,
      KeyMissing = 0x040,
      CrlMissing = 0x080,
      CrlTooOld  = 0x100,
      BadPolicy  = 0x200,
      SysError   = 0x400
    };
    Summary summary() const;

    const char * fingerprint() const;

    Error status() const;

    time_t creationTime() const;
    time_t expirationTime() const;
    bool neverExpires() const;

    bool wrongKeyUsage() const;

    enum Validity {
      Unknown, Undefined, Never, Marginal, Full, Ultimate
    };
    Validity validity() const;
    char validityAsString() const;
    Error nonValidityReason() const;

    Notation notation( unsigned int index ) const;
    std::vector<Notation> notations() const;

  private:
    VerificationResult::Private * d;
    unsigned int idx;
  };

  class KDE_EXPORT Signature::Notation {
    friend class Signature;
    Notation( VerificationResult::Private * parent, unsigned int sindex, unsigned int nindex );
  public:
    Notation();
    Notation( const Notation & other );
    ~Notation();

    const Notation & operator=( const Notation & other );

    bool isNull() const;

    const char * name() const;
    const char * value() const;

  private:
    VerificationResult::Private * d;
    unsigned int sidx;
    unsigned int nidx;
  };

}

#endif // __GPGMEPP_VERIFICATIONRESULT_H__
