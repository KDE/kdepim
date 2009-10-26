/*
    qgpgmesignjob.cpp

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

#include "qgpgmesignjob.h"

#include "ui/messagebox.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/data.h>

#include <QBuffer>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMESignJob::QGpgMESignJob( Context * context )
  : mixin_type( context ),
    mOutputIsBase64Encoded( false )
{
  lateInitialization();
}

QGpgMESignJob::~QGpgMESignJob() {}

void QGpgMESignJob::setOutputIsBase64Encoded( bool on ) {
  mOutputIsBase64Encoded = on;
}

static QGpgMESignJob::result_type sign( Context * ctx, QThread * thread,
                                        const std::vector<Key> & signers,
                                        const weak_ptr<QIODevice> & plainText_,
                                        const weak_ptr<QIODevice> & signature_,
                                        SignatureMode mode,
                                        bool outputIsBsse64Encoded ) {

  const shared_ptr<QIODevice> plainText = plainText_.lock();
  const shared_ptr<QIODevice> signature = signature_.lock();

  const _detail::ToThreadMover ptMover( plainText, thread );
  const _detail::ToThreadMover sgMover( signature, thread );

  QGpgME::QIODeviceDataProvider in( plainText );
  const Data indata( &in );

  ctx->clearSigningKeys();
  Q_FOREACH( const Key & signer, signers )
    if ( !signer.isNull() )
      if ( const Error err = ctx->addSigningKey( signer ) )
        return make_tuple( SigningResult( err ), QByteArray(), QString(), Error() );

  if ( !signature ) {
    QGpgME::QByteArrayDataProvider out;
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const SigningResult res = ctx->sign( indata, outdata, mode );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, out.data(), log, ae );
  } else {
    QGpgME::QIODeviceDataProvider out( signature );
    Data outdata( &out );

    if ( outputIsBsse64Encoded )
      outdata.setEncoding( Data::Base64Encoding );

    const SigningResult res = ctx->sign( indata, outdata, mode );
    Error ae;
    const QString log = _detail::audit_log_as_html( ctx, ae );
    return make_tuple( res, QByteArray(), log, ae );
  }

}

static QGpgMESignJob::result_type sign_qba( Context * ctx,
                                            const std::vector<Key> & signers,
                                            const QByteArray & plainText,
                                            SignatureMode mode,
                                            bool outputIsBsse64Encoded ) {
  const shared_ptr<QBuffer> buffer( new QBuffer );
  buffer->setData( plainText );
  if ( !buffer->open( QIODevice::ReadOnly ) )
    assert( !"This should never happen: QBuffer::open() failed" );
  return sign( ctx, 0, signers, buffer, shared_ptr<QIODevice>(), mode, outputIsBsse64Encoded );
}

Error QGpgMESignJob::start( const std::vector<Key> & signers, const QByteArray & plainText, SignatureMode mode ) {
  run( bind( &sign_qba, _1, signers, plainText, mode, mOutputIsBase64Encoded ) );
  return Error();
}

void QGpgMESignJob::start( const std::vector<Key> & signers, const shared_ptr<QIODevice> & plainText, const shared_ptr<QIODevice> & signature, SignatureMode mode ) {
  run( bind( &sign, _1, _2, signers, _3, _4, mode, mOutputIsBase64Encoded ), plainText, signature );
}

SigningResult QGpgMESignJob::exec( const std::vector<Key> & signers, const QByteArray & plainText, SignatureMode mode, QByteArray & signature ) {
  const result_type r = sign_qba( context(), signers, plainText, mode, mOutputIsBase64Encoded );
  signature = get<1>( r );
  resultHook( r );
  return mResult;
}

void QGpgMESignJob::resultHook( const result_type & tuple ) {
  mResult = get<0>( tuple );
}

void QGpgMESignJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( mResult.error() && !mResult.error().isCanceled() )
      MessageBox::error( parent, mResult, this, caption );
}

#include "qgpgmesignjob.moc"
