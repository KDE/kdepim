/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmedecryptjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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

#include "qgpgmedecryptjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/decryptionresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEDecryptJob::QGpgMEDecryptJob( GpgME::Context * context )
  : DecryptJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEDecryptJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mPlainTextDataProvider( 0 ),
    mPlainText( 0 ),
    mCipherTextDataProvider( 0 ),
    mCipherText( 0 )
{
  assert( context );
}

Kleo::QGpgMEDecryptJob::~QGpgMEDecryptJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  delete mCipherText; mCipherText = 0;
  delete mCipherTextDataProvider; mCipherTextDataProvider = 0;
  delete mPlainText; mPlainText = 0;
  delete mPlainTextDataProvider; mPlainTextDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEDecryptJob::start( const QByteArray & cipherText ) {
  assert( !mCipherText );
  assert( !mPlainText );

  // set up data object for cipherText
  mCipherTextDataProvider = new QGpgME::QByteArrayDataProvider( cipherText );
  mCipherText = new GpgME::Data( mCipherTextDataProvider );
  assert( !mCipherText->isNull() );

  // set up empty data object for plaintext
  mPlainTextDataProvider = new QGpgME::QByteArrayDataProvider();
  mPlainText = new GpgME::Data( mPlainTextDataProvider );
  assert( !mPlainText->isNull() );

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startDecryption( *mCipherText, *mPlainText );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEDecryptJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    emit result( mCtx->decryptionResult(), mPlainTextDataProvider->data() );
    deleteLater();
  }
}

void Kleo::QGpgMEDecryptJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEDecryptJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmedecryptjob.moc"
