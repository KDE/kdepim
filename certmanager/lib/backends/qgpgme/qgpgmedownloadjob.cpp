/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmedownloadjob.cpp

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

#include "qgpgmedownloadjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

//#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <qstringlist.h>

#include <assert.h>

Kleo::QGpgMEDownloadJob::QGpgMEDownloadJob( GpgME::Context * context )
  : DownloadJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEDownloadJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mPatterns( 0 ),
    mKeyDataDataProvider( 0 ),
    mKeyData( 0 )
{
  assert( context );
}

Kleo::QGpgMEDownloadJob::~QGpgMEDownloadJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;

  if ( mPatterns )
    for ( const char* * it = mPatterns ; *it ; ++it )
      free( (void*)*it );
  delete mPatterns; mPatterns = 0;

  delete mKeyData; mKeyData = 0;
  delete mKeyDataDataProvider; mKeyDataDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEDownloadJob::start( const QStringList & patterns ) {
  assert( !mPatterns );
  assert( !mKeyData );

  // set up empty data object for key data
  mKeyDataDataProvider = new QGpgME::QByteArrayDataProvider();
  mKeyData = new GpgME::Data( mKeyDataDataProvider );
  assert( !mKeyData->isNull() );

  // create a new null-terminated C array of char* from patterns:
  mPatterns = new const char*[ patterns.size() + 1 ];
  const char* * pat_it = mPatterns;
  for ( QStringList::const_iterator it = patterns.begin() ; it != patterns.end() ; ++it ) {
    if ( (*it).isEmpty() )
      continue;
    *pat_it++ = strdup( (*it).utf8().data() );
  }
  *pat_it++ = 0;

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startPublicKeyExport( mPatterns, *mKeyData );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEDownloadJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEDownloadJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & error ) {
  if ( context == mCtx ) {
    emit done();
    emit result( error, mKeyDataDataProvider->data() );
    deleteLater();
  }
}

void Kleo::QGpgMEDownloadJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmedownloadjob.moc"
