/* importresult.cpp - wraps a gpgme import result
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/importresult.h>
#include "shared.h"

#include <gpgme.h>

class GpgME::ImportResult::Private : public GpgME::Shared {
public:
  Private( const _gpgme_op_import_result & r ) : Shared(), res( r ) {
    // copy recursively, using compiler-generated copy ctor.
    // We just need to handle the pointers in the structs:
    for ( gpgme_import_status_t is = r.imports ; is ; is = is->next ) {
      gpgme_import_status_t copy = new _gpgme_import_status( *is );
      copy->fpr = strdup( is->fpr );
      copy->next = 0;
      imports.push_back( copy );
    }
    res.imports = 0;
  }
  ~Private() {
    for ( std::vector<gpgme_import_status_t>::iterator it = imports.begin() ; it != imports.end() ; ++it ) {
      std::free( (*it)->fpr );
      delete *it; *it = 0;
    }
  }

  _gpgme_op_import_result res;
  std::vector<gpgme_import_status_t> imports;
};

GpgME::ImportResult::ImportResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_import_result_t res = gpgme_op_import_result( ctx );
  if ( !res )
    return;
  d = new Private( *res );
  d->ref();
}

GpgME::ImportResult::ImportResult( const Error & error )
  : GpgME::Result( error ), d( 0 )
{

}

GpgME::ImportResult::ImportResult( const ImportResult & other )
  : GpgME::Result( other ), d( other.d )
{
  if ( d )
    d->ref();
}

const GpgME::ImportResult & GpgME::ImportResult::operator=( const ImportResult & other ) {
  if ( this->d == other.d ) return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->ref();
  this->d = other.d;

  return *this;
}

GpgME::ImportResult::~ImportResult() {
  if ( d )
    d->unref();
  d = 0;
}

bool GpgME::ImportResult::isNull() const {
  return !d;
}

int GpgME::ImportResult::numConsidered() const {
  return d ? d->res.considered : 0 ;
}

int GpgME::ImportResult::numKeysWithoutUserID() const {
  return d ? d->res.no_user_id : 0 ;
}

int GpgME::ImportResult::numImported() const {
  return d ? d->res.imported : 0 ;
}

int GpgME::ImportResult::numRSAImported() const {
  return d ? d->res.imported_rsa : 0 ;
}

int GpgME::ImportResult::numUnchanged() const {
  return d ? d->res.unchanged : 0 ;
}

int GpgME::ImportResult::newUserIDs() const {
  return d ? d->res.new_user_ids : 0 ;
}

int GpgME::ImportResult::newSubkeys() const {
  return d ? d->res.new_sub_keys : 0 ;
}

int GpgME::ImportResult::newSignatures() const {
  return d ? d->res.new_signatures : 0 ;
}

int GpgME::ImportResult::newRevocations() const {
  return d ? d->res.new_revocations : 0 ;
}

int GpgME::ImportResult::numSecretKeysConsidered() const {
  return d ? d->res.secret_read : 0 ;
}

int GpgME::ImportResult::numSecretKeysImported() const {
  return d ? d->res.secret_imported : 0 ;
}

int GpgME::ImportResult::numSecretKeysUnchanged() const {
  return d ? d->res.secret_unchanged : 0 ;
}

int GpgME::ImportResult::notImported() const {
  return d ? d->res.not_imported : 0 ;
}

GpgME::Import GpgME::ImportResult::import( unsigned int idx ) const {
  return Import( d, idx );
}

std::vector<GpgME::Import> GpgME::ImportResult::imports() const {
  if ( !d )
    return std::vector<Import>();
  std::vector<Import> result;
  result.reserve( d->imports.size() );
  for ( unsigned int i = 0 ; i < d->imports.size() ; ++i )
    result.push_back( Import( d, i ) );
  return result;
}






GpgME::Import::Import( ImportResult::Private * parent, unsigned int i )
  : d( parent ), idx( i )
{
  if ( d )
    d->ref();
}

GpgME::Import::Import() : d( 0 ), idx( 0 ) {}

GpgME::Import::Import( const Import & other )
  : d( other.d ), idx( other.idx )
{
  if ( d )
    d->ref();
}

GpgME::Import::~Import() {
  if ( d )
    d->unref();
}

const GpgME::Import & GpgME::Import::operator=( const Import & other ) {
  if ( this->d != other.d ) {
    if ( other.d )
      other.d->ref();
    if ( this->d )
      this->d->unref();
    this->d = other.d;
  }

  this->idx = other.idx;
  return *this;
}


bool GpgME::Import::isNull() const {
  return !d || idx >= d->imports.size() ;
}




const char * GpgME::Import::fingerprint() const {
  return isNull() ? 0 : d->imports[idx]->fpr ;
}

GpgME::Error GpgME::Import::error() const {
  return isNull() ? 0 : d->imports[idx]->result ;
}

GpgME::Import::Status GpgME::Import::status() const {
  if ( isNull() )
    return Unknown;
  unsigned int s = d->imports[idx]->status;
  unsigned int result = Unknown;
  if ( s & GPGME_IMPORT_NEW )    result |= NewKey;
  if ( s & GPGME_IMPORT_UID )    result |= NewUserIDs;
  if ( s & GPGME_IMPORT_SIG )    result |= NewSignatures;
  if ( s & GPGME_IMPORT_SUBKEY ) result |= NewSubkeys;
  if ( s & GPGME_IMPORT_SECRET ) result |= ContainedSecretKey;
  return static_cast<Status>( result );
}
