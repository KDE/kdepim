/* gpgmefw.h - Forwards declarations for gpgme (0.3 and 0.4)
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

#ifndef __GPGMEPP_GPGME_0_3_COMPAT_H__
#define __GPGMEPP_GPGME_0_3_COMPAT_H__

#include <gpgme.h>

#ifndef HAVE_GPGME_0_4_BRANCH
// make gpgme-0.4 names available even if we have only 0.3:
typedef GpgmeError gpgme_error_t;
typedef GpgmeIOCb gpgme_io_cb_t;
typedef GpgmeIOCbs gpgme_io_cbs;
typedef GpgmeEventIO gpgme_event_io_t;
typedef GpgmeEventIOCb gpgme_event_io_cb_t;
typedef GpgmeRegisterIOCb gpgme_register_io_cb_t;
typedef GpgmeRemoveIOCb gpgme_remove_io_cb_t;
typedef GpgmeSigStat gpgme_sig_stat_t;
typedef GpgmeAttr gpgme_attr_t;
typedef GpgmeTrustItem gpgme_trust_item_t;
typedef GpgmeCtx gpgme_ctx_t;
typedef GpgmeProtocol gpgme_protocol_t;
typedef GpgmeData gpgme_data_t;
typedef GpgmeKey gpgme_key_t;

#define GPG_ERR_GENERAL GPGME_General_Error
#define GPG_ERR_NO_ERROR GPGME_No_Error
#define GPG_ERR_EOF GPGME_EOF
#define gpg_err_code(x) (x)
#endif

#endif // __GPGMEPP_GPGME_0_3_COMPAT_H__
