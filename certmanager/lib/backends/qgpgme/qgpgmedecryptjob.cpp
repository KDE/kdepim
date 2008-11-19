/*
    qgpgmedecryptjob.cpp

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

#include "qgpgmedecryptjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/decryptionresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEDecryptJob::QGpgMEDecryptJob( GpgME::Context * context )
  : DecryptJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEDecryptJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEDecryptJob::~QGpgMEDecryptJob() {
}

void Kleo::QGpgMEDecryptJob::setup( const QByteArray & cipherText ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( cipherText );
  createOutData();
}

GpgME::Error Kleo::QGpgMEDecryptJob::start( const QByteArray & cipherText ) {
  setup( cipherText );

  hookupContextToEventLoopInteractor();

  const GpgME::Error err = mCtx->startDecryption( *mInData, *mOutData );
						  
  if ( err )
    deleteLater();
  return err;
}

GpgME::DecryptionResult Kleo::QGpgMEDecryptJob::exec( const QByteArray & cipherText,
						      QByteArray & plainText ) {
  setup( cipherText );
  const GpgME::DecryptionResult result = mCtx->decrypt( *mInData, *mOutData );
  plainText = mOutDataDataProvider->data();
  getAuditLog();
  return result;
}

void Kleo::QGpgMEDecryptJob::doOperationDoneEvent( const GpgME::Error & ) {
  const GpgME::DecryptionResult res = mCtx->decrypt( *mInData, *mOutData );
  const QByteArray plainText = mOutDataDataProvider->data();
  getAuditLog();
  emit result( res, plainText );
}

#include "qgpgmedecryptjob.moc"
