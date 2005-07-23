/*
    qgpgmeverifydetachedjob.cpp

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

#include "qgpgmeverifydetachedjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/verificationresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEVerifyDetachedJob::QGpgMEVerifyDetachedJob( GpgME::Context * context )
  : VerifyDetachedJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEVerifyDetachedJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEVerifyDetachedJob::~QGpgMEVerifyDetachedJob() {
}

void Kleo::QGpgMEVerifyDetachedJob::setup( const QByteArray & signature, const QByteArray & signedData ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( signature );

  // two "in" data objects - (mis|re)use the "out" data object for the second...
  mOutDataDataProvider = new QGpgME::QByteArrayDataProvider( signedData );
  mOutData = new GpgME::Data( mOutDataDataProvider );
  assert( !mOutData->isNull() );
}

GpgME::Error Kleo::QGpgMEVerifyDetachedJob::start( const QByteArray & signature,
						   const QByteArray & signedData ) {
  setup( signature, signedData );

  hookupContextToEventLoopInteractor();

  const GpgME::Error err = mCtx->startDetachedSignatureVerification( *mInData, *mOutData );
						  
  if ( err )
    deleteLater();
  return err;
}

GpgME::VerificationResult Kleo::QGpgMEVerifyDetachedJob::exec( const QByteArray & signature,
							       const QByteArray & signedData ) {
  setup( signature, signedData );
  return mCtx->verifyDetachedSignature( *mInData, *mOutData );
}

void Kleo::QGpgMEVerifyDetachedJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mCtx->verificationResult() );
}


#include "qgpgmeverifydetachedjob.moc"
