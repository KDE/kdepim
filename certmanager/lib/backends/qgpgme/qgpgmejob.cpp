/*
    qgpgmejob.cpp

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

#include "qgpgmejob.h"
#include "qgpgmeprogresstokenmapper.h"

#include <kleo/job.h>
#include <ui/passphrasedialog.h>

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qstring.h>
#include <qstringlist.h>

#include <algorithm>

#include <assert.h>
#include <string.h>

namespace {
  class InvarianceChecker {
  public:
#ifdef NDEBUG
    InvarianceChecker( const Kleo::QGpgMEJob * ) {}
#else
    InvarianceChecker( const Kleo::QGpgMEJob * job )
      : _this( job )
    {
      assert( _this );
      _this->checkInvariants();
    }
    ~InvarianceChecker() {
      _this->checkInvariants();
    }
  private:
    const Kleo::QGpgMEJob * _this;
#endif
  };
}

Kleo::QGpgMEJob::QGpgMEJob( Kleo::Job * _this, GpgME::Context * context )
  : GpgME::ProgressProvider(),
    GpgME::PassphraseProvider(),
    mThis( _this ),
    mCtx( context ),
    mInData( 0 ),
    mInDataDataProvider( 0 ),
    mOutData( 0 ),
    mOutDataDataProvider( 0 ),
    mPatterns( 0 ),
    mReplacedPattern( 0 ),
    mNumPatterns( 0 ),
    mChunkSize( 1024 ),
    mPatternStartIndex( 0 ), mPatternEndIndex( 0 )
{
  InvarianceChecker check( this );
  assert( context );
  QObject::connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(aboutToDestroy()),
		    _this, SLOT(slotCancel()) );
  context->setProgressProvider( this );
  // (mmutz) work around a gpgme bug in versions at least <= 0.9.0.
  //         These versions will return GPG_ERR_NOT_IMPLEMENTED from
  //         a CMS sign operation when a passphrase callback is set.
  if ( context->protocol() == GpgME::Context::OpenPGP )
    context->setPassphraseProvider( this );
}

void Kleo::QGpgMEJob::checkInvariants() const {
#ifndef NDEBUG
  if ( mPatterns ) {
    assert( mPatterns[mNumPatterns] == 0 );
    if ( mPatternEndIndex > 0 ) {
      assert( mPatternEndIndex > mPatternStartIndex );
      assert( mPatternEndIndex - mPatternStartIndex == mChunkSize );
    } else {
      assert( mPatternEndIndex == mPatternStartIndex );
    }
    if ( mPatternEndIndex < mNumPatterns ) {
      assert( mPatterns[mPatternEndIndex] == 0 );
      assert( mReplacedPattern != 0 );
    } else {
      assert( mReplacedPattern == 0 );
    }
  } else {
    assert( mNumPatterns == 0 );
    assert( mPatternStartIndex == 0 );
    assert( mPatternEndIndex == 0 );
    assert( mReplacedPattern == 0 );
  }
#endif
}

Kleo::QGpgMEJob::~QGpgMEJob() {
  InvarianceChecker check( this );
  delete mCtx; mCtx = 0;
  delete mInData; mInData = 0;
  delete mInDataDataProvider; mInDataDataProvider = 0;
  delete mOutData; mOutData = 0;
  delete mOutDataDataProvider; mOutDataDataProvider = 0;
  deleteAllPatterns();
}

void Kleo::QGpgMEJob::deleteAllPatterns() {
  if ( mPatterns )
    for ( unsigned int i = 0 ; i < mNumPatterns ; ++i )
      free( (void*)mPatterns[i] );
  free( (void*)mReplacedPattern ); mReplacedPattern = 0;
  delete[] mPatterns; mPatterns = 0;
  mPatternEndIndex = mPatternStartIndex = mNumPatterns = 0;
}

void Kleo::QGpgMEJob::hookupContextToEventLoopInteractor() {
  mCtx->setManagedByEventLoopInteractor( true );
  QObject::connect( QGpgME::EventLoopInteractor::instance(),
		    SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
		    mThis, SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
}

void Kleo::QGpgMEJob::setPatterns( const QStringList & sl, bool allowEmpty ) {
  InvarianceChecker check( this );
  deleteAllPatterns();
  // create a new null-terminated C array of char* from patterns:
  mPatterns = new const char*[ sl.size() + 1 ];
  const char* * pat_it = mPatterns;
  mNumPatterns = 0;
  for ( QStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it ) {
    if ( (*it).isNull() )
      continue;
    if ( (*it).isEmpty() && !allowEmpty )
      continue;
    *pat_it++ = strdup( (*it).utf8().data() );
    ++mNumPatterns;
  }
  *pat_it++ = 0;
  mReplacedPattern = 0;
  mPatternEndIndex = mChunkSize = mNumPatterns;
}

void Kleo::QGpgMEJob::setChunkSize( unsigned int chunksize ) {
  InvarianceChecker check( this );
  if ( mReplacedPattern ) {
    mPatterns[mPatternEndIndex] = mReplacedPattern;
    mReplacedPattern = 0;
  }
  mChunkSize = std::min( chunksize, mNumPatterns );
  mPatternStartIndex = 0;
  mPatternEndIndex = mChunkSize;
  mReplacedPattern = mPatterns[mPatternEndIndex];
  mPatterns[mPatternEndIndex] = 0;
}

const char* * Kleo::QGpgMEJob::nextChunk() {
  InvarianceChecker check( this );
  if ( mReplacedPattern ) {
    mPatterns[mPatternEndIndex] = mReplacedPattern;
    mReplacedPattern = 0;
  }
  mPatternStartIndex += mChunkSize;
  mPatternEndIndex += mChunkSize;
  if ( mPatternEndIndex < mNumPatterns ) { // could safely be <=, but the last entry is NULL anyway
    mReplacedPattern = mPatterns[mPatternEndIndex];
    mPatterns[mPatternEndIndex] = 0;
  }
  return patterns();
}

const char* * Kleo::QGpgMEJob::patterns() const {
  InvarianceChecker check( this );
  if ( mPatternStartIndex < mNumPatterns )
    return mPatterns + mPatternStartIndex;
  return 0;
}

GpgME::Error Kleo::QGpgMEJob::setSigningKeys( const std::vector<GpgME::Key> & signers ) {
  mCtx->clearSigningKeys();
  for ( std::vector<GpgME::Key>::const_iterator it = signers.begin() ; it != signers.end() ; ++it ) {
    if ( (*it).isNull() )
      continue;
    if ( const GpgME::Error err = mCtx->addSigningKey( *it ) )
      return err;
  }
  return 0;
}

void Kleo::QGpgMEJob::createInData( const QByteArray & in ) {
  mInDataDataProvider = new QGpgME::QByteArrayDataProvider( in );
  mInData = new GpgME::Data( mInDataDataProvider );
  assert( !mInData->isNull() );
}

void Kleo::QGpgMEJob::createOutData() {
  mOutDataDataProvider = new QGpgME::QByteArrayDataProvider();
  mOutData = new GpgME::Data( mOutDataDataProvider );
  assert( !mOutData->isNull() );
}

void Kleo::QGpgMEJob::doSlotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & e ) {
  if ( context == mCtx ) {
    doEmitDoneSignal();
    doOperationDoneEvent( e );
    mThis->deleteLater();
  }
}

void Kleo::QGpgMEJob::doSlotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEJob::showProgress( const char * what, int type, int current, int total ) {
  doEmitProgressSignal( QGpgMEProgressTokenMapper::instance()->map( what, type, current, total ), current, total );
}

char * Kleo::QGpgMEJob::getPassphrase( const char * useridHint, const char * /*description*/,
 				       bool previousWasBad, bool & canceled ) {
  // DF: here, description is the key fingerprint, twice, then "17 0". Not really descriptive.
  //     So I'm ignoring QString::fromLocal8Bit( description ) )
  QString msg = previousWasBad ?
                i18n( "You need a passphrase to unlock the secret key for user:<br/> %1 (retry)" ) :
                i18n( "You need a passphrase to unlock the secret key for user:<br/> %1" );
  msg = msg.arg( QString::fromUtf8( useridHint ) ) + "<br/><br/>";
  msg.prepend( "<qt>" );
  msg += i18n( "This dialog will reappear every time the passphrase is needed. For a more secure solution that also allows caching the passphrase, use gpg-agent." ) + "<br/>";
  const QString gpgAgent = KStandardDirs::findExe( "gpg-agent" );
  if ( !gpgAgent.isEmpty() ) {
    msg += i18n( "gpg-agent was found in %1, but does not appear to be running." )
           .arg( gpgAgent );
  } else {
    msg += i18n( "gpg-agent is part of gnupg-%1, which you can download from %2" )
           .arg( "1.9" )
           .arg( "http://www.gnupg.org/download" );  // add #gnupg2 if you can make this a real link
  }
  msg += "<br/>";
  msg += i18n( "For information on how to set up gpg-agent, see %1" )
         .arg( "http://kmail.kde.org/kmail-pgpmime-howto.html" );
  msg += "<br/><br/>";
  msg += i18n( "Enter passphrase:" );
  Kleo::PassphraseDialog dlg( msg, i18n("Passphrase Dialog") );
  if ( dlg.exec() != QDialog::Accepted ) {
    canceled = true;
    return 0;
  }
  canceled = false;
  // gpgme++ free()s it, and we need to copy as long as dlg isn't deleted :o
  return strdup( dlg.passphrase() );
}
