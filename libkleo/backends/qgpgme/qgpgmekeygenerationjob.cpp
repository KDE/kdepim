/*
    qgpgmekeygenerationjob.cpp

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

#include "qgpgmekeygenerationjob.h"

#include <qgpgme/dataprovider.h>
#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/context.h>
#include <gpgme++/keygenerationresult.h>
#include <gpgme++/data.h>

#include <assert.h>

Kleo::QGpgMEKeyGenerationJob::QGpgMEKeyGenerationJob( GpgME::Context * context )
  : KeyGenerationJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context ),
    mPubKeyDataProvider( 0 ),
    mPubKey( 0 )
{
  assert( context );
}

Kleo::QGpgMEKeyGenerationJob::~QGpgMEKeyGenerationJob() {
  delete mPubKey; mPubKey = 0;
  delete mPubKeyDataProvider; mPubKeyDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEKeyGenerationJob::start( const QString & parameters ) {
  assert( !mPubKey );

  // set up empty data object for the public key data
  if ( mCtx->protocol() == GpgME::CMS ) {
    mPubKeyDataProvider = new QGpgME::QByteArrayDataProvider();
    mPubKey = new GpgME::Data( mPubKeyDataProvider );
    assert( !mPubKey->isNull() );
  }

  hookupContextToEventLoopInteractor();

  GpgME::Data null = GpgME::Data::null;
  const GpgME::Error err =
    mCtx->startKeyGeneration( parameters.toUtf8().data(), mPubKey ? *mPubKey : null );

  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEKeyGenerationJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mCtx->keyGenerationResult(), mPubKeyDataProvider ? mPubKeyDataProvider->data() : QByteArray() );
}

#include "qgpgmekeygenerationjob.moc"
