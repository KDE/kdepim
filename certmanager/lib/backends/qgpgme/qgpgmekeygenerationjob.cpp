/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmekeygenerationjob.cpp

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

#include "qgpgmekeygenerationjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/keygenerationresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEKeyGenerationJob::QGpgMEKeyGenerationJob( GpgME::Context * context )
  : KeyGenerationJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEKeyGenerationJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mPubKeyDataProvider( 0 ),
    mPubKey( 0 )
{
  assert( context );
}

Kleo::QGpgMEKeyGenerationJob::~QGpgMEKeyGenerationJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  delete mPubKey; mPubKey = 0;
  delete mPubKeyDataProvider; mPubKeyDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEKeyGenerationJob::start( const QString & parameters ) {
  assert( !mPubKey );

  // set up empty data object for the public key data
  if ( mCtx->protocol() == GpgME::Context::CMS ) {
    mPubKeyDataProvider = new QGpgME::QByteArrayDataProvider();
    mPubKey = new GpgME::Data( mPubKeyDataProvider );
    assert( !mPubKey->isNull() );
  }

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err =
    mCtx->startKeyGeneration( parameters.utf8().data(), mPubKey ? *mPubKey : GpgME::Data::null );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEKeyGenerationJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    emit result( mCtx->keyGenerationResult(), mPubKeyDataProvider ? mPubKeyDataProvider->data() : QByteArray() );
    deleteLater();
  }
}

void Kleo::QGpgMEKeyGenerationJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEKeyGenerationJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmekeygenerationjob.moc"
