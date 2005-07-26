/*
    chiasmusjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

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

#include "chiasmusjob.h"
#include "chiasmusbackend.h"
#include "symcryptrunprocessbase.h"

#include "kleo/cryptoconfig.h"
#include "ui/passphrasedialog.h"

#include <gpg-error.h>

#include <kshell.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qfileinfo.h>
#include <qvariant.h>

#include <memory>

#include <cassert>

Kleo::ChiasmusJob::ChiasmusJob( Mode mode )
  : Kleo::SpecialJob( 0, 0 ),
    mSymCryptRun( 0 ),
    mError( 0 ),
    mCanceled( false ),
    mTimeout( false ),
    mMode( mode )
{

}

Kleo::ChiasmusJob::~ChiasmusJob() {}

GpgME::Error Kleo::ChiasmusJob::setup() {
  if ( !checkPreconditions() )
    return mError = gpg_error( GPG_ERR_INV_VALUE );

  const Kleo::CryptoConfigEntry * class_
    = ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "symcryptrun-class" );
  const Kleo::CryptoConfigEntry * chiasmus
    = ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "path" );
  const Kleo::CryptoConfigEntry * timeoutEntry
    = ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "timeout" );
  if ( !class_ || !chiasmus || !timeoutEntry )
    return mError = gpg_error( GPG_ERR_INTERNAL );

  mSymCryptRun = new SymCryptRunProcessBase( class_->stringValue(),
                                             KShell::tildeExpand( chiasmus->urlValue().path() ),
                                             mKey, mOptions,
                                             mMode == Encrypt
                                             ? SymCryptRunProcessBase::Encrypt
                                             : SymCryptRunProcessBase::Decrypt,
                                             this, "symcryptrun" );
  QTimer::singleShot( timeoutEntry->uintValue() * 1000, this, 
                      SLOT( slotTimeout() ) );
  return 0;
}

namespace {
  struct LaterDeleter {
    QObject * _this;
    LaterDeleter( QObject * o ) : _this( o ) {}
    ~LaterDeleter() { if ( _this ) _this->deleteLater(); }
    void disable() { _this = 0; }
  };
}

GpgME::Error Kleo::ChiasmusJob::start() {

  LaterDeleter d( this );

  if ( const GpgME::Error err = setup() )
    return mError = err;

  connect( mSymCryptRun, SIGNAL(processExited(KProcess*)),
           this, SLOT(slotProcessExited(KProcess*)) );

  if ( !mSymCryptRun->launch( mInput ) )
    return mError = gpg_error( GPG_ERR_ENOENT ); // what else?

  d.disable();
  return mError = 0;
}

GpgME::Error Kleo::ChiasmusJob::slotProcessExited( KProcess * proc ) {
  if ( proc != mSymCryptRun )
    mError = gpg_error( GPG_ERR_INTERNAL );
  else if ( mCanceled )
    mError = gpg_error( GPG_ERR_CANCELED );
  else if ( mTimeout )
    mError = gpg_error( GPG_ERR_TIMEOUT );
  else if ( !proc->normalExit() )
    mError = gpg_error( GPG_ERR_GENERAL );
  else
    switch ( proc->exitStatus() ) {
    case 0: // success
      mOutput = mSymCryptRun->output();
      mError = 0;
      break;
    default:
    case 1: // Some error occured
      mStderr = mSymCryptRun->stderr();
      mError = gpg_error( GPG_ERR_GENERAL );
      break;
    case 2: // No valid passphrase was provided
      mError = gpg_error( GPG_ERR_INV_PASSPHRASE );
      break;
    case 3: // Canceled
      mError = gpg_error( GPG_ERR_CANCELED );
      break;
    }

  const Kleo::CryptoConfigEntry * showOutput
    = ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "show-output" );
  if ( showOutput && showOutput->boolValue() ) {
    showChiasmusOutput();
  }

  emit done();
  emit SpecialJob::result( mError, QVariant( mOutput ) );
  return mError;
}

void Kleo::ChiasmusJob::showChiasmusOutput() {
  kdDebug() << k_funcinfo << endl;
  if ( mStderr.isEmpty() )
    return;
  KMessageBox::information( 0 /*how to get a parent widget?*/,
                            mStderr,
                            i18n( "Output from chiasmus" ) );
}

GpgME::Error Kleo::ChiasmusJob::exec() {
  if ( const GpgME::Error err = setup() )
    return mError = err;

  if ( !mSymCryptRun->launch( mInput, KProcess::Block ) ) {
    delete mSymCryptRun; mSymCryptRun = 0;
    return mError = gpg_error( GPG_ERR_ENOENT ); // what else?
  }

  const GpgME::Error err = slotProcessExited( mSymCryptRun );
  delete mSymCryptRun; mSymCryptRun = 0;
  return err;
}

bool Kleo::ChiasmusJob::checkPreconditions() const {
  return !mKey.isEmpty();
}

void Kleo::ChiasmusJob::slotCancel() {
  if ( mSymCryptRun )
    mSymCryptRun->kill();
  mCanceled = true;
}

void Kleo::ChiasmusJob::slotTimeout() {
  if ( !mSymCryptRun ) 
    return;
  mSymCryptRun->kill();
  mTimeout = true;
}


void Kleo::ChiasmusJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mError )
    return;
  if ( mError.isCanceled() )
    return;
  const QString msg = ( mMode == Encrypt
                        ? i18n( "Encryption failed: %1" )
                        : i18n( "Decryption failed: %1" ) )
    .arg( QString::fromLocal8Bit( mError.asString() ) );
  if ( !mStderr.isEmpty() ) {
    const QString details = i18n( "The following was received on stderr:\n%1" ).arg( mStderr );
    KMessageBox::detailedError( parent, msg, details, caption );
  } else {
    KMessageBox::error( parent, msg, caption );
  }
}

#include "chiasmusjob.moc"
