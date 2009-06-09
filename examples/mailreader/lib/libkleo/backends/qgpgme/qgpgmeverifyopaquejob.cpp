/*
    qgpgmeverifyopaquejob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2007,2008 Klarälvdalens Datakonsult AB

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

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <QBuffer>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEVerifyOpaqueJob::QGpgMEVerifyOpaqueJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEVerifyOpaqueJob::~QGpgMEVerifyOpaqueJob() {}

static QGpgMEVerifyOpaqueJob::result_type verify_opaque( Context * ctx, const weak_ptr<QIODevice> & signedData_, const weak_ptr<QIODevice> & plainText_ ) {

  const shared_ptr<QIODevice> plainText = plainText_.lock();
  const shared_ptr<QIODevice> signedData = signedData_.lock();

  QGpgME::QIODeviceDataProvider in( signedData );
  const Data indata( &in );

  if ( !plainText ) {
    QGpgME::QByteArrayDataProvider out;
    Data outdata( &out );

    const VerificationResult res = ctx->verifyOpaqueSignature( indata, outdata );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, out.data(), log, ae );
  } else {
    QGpgME::QIODeviceDataProvider out( plainText );
    Data outdata( &out );

    const VerificationResult res = ctx->verifyOpaqueSignature( indata, outdata );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, QByteArray(), log, ae );
  }

}

static QGpgMEVerifyOpaqueJob::result_type verify_opaque_qba( Context * ctx, const QByteArray & signedData ) {
  const shared_ptr<QBuffer> buffer( new QBuffer );
  buffer->setData( signedData );
  if ( !buffer->open( QIODevice::ReadOnly ) )
    assert( !"This should never happen: QBuffer::open() failed" );
  return verify_opaque( ctx, buffer, shared_ptr<QIODevice>() );
}

Error QGpgMEVerifyOpaqueJob::start( const QByteArray & signedData ) {
  run( bind( &verify_opaque_qba, _1, signedData ) );
  return Error();
}

void QGpgMEVerifyOpaqueJob::start( const shared_ptr<QIODevice> & signedData, const shared_ptr<QIODevice> & plainText ) {
  // the arguments passed here to the functor are stored in a QFuture, and are not
  // necessarily destroyed (living outside the UI thread) at the time the result signal
  // is emitted and the signal receiver wants to clean up IO devices.
  // To avoid such races, we pass weak_ptr's to the functor.
  run( bind( &verify_opaque, _1, weak_ptr<QIODevice>( signedData ), weak_ptr<QIODevice>( plainText ) ) );
}

#include "qgpgmeverifyopaquejob.moc"
