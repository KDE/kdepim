/* encryptionresult.cpp - wraps a gpgme verify result
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

#include <gpgmepp/encryptionresult.h>
#include "shared.h"

#include <gpgme.h>

#include <cstring>
#include <cstdlib>

class GpgME::EncryptionResult::Private : public GpgME::Shared {
public:
  Private( const gpgme_encrypt_result_t r ) : Shared() {
    if ( !r )
      return;
    for ( gpgme_invalid_key_t ik = r->invalid_recipients ; ik ; ik = ik->next ) {
      gpgme_invalid_key_t copy = new _gpgme_invalid_key( *ik );
      if ( ik->fpr )
	copy->fpr = strdup( ik->fpr );
      copy->next = 0;
      invalid.push_back( copy );
    }
  }
  ~Private() {
    for ( std::vector<gpgme_invalid_key_t>::iterator it = invalid.begin() ; it != invalid.end() ; ++it ) {
      std::free( (*it)->fpr );
      delete *it; *it = 0;
    }
  }

  std::vector<gpgme_invalid_key_t> invalid;
};

GpgME::EncryptionResult::EncryptionResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_encrypt_result_t res = gpgme_op_encrypt_result( ctx );
  if ( !res )
    return;
  d = new Private( res );
  d->ref();
}

GpgME::EncryptionResult::EncryptionResult( const EncryptionResult & other )
  : GpgME::Result( other ), d( other.d )
{
  if ( d )
    d->ref();
}

const GpgME::EncryptionResult & GpgME::EncryptionResult::operator=( const EncryptionResult & other ) {
  if ( this->d == other.d ) return *this;

  if ( other.d )
    other.d->ref();
  if ( this->d )
    this->d->ref();
  this->d = other.d;

  return *this;
}

GpgME::EncryptionResult::~EncryptionResult() {
  if ( d )
    d->unref();
  d = 0;
}

bool GpgME::EncryptionResult::isNull() const {
  return !d;
}


unsigned int GpgME::EncryptionResult::numInvalidRecipients() const {
  return d ? d->invalid.size() : 0 ;
}

GpgME::InvalidRecipient GpgME::EncryptionResult::invalidEncryptionKey( unsigned int idx ) const {
  return InvalidRecipient( d, idx );
}

std::vector<GpgME::InvalidRecipient> GpgME::EncryptionResult::invalidEncryptionKeys() const {
  if ( !d )
    return std::vector<GpgME::InvalidRecipient>();
  std::vector<GpgME::InvalidRecipient> result;
  result.reserve( d->invalid.size() );
  for ( unsigned int i = 0 ; i < d->invalid.size() ; ++i )
    result.push_back( InvalidRecipient( d, i ) );
  return result;
}




GpgME::InvalidRecipient::InvalidRecipient( EncryptionResult::Private * parent, unsigned int i )
  : d( parent ), idx( i )
{
  if ( d )
    d->ref();
}

GpgME::InvalidRecipient::InvalidRecipient() : d( 0 ), idx( 0 ) {}

GpgME::InvalidRecipient::InvalidRecipient( const InvalidRecipient & other )
  : d( other.d ), idx( other.idx )
{
  if ( d )
    d->ref();
}

GpgME::InvalidRecipient::~InvalidRecipient() {
  if ( d )
    d->unref();
}

const GpgME::InvalidRecipient & GpgME::InvalidRecipient::operator=( const InvalidRecipient & other ) {
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


bool GpgME::InvalidRecipient::isNull() const {
  return !d || idx >= d->invalid.size() ;
}

const char * GpgME::InvalidRecipient::fingerprint() const {
  return isNull() ? 0 : d->invalid[idx]->fpr ;
}

GpgME::Error GpgME::InvalidRecipient::reason() const {
  return isNull() ? 0 : d->invalid[idx]->reason ;
}

