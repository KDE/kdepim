/*
    qgpgmedownloadjob.cpp

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

#include "qgpgmedownloadjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>

#include <QStringList>

#include <boost/weak_ptr.hpp>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEDownloadJob::QGpgMEDownloadJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEDownloadJob::~QGpgMEDownloadJob() {}

static QGpgMEDownloadJob::result_type download_qsl( Context * ctx, const QStringList & pats ) {
  QGpgME::QByteArrayDataProvider dp;
  Data data( &dp );

  const _detail::PatternConverter pc( pats );

  const Error err= ctx->exportPublicKeys( pc.patterns(), data );
  Error ae;
  const QString log = _detail::audit_log_as_html( ctx, ae );
  return make_tuple( err, dp.data(), log, ae );
}

static QGpgMEDownloadJob::result_type download( Context * ctx, QThread * thread, const QByteArray & fpr, const weak_ptr<QIODevice> & keyData_ ) {
  const shared_ptr<QIODevice> keyData = keyData_.lock();
  if ( !keyData )
    return download_qsl( ctx, QStringList( QString::fromUtf8( fpr ) ) );

  const _detail::ToThreadMover kdMover( keyData, thread );

  QGpgME::QIODeviceDataProvider dp( keyData );
  Data data( &dp );

  const _detail::PatternConverter pc( fpr );

  const Error err = ctx->exportPublicKeys( pc.patterns(), data );
  Error ae;
  const QString log = _detail::audit_log_as_html( ctx, ae );
  return make_tuple( err, QByteArray(), log, ae );
}

Error QGpgMEDownloadJob::start( const QStringList & pats ) {
  run( bind( &download_qsl, _1, pats ) );
  return Error();
}

Error QGpgMEDownloadJob::start( const QByteArray & fpr, const boost::shared_ptr<QIODevice> & keyData ) {
  run( bind( &download, _1, _2, fpr, _3 ), keyData );
  return Error();
}

#include "qgpgmedownloadjob.moc"
