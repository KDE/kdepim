/* importresult.h - wraps a gpgme import result
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

#ifndef __GPGMEPP_IMPORTRESULT_H__
#define __GPGMEPP_IMPORTRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>

#include <vector>
#include <kdepimmacros.h>

namespace GpgME {

  class Error;
  class Import;

  class KDE_EXPORT ImportResult : public Result {
  public:
    ImportResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit ImportResult( const Error & error );
    ImportResult( const ImportResult & other );
    ~ImportResult();

    const ImportResult & operator=( const ImportResult & other );

    bool isNull() const;

    int numConsidered() const;
    int numKeysWithoutUserID() const;
    int numImported() const;
    int numRSAImported() const;
    int numUnchanged() const;

    int newUserIDs() const;
    int newSubkeys() const;
    int newSignatures() const;
    int newRevocations() const;

    int numSecretKeysConsidered() const;
    int numSecretKeysImported() const;
    int numSecretKeysUnchanged() const;

    int notImported() const;

    Import import( unsigned int idx ) const;
    std::vector<Import> imports() const;

    class Private;
  private:
    Private * d;
  };

  class KDE_EXPORT Import {
    friend class ImportResult;
    Import( ImportResult::Private * parent, unsigned int idx );
  public:
    Import();
    Import( const Import & other );
    ~Import();

    const Import & operator=( const Import & other );

    bool isNull() const;

    const char * fingerprint() const;
    Error error() const;

    enum Status {
      Unknown = 0x0,
      NewKey = 0x1,
      NewUserIDs = 0x2,
      NewSignatures = 0x4,
      NewSubkeys = 0x8,
      ContainedSecretKey = 0x10
    };
    Status status() const;

  private:
    ImportResult::Private * d;
    unsigned int idx;
  };

}

#endif // __GPGMEPP_IMPORTRESULT_H__
