/*
    qgpgmeencryptjob.cpp

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

#include "qgpgmeencryptjob.h"

#include "ui/messagebox.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/data.h>

#include <QBuffer>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEEncryptJob::QGpgMEEncryptJob( Context * context )
  : mixin_type( context ),
    mOutputIsBase64Encoded( false )
{
  lateInitialization();
}

QGpgMEEncryptJob::~QGpgMEEncryptJob() {}

void QGpgMEEncryptJob::setOutputIsBase64Encoded( bool on ) {
  mOutputIsBase64Encoded = on;
}

static QGpgMEEncryptJob::result_type encrypt( Context * ctx,
                                              const std::vector<Key> & recipients,
                                              const weak_ptr<QIODevice> & plainText_,
                                              const weak_ptr<QIODevice> & cipherText_,
                                              bool alwaysTrust,
                                              bool outputIsBsse64Encoded ) {

  const shared_ptr<QIODevice> plainText = plainText_.lock();
  const shared_ptr<QIODevice> cipherText = cipherText_.lock();

  QGpgME::QIODeviceDataProvider in( plainText );
  const Data indata( &in );

  const Context::EncryptionFlags eflags =
    alwaysTrust ? Context::AlwaysTrust : Context::None ;

  if ( !cipherText ) {
    QGpgME::QByteArrayDataProvider out;
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const EncryptionResult res = ctx->encrypt( recipients, indata, outdata, eflags );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, out.data(), log, ae );
  } else {
    QGpgME::QIODeviceDataProvider out( cipherText );
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const EncryptionResult res = ctx->encrypt( recipients, indata, outdata, eflags );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, QByteArray(), log, ae );
  }

}

static QGpgMEEncryptJob::result_type encrypt_qba( Context * ctx, const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust, bool outputIsBsse64Encoded ) {
  const shared_ptr<QBuffer> buffer( new QBuffer );
  buffer->setData( plainText );
  if ( !buffer->open( QIODevice::ReadOnly ) )
    assert( !"This should never happen: QBuffer::open() failed" );
  return encrypt( ctx, recipients, buffer, shared_ptr<QIODevice>(), alwaysTrust, outputIsBsse64Encoded );
}

Error QGpgMEEncryptJob::start( const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust ) {
  run( bind( &encrypt_qba, _1, recipients, plainText, alwaysTrust, mOutputIsBase64Encoded ) );
  return Error();
}

void QGpgMEEncryptJob::start( const std::vector<Key> & recipients, const shared_ptr<QIODevice> & plainText, const shared_ptr<QIODevice> & cipherText, bool alwaysTrust ) {
  // the arguments passed here to the functor are stored in a QFuture, and are not
  // necessarily destroyed (living outside the UI thread) at the time the result signal
  // is emitted and the signal receiver wants to clean up IO devices.
  // To avoid such races, we pass weak_ptr's to the functor.
  run( bind( &encrypt,
             _1,
             recipients,
             weak_ptr<QIODevice>( plainText ),
             weak_ptr<QIODevice>( cipherText ),
             alwaysTrust,
             mOutputIsBase64Encoded ) );
}

EncryptionResult QGpgMEEncryptJob::exec( const std::vector<Key> & recipients, const QByteArray & plainText, bool alwaysTrust, QByteArray & cipherText ) {
  const result_type r = encrypt_qba( context(), recipients, plainText, alwaysTrust, mOutputIsBase64Encoded );
  cipherText = get<1>( r );
  resultHook( r );
  return mResult;
}

void QGpgMEEncryptJob::resultHook( const result_type & tuple ) {
  mResult = get<0>( tuple );
}

void QGpgMEEncryptJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( mResult.error() && !mResult.error().isCanceled() )
      MessageBox::error( parent, mResult, this, caption );
}

#include "qgpgmeencryptjob.moc"
