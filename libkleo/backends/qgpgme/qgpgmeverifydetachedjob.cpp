/*
    qgpgmeverifydetachedjob.cpp

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

#include "qgpgmeverifydetachedjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/data.h>

#include <KLocale>

#include <cassert>

#include <boost/weak_ptr.hpp>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEVerifyDetachedJob::QGpgMEVerifyDetachedJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEVerifyDetachedJob::~QGpgMEVerifyDetachedJob() {}

static QGpgMEVerifyDetachedJob::result_type verify_detached( Context * ctx, const weak_ptr<QIODevice> & signature_, const weak_ptr<QIODevice> & signedData_ ) {
  const shared_ptr<QIODevice> signature = signature_.lock();
  const shared_ptr<QIODevice> signedData = signedData_.lock();

  QGpgME::QIODeviceDataProvider sigDP( signature );
  Data sig( &sigDP );

  QGpgME::QIODeviceDataProvider dataDP( signedData );
  Data data( &dataDP );

  const VerificationResult res = ctx->verifyDetachedSignature( sig, data );
  const QString log = _detail::audit_log_as_html( ctx );

  return make_tuple( res, log );
}

static QGpgMEVerifyDetachedJob::result_type verify_detached_qba( Context * ctx, const QByteArray & signature, const QByteArray & signedData ) {
  QGpgME::QByteArrayDataProvider sigDP( signature );
  Data sig( &sigDP );

  QGpgME::QByteArrayDataProvider dataDP( signedData );
  Data data( &dataDP );

  const VerificationResult res = ctx->verifyDetachedSignature( sig, data );
  const QString log = _detail::audit_log_as_html( ctx );

  return make_tuple( res, log );

}

Error QGpgMEVerifyDetachedJob::start( const QByteArray & signature, const QByteArray & signedData ) {
  run( bind( &verify_detached_qba, _1, signature, signedData ) );
  return Error();
}

void QGpgMEVerifyDetachedJob::start( const shared_ptr<QIODevice> & signature, const shared_ptr<QIODevice> & signedData ) {
  // the arguments passed here to the functor are stored in a QFuture, and are not
  // necessarily destroyed (living outside the UI thread) at the time the result signal
  // is emitted and the signal receiver wants to clean up IO devices.
  // To avoid such races, we pass weak_ptr's to the functor.
  run( bind( &verify_detached, _1, weak_ptr<QIODevice>( signature ), weak_ptr<QIODevice>( signedData ) ) );
}

#include "qgpgmeverifydetachedjob.moc"
