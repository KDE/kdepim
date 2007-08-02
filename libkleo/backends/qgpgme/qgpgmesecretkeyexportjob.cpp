/*
    qgpgmesecretexportjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

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

#include "qgpgmesecretkeyexportjob.h"

#include "gnupgprocessbase.h"
#include "qgpgmeprogresstokenmapper.h"

#include <kdebug.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>

#include <qgpgme/eventloopinteractor.h>

#include <QStringList>

#include <gpg-error.h>

#include <string.h>
#include <assert.h>

Kleo::QGpgMESecretKeyExportJob::QGpgMESecretKeyExportJob( bool armour, const QString& charset )
  : ExportJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMESecretKeyExportJob" ),
    mProcess( 0 ),
    mError( 0 ),
    mArmour( armour ),
    mCharset( charset )
{

}

Kleo::QGpgMESecretKeyExportJob::~QGpgMESecretKeyExportJob() {

}

GpgME::Error Kleo::QGpgMESecretKeyExportJob::start( const QStringList & patterns ) {
  assert( mKeyData.isEmpty() );

  if ( patterns.size() != 1 || patterns.front().isEmpty() ) {
    deleteLater();
    return mError = GpgME::Error( gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_INV_VALUE ) );
  }

  // create and start gpgsm process:
  mProcess = new GnuPGProcessBase( this );
  mProcess->setObjectName( "gpgsm --export-secret-key-p12" );

  // FIXME: obbtain the path to gpgsm from gpgme, so we use the same instance.
  *mProcess << "gpgsm" << "--export-secret-key-p12";
  if ( mArmour )
    *mProcess << "--armor";
  if ( !mCharset.isEmpty() )
    *mProcess << "--p12-charset" << mCharset;
  *mProcess << patterns.front().toUtf8();

  mProcess->setUseStatusFD( true );

  connect( mProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
	   SLOT(slotProcessExited( int, QProcess::ExitStatus)) );
  connect( mProcess, SIGNAL(readyReadStandardOutput()),
	   SLOT(slotStdout()) );
  connect( mProcess, SIGNAL(readyReadStandardError()),
	   SLOT(slotStderr()) );

  connect( mProcess, SIGNAL(status(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)),
	   SLOT(slotStatus(Kleo::GnuPGProcessBase*,const QString&,const QStringList&)) );

  mProcess->setOutputChannelMode( KProcess::SeparateChannels );
  mProcess->start();
  if ( !mProcess->waitForStarted() ) {
    mError = GpgME::Error( gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_ENOENT ) ); // what else?
    deleteLater();
    return mError;
  } else
    return GpgME::Error();
}

void Kleo::QGpgMESecretKeyExportJob::slotCancel() {
  if ( mProcess )
    mProcess->kill();
  mProcess = 0;
  mError = GpgME::Error( gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_CANCELED ) );
}

void Kleo::QGpgMESecretKeyExportJob::slotStatus( GnuPGProcessBase * proc, const QString & type, const QStringList & args ) {
  if ( proc != mProcess )
    return;
  QStringList::const_iterator it = args.begin();
  bool ok = false;

  if ( type == "ERROR" ) {


    if ( args.size() < 2 ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() not recognising ERROR with < 2 args!";
      return;
    }
    const int source = (*++it).toInt( &ok );
    if ( !ok ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for first ERROR arg, got something else";
      return;
    }
    ok = false;
    const int code = (*++it).toInt( &ok );
    if ( !ok ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for second ERROR arg, got something else";
      return;
    }
    mError = GpgME::Error( gpg_err_make( (gpg_err_source_t)source, (gpg_err_code_t)code ) );


  } else if ( type == "PROGRESS" ) {


    if ( args.size() < 4 ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() not recognising PROGRESS with < 4 args!";
      return;
    }
    const QString what = *++it;
    ++it; // don't use "type"...
    const int cur = (*++it).toInt( &ok );
    if ( !ok ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for \"cur\", got something else";
      return;
    }
    ok = false;
    const int total = (*++it).toInt( &ok );
    if ( !ok ) {
      kDebug( 5150 ) <<"Kleo::QGpgMESecretKeyExportJob::slotStatus() expected number for \"total\", got something else";
      return;
    }
    emit progress( QGpgMEProgressTokenMapper::instance()->map( what, 0, cur, total ), cur, total );


  }
}

void Kleo::QGpgMESecretKeyExportJob::slotStdout() {
  QString line = QString::fromLocal8Bit( mProcess->readLine() );
  if ( !line.isEmpty() )
    return;
  const unsigned int oldlen = mKeyData.size();
  mKeyData.resize( oldlen + line.length() );
  memcpy( mKeyData.data() + oldlen, line.toLatin1(), line.length() );
}

void Kleo::QGpgMESecretKeyExportJob::slotStderr() {
  // implement? or not?
}

void Kleo::QGpgMESecretKeyExportJob::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus) {
  emit done();
  if ( !mError &&
       ( exitStatus != QProcess::NormalExit || exitCode != 0 ) )
    mError = GpgME::Error( gpg_err_make( GPG_ERR_SOURCE_GPGSM, GPG_ERR_GENERAL ) );
  emit result( mError, mKeyData );
  deleteLater();
}

#include "qgpgmesecretkeyexportjob.moc"
