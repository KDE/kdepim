/*
    qgpgmesignjob.cpp

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

#include "qgpgmesignjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/exception.h>

#include <kmessagebox.h>
#include <klocale.h>

#include <assert.h>

Kleo::QGpgMESignJob::QGpgMESignJob( GpgME::Context * context )
  : SignJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context ),
    mOutputIsBase64Encoded( false )
{
  assert( context );
}

Kleo::QGpgMESignJob::~QGpgMESignJob() {
}

void Kleo::QGpgMESignJob::setOutputIsBase64Encoded( bool on ) {
  mOutputIsBase64Encoded = on;
}

GpgME::Error Kleo::QGpgMESignJob::setup( const std::vector<GpgME::Key> & signers,
					 const QByteArray & plainText, GpgME::SignatureMode mode ) {
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

  return mCtx->startSigning( *mInData, *mOutData, mode );
}

GpgME::Error Kleo::QGpgMESignJob::start( const std::vector<GpgME::Key> & signers,
					 const QByteArray & plainText,
					 GpgME::SignatureMode mode ) {
  const GpgME::Error err = setup( signers, plainText, mode );
  if ( err )
    deleteLater();
  mResult = GpgME::SigningResult( err );
  return err;
}

void Kleo::QGpgMESignJob::setup( const std::vector<GpgME::Key> & signers,
                                 const boost::shared_ptr<QIODevice> & plainText,
                                 const boost::shared_ptr<QIODevice> & signature,
                                 GpgME::SignatureMode mode )
{
    assert( !mInData );
    assert( !mOutData );

    createInData( plainText );
    if ( signature )
        createOutData( signature );
    else
        createOutData();

    if ( mOutputIsBase64Encoded )
      mOutData->setEncoding( GpgME::Data::Base64Encoding );

    setSigningKeys( signers );
    hookupContextToEventLoopInteractor();

    if ( const GpgME::Error err = mCtx->startSigning( *mInData, *mOutData, mode ) )
        doThrow( err, i18n("Can't start sign job") );
}

void Kleo::QGpgMESignJob::start( const std::vector<GpgME::Key> & signers,
                                 const boost::shared_ptr<QIODevice> & plainText,
                                 const boost::shared_ptr<QIODevice> & signature,
                                 GpgME::SignatureMode mode )
{
    try {
        setup( signers, plainText, signature, mode );
    } catch ( const GpgME::Exception & e ) {
        mResult = GpgME::SigningResult( e.error() );
        throw;
    }
}

GpgME::SigningResult Kleo::QGpgMESignJob::exec( const std::vector<GpgME::Key> & signers,
						const QByteArray & plainText,
						GpgME::SignatureMode mode,
						QByteArray & signature ) {
  if ( const GpgME::Error err = setup( signers, plainText, mode ) )
    return mResult = GpgME::SigningResult( err );

  waitForFinished();

  signature = outData();
  return mResult = mCtx->signingResult();
}

void Kleo::QGpgMESignJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mResult = mCtx->signingResult(), outData() );
}

void Kleo::QGpgMESignJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.error() || mResult.error().isCanceled() )
    return;
  const QString msg = i18n("Signing failed: %1",
      QString::fromLocal8Bit( mResult.error().asString() ) );
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmesignjob.moc"
