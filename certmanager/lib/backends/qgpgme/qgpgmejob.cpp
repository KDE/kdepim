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

#include <assert.h>
#include <string.h>

Kleo::QGpgMEJob::QGpgMEJob( Kleo::Job * _this, GpgME::Context * context )
  : GpgME::ProgressProvider(),
    GpgME::PassphraseProvider(),
    mThis( _this ),
    mCtx( context ),
    mPatterns( 0 ),
    mInData( 0 ),
    mInDataDataProvider( 0 ),
    mOutData( 0 ),
    mOutDataDataProvider( 0 )
{
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

Kleo::QGpgMEJob::~QGpgMEJob() {
  delete mCtx; mCtx = 0;
  if ( mPatterns )
    for ( const char* * it = mPatterns ; *it ; ++it )
      free( (void*)*it );
  delete[] mPatterns; mPatterns = 0;
  delete mInData; mInData = 0;
  delete mInDataDataProvider; mInDataDataProvider = 0;
  delete mOutData; mOutData = 0;
  delete mOutDataDataProvider; mOutDataDataProvider = 0;
}

void Kleo::QGpgMEJob::hookupContextToEventLoopInteractor() {
  mCtx->setManagedByEventLoopInteractor( true );
  QObject::connect( QGpgME::EventLoopInteractor::instance(),
		    SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
		    mThis, SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
}

void Kleo::QGpgMEJob::setPatterns( const QStringList & sl, bool allowEmpty ) {
  // create a new null-terminated C array of char* from patterns:
  mPatterns = new const char*[ sl.size() + 1 ];
  const char* * pat_it = mPatterns;
  for ( QStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it ) {
    if ( (*it).isNull() )
      continue;
    if ( (*it).isEmpty() && !allowEmpty )
      continue;
    *pat_it++ = strdup( (*it).utf8().data() );
  }
  *pat_it++ = 0;
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
    msg += i18n( "gpg-agent was found in %1, but does not appear to be running." );
  } else {
    msg += i18n( "gpg-agent is part of gnupg-%1, which you can download from %2" )
           .arg( "1.9" )
           .arg( "http://www.gnupg.org/download" );  // add #gnupg2 if you can make this a real link
  }
  msg += "<br/>";
  msg += i18n( "For information on how to set up gpg-agent, see %1" )
         .arg( "http://kmail.kde.org/kmail-pgpmime-howto.html" ) + "<br/>";
  Kleo::PassphraseDialog dlg( msg, i18n("Passphrase Dialog") );
  if ( dlg.exec() != QDialog::Accepted ) {
    canceled = true;
    return 0;
  }
  canceled = false;
  // gpgme++ free()s it, and we need to copy as long as dlg isn't deleted :o
  return strdup( dlg.passphrase() );
}
