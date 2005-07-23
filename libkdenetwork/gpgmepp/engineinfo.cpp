/* engineinfo.h
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "engineinfo.h"
#include "shared.h"

#include <gpgme.h>

struct GpgME::EngineInfo::Private : public GpgME::Shared {
  Private( gpgme_engine_info_t engine=0 ) : Shared(), info( engine ) {}
  ~Private() { info = 0; }

  gpgme_engine_info_t info;
};


GpgME::EngineInfo::EngineInfo() : d(0) {}

GpgME::EngineInfo::EngineInfo( gpgme_engine_info_t engine )
  : d(0)
{
  d = new Private( engine );
  d->ref();
}

GpgME::EngineInfo::EngineInfo( const EngineInfo & other )
  : d( other.d )
{
  if ( d )
    d->ref();
}


GpgME::EngineInfo::~EngineInfo() {
  if ( d )
    d->deref();
}

const GpgME::EngineInfo & GpgME::EngineInfo::operator=( const GpgME::EngineInfo & other ) {
  if ( this->d == other.d )
    return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->unref();

  this->d = other.d;
  return *this;
}

bool GpgME::EngineInfo::isNull() const {
  return !d || !d->info;
}

GpgME::Context::Protocol GpgME::EngineInfo::protocol() const {
  if ( isNull() )
    return Context::Unknown;
  switch( d->info->protocol ) {
  case GPGME_PROTOCOL_OpenPGP: return Context::OpenPGP;
  case GPGME_PROTOCOL_CMS:     return Context::CMS;
  default:
    return Context::Unknown;
  }
}

const char * GpgME::EngineInfo::fileName() const {
  return isNull() ? 0 : d->info->file_name;
}

const char * GpgME::EngineInfo::version() const {
  return isNull() ? 0 : d->info->version;
}

const char * GpgME::EngineInfo::requiredVersion() const {
  return isNull() ? 0 : d->info->req_version;
}

