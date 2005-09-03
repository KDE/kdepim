/*
    qgpgmerefreshkeysjob.cpp

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "qgpgmerefreshkeysjob.h"
//Added by qt3to4:
#include <Q3CString>

#include "gnupgprocessbase.h"
#include "qgpgmeprogresstokenmapper.h"

#include <kdebug.h>

#include <gpgmepp/context.h>

#include <qgpgme/eventloopinteractor.h>

#include <qstringlist.h>

#include <gpg-error.h>

#include <assert.h>

Kleo::QGpgMERefreshKeysJob::QGpgMERefreshKeysJob()
  : RefreshKeysJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMERefreshKeysJob" ),
    mProcess( 0 ),
    mError( 0 )
{

}

Kleo::QGpgMERefreshKeysJob::~QGpgMERefreshKeysJob() {

}

GpgME::Error Kleo::QGpgMERefreshKeysJob::start( const QStringList & patterns ) {
  assert( mPatternsToDo.empty() );

  mPatternsToDo = patterns;
  if ( mPatternsToDo.empty() )
    mPatternsToDo.push_back( " " ); // empty list means all -> mae
				    // sure to fail the first
				    // startAProcess() guard clause

  return startAProcess();
}

#if MAX_CMD_LENGTH < 65 + 128
#error MAX_CMD_LENGTH is too low
#endif

GpgME::Error Kleo::QGpgMERefreshKeysJob::startAProcess() {
  if ( mPatternsToDo.empty() )
    return 0;
  // create and start gpgsm process:
  mProcess = new GnuPGProcessBase( this, "gpgsm -k --with-validation --force-crl-refresh --enable-crl-checks" );

  // FIXME: obbtain the path to gpgsm from gpgme, so we use the same instance.
  *mProcess << "gpgsm" << "-k" << "--with-validation" << "--force-crl-refresh"
	    << "--enable-crl-checks";
  unsigned int commandLineLength = MAX_CMD_LENGTH;
  commandLineLength -=
    strlen("gpgsm") + 1 + strlen("-k") + 1 +
    strlen("--with-validation") + 1 + strlen("--force-crl-refresh") + 1 +
    strlen("--enable-crl-checks") + 1;
  while ( !mPatternsToDo.empty() ) {
    const Q3CString pat = mPatternsToDo.front().utf8().stripWhiteSpace();
    const unsigned int patLength = pat.length();
    if ( patLength >= commandLineLength )
      break;
    mPatternsToDo.pop_front();
    if ( pat.isEmpty() )
      continue;
    *mProcess << pat;
    commandLineLength -= patLength + 1;
  }

  mProcess->setUseStatusFD( true );

  connect( mProcess, SIGNAL(processExited(KProcess*)),
	   SLOT(slotProcessExited(KProcess*)) );
  connect( mProcess, SIGNAL(receivedStderr(KProcess*,char*,int)),
	   SLOT(slotStderr(KProcess*,char*,int)) );
  connect( mProcess, SIGNAL(status(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)),
	   SLOT(slotStatus(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)) );

  if ( !mProcess->start( KProcess::NotifyOnExit, KProcess::Stderr ) ) {
    mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_ENOENT ); // what else?
    deleteLater();
    return mError;
  } else
    return 0;
}

void Kleo::QGpgMERefreshKeysJob::slotCancel() {
  if ( mProcess )
    mProcess->kill();
  mProcess = 0;
  mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_CANCELED );
}

void Kleo::QGpgMERefreshKeysJob::slotStatus( GnuPGProcessBase * proc, const QString & type, const QStringList & args ) {
  if ( proc != mProcess )
    return;
  QStringList::const_iterator it = args.begin();
  bool ok = false;

  if ( type == "ERROR" ) {


    if ( args.size() < 2 ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() not recognising ERROR with < 2 args!" << endl;
      return;
    }
    const int source = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() expected number for first ERROR arg, got something else" << endl;
      return;
    }
    ok = false;
    const int code = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() expected number for second ERROR arg, got something else" << endl;
      return;
    }
    mError = gpg_err_make( (gpg_err_source_t)source, (gpg_err_code_t)code );


  } else if ( type == "PROGRESS" ) {


    if ( args.size() < 4 ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() not recognising PROGRESS with < 4 args!" << endl;
      return;
    }
    const QString what = *++it;
    ++it; // don't use "type"...
    const int cur = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() expected number for \"cur\", got something else" << endl;
      return;
    }
    ok = false;
    const int total = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMERefreshKeysJob::slotStatus() expected number for \"total\", got something else" << endl;
      return;
    }
    emit progress( QGpgMEProgressTokenMapper::instance()->map( what, 0, cur, total ), cur, total );


  }
}

void Kleo::QGpgMERefreshKeysJob::slotStderr( KProcess *, char *, int ) {
  // implement? or not?
}

void Kleo::QGpgMERefreshKeysJob::slotProcessExited( KProcess * proc ) {
  if ( proc != mProcess )
    return;

  if ( !mError && !mPatternsToDo.empty() )
    if ( const GpgME::Error err = startAProcess() )
      mError = err;
    else
      return;

  emit done();
  if ( !mError &&
       ( !mProcess->normalExit() || mProcess->exitStatus() != 0 ) )
    mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_GENERAL );
  emit result( mError );
  deleteLater();
}

#include "qgpgmerefreshkeysjob.moc"
