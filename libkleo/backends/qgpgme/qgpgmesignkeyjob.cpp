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

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/gpgsignkeyeditinteractor.h>

#include <cassert>
#include <memory>


using GpgME::Key;
using GpgME::UserID;

Kleo::QGpgMESignKeyJob::QGpgMESignKeyJob( GpgME::Context * context )
  : SignKeyJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context )
{
  assert( context );
}


Kleo::QGpgMESignKeyJob::~QGpgMESignKeyJob() {
}

GpgME::Error Kleo::QGpgMESignKeyJob::start( const GpgME::Key & key,
                                            const std::vector<unsigned int> & userIdsToSign,
                                            const GpgME::Key & signingKey,
                                            unsigned int checkLevel,
                                            SigningOption option  ) {
  assert( !mOutData );
#if 0
  createOutData();
  hookupContextToEventLoopInteractor();

  std::auto_ptr<GpgME::EditInteractor> ei( new GpgME::GpgSignKeyEditInteractor( userIdsToSign,
                                                                                signingKey,
                                                                                checkLevel,
                                                                                option == LocalSignature ? GpgME::GpgSignKeyEditInteractor::Local : GpgME::GpgSignKeyEditInteractor::Exportable ) );

  const GpgME::Error err = mCtx->startEditing( key, ei, *mOutData );
#endif
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMESignKeyJob::doOperationDoneEvent( const GpgME::Error & error ) {
  getAuditLog();
  emit result( error );
}

#include "qgpgmesignkeyjob.moc"
