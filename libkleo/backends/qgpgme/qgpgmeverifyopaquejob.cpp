/*
    qgpgmeverifyopaquejob.cpp

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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "qgpgmeverifyopaquejob.h"
#include "qgpgmekeylistjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <QStringList>

#include <assert.h>

struct Kleo::QGpgMEVerifyOpaqueJob::Private {
    Private(){}

    GpgME::VerificationResult verificationResult;
    std::vector<GpgME::Key> keys;
};



Kleo::QGpgMEVerifyOpaqueJob::QGpgMEVerifyOpaqueJob( GpgME::Context * context )
  : VerifyOpaqueJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context ), d( new Private )
{
  assert( context );
  setAutoDelete( false );
}

Kleo::QGpgMEVerifyOpaqueJob::~QGpgMEVerifyOpaqueJob() {
  delete d;
}

void Kleo::QGpgMEVerifyOpaqueJob::setup( const QByteArray & signedData ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( signedData );
  createOutData();
}

void Kleo::QGpgMEVerifyOpaqueJob::setup( const GpgME::Data & data ) {
  assert( !mInData );
  assert( !mOutData );
  mInData = new GpgME::Data( data );
  createOutData();
}

GpgME::Error Kleo::QGpgMEVerifyOpaqueJob::start( const QByteArray & signedData ) {
  setup( signedData );

  hookupContextToEventLoopInteractor();

  const GpgME::Error err = mCtx->startOpaqueSignatureVerification( *mInData, *mOutData );

  if ( err )
    deleteLater();
  return err;
}


GpgME::Error Kleo::QGpgMEVerifyOpaqueJob::start( const GpgME::Data & signedData ) {
  setup( signedData );

  hookupContextToEventLoopInteractor();

  const GpgME::Error err = mCtx->startOpaqueSignatureVerification( *mInData, *mOutData );

  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEVerifyOpaqueJob::doOperationDoneEvent( const GpgME::Error & ) {
   // FIXME handle error?

   // We no longer want this job to get signals, so unhook
   QObject::disconnect( QGpgME::EventLoopInteractor::instance(),
                       SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
                       mThis, SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );

   d->verificationResult = mCtx->verificationResult();
   QStringList keys;
   Q_FOREACH( GpgME::Signature sig, d->verificationResult.signatures() ) {
     keys.append( sig.fingerprint() );
   }
   Kleo::QGpgMEKeyListJob *keylisting = new Kleo::QGpgMEKeyListJob( mCtx );
   connect( keylisting, SIGNAL( result( GpgME::KeyListResult ) ),
            this, SLOT( slotKeyListingDone( GpgME::KeyListResult ) ) );
   connect( keylisting, SIGNAL( nextKey( GpgME::Key ) ),
            this, SLOT( slotNextKey( GpgME::Key ) ) );
   GpgME::Error err = keylisting->start( keys, false /*secretOnly*/ );
   if ( !err ) {
     mCtx = 0; // will be deleted by the list job
     return;
   }

   emit result( d->verificationResult, mOutDataDataProvider->data() );
   emit result( d->verificationResult, mOutDataDataProvider->data(), d->keys );
   deleteLater();
}

void Kleo::QGpgMEVerifyOpaqueJob::slotNextKey( const GpgME::Key & key )
{
    d->keys.push_back( key );
}

void Kleo::QGpgMEVerifyOpaqueJob::slotKeyListingDone( const GpgME::KeyListResult & errors )
{
    // FIXME handle error?
   emit result( d->verificationResult, mOutDataDataProvider->data() );
   emit result( d->verificationResult, mOutDataDataProvider->data(), d->keys );
   deleteLater();
}


#include "qgpgmeverifyopaquejob.moc"
