/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmekeylistjob.cpp

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

#include "qgpgmekeylistjob.h"

#include <qgpgme/eventloopinteractor.h>

#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/keylistresult.h>

#include <qstringlist.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

Kleo::QGpgMEKeyListJob::QGpgMEKeyListJob( GpgME::Context * context )
  : KeyListJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEKeyListJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ), mPatterns( 0 )
{
  assert( context );
}

Kleo::QGpgMEKeyListJob::~QGpgMEKeyListJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  if ( mPatterns )
    for ( const char* * it = mPatterns ; *it ; ++it )
      free( (void*)*it );
  delete mPatterns; mPatterns = 0;
}

GpgME::Error Kleo::QGpgMEKeyListJob::start( const QStringList & patterns, bool secretOnly ) {
  assert( !mPatterns );

  // create a new null-terminated C array of char* from patterns:
  mPatterns = new (const char*)[ patterns.size() + 1 ];
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
	   SIGNAL(nextKeyEventSignal(GpgME::Context*,const GpgME::Key&)),
	   SLOT(slotNextKeyEvent(GpgME::Context*,const GpgME::Key&)) );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startKeyListing( mPatterns, secretOnly );
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEKeyListJob::slotNextKeyEvent( GpgME::Context * context, const GpgME::Key & key ) {
  if ( context == mCtx )
    emit nextKey( key );
}

void Kleo::QGpgMEKeyListJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    // ### hmm, shall we call endKeyListing() or keyListResult() (then t.b.i.) here?
    emit result( mCtx->endKeyListing() );
    deleteLater();
  }
}

void Kleo::QGpgMEKeyListJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEKeyListJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmekeylistjob.moc"
