/*
    qgpgmekeylistjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2008 Klarälvdalens Datakonsult AB

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

#include "qgpgmekeylistjob.h"

#include <gpgme++/key.h>
#include <gpgme++/context.h>
#include <gpgme++/keylistresult.h>
#include <gpg-error.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <QStringList>

#include <algorithm>

#include <cstdlib>
#include <cstring>
#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

QGpgMEKeyListJob::QGpgMEKeyListJob( Context * context )
  : mixin_type( context ),
    mResult(), mSecretOnly( false )
{
  lateInitialization();
}

QGpgMEKeyListJob::~QGpgMEKeyListJob() {}

static KeyListResult do_list_keys( Context * ctx, const QStringList & pats, std::vector<Key> & keys, bool secretOnly ) {

  const _detail::PatternConverter pc( pats );

  if ( const Error err = ctx->startKeyListing( pc.patterns(), secretOnly ) )
    return KeyListResult( 0, err );

  Error err;
  do
    keys.push_back( ctx->nextKey( err ) );
  while ( !err );

  keys.pop_back();

  const KeyListResult result = ctx->endKeyListing();
  ctx->cancelPendingOperation();
  return result;
}

static QGpgMEKeyListJob::result_type list_keys( Context * ctx, QStringList pats, bool secretOnly ) {
  if ( pats.size() < 2 ) {
    std::vector<Key> keys;
    const KeyListResult r = do_list_keys( ctx, pats, keys, secretOnly );
    return make_tuple( r, keys, QString(), Error() );
  }

  // The communication channel between gpgme and gpgsm is limited in
  // the number of patterns that can be transported, but they won't
  // say to how much, so we need to find out ourselves if we get a
  // LINE_TOO_LONG error back...

  // We could of course just feed them single patterns, and that would
  // probably be easier, but the performance penalty would currently
  // be noticeable.

  unsigned int chunkSize = pats.size();
retry:
  std::vector<Key> keys;
  keys.reserve( pats.size() );
  KeyListResult result;
  do {
    const KeyListResult this_result = do_list_keys( ctx, pats.mid( 0, chunkSize ), keys, secretOnly );
    if ( this_result.error().code() == GPG_ERR_LINE_TOO_LONG ) {
      // got LINE_TOO_LONG, try a smaller chunksize:
      chunkSize /= 2;
      if ( chunkSize < 1 )
        // chunks smaller than one can't be -> return the error.
        return make_tuple( this_result, keys, QString(), Error() );
      else
        goto retry;
    }
    // ok, that seemed to work...
    result.mergeWith( this_result );
    if ( result.error().code() )
      break;
    pats = pats.mid( chunkSize );
  } while ( !pats.empty() );
  return make_tuple( result, keys, QString(), Error() );
}

Error QGpgMEKeyListJob::start( const QStringList & patterns, bool secretOnly ) {
  mSecretOnly = secretOnly;
  run( bind( &list_keys, _1, patterns, secretOnly ) );
  return Error();
}

KeyListResult QGpgMEKeyListJob::exec( const QStringList & patterns, bool secretOnly, std::vector<Key> & keys ) {
  mSecretOnly = secretOnly;
  const result_type r = list_keys( context(), patterns, secretOnly );
  resultHook( r );
  keys = get<1>( r );
  return get<0>( r );
}

void QGpgMEKeyListJob::resultHook( const result_type & tuple ) {
  mResult = get<0>( tuple );
  Q_FOREACH( const Key & key, get<1>( tuple ) )
    emit nextKey( key );
}

void QGpgMEKeyListJob::showErrorDialog( QWidget * parent, const QString & caption ) const {
  if ( !mResult.error() || mResult.error().isCanceled() )
    return;
  const QString msg = i18n( "<qt><p>An error occurred while fetching "
			    "the keys from the backend:</p>"
			    "<p><b>%1</b></p></qt>" ,
      QString::fromLocal8Bit( mResult.error().asString() ) );
  KMessageBox::error( parent, msg, caption );
}

#include "qgpgmekeylistjob.moc"
