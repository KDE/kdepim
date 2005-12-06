/* interface/passphraseprovider.h - Interface for passphrase callbacks
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
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __GPGMEPP_INTERFACES_PASSPHRASEPROVIDER_H__
#define __GPGMEPP_INTERFACES_PASSPHRASEPROVIDER_H__

#include <string>

namespace GpgME {

  class PassphraseProvider {
  public:
    virtual ~PassphraseProvider() {}

    virtual char * getPassphrase( const char * useridHint, const char * description,
				  bool previousWasbad, bool & canceled ) = 0;
  };

} // namespace GpgME

#endif // __GPGMEPP_INTERFACES_PASSPHRASEPROVIDER_H__
