/*
    qgpgmesignjob.cpp

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

#include "qgpgmesignjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/signingresult.h>
#include <gpgmepp/data.h>
#include <gpgmepp/key.h>

#include <kmessagebox.h>
#include <klocale.h>

#include <assert.h>

Kleo::QGpgMESignJob::QGpgMESignJob( GpgME::Context * context )
  : SignJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMESignJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMESignJob::~QGpgMESignJob() {
}

GpgME::Error Kleo::QGpgMESignJob::setup( const std::vector<GpgME::Key> & signers,
					 const QByteArray & plainText ) {
  assert( !mInData );
  assert( !mOutData );

  createInData( plainText );
  createOutData();

  return setSigningKeys( signers );
}

GpgME::Error Kleo::QGpgMESignJob::start( const std::vector<GpgME::Key> & signers,
					 const QByteArray & plainText,
					 GpgME::Context::SignatureMode mode ) {
  if ( const GpgME::Error error = setup( signers, plainText ) ) {
    deleteLater();
    return error;
  }

  hookupContextToEventLoopInteractor();

  const GpgME::Error err = mCtx->startSigning( *mInData, *mOutData, mode );
						  
  if ( err )
    deleteLater();
  return err;
}

GpgME::SigningResult Kleo::QGpgMESignJob::exec( const std::vector<GpgME::Key> & signers,
						const QByteArray & plainText,
						GpgME::Context::SignatureMode mode,
						QByteArray & signature ) {
  if ( const GpgME::Error err = setup( signers, plainText ) )
    return mResult = GpgME::SigningResult( 0, err );
  mResult = mCtx->sign( *mInData, *mOutData, mode );
  signature = mOutDataDataProvider->data();
  return mResult;
}

void Kleo::QGpgMESignJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mResult, mOutDataDataProvider->data() );
}

void Kleo::QGpgMESignJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.error() || mResult.error().isCanceled() )
    return;
  const QString msg = i18n("Signing failed: %1")
    .arg( QString::fromLocal8Bit( mResult.error().asString() ) );
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmesignjob.moc"
