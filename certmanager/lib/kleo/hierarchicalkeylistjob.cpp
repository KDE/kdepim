/*
    hierarchicalkeylistjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "hierarchicalkeylistjob.h"
#include "cryptobackend.h"
#include "keylistjob.h"

#include <klocale.h>

#include <qstringlist.h>
#include <qtl.h>

#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <gpg-error.h>

#include <iterator>
#include <algorithm>

#include <assert.h>

Kleo::HierarchicalKeyListJob::HierarchicalKeyListJob( const CryptoBackend::Protocol * protocol,
						      bool remote, bool includeSigs, bool validating )
  : KeyListJob( 0, "Kleo::HierarchicalKeyListJob" ),
    mProtocol( protocol ),
    mRemote( remote ),
    mIncludeSigs( includeSigs ),
    mValidating( validating ),
    mTruncated( false ),
    mIntermediateResult(),
    mJob( 0 )
{
  assert( protocol );
}

Kleo::HierarchicalKeyListJob::~HierarchicalKeyListJob() {

}

GpgME::Error Kleo::HierarchicalKeyListJob::start( const QStringList & patterns, bool secretOnly ) {
  if ( secretOnly || patterns.empty() )
    return gpg_err_make( GPG_ERR_SOURCE_GPGME, GPG_ERR_UNSUPPORTED_OPERATION );
  qCopy( patterns.begin(), patterns.end(),
	 std::inserter( mNextSet, mNextSet.begin() ) );
  const GpgME::Error err = startAJob();
  if ( err )
    deleteLater();
  return err;
}

GpgME::KeyListResult Kleo::HierarchicalKeyListJob::exec( const QStringList &, bool,
							 std::vector<GpgME::Key> & keys ) {
  keys.clear();
  return GpgME::KeyListResult( gpg_err_make( GPG_ERR_SOURCE_GPGME, GPG_ERR_UNSUPPORTED_OPERATION ) );
}

void Kleo::HierarchicalKeyListJob::slotNextKey( const GpgME::Key & key ) {
  if ( const char * chain_id = key.chainID() )
    mNextSet.insert( chain_id );
  if ( const char * fpr = key.primaryFingerprint() )
    if ( mSentSet.find( fpr ) == mSentSet.end() ) {
      mSentSet.insert( fpr );
      emit nextKey( key );
    }
}

void Kleo::HierarchicalKeyListJob::slotCancel() {
  if ( mJob ) mJob->slotCancel();
  mNextSet.clear();
}

void Kleo::HierarchicalKeyListJob::slotResult( const GpgME::KeyListResult & res ) {
  mJob = 0;
  mIntermediateResult.mergeWith( res );
  std::set<QString> tmp;
  std::set_difference( mNextSet.begin(), mNextSet.end(),
		       mScheduledSet.begin(), mScheduledSet.end(),
		       std::inserter( tmp, tmp.begin() ) );
  mNextSet.clear();
  std::set_difference( tmp.begin(), tmp.end(),
		       mSentSet.begin(), mSentSet.end(),
		       std::inserter( mNextSet, mNextSet.begin() ) );
  if ( mIntermediateResult.error() || mNextSet.empty() ) {
    emit done();
    emit result( mIntermediateResult );
    deleteLater();
    return;
  }
  if ( const GpgME::Error error = startAJob() ) { // error starting the job for next keys
    mIntermediateResult.mergeWith( GpgME::KeyListResult( error ) );
    emit done();
    emit result( mIntermediateResult );
    deleteLater();
    return;
  }
#if 0 // FIXME
  const int current = mIt - mKeys.begin();
  const int total = mKeys.size();
  emit progress( i18n("progress info: \"%1 of %2\"","%1/%2").arg( current ).arg( total ), current, total );
#endif
}

GpgME::Error Kleo::HierarchicalKeyListJob::startAJob() {
  if ( mNextSet.empty() )
    return 0;
  mJob = mProtocol->keyListJob( mRemote, mIncludeSigs, mValidating );
  assert( mJob ); // FIXME: we need a way to generate errors ourselves,
		  // but I don't like the dependency on gpg-error :/

  connect( mJob, SIGNAL(nextKey(const GpgME::Key&)), SLOT(slotNextKey(const GpgME::Key&)) );
  connect( mJob, SIGNAL(result(const GpgME::KeyListResult&)), SLOT(slotResult(const GpgME::KeyListResult&)) );

  QStringList patterns;
  for ( std::set<QString>::const_iterator it = mNextSet.begin() ; it != mNextSet.end() ; ++it )
    patterns.push_back( *it );

  mScheduledSet.insert( mNextSet.begin(), mNextSet.end() );
  mNextSet.clear();

  return mJob->start( patterns, false );
}

#include "hierarchicalkeylistjob.moc"
