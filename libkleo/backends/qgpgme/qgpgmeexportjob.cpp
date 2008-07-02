/*
    qgpgmeexportjob.cpp

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

#include "qgpgmeexportjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>

#include <QStringList>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEExportJob::QGpgMEExportJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEExportJob::~QGpgMEExportJob() {}

static QGpgMEExportJob::result_type export_qba( Context * ctx, const QStringList & patterns ) {

  const _detail::PatternConverter pc( patterns );

  QGpgME::QByteArrayDataProvider dp;
  Data data( &dp );

  const Error err = ctx->exportPublicKeys( pc.patterns(), data );
  const QString log = _detail::audit_log_as_html( ctx );
  return make_tuple( err, dp.data(), log );
}

Error QGpgMEExportJob::start( const QStringList & patterns ) {
  run( bind( &export_qba, _1, patterns ) );
  return Error();
}

#include "qgpgmeexportjob.moc"
