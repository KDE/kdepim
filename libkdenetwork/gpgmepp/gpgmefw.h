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

#ifndef __GPGMEPP_GPGMEFW_H__
#define __GPGMEPP_GPGMEFW_H__

#ifndef HAVE_GPGME_0_4_BRANCH
#error You need gpgme 0.4.x, x >= 4, to compile gpgme++
#endif

struct gpgme_context;
typedef gpgme_context * gpgme_ctx_t;

struct gpgme_data;
typedef gpgme_data * gpgme_data_t;

struct gpgme_io_cbs;

struct _gpgme_key;
typedef struct _gpgme_key * gpgme_key_t;

struct _gpgme_trust_item;
typedef struct _gpgme_trust_item * gpgme_trust_item_t;

struct _gpgme_subkey;
typedef struct _gpgme_subkey * gpgme_sub_key_t;

struct _gpgme_user_id;
typedef struct _gpgme_user_id * gpgme_user_id_t;

struct _gpgme_key_sig;
typedef struct _gpgme_key_sig * gpgme_key_sig_t;

struct _gpgme_sig_notation;
typedef struct _gpgme_sig_notation * gpgme_sig_notation_t;

#endif // __GPGMEPP_GPGMEFW_H__
