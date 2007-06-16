/* keylistresult.h - wraps a gpgme keylist result
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
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __GPGMEPP_KEYLISTRESULT_H__
#define __GPGMEPP_KEYLISTRESULT_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/result.h>
#include <gpgmepp/gpgmepp_export.h>

namespace GpgME {

    class Error;

  class QPGMEPP_EXPORT KeyListResult : public Result {
  public:
    KeyListResult( gpgme_ctx_t ctx=0, int error=0 );
    explicit KeyListResult( const Error & err );
    KeyListResult( const Error & err, const _gpgme_op_keylist_result & res );
    KeyListResult( const KeyListResult & other );
    ~KeyListResult();

    const KeyListResult & operator=( const KeyListResult & other );

    const KeyListResult & operator+=( const KeyListResult & other ) {
      mergeWith( other );
      return *this;
    }

    void mergeWith( const KeyListResult & other );

    bool isNull() const;

    bool isTruncated() const;

  private:
    void detach();
    class Private;
    Private * d;
  };

}

#endif // __GPGMEPP_KEYLISTRESULT_H__
