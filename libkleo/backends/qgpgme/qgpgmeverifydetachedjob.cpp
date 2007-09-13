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

#include "qgpgmeverifydetachedjob.h"
#include "qgpgmekeylistjob.h"

#include <vector>

#include <QStringList>

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <assert.h>

struct Kleo::QGpgMEVerifyDetachedJob::Private {
    Private(){}

    GpgME::VerificationResult verificationResult;
    std::vector<GpgME::Key> keys;
};

Kleo::QGpgMEVerifyDetachedJob::QGpgMEVerifyDetachedJob( GpgME::Context * context )
  : VerifyDetachedJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context ), d( new Private )
{
  assert( context );
  setAutoDelete( false );
}

Kleo::QGpgMEVerifyDetachedJob::~QGpgMEVerifyDetachedJob()
{
  delete d;
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

void Kleo::QGpgMEVerifyDetachedJob::doOperationDoneEvent( const GpgME::Error & ) {

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

   emit result( d->verificationResult );
   deleteLater();
}


void Kleo::QGpgMEVerifyDetachedJob::slotNextKey( const GpgME::Key & key )
{
    d->keys.push_back( key );
}

void Kleo::QGpgMEVerifyDetachedJob::slotKeyListingDone( const GpgME::KeyListResult & keys )
{
   emit result( d->verificationResult );
   deleteLater();
}

#include "qgpgmeverifydetachedjob.moc"
