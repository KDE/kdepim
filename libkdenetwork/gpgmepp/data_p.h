/* data_p.h - wraps a gpgme data object, private part -*- c++ -*-
   Copyright (C) 2003,2004 Klarälvdalens Datakonsult AB

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

#ifndef __GPGMEPP_DATA_P_H__
#define __GPGMEPP_DATA_P_H__

#include <gpgmepp/data.h>
#include "shared.h"
#include "callbacks.h"

class GpgME::Data::Private : public GpgME::Shared {
public:
  Private( gpgme_data_t d=0 )
    : Shared(), data( d ), cbs( data_provider_callbacks ) {}
  ~Private();
    
  gpgme_data_t data;
  gpgme_data_cbs cbs;
};


#endif // __GPGMEPP_DATA_P_H__
