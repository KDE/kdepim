/*
    multideletejob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "multideletejob.h"
#include "cryptobackend.h"
#include "deletejob.h"

#include <klocale.h>

#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <iterator>

#include <assert.h>

Kleo::MultiDeleteJob::MultiDeleteJob( const CryptoBackend::Protocol * protocol )
  : Job( 0, "Kleo::MultiDeleteJob" ),
    mProtocol( protocol ),
    mJob( 0 )
{
  assert( protocol );
}

Kleo::MultiDeleteJob::~MultiDeleteJob() {

}

GpgME::Error Kleo::MultiDeleteJob::start( const std::vector<GpgME::Key> & keys, bool allowSecretKeyDeletion ) {
  mKeys = keys;
  mAllowSecretKeyDeletion = allowSecretKeyDeletion;
  mIt = mKeys.begin();

  const GpgME::Error err = startAJob();

  if ( err )
    deleteLater();
  return err;
}

void Kleo::MultiDeleteJob::slotCancel() {
  if ( mJob ) mJob->slotCancel();
  mIt = mKeys.end();
}

void Kleo::MultiDeleteJob::slotResult( const GpgME::Error & err ) {
  mJob = 0;
  GpgME::Error error = err;
  if ( error || // error in last op
       mIt == mKeys.end() || // (shouldn't happen)
       ++mIt == mKeys.end() || // was the last key
       (error = startAJob()) ) { // error starting the job for the new key
    emit done();
    emit result( error, error && mIt != mKeys.end() ? *mIt : GpgME::Key::null );
    deleteLater();
    return;
  }

  const int current = mIt - mKeys.begin();
  const int total = mKeys.size();
  emit progress( i18n("progress info: \"%1 of %2\"","%1/%2").arg( current ).arg( total ), current, total );
}

GpgME::Error Kleo::MultiDeleteJob::startAJob() {
  if ( mIt == mKeys.end() )
    return 0;
  mJob = mProtocol->deleteJob();
  assert( mJob ); // FIXME: we need a way to generate errors ourselves,
		  // but I don't like the dependency on gpg-error :/

  connect( mJob, SIGNAL(result(const GpgME::Error&)), SLOT(slotResult(const GpgME::Error&)) );

  return mJob->start( *mIt, mAllowSecretKeyDeletion );
}

#include "multideletejob.moc"
