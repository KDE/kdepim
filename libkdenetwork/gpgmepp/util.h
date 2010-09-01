/* util.h - some inline helper functions
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
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.  */

// -*- c++ -*-
#ifndef __GPGMEPP_UTIL_H__
#define __GPGMEPP_UTIL_H__

#include <gpgme.h>
#include <context.h>

#ifndef NDEBUG
#include <iostream>
#endif

static inline const char * protect( const char * s ) {
    return s ? s : "<null>" ;
}

static inline gpgme_keylist_mode_t add_to_gpgme_keylist_mode_t( unsigned int oldmode, unsigned int newmodes ) {
  if ( newmodes & GpgME::Context::Local ) oldmode |= GPGME_KEYLIST_MODE_LOCAL;
  if ( newmodes & GpgME::Context::Extern ) oldmode |= GPGME_KEYLIST_MODE_EXTERN;
  if ( newmodes & GpgME::Context::Signatures ) oldmode |= GPGME_KEYLIST_MODE_SIGS;
  if ( newmodes & GpgME::Context::Validate ) {
#ifdef HAVE_GPGME_KEYLIST_MODE_VALIDATE
    oldmode |= GPGME_KEYLIST_MODE_VALIDATE;
#elif !defined(NDEBUG)
    std::cerr << "GpgME::Context: ignoring Valdidate keylist flag (gpgme too old)." << std::endl;
#endif
  }
#ifndef NDEBUG
  if ( newmodes & ~(GpgME::Context::Local|GpgME::Context::Extern|GpgME::Context::Signatures|GpgME::Context::Validate) )
    std::cerr << "GpgME::Context: keylist mode must be one of Local, "
      "Extern, Signatures, or Validate, or a combination thereof!" << std::endl;
#endif
  return static_cast<gpgme_keylist_mode_t>( oldmode );
}

static inline unsigned int convert_from_gpgme_keylist_mode_t( unsigned int mode ) {
  unsigned int result = 0;
  if ( mode & GPGME_KEYLIST_MODE_LOCAL ) result |= GpgME::Context::Local;
  if ( mode & GPGME_KEYLIST_MODE_EXTERN ) result |= GpgME::Context::Extern;
  if ( mode & GPGME_KEYLIST_MODE_SIGS ) result |= GpgME::Context::Signatures;
#ifdef HAVE_GPGME_KEYLIST_MODE_VALIDATE
  if ( mode & GPGME_KEYLIST_MODE_VALIDATE ) result |= GpgME::Context::Validate;
#endif
#ifndef NDEBUG
  if ( mode & ~(GPGME_KEYLIST_MODE_LOCAL|
		GPGME_KEYLIST_MODE_EXTERN|
#ifdef HAVE_GPGME_KEYLIST_MODE_VALIDATE
		GPGME_KEYLIST_MODE_VALIDATE|
#endif
		GPGME_KEYLIST_MODE_SIGS) )
    std::cerr << "GpgME::Context: WARNING: gpgme_get_keylist_mode() returned an unknown flag!" << std::endl;
#endif // NDEBUG
  return result;
}


#endif // __GPGMEPP_UTIL_H__
