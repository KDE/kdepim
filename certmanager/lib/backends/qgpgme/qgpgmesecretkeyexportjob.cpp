/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmesecretexportjob.cpp

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

#include "qgpgmesecretkeyexportjob.h"

#include "gnupgprocessbase.h"

#include <kdebug.h>

#include <gpgmepp/context.h>
#include <gpgmepp/data.h>

#include <qgpgme/eventloopinteractor.h>

#include <qstringlist.h>

#include <gpg-error.h>

#include <string.h>
#include <assert.h>

static inline int make_error( int src, int code ) {
  return gpg_err_make( (gpg_err_source_t)src, (gpg_err_code_t)code );
}

Kleo::QGpgMESecretKeyExportJob::QGpgMESecretKeyExportJob( bool armour )
  : ExportJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMESecretKeyExportJob" ),
    mProcess( 0 ),
    mError( 0 ),
    mArmour( armour )
{

}

Kleo::QGpgMESecretKeyExportJob::~QGpgMESecretKeyExportJob() {

}

GpgME::Error Kleo::QGpgMESecretKeyExportJob::start( const QStringList & patterns ) {
  assert( mKeyData.isEmpty() );

  if ( patterns.size() != 1 || patterns.front().isEmpty() )
    return mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_INV_VALUE );

  // create and start gpgsm process:
  mProcess = new GnuPGProcessBase( this, "gpgsm --export-secret-key-p12" );

  // FIXME: obbtain the path to gpgsm from gpgme, so we use the same instance.
  *mProcess << "gpgsm" << "--export-secret-key-p12";
  if ( mArmour )
    *mProcess << "--armor";
  *mProcess << patterns.front().utf8();

  mProcess->setUseStatusFD( true );

  connect( mProcess, SIGNAL(processExited(KProcess*)),
	   SLOT(slotProcessExited(KProcess*)) );
  connect( mProcess, SIGNAL(receivedStdout(KProcess*,char*,int)),
	   SLOT(slotStdout(KProcess*,char*,int)) );
  connect( mProcess, SIGNAL(receivedStderr(KProcess*,char*,int)),
	   SLOT(slotStderr(KProcess*,char*,int)) );
  connect( mProcess, SIGNAL(status(const QString&,const QStringList&)),
	   SLOT(slotStatus(const QString&,const QStringList&)) );

  if ( !mProcess->start( KProcess::NotifyOnExit, KProcess::AllOutput ) ) {
    return mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_ENOENT ); // what else?
    deleteLater();
  } else
    return 0;
}

void Kleo::QGpgMESecretKeyExportJob::slotCancel() {
  if ( mProcess )
    mProcess->kill();
  mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_CANCELED );
}

void Kleo::QGpgMESecretKeyExportJob::slotStatus( GnuPGProcessBase * proc, const QString & type, const QStringList & args ) {
  if ( proc != mProcess )
    return;
  QStringList::const_iterator it = args.begin();
  bool ok = false;

  if ( type == "ERROR" ) {


    if ( args.size() < 2 ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() not recognising ERROR with < 2 args!" << endl;
      return;
    }
    const int source = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for first ERROR arg, got something else" << endl;
      return;
    }
    ok = false;
    const int code = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for second ERROR arg, got something else" << endl;
      return;
    }
    mError = make_error( source, code );


  } else if ( type == "PROGRESS" ) {


    if ( args.size() < 4 ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() not recognising PROGRESS with < 4 args!" << endl;
      return;
    }
    const QString what = *++it;
    ++it; // don't use "type"...
    const int cur = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for \"cur\", got something else" << endl;
      return;
    }
    ok = false;
    const int total = (*++it).toInt( &ok );
    if ( !ok ) {
      kdDebug( 5150 ) << "Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for \"total\", got something else" << endl;
      return;
    }
    emit progress( what.latin1(), 0, cur, total );


  }
}

void Kleo::QGpgMESecretKeyExportJob::slotStdout( KProcess * proc, char * buf, int buflen ) {
  if ( proc != mProcess )
    return;
  if ( buflen <= 0 )
    return;
  if ( !buf )
    return;
  const unsigned int oldlen = mKeyData.size();
  mKeyData.resize( oldlen + buflen );
  memcpy( mKeyData.data() + oldlen, buf, buflen );
}

void Kleo::QGpgMESecretKeyExportJob::slotStderr( KProcess *, char *, int ) {
  // implement? or not?
}

void Kleo::QGpgMESecretKeyExportJob::slotProcessExited( KProcess * proc ) {
  if ( proc != mProcess )
    return;

  emit done();
  if ( !mError &&
       ( !mProcess->normalExit() || mProcess->exitStatus() != 0 ) )
    mError = gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_GENERAL );
  emit result( mError, mKeyData );
  deleteLater();
}

#include "qgpgmesecretkeyexportjob.moc"
