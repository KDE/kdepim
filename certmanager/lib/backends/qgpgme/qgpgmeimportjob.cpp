/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmeimportjob.cpp

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

#include "qgpgmeimportjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

//#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/importresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEImportJob::QGpgMEImportJob( GpgME::Context * context )
  : ImportJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEImportJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mKeyDataDataProvider( 0 ),
    mKeyData( 0 )
{
  assert( context );
}

Kleo::QGpgMEImportJob::~QGpgMEImportJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  delete mKeyData; mKeyData = 0;
  delete mKeyDataDataProvider; mKeyDataDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEImportJob::start( const QByteArray & keyData ) {
  assert( !mKeyData );

  // set up data object for keyData
  mKeyDataDataProvider = new QGpgME::QByteArrayDataProvider( keyData );
  mKeyData = new GpgME::Data( mKeyDataDataProvider );
  assert( !mKeyData->isNull() );

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startKeyImport( *mKeyData );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEImportJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEImportJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    emit result( mCtx->importResult() );
    deleteLater();
  }
}

void Kleo::QGpgMEImportJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmeimportjob.moc"
