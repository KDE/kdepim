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

#include <qstringlist.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

Kleo::QGpgMEKeyListJob::QGpgMEKeyListJob( GpgME::Context * context )
  : KeyListJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEKeyListJob" ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEKeyListJob::~QGpgMEKeyListJob() {
}

void Kleo::QGpgMEKeyListJob::setup( const QStringList & patterns ) {
  assert( !mPatterns );

  setPatterns( patterns );
}

GpgME::Error Kleo::QGpgMEKeyListJob::start( const QStringList & patterns, bool secretOnly ) {
  setup( patterns );

  hookupContextToEventLoopInteractor();
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(nextKeyEventSignal(GpgME::Context*,const GpgME::Key&)),
	   SLOT(slotNextKeyEvent(GpgME::Context*,const GpgME::Key&)) );

  const GpgME::Error err = mCtx->startKeyListing( mPatterns, secretOnly );

  if ( err )
    deleteLater();
  mResult = GpgME::KeyListResult( 0, err );
  return err;
}

GpgME::KeyListResult Kleo::QGpgMEKeyListJob::exec( const QStringList & patterns, bool secretOnly, std::vector<GpgME::Key> & keys ) {
  keys.clear();
  setup( patterns );
  GpgME::Error err = mCtx->startKeyListing( mPatterns, secretOnly );
  if ( !err ) {
    do
      keys.push_back( mCtx->nextKey( err ) );
    while ( !err );
    keys.pop_back();
  }
  return mResult = mCtx->endKeyListing();
}

void Kleo::QGpgMEKeyListJob::slotNextKeyEvent( GpgME::Context * context, const GpgME::Key & key ) {
  if ( context == mCtx )
    emit nextKey( key );
}

void Kleo::QGpgMEKeyListJob::doOperationDoneEvent( const GpgME::Error & ) {
  emit result( mResult = mCtx->keyListResult() );
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
