/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmesignencryptjob.cpp

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

#include "qgpgmesignencryptjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/signingresult.h>
#include <gpgmepp/encryptionresult.h>
#include <gpgmepp/data.h>
#include <gpgmepp/key.h>

#include <assert.h>

Kleo::QGpgMESignEncryptJob::QGpgMESignEncryptJob( GpgME::Context * context )
  : SignEncryptJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMESignEncryptJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mPlainTextDataProvider( 0 ),
    mPlainText( 0 ),
    mCipherTextDataProvider( 0 ),
    mCipherText( 0 )
{
  assert( context );
}

Kleo::QGpgMESignEncryptJob::~QGpgMESignEncryptJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  delete mCipherText; mCipherText = 0;
  delete mCipherTextDataProvider; mCipherTextDataProvider = 0;
  delete mPlainText; mPlainText = 0;
  delete mPlainTextDataProvider; mPlainTextDataProvider = 0;
}

GpgME::Error Kleo::QGpgMESignEncryptJob::start( const std::vector<GpgME::Key> & signers,
						const std::vector<GpgME::Key> & recipients,
						const QByteArray & plainText, bool alwaysTrust ) {
  assert( !mCipherText );
  assert( !mPlainText );

  // set up empty data object for the ciphertext
  mCipherTextDataProvider = new QGpgME::QByteArrayDataProvider();
  mCipherText = new GpgME::Data( mCipherTextDataProvider );
  assert( !mCipherText->isNull() );

  // set up data object for plainText
  mPlainTextDataProvider = new QGpgME::QByteArrayDataProvider( plainText );
  mPlainText = new GpgME::Data( mPlainTextDataProvider );
  assert( !mPlainText->isNull() );

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  mCtx->clearSigningKeys();
  for ( std::vector<GpgME::Key>::const_iterator it = signers.begin() ; it != signers.end() ; ++it ) {
    if ( (*it).isNull() )
      continue;
    if ( const GpgME::Error err = mCtx->addSigningKey( *it ) ) {
      deleteLater();
      return err;
    }
  }

  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None ;
  const GpgME::Error err = mCtx->startCombinedSigningAndEncryption( recipients, *mPlainText, *mCipherText, flags );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMESignEncryptJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    emit result( mCtx->signingResult(),
		 mCtx->encryptionResult(),
		 mCipherTextDataProvider->data() );
    deleteLater();
  }
}

void Kleo::QGpgMESignEncryptJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMESignEncryptJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmesignencryptjob.moc"
