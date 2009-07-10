/*
    qgpgmesignencryptjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004, 2007 Klarälvdalens Datakonsult AB

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

#include "qgpgmesignencryptjob.h"

#include "ui/messagebox.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/exception.h>

#include <klocale.h>

#include <QBuffer>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMESignEncryptJob::QGpgMESignEncryptJob( Context * context )
  : mixin_type( context ),
    mOutputIsBase64Encoded( false )
{
  lateInitialization();
}

QGpgMESignEncryptJob::~QGpgMESignEncryptJob() {}

void QGpgMESignEncryptJob::setOutputIsBase64Encoded( bool on ) {
  mOutputIsBase64Encoded = on;
}

static QGpgMESignEncryptJob::result_type sign_encrypt( Context * ctx, const std::vector<Key> & signers, const std::vector<Key> & recipients, const weak_ptr<QIODevice> & plainText_, const weak_ptr<QIODevice> & cipherText_, bool alwaysTrust, bool outputIsBsse64Encoded ) {
  const shared_ptr<QIODevice> & plainText = plainText_.lock();
  const shared_ptr<QIODevice> & cipherText = cipherText_.lock();

  QGpgME::QIODeviceDataProvider in( plainText );
  const Data indata( &in );

  const Context::EncryptionFlags eflags =
    alwaysTrust ? Context::AlwaysTrust : Context::None ;

  ctx->clearSigningKeys();
  Q_FOREACH( const Key & signer, signers )
    if ( !signer.isNull() )
      if ( const Error err = ctx->addSigningKey( signer ) )
        return make_tuple( SigningResult( err ), EncryptionResult(), QByteArray(), QString(), Error() );

  if ( !cipherText ) {
    QGpgME::QByteArrayDataProvider out;
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const std::pair<SigningResult, EncryptionResult> res = ctx->signAndEncrypt( recipients, indata, outdata, eflags );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res.first, res.second, out.data(), log, ae );
  } else {
    QGpgME::QIODeviceDataProvider out( cipherText );
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const std::pair<SigningResult, EncryptionResult> res = ctx->signAndEncrypt( recipients, indata, outdata, eflags );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res.first, res.second, QByteArray(), log, ae );
  }

}

static QGpgMESignEncryptJob::result_type sign_encrypt_qba( Context * ctx, const std::vector<Key> & signers, const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust, bool outputIsBsse64Encoded ) {
  const shared_ptr<QBuffer> buffer( new QBuffer );
  buffer->setData( plainText );
  if ( !buffer->open( QIODevice::ReadOnly ) )
    assert( !"This should never happen: QBuffer::open() failed" );
  return sign_encrypt( ctx, signers, recipients, buffer, shared_ptr<QIODevice>(), alwaysTrust, outputIsBsse64Encoded );
}

Error QGpgMESignEncryptJob::start( const std::vector<Key> & signers, const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust ) {
  run( bind( &sign_encrypt_qba, _1, signers, recipients, plainText, alwaysTrust, mOutputIsBase64Encoded ) );
  return Error();
}

void QGpgMESignEncryptJob::start( const std::vector<Key> & signers, const std::vector<Key> & recipients, const shared_ptr<QIODevice> & plainText, const shared_ptr<QIODevice> & cipherText, bool alwaysTrust ) {
    // the arguments passed here to the functor are stored in a QFuture, and are not
    // necessarily destroyed (living outside the UI thread) at the time the result signal
    // is emitted and the signal receiver wants to clean up IO devices.
    // To avoid such races, we pass weak_ptr's to the functor.
  run( bind( &sign_encrypt, _1, signers, recipients, weak_ptr<QIODevice>( plainText ), weak_ptr<QIODevice>( cipherText ), alwaysTrust, mOutputIsBase64Encoded ) );
}

std::pair<SigningResult,EncryptionResult> QGpgMESignEncryptJob::exec( const std::vector<Key> & signers, const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust, QByteArray & cipherText ) {
  const result_type r = sign_encrypt_qba( context(), signers, recipients, plainText, alwaysTrust, mOutputIsBase64Encoded );
  cipherText = get<2>( r );
  resultHook( r );
  return mResult;
}

void QGpgMESignEncryptJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
    if ( ( mResult.first.error()  && !mResult.first.error().isCanceled() ) ||
         ( mResult.second.error() && !mResult.second.error().isCanceled() ) )
        MessageBox::error( parent, mResult.first, mResult.second, this, caption );
}

void QGpgMESignEncryptJob::resultHook( const result_type & tuple ) {
  mResult = std::make_pair( get<0>( tuple ), get<1>( tuple ) );
}

#include "qgpgmesignencryptjob.moc"
