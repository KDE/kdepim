/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmedeletejob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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

#include "qgpgmedeletejob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

//#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEDeleteJob::QGpgMEDeleteJob( GpgME::Context * context )
  : DeleteJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEDeleteJob" ),
    GpgME::ProgressProvider(),
    mCtx( context )
{
  assert( context );
}

Kleo::QGpgMEDeleteJob::~QGpgMEDeleteJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
}

GpgME::Error Kleo::QGpgMEDeleteJob::start( const GpgME::Key & key, bool allowSecretKeyDeletion ) {
  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startKeyDeletion( key, allowSecretKeyDeletion );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEDeleteJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEDeleteJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & error ) {
  if ( context == mCtx ) {
    emit done();
    emit result( error );
    deleteLater();
  }
}

void Kleo::QGpgMEDeleteJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmedeletejob.moc"
