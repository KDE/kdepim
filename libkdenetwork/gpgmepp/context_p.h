/* context_p.h - wraps a gpgme key context (private part)
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

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
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */


// -*- c++ -*- 
#ifndef __GPGMEPP_CONTEXT_P_H__
#define __GPGMEPP_CONTEXT_P_H__

#include <gpgmepp/context.h>

#include <gpgme.h>

namespace GpgME {


  struct Context::Private {
    enum Operation {
      None = 0,

      Encrypt   = 0x001,
      Decrypt   = 0x002,
      Sign      = 0x004,
      Verify    = 0x008,
      DecryptAndVerify = Decrypt|Verify,
      SignAndEncrypt   = Sign|Encrypt,

      Import    = 0x010,
      Export    = 0x020, // no gpgme_export_result_t, but nevertheless...
      Delete    = 0x040, // no gpgme_delete_result_t, but nevertheless...

      KeyGen    = 0x080,
      KeyList   = 0x100,
      TrustList = 0x200 // gpgme_trustlist_result_t, but nevertheless...
    };

    Private( gpgme_ctx_t c=0 )
      : ctx( c ),
	 iocbs( 0 ),
	 lastop( None ),
	 lasterr( GPG_ERR_NO_ERROR ) {}
    ~Private() {
      if ( ctx ) {
	gpgme_release( ctx );
	ctx = 0;
      }
      delete iocbs;
    }

    gpgme_ctx_t ctx;
    gpgme_io_cbs * iocbs;
    //EditInteractor * edit;
    Operation lastop;
    gpgme_error_t lasterr;
  };

} // namespace GpgME

#endif // __GPGMEPP_CONTEXT_P_H__
