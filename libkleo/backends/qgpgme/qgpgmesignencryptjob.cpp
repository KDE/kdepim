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

#include <klocale.h>
#include <kmessagebox.h>

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/exception.h>

#include <assert.h>

Kleo::QGpgMESignEncryptJob::QGpgMESignEncryptJob( GpgME::Context * context )
  : SignEncryptJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context ),
    mOutputIsBase64Encoded( false )
{
  assert( context );
}

Kleo::QGpgMESignEncryptJob::~QGpgMESignEncryptJob() {
}

void Kleo::QGpgMESignEncryptJob::setOutputIsBase64Encoded( bool on ) {
  mOutputIsBase64Encoded = on;
}

GpgME::Error Kleo::QGpgMESignEncryptJob::setup( const std::vector<GpgME::Key> & signers,
                                                const std::vector<GpgME::Key> & recipients,
						const QByteArray & plainText, bool alwaysTrust ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( plainText );
  createOutData();

  if ( mOutputIsBase64Encoded )
    mOutData->setEncoding( GpgME::Data::Base64Encoding );

  try {
      setSigningKeys( signers );
  } catch ( const GpgME::Exception & e ) {
      return e.error();
  }

  hookupContextToEventLoopInteractor();

  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None ;
  return mCtx->startCombinedSigningAndEncryption( recipients, *mInData, *mOutData, flags );
}

GpgME::Error Kleo::QGpgMESignEncryptJob::start( const std::vector<GpgME::Key> & signers,
						const std::vector<GpgME::Key> & recipients,
						const QByteArray & plainText, bool alwaysTrust ) {
  const GpgME::Error err = setup( signers, recipients, plainText, alwaysTrust );
  if ( err )
    deleteLater();
  mResult.first = GpgME::SigningResult( err );
  mResult.second = GpgME::EncryptionResult();
  return err;
}

void Kleo::QGpgMESignEncryptJob::setup( const std::vector<GpgME::Key> & signers,
                                        const std::vector<GpgME::Key> & recipients,
                                        const boost::shared_ptr<QIODevice> & plainText,
                                        const boost::shared_ptr<QIODevice> & cipherText, bool alwaysTrust )
{
  assert( !mInData );
  assert( !mOutData );

  createInData( plainText );
  if ( cipherText )
      createOutData( cipherText );
  else
      createOutData();

  if ( mOutputIsBase64Encoded )
    mOutData->setEncoding( GpgME::Data::Base64Encoding );

  setSigningKeys( signers );
  hookupContextToEventLoopInteractor();

  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None ;
  if ( const GpgME::Error err = mCtx->startCombinedSigningAndEncryption( recipients, *mInData, *mOutData, flags ) )
      doThrow( err, i18n("Can't start combined sign-encrypt job") );
}

void Kleo::QGpgMESignEncryptJob::start( const std::vector<GpgME::Key> & signers,
                                        const std::vector<GpgME::Key> & recipients,
                                        const boost::shared_ptr<QIODevice> & plainText,
                                        const boost::shared_ptr<QIODevice> & cipherText, bool alwaysTrust )
{
    try {
        setup( signers, recipients, plainText, cipherText, alwaysTrust );
    } catch ( const GpgME::Exception & e ) {
        mResult.first = GpgME::SigningResult( e.error() );
        mResult.second = GpgME::EncryptionResult();
        throw;
    }
}

std::pair<GpgME::SigningResult,GpgME::EncryptionResult>
Kleo::QGpgMESignEncryptJob::exec( const std::vector<GpgME::Key> & signers,
				  const std::vector<GpgME::Key> & recipients,
				  const QByteArray & plainText, bool alwaysTrust,
				  QByteArray & cipherText ) {
  if ( GpgME::Error err = setup( signers, recipients, plainText, alwaysTrust ) )
    return mResult = std::make_pair( GpgME::SigningResult( err ), GpgME::EncryptionResult() );

  waitForFinished();

  cipherText = outData();
  mResult.first = mCtx->signingResult();
  mResult.second = mCtx->encryptionResult();
  return mResult;
}

void Kleo::QGpgMESignEncryptJob::doOperationDoneEvent( const GpgME::Error & ) {
  mResult.first = mCtx->signingResult();
  mResult.second = mCtx->encryptionResult();
  emit result( mResult.first, mResult.second, outData() );
}

void Kleo::QGpgMESignEncryptJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.first.error() && !mResult.second.error() )
    return;
  if ( mResult.first.error().isCanceled() || mResult.second.error().isCanceled() )
    return;
  const QString msg = mResult.first.error()
    ? i18n("Signing failed: %1", QString::fromLocal8Bit( mResult.first.error().asString() ) )
    : i18n("Encryption failed: %1", QString::fromLocal8Bit( mResult.second.error().asString() ) ) ;
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmesignencryptjob.moc"
