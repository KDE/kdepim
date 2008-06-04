/*
    qgpgmesignkeyjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "qgpgmesignkeyjob.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/gpgsignkeyeditinteractor.h>

#include <cassert>
#include <memory>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMESignKeyJob::QGpgMESignKeyJob( Context * context )
  : mixin_type( context ),
    m_userIDsToSign(),
    m_checkLevel( 0 ),
    m_exportable( false ),
    m_signingKey(),
    m_nonRevocable( false ),
    m_started( false )
{
  lateInitialization();
}


QGpgMESignKeyJob::~QGpgMESignKeyJob() {}

static QGpgMESignKeyJob::result_type sign_key( Context * ctx, const Key & key, const std::vector<unsigned int> & uids, unsigned int checkLevel, const Key & signer, unsigned int opts ) {
  QGpgME::QByteArrayDataProvider dp;
  Data data( &dp );

  std::auto_ptr<GpgSignKeyEditInteractor> skei( new GpgSignKeyEditInteractor );
  skei->setUserIDsToSign( uids );
  skei->setCheckLevel( checkLevel );
  skei->setSigningOptions( opts );
  skei->setSigningKey( signer );

  std::auto_ptr<EditInteractor> ei( skei );

  const Error err = ctx->edit( key, ei, data );
  const QString log = _detail::audit_log_as_html( ctx );
  return make_tuple( err, log );
}

Error QGpgMESignKeyJob::start( const Key & key ) {
  unsigned int opts = 0;
  if ( m_nonRevocable )
      opts |= GpgSignKeyEditInteractor::NonRevocable;
  if ( m_exportable )
      opts |= GpgSignKeyEditInteractor::Exportable;
  run( bind( &sign_key, _1, key, m_userIDsToSign, m_checkLevel, m_signingKey, opts ) );
  m_started = true;
  return Error();
}

void QGpgMESignKeyJob::setUserIDsToSign( const std::vector<unsigned int> & idsToSign ) {
    assert( !m_started );
    m_userIDsToSign = idsToSign;
}

void QGpgMESignKeyJob::setCheckLevel( unsigned int checkLevel ) {
    assert( !m_started );
    m_checkLevel = checkLevel;
}

void QGpgMESignKeyJob::setExportable( bool exportable ) {
    assert( !m_started );
    m_exportable = exportable;
}

void QGpgMESignKeyJob::setSigningKey( const Key & key ) {
    assert( !m_started );
    m_signingKey = key;
}

void QGpgMESignKeyJob::setNonRevocable( bool nonRevocable ) {
    assert( !m_started );
    m_nonRevocable = nonRevocable;
}

#include "qgpgmesignkeyjob.moc"

