/* signingresult.cpp - wraps a gpgme verify result
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

#include <gpgmepp/signingresult.h>
#include "shared.h"

#include <gpgme.h>

#include <cstring>
#include <cstdlib>

class GpgME::SigningResult::Private : public GpgME::Shared {
public:
  Private( const gpgme_sign_result_t r ) : Shared() {
    if ( !r )
      return;
    for ( gpgme_new_signature_t is = r->signatures ; is ; is = is->next ) {
      gpgme_new_signature_t copy = new _gpgme_new_signature( *is );
      if ( is->fpr )
	copy->fpr = strdup( is->fpr );
      copy->next = 0;
      created.push_back( copy );
    }
    for ( gpgme_invalid_key_t ik = r->invalid_signers ; ik ; ik = ik->next ) {
      gpgme_invalid_key_t copy = new _gpgme_invalid_key( *ik );
      if ( ik->fpr )
	copy->fpr = strdup( ik->fpr );
      copy->next = 0;
      invalid.push_back( copy );
    }
  }
  ~Private() {
    for ( std::vector<gpgme_new_signature_t>::iterator it = created.begin() ; it != created.end() ; ++it ) {
      std::free( (*it)->fpr );
      delete *it; *it = 0;
    }
    for ( std::vector<gpgme_invalid_key_t>::iterator it = invalid.begin() ; it != invalid.end() ; ++it ) {
      std::free( (*it)->fpr );
      delete *it; *it = 0;
    }
  }

  std::vector<gpgme_new_signature_t> created;
  std::vector<gpgme_invalid_key_t> invalid;
};

GpgME::SigningResult::SigningResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_sign_result_t res = gpgme_op_sign_result( ctx );
  if ( !res )
    return;
  d = new Private( res );
  d->ref();
}

GpgME::SigningResult::SigningResult( const SigningResult & other )
  : GpgME::Result( other ), d( other.d )
{
  if ( d )
    d->ref();
}

const GpgME::SigningResult & GpgME::SigningResult::operator=( const SigningResult & other ) {
  if ( this->d == other.d ) return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->ref();
  this->d = other.d;

  return *this;
}

GpgME::SigningResult::~SigningResult() {
  if ( d )
    d->unref();
  d = 0;
}

bool GpgME::SigningResult::isNull() const {
  return !d;
}


GpgME::CreatedSignature GpgME::SigningResult::createdSignature( unsigned int idx ) const {
  return CreatedSignature( d, idx );
}

std::vector<GpgME::CreatedSignature> GpgME::SigningResult::createdSignatures() const {
  if ( !d )
    return std::vector<CreatedSignature>();
  std::vector<CreatedSignature> result;
  result.reserve( d->created.size() );
  for ( unsigned int i = 0 ; i < d->created.size() ; ++i )
    result.push_back( CreatedSignature( d, i ) );
  return result;
}


GpgME::InvalidSigningKey GpgME::SigningResult::invalidSigningKey( unsigned int idx ) const {
  return InvalidSigningKey( d, idx );
}

std::vector<GpgME::InvalidSigningKey> GpgME::SigningResult::invalidSigningKeys() const {
  if ( !d )
    return std::vector<GpgME::InvalidSigningKey>();
  std::vector<GpgME::InvalidSigningKey> result;
  result.reserve( d->invalid.size() );
  for ( unsigned int i = 0 ; i < d->invalid.size() ; ++i )
    result.push_back( InvalidSigningKey( d, i ) );
  return result;
}




GpgME::InvalidSigningKey::InvalidSigningKey( SigningResult::Private * parent, unsigned int i )
  : d( parent ), idx( i )
{
  if ( d )
    d->ref();
}

GpgME::InvalidSigningKey::InvalidSigningKey() : d( 0 ), idx( 0 ) {}

GpgME::InvalidSigningKey::InvalidSigningKey( const InvalidSigningKey & other )
  : d( other.d ), idx( other.idx )
{
  if ( d )
    d->ref();
}

GpgME::InvalidSigningKey::~InvalidSigningKey() {
  if ( d )
    d->unref();
}

const GpgME::InvalidSigningKey & GpgME::InvalidSigningKey::operator=( const InvalidSigningKey & other ) {
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


bool GpgME::InvalidSigningKey::isNull() const {
  return !d || idx >= d->invalid.size() ;
}

const char * GpgME::InvalidSigningKey::fingerprint() const {
  return isNull() ? 0 : d->invalid[idx]->fpr ;
}

GpgME::Error GpgME::InvalidSigningKey::reason() const {
  return isNull() ? 0 : d->invalid[idx]->reason ;
}



GpgME::CreatedSignature::CreatedSignature( SigningResult::Private * parent, unsigned int i )
  : d( parent ), idx( i )
{
  if ( d )
    d->ref();
}

GpgME::CreatedSignature::CreatedSignature() : d( 0 ), idx( 0 ) {}

GpgME::CreatedSignature::CreatedSignature( const CreatedSignature & other )
  : d( other.d ), idx( other.idx )
{
  if ( d )
    d->ref();
}

GpgME::CreatedSignature::~CreatedSignature() {
  if ( d )
    d->unref();
}

const GpgME::CreatedSignature & GpgME::CreatedSignature::operator=( const CreatedSignature & other ) {
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


bool GpgME::CreatedSignature::isNull() const {
  return !d || idx >= d->created.size() ;
}

const char * GpgME::CreatedSignature::fingerprint() const {
  return isNull() ? 0 : d->created[idx]->fpr ;
}

time_t GpgME::CreatedSignature::creationTime() const {
  return static_cast<time_t>( isNull() ? 0 : d->created[idx]->timestamp );
}

GpgME::Context::SignatureMode GpgME::CreatedSignature::mode() const {
  if ( isNull() )
    return Context::Normal;
  switch ( d->created[idx]->type ) {
  default:
  case GPGME_SIG_MODE_NORMAL: return Context::Normal;
  case GPGME_SIG_MODE_DETACH: return Context::Detached;
  case GPGME_SIG_MODE_CLEAR:  return Context::Clearsigned;
  }
}

unsigned int GpgME::CreatedSignature::publicKeyAlgorithm() const {
  return isNull() ? 0 : d->created[idx]->pubkey_algo ;
}

const char * GpgME::CreatedSignature::publicKeyAlgorithmAsString() const {
  return gpgme_pubkey_algo_name( isNull() ? (gpgme_pubkey_algo_t)0 : d->created[idx]->pubkey_algo );
}

unsigned int GpgME::CreatedSignature::hashAlgorithm() const {
  return isNull() ? 0 : d->created[idx]->hash_algo ;
}

const char * GpgME::CreatedSignature::hashAlgorithmAsString() const {
  return gpgme_hash_algo_name( isNull() ? (gpgme_hash_algo_t)0 : d->created[idx]->hash_algo );
}

unsigned int GpgME::CreatedSignature::signatureClass() const {
  return isNull() ? 0 : d->created[idx]->sig_class ;
}

