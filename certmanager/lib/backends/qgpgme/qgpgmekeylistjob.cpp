/*
    qgpgmekeylistjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

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

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qstringlist.h>

#include <algorithm>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

Kleo::QGpgMEKeyListJob::QGpgMEKeyListJob( GpgME::Context * context )
  : KeyListJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEKeyListJob" ),
    QGpgMEJob( this, context ),
    mResult(), mSecretOnly( false )
{
  assert( context );
}

Kleo::QGpgMEKeyListJob::~QGpgMEKeyListJob() {
}

void Kleo::QGpgMEKeyListJob::setup( const QStringList & pats, bool secretOnly ) {
  assert( !patterns() );

  mSecretOnly = secretOnly;
  setPatterns( pats );
}

GpgME::Error Kleo::QGpgMEKeyListJob::start( const QStringList & pats, bool secretOnly ) {
  setup( pats, secretOnly );

  hookupContextToEventLoopInteractor();
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(nextKeyEventSignal(GpgME::Context*,const GpgME::Key&)),
	   SLOT(slotNextKeyEvent(GpgME::Context*,const GpgME::Key&)) );

  // The communication channel between gpgme and gpgsm is limited in
  // the number of patterns that can be transported, but they won't
  // say to how much, so we need to find out ourselves if we get a
  // LINE_TOO_LONG error back...

  // We could of course just feed them single patterns, and that would
  // probably be easier, but the performance penalty would currently
  // be noticable.

  while ( const GpgME::Error err = mCtx->startKeyListing( patterns(), mSecretOnly ) ) {
    if ( err.code() == GPG_ERR_LINE_TOO_LONG ) {
      setChunkSize( chunkSize()/2 );
      if ( chunkSize() >= 1 ) {
	kdDebug(5150) << "QGpgMEKeyListJob::start(): retrying keylisting with chunksize " << chunkSize() << endl;
	continue;
      }
    }
    deleteLater();
    mResult = GpgME::KeyListResult( 0, err );
    return err;
  }
  mResult = GpgME::KeyListResult( 0, 0 );
  return 0;
}

GpgME::KeyListResult Kleo::QGpgMEKeyListJob::exec( const QStringList & pats, bool secretOnly, std::vector<GpgME::Key> & keys ) {
  setup( pats, secretOnly );

  // The communication channel between gpgme and gpgsm is limited in
  // the number of patterns that can be transported, but they won't
  // say to how much, so we need to find out ourselves if we get a
  // LINE_TOO_LONG error back...

  // We could of course just feed them single patterns, and that would
  // probably be easier, but the performance penalty would currently
  // be noticable.

  for (;;) {
    keys.clear();
    mResult = attemptSyncKeyListing( keys );
    if ( !mResult.error() || mResult.error().code() != GPG_ERR_LINE_TOO_LONG )
      return mResult;
    // got LINE_TOO_LONG, try a smaller chunksize:
    setChunkSize( chunkSize()/2 );
    if ( chunkSize() < 1 )
      // chunks smaller than one can't be -> return the error.
      return mResult;
    kdDebug(5150) << "QGpgMEKeyListJob::exec(): retrying keylisting with chunksize " << chunkSize() << endl;
  }
  kdFatal(5150) << "QGpgMEKeyListJob::exec(): Oops, this is not supposed to happen!" << endl;
  return GpgME::KeyListResult();
}

GpgME::KeyListResult Kleo::QGpgMEKeyListJob::attemptSyncKeyListing( std::vector<GpgME::Key> & keys ) {
  GpgME::KeyListResult result;
  for ( const char* * chunk = patterns() ; chunk ; chunk = nextChunk() ) {

    if ( const GpgME::Error err = mCtx->startKeyListing( chunk, mSecretOnly ) )
      return GpgME::KeyListResult( 0, err );

    GpgME::Error err;
    do
      keys.push_back( mCtx->nextKey( err ) );
    while ( !err );
    keys.pop_back();
    result.mergeWith( mCtx->endKeyListing() );
    if ( result.error() )
      break;
  }
  return result;
}

void Kleo::QGpgMEKeyListJob::slotNextKeyEvent( GpgME::Context * context, const GpgME::Key & key ) {
  if ( context == mCtx )
    emit nextKey( key );
}

void Kleo::QGpgMEKeyListJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context != mCtx )
    return;
  mResult.mergeWith( mCtx->keyListResult() );
  if ( !mResult.error() )
    if ( const char* * chunk = nextChunk() ) {
      if ( const GpgME::Error err = mCtx->startKeyListing( chunk, mSecretOnly ) )
	mResult.mergeWith( GpgME::KeyListResult( 0, err ) );
      else
	return;
    }
  emit done();
  emit result( mResult );
  deleteLater();
}

void Kleo::QGpgMEKeyListJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.error() || mResult.error().isCanceled() )
    return;
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the keys from the backend:</p>"
			    "<p><b>%1</b></p></qt>" )
    .arg( QString::fromLocal8Bit( mResult.error().asString() ) );
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmekeylistjob.moc"
