/*
    qgpgmesignencryptjob.cpp

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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "qgpgmesignencryptjob.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/data.h>
#include <gpgmepp/key.h>

#include <assert.h>

Kleo::QGpgMESignEncryptJob::QGpgMESignEncryptJob( GpgME::Context * context )
  : SignEncryptJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMESignEncryptJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMESignEncryptJob::~QGpgMESignEncryptJob() {
}

GpgME::Error Kleo::QGpgMESignEncryptJob::setup( const std::vector<GpgME::Key> & signers,
						const QByteArray & plainText ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( plainText );
  createOutData();

  return setSigningKeys( signers );
}

GpgME::Error Kleo::QGpgMESignEncryptJob::start( const std::vector<GpgME::Key> & signers,
						const std::vector<GpgME::Key> & recipients,
						const QByteArray & plainText, bool alwaysTrust ) {
  if ( const GpgME::Error error = setup( signers, plainText ) ) {
    deleteLater();
    return error;
  }

  hookupContextToEventLoopInteractor();

  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None ;
  const GpgME::Error err = mCtx->startCombinedSigningAndEncryption( recipients, *mInData, *mOutData, flags );
						  
  if ( err )
    deleteLater();
  return err;
}

std::pair<GpgME::SigningResult,GpgME::EncryptionResult>
Kleo::QGpgMESignEncryptJob::exec( const std::vector<GpgME::Key> & signers,
				  const std::vector<GpgME::Key> & recipients,
				  const QByteArray & plainText, bool alwaysTrust,
				  QByteArray & cipherText ) {
  if ( GpgME::Error err = setup( signers, plainText ) )
    return std::make_pair( GpgME::SigningResult( 0, err ), GpgME::EncryptionResult() );
  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None ;
  const std::pair<GpgME::SigningResult,GpgME::EncryptionResult> result =
    mCtx->signAndEncrypt( recipients, *mInData, *mOutData, flags );
  cipherText = mOutDataDataProvider->data();
  return result;
}

void Kleo::QGpgMESignEncryptJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mCtx->signingResult(),
	       mCtx->encryptionResult(),
	       mOutDataDataProvider->data() );
}

void Kleo::QGpgMESignEncryptJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.first.error() && !mResult.second.error() )
    return;
  if ( mResult.first.error().isCanceled() || mResult.second.error().isCanceled() )
    return;
  const QString msg = mResult.first.error()
    ? i18n("Signing failed: %1" ).arg( QString::fromLocal8Bit( mResult.first.error().asString() ) )
    : i18n("Encryption failed: %1").arg( QString::fromLocal8Bit( mResult.second.error().asString() ) ) ;
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmesignencryptjob.moc"
