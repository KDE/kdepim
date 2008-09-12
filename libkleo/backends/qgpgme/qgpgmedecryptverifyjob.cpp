/*
    qgpgmedecryptverifyjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2008 Klarälvdalens Datakonsult AB

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

#include "qgpgmedecryptverifyjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <KDebug>

#include <QBuffer>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEDecryptVerifyJob::QGpgMEDecryptVerifyJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEDecryptVerifyJob::~QGpgMEDecryptVerifyJob() {}

static QGpgMEDecryptVerifyJob::result_type decrypt_verify( Context * ctx, const weak_ptr<QIODevice> & cipherText_, const weak_ptr<QIODevice> & plainText_ ) {

  kDebug();

  const shared_ptr<QIODevice> cipherText = cipherText_.lock();
  const shared_ptr<QIODevice> plainText = plainText_.lock();

  QGpgME::QIODeviceDataProvider in( cipherText );
  const Data indata( &in );

  if ( !plainText ) {
    QGpgME::QByteArrayDataProvider out;
    Data outdata( &out );

    const std::pair<DecryptionResult,VerificationResult> res = ctx->decryptAndVerify( indata, outdata );
    const QString log = _detail::audit_log_as_html( ctx );
    kDebug() << "end";
    return make_tuple( res.first, res.second, out.data(), log );
  } else {
    QGpgME::QIODeviceDataProvider out( plainText );
    Data outdata( &out );

    const std::pair<DecryptionResult,VerificationResult> res = ctx->decryptAndVerify( indata, outdata );
    const QString log = _detail::audit_log_as_html( ctx );
    kDebug() << "end";
    return make_tuple( res.first, res.second, QByteArray(), log );
  }

}

static QGpgMEDecryptVerifyJob::result_type decrypt_verify_qba( Context * ctx, const QByteArray & cipherText ) {
  const shared_ptr<QBuffer> buffer( new QBuffer );
  buffer->setData( cipherText );
  if ( !buffer->open( QIODevice::ReadOnly ) )
    assert( !"This should never happen: QBuffer::open() failed" );
  return decrypt_verify( ctx, buffer, shared_ptr<QIODevice>() );
}

Error QGpgMEDecryptVerifyJob::start( const QByteArray & cipherText ) {
  run( bind( &decrypt_verify_qba, _1, cipherText ) );
  return Error();
}

void QGpgMEDecryptVerifyJob::start( const shared_ptr<QIODevice> & cipherText, const shared_ptr<QIODevice> & plainText ) {
  // the arguments passed here to the functor are stored in a QFuture, and are not
  // necessarily destroyed (living outside the UI thread) at the time the result signal
  // is emitted and the signal receiver wants to clean up IO devices.
  // To avoid such races, we pass weak_ptr's to the functor.
  run( bind( &decrypt_verify, _1, weak_ptr<QIODevice>( cipherText ), weak_ptr<QIODevice>( plainText ) ) );
}

#include "qgpgmedecryptverifyjob.moc"
