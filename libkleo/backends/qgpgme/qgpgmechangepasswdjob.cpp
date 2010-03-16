/*
    qgpgmechangepasswdjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include "qgpgmechangepasswdjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>

#include <cassert>
#include <memory>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEChangePasswdJob::QGpgMEChangePasswdJob( Context * context )
  : mixin_type( context )
{
  lateInitialization();
}

QGpgMEChangePasswdJob::~QGpgMEChangePasswdJob() {}

static QGpgMEChangePasswdJob::result_type change_passwd( Context * ctx, const Key & key ) {
#if 0 // in case we want to fall back to edit interactor for gpg...
  std::auto_ptr<EditInteractor> ei( new GpgChangePasswdEditInteractor );

  QGpgME::QByteArrayDataProvider dp;
  Data data( &dp );
  assert( !data.isNull() );
  const Error err = ctx->edit( key, ei, data );
#else
  const Error err = ctx->passwd( key );
#endif
  Error ae;
  const QString log = _detail::audit_log_as_html( ctx, ae );
  return make_tuple( err, log, ae );
}

Error QGpgMEChangePasswdJob::start( const Key & key ) {
  run( bind( &change_passwd, _1, key ) );
  return Error();
}

#include "qgpgmechangepasswdjob.moc"
