/* verificationresult.cpp - wraps a gpgme verify result
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

#include <gpgmepp/verificationresult.h>
#include "shared.h"
#include "result_p.h"

#include <gpgme.h>

#include <algorithm>
#include <cstring>
#include <cstdlib>

class GpgME::VerificationResult::Private : public GpgME::Shared {
public:
  Private( const gpgme_verify_result_t r ) : Shared() {
    if ( !r )
      return;
    // copy recursively, using compiler-generated copy ctor.
    // We just need to handle the pointers in the structs:
    for ( gpgme_signature_t is = r->signatures ; is ; is = is->next ) {
      gpgme_signature_t scopy = new _gpgme_signature( *is );
      if ( is->fpr )
	scopy->fpr = strdup( is->fpr );
      scopy->next = 0;
      sigs.push_back( scopy );
      // copy notations:
      nota.push_back( std::vector<Nota>() );
      purls.push_back( 0 );
      for ( gpgme_sig_notation_t in = is->notations ; in ; in = in->next ) {
	if ( !in->name ) {
	  if ( in->value )
	    purls.back() = strdup( in->value ); // policy url
	  continue;
	}
	Nota n = { 0, 0 };
	n.name = strdup( in->name );
	if ( in->value )
	  n.value = strdup( in->value );
	nota.back().push_back( n );
      }
    }
  }
  ~Private() {
    for ( std::vector<gpgme_signature_t>::iterator it = sigs.begin() ; it != sigs.end() ; ++it ) {
      std::free( (*it)->fpr );
      delete *it; *it = 0;
    }
    for ( std::vector< std::vector<Nota> >::iterator it = nota.begin() ; it != nota.end() ; ++it )
      for ( std::vector<Nota>::iterator jt = it->begin() ; jt != it->end() ; ++jt ) {
	std::free( jt->name );  jt->name = 0;
	std::free( jt->value ); jt->value = 0;
      }
    std::for_each( purls.begin(), purls.end(), &std::free );
  }

  struct Nota {
    char * name;
    char * value;
  };

  std::vector<gpgme_signature_t> sigs;
  std::vector< std::vector<Nota> > nota;
  std::vector<char*> purls;
};

GpgME::VerificationResult::VerificationResult( gpgme_ctx_t ctx, int error )
  : GpgME::Result( error ), d( 0 )
{
  if ( error || !ctx )
    return;
  gpgme_verify_result_t res = gpgme_op_verify_result( ctx );
  if ( !res )
    return;
  d = new Private( res );
  d->ref();
}

make_standard_stuff(VerificationResult)

GpgME::Signature GpgME::VerificationResult::signature( unsigned int idx ) const {
  return Signature( d, idx );
}

std::vector<GpgME::Signature> GpgME::VerificationResult::signatures() const {
  if ( !d )
    return std::vector<Signature>();
  std::vector<Signature> result;
  result.reserve( d->sigs.size() );
  for ( unsigned int i = 0 ; i < d->sigs.size() ; ++i )
    result.push_back( Signature( d, i ) );
  return result;
}






GpgME::Signature::Signature( VerificationResult::Private * parent, unsigned int i )
  : d( parent ), idx( i )
{
  if ( d )
    d->ref();
}

GpgME::Signature::Signature() : d( 0 ), idx( 0 ) {}

GpgME::Signature::Signature( const Signature & other )
  : d( other.d ), idx( other.idx )
{
  if ( d )
    d->ref();
}

GpgME::Signature::~Signature() {
  if ( d )
    d->unref();
}

const GpgME::Signature & GpgME::Signature::operator=( const Signature & other ) {
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


bool GpgME::Signature::isNull() const {
  return !d || idx >= d->sigs.size() ;
}


GpgME::Signature::Summary GpgME::Signature::summary() const {
  if ( isNull() )
    return None;
  gpgme_sigsum_t sigsum = d->sigs[idx]->summary;
  unsigned int result = 0;
  if ( sigsum & GPGME_SIGSUM_VALID       ) result |= Valid;
  if ( sigsum & GPGME_SIGSUM_GREEN       ) result |= Green;
  if ( sigsum & GPGME_SIGSUM_RED         ) result |= Red;
  if ( sigsum & GPGME_SIGSUM_KEY_REVOKED ) result |= KeyRevoked;
  if ( sigsum & GPGME_SIGSUM_KEY_EXPIRED ) result |= KeyExpired;
  if ( sigsum & GPGME_SIGSUM_SIG_EXPIRED ) result |= SigExpired;
  if ( sigsum & GPGME_SIGSUM_KEY_MISSING ) result |= KeyMissing;
  if ( sigsum & GPGME_SIGSUM_CRL_MISSING ) result |= CrlMissing;
  if ( sigsum & GPGME_SIGSUM_CRL_TOO_OLD ) result |= CrlTooOld;
  if ( sigsum & GPGME_SIGSUM_BAD_POLICY  ) result |= BadPolicy;
  if ( sigsum & GPGME_SIGSUM_SYS_ERROR   ) result |= SysError;
  return static_cast<Summary>( result );
}

const char * GpgME::Signature::fingerprint() const {
  return isNull() ? 0 : d->sigs[idx]->fpr ;
}

GpgME::Error GpgME::Signature::status() const {
  return isNull() ? 0 : d->sigs[idx]->status ;
}

time_t GpgME::Signature::creationTime() const {
  return static_cast<time_t>( isNull() ? 0 : d->sigs[idx]->timestamp );
}

time_t GpgME::Signature::expirationTime() const {
  return static_cast<time_t>( isNull() ? 0 : d->sigs[idx]->exp_timestamp );
}

bool GpgME::Signature::neverExpires() const {
  return expirationTime() == (time_t)0;
}

bool GpgME::Signature::wrongKeyUsage() const {
  return !isNull() && d->sigs[idx]->wrong_key_usage;
}

GpgME::Signature::Validity GpgME::Signature::validity() const {
  if ( isNull() )
    return Unknown;
  switch ( d->sigs[idx]->validity ) {
  default:
  case GPGME_VALIDITY_UNKNOWN:   return Unknown;
  case GPGME_VALIDITY_UNDEFINED: return Undefined;
  case GPGME_VALIDITY_NEVER:     return Never;
  case GPGME_VALIDITY_MARGINAL:  return Marginal;
  case GPGME_VALIDITY_FULL:      return Full;
  case GPGME_VALIDITY_ULTIMATE:  return Ultimate;
  }
}


char GpgME::Signature::validityAsString() const {
  if ( isNull() )
    return '?';
  switch ( d->sigs[idx]->validity ) {
  default:
  case GPGME_VALIDITY_UNKNOWN:   return '?';
  case GPGME_VALIDITY_UNDEFINED: return 'q';
  case GPGME_VALIDITY_NEVER:     return 'n';
  case GPGME_VALIDITY_MARGINAL:  return 'm';
  case GPGME_VALIDITY_FULL:      return 'f';
  case GPGME_VALIDITY_ULTIMATE:  return 'u';
  }
}

GpgME::Error GpgME::Signature::nonValidityReason() const {
  return isNull() ? 0 : d->sigs[idx]->validity_reason ;
}


GpgME::Signature::Notation GpgME::Signature::notation( unsigned int nidx ) const {
  return Notation( d, idx, nidx );
}

std::vector<GpgME::Signature::Notation> GpgME::Signature::notations() const {
  if ( isNull() )
    return std::vector<Notation>();
  std::vector<Notation> result;
  result.reserve( d->nota[idx].size() );
  for ( unsigned int i = 0 ; i < d->nota[idx].size() ; ++i )
    result.push_back( Notation( d, idx, i ) );
  return result;
}


GpgME::Signature::Notation::Notation( VerificationResult::Private * parent, unsigned int sindex, unsigned int nindex )
  : d( parent ), sidx( sindex ), nidx( nindex )
{
  if ( d )
    d->ref();
}

GpgME::Signature::Notation::Notation()
  : d( 0 ), sidx( 0 ), nidx( 0 ) {}

GpgME::Signature::Notation::Notation( const Notation & other )
  : d( other.d ), sidx( other.sidx ), nidx( other.nidx )
{
  if ( d )
    d->ref();
}

GpgME::Signature::Notation::~Notation() {
  if ( d )
    d->unref();
}

const GpgME::Signature::Notation & GpgME::Signature::Notation::operator=( const Notation & other ) {
  if ( this->d != other.d ) {
    if ( other.d )
      other.d->ref();
    if ( this->d )
      this->d->ref();
    this->d = other.d;
  }

  sidx = other.sidx;
  nidx = other.nidx;
  return *this;
}

bool GpgME::Signature::Notation::isNull() const {
  return !d || sidx >= d->nota.size() || nidx >= d->nota[sidx].size() ;
}


const char * GpgME::Signature::Notation::name() const {
  return isNull() ? 0 : d->nota[sidx][nidx].name ;
}

const char * GpgME::Signature::Notation::value() const {
  return isNull() ? 0 : d->nota[sidx][nidx].value ;
}

