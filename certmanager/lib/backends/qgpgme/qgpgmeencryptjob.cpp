/*
    qgpgmeencryptjob.cpp

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "qgpgmeencryptjob.h"

#include "ui/messagebox.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/encryptionresult.h>
#include <gpgmepp/data.h>

#include <klocale.h>

#include <assert.h>

Kleo::QGpgMEEncryptJob::QGpgMEEncryptJob( GpgME::Context * context )
  : EncryptJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEEncryptJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEEncryptJob::~QGpgMEEncryptJob() {
}

void Kleo::QGpgMEEncryptJob::setup( const QByteArray & plainText ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( plainText );
  createOutData();
}

GpgME::Error Kleo::QGpgMEEncryptJob::start( const std::vector<GpgME::Key> & recipients,
					    const QByteArray & plainText, bool alwaysTrust ) {
  setup( plainText );

  hookupContextToEventLoopInteractor();

  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None;
  const GpgME::Error err = mCtx->startEncryption( recipients, *mInData, *mOutData, flags );
						  
  if ( err )
    deleteLater();
  mResult = GpgME::EncryptionResult( err );
  return err;
}

GpgME::EncryptionResult Kleo::QGpgMEEncryptJob::exec( const std::vector<GpgME::Key> & recipients,
						      const QByteArray & plainText,
						      bool alwaysTrust,
						      QByteArray & ciphertext ) {
  setup( plainText );
  const GpgME::Context::EncryptionFlags flags =
    alwaysTrust ? GpgME::Context::AlwaysTrust : GpgME::Context::None;
  mResult = mCtx->encrypt( recipients, *mInData, *mOutData, flags );
  ciphertext = mOutDataDataProvider->data();
  getAuditLog();
  return mResult;
}

void Kleo::QGpgMEEncryptJob::doOperationDoneEvent( const GpgME::Error & ) {
  mResult = mCtx->encryptionResult();
  const QByteArray ciphertext = mOutDataDataProvider->data();
  getAuditLog();
  emit result( mResult, ciphertext );
}

void Kleo::QGpgMEEncryptJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( mResult.error() && !mResult.error().isCanceled() )
      Kleo::MessageBox::error( parent, mResult, this, caption );
}

#include "qgpgmeencryptjob.moc"
