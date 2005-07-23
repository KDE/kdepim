/*
    obtainkeysjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klar�lvdalens Datakonsult AB

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

#include "obtainkeysjob.h"

#include "chiasmusbackend.h"

#include "kleo/cryptoconfig.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kshell.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qtimer.h>
#include <qfileinfo.h>

#include <gpg-error.h>

#include <cassert>

Kleo::ObtainKeysJob::ObtainKeysJob()
  : SpecialJob( 0, 0 ),
    mIndex( 0 ),
    mCanceled( false )
{
  assert( ChiasmusBackend::instance() );
  assert( ChiasmusBackend::instance()->config() );
  const CryptoConfigEntry * keypaths =
    ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "keydir" );
  assert( keypaths );
  mKeyPaths = QStringList( keypaths->urlValue().path() );
}

Kleo::ObtainKeysJob::~ObtainKeysJob() {}

GpgME::Error Kleo::ObtainKeysJob::start() {
  QTimer::singleShot( 0, this, SLOT(slotPerform()) );
  return mError = 0;
}

GpgME::Error Kleo::ObtainKeysJob::exec() {
  slotPerform( false );
  return mError;
}

void Kleo::ObtainKeysJob::slotCancel() {
  mCanceled = true;
}

void Kleo::ObtainKeysJob::slotPerform() {
  slotPerform( true );
}

void Kleo::ObtainKeysJob::slotPerform( bool async ) {
  if ( mCanceled && !mError )
    mError = gpg_error( GPG_ERR_CANCELED );
  if ( mIndex >= mKeyPaths.size() || mError ) {
    emit done();
    emit SpecialJob::result( mError, QVariant( mResult ) );
    return;
  }

  emit progress( i18n( "Scanning directory %1..." ).arg( mKeyPaths[mIndex] ),
                 mIndex, mKeyPaths.size() );

  const QDir dir( KShell::tildeExpand( mKeyPaths[mIndex] ) );

  if ( const QFileInfoList * xisFiles = dir.entryInfoList( "*.xis;*.XIS", QDir::Files ) )
    for ( QFileInfoList::const_iterator it = xisFiles->begin(), end = xisFiles->end() ; it != end ; ++it )
      if ( (*it)->isReadable() )
        mResult.push_back( (*it)->absFilePath() );

  ++mIndex;

  if ( async )
    QTimer::singleShot( 0, this, SLOT(slotPerform()) );
  else
    slotPerform( false );
}

void Kleo::ObtainKeysJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mError )
    return;
  if ( mError.isCanceled() )
    return;
  const QString msg = QString::fromUtf8( mError.asString() );
  KMessageBox::error( parent, msg, caption );
}

#include "obtainkeysjob.moc"
