/*  -*- mode: C++; c-file-style: "gnu" -*-
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

#ifndef HAVE_CONFIG_H
#include <config.h>
#endif

#include "chiasmusjob.h"

#include "ui/passphrasedialog.h"

#include <gpg-error.h>

#include <qtimer.h>
#include <qfileinfo.h>

#include <cassert>

Kleo::ChiasmusJob::ChiasmusJob( Mode mode )
  : Kleo::SpecialJob( 0, 0 ),
    mError( 0 ),
    mCanceled( false ),
    mMode( mode )
{

}

Kleo::ChiasmusJob::~ChiasmusJob() {}

GpgME::Error Kleo::ChiasmusJob::start() {
  if ( !checkPreconditions() )
    return mError = gpg_error( GPG_ERR_INV_VALUE );
  QTimer::singleShot( 0, this, SLOT(slotPerform()) );
  return mError = 0;
}

GpgME::Error Kleo::ChiasmusJob::exec() {
  if ( !checkPreconditions() )
    return mError = gpg_error( GPG_ERR_INV_VALUE );
  slotPerform();
  return mError;
}

bool Kleo::ChiasmusJob::checkPreconditions() const {
  return !mKey.isEmpty();
}

void Kleo::ChiasmusJob::slotPerform() {
  if ( mCanceled ) {
    mError = gpg_error( GPG_ERR_CANCELED );
    return;
  }

  assert( checkPreconditions() );

  Kleo::PassphraseDialog dlg( i18n( "<qt><p>Please enter the passphrase to unlock Chiasmus key</p>"
                                    "<p align=\"center\">%1</p></qt>" ).arg( mKey ),
                              i18n( "Enter Chiasmus Passphrase" ) );
  if ( dlg.exec() != QDialog::Accepted ) {
    mError = gpg_error( GPG_ERR_CANCELED );
    return;
  }

  const QFileInfo fi( mKey );
  const QCString key = fi.baseName().latin1();
  const unsigned int key_len = key.length();
  const char * const pass = dlg.passphrase();
  const unsigned int pass_len = qstrlen( pass );

  mOutput.resize( mInput.size() );

  // simple XOR cipher for now :)
  for ( unsigned int i = 0, i_end = mInput.size() ; i < i_end ; ++i )
    mOutput[i] = mInput[i] ^ key[ i % key_len ] ^ pass[ i % pass_len ] ;
}

void Kleo::ChiasmusJob::slotCancel() {
  mCanceled = true;
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
  KMessageBox::error( parent, msg, caption );
}

#include "chiasmusjob.moc"
