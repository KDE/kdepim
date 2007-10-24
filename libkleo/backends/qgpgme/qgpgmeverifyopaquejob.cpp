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

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <KLocale>

#include <assert.h>

Kleo::QGpgMEVerifyOpaqueJob::QGpgMEVerifyOpaqueJob( GpgME::Context * context )
  : VerifyOpaqueJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEVerifyOpaqueJob::~QGpgMEVerifyOpaqueJob() {
}

void Kleo::QGpgMEVerifyOpaqueJob::setup( const QByteArray & signedData ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( signedData );
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

void Kleo::QGpgMEVerifyOpaqueJob::setup( QIODevice * signedData, QIODevice * plainText ) {
    assert( signedData );
    assert( !mInData );
    assert( !mOutData );

    createInData( signedData );
    if ( plainText )
        createOutData( plainText );
    else
        createOutData();
}

void Kleo::QGpgMEVerifyOpaqueJob::start( QIODevice * signedData, QIODevice * plainText ) {
    setup( signedData, plainText );

    hookupContextToEventLoopInteractor();

    if ( const GpgME::Error err = mCtx->startOpaqueSignatureVerification( *mInData, *mOutData ) )
        doThrow( err, i18n("Can't start opaque signature verification") );
}

void Kleo::QGpgMEVerifyOpaqueJob::doOperationDoneEvent( const GpgME::Error & ) {
    emit result( mCtx->verificationResult(), outData() );
}


#include "qgpgmeverifyopaquejob.moc"
