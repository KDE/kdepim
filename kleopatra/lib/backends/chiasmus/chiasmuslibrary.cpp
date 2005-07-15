/*
    chiasmuslibrary.cpp

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


#include "chiasmuslibrary.h"

#include "chiasmusbackend.h"

#include "kleo/cryptoconfig.h"

#include <klibloader.h>
#include <kdebug.h>
#include <klocale.h>

#include <qfile.h>

#include <vector>
#include <algorithm>

#include <cassert>
#include <cstdlib>
#include <cstring>

Kleo::ChiasmusLibrary * Kleo::ChiasmusLibrary::self = 0;

Kleo::ChiasmusLibrary::ChiasmusLibrary() : mXiaLibrary( 0 ) {
  self = this;
}

Kleo::ChiasmusLibrary::~ChiasmusLibrary() {
  //delete mXiaLibrary; // hmm, how to get rid of it, then?
}

Kleo::ChiasmusLibrary::main_func Kleo::ChiasmusLibrary::chiasmus( QString * reason ) const {
  assert( ChiasmusBackend::instance() );
  assert( ChiasmusBackend::instance()->config() );
  const CryptoConfigEntry * lib = ChiasmusBackend::instance()->config()->entry( "Chiasmus", "General", "lib" );
  assert( lib );
  const QString libfile = lib->urlValue().path();
  if ( !mXiaLibrary )
    mXiaLibrary = KLibLoader::self()->library( QFile::encodeName( libfile ) );
  if ( !mXiaLibrary ) {
    if ( reason )
      *reason = i18n( "Failed to load %1: %2" )
                .arg( libfile,KLibLoader::self()->lastErrorMessage() );
    kdDebug(5150) << "ChiasmusLibrary: loading \"" << libfile
                  << "\" failed: " << KLibLoader::self()->lastErrorMessage() << endl;
    return 0;
  }
  if ( !mXiaLibrary->hasSymbol( "Chiasmus" ) ) {
    if ( reason )
      *reason = i18n( "Failed to load %1: %2" )
                .arg( libfile, i18n( "Library does not contain the symbol \"Chiasmus\"." ) );
    kdDebug(5150) << "ChiasmusLibrary: loading \"" << libfile
                  << "\" failed: " << "Library does not contain the symbol \"Chiasmus\"." << endl;
    return 0;
  }
  void * symbol = mXiaLibrary->symbol( "Chiasmus" );
  assert( symbol );
  return ( main_func )symbol;
}

namespace {
  class ArgvProvider {
    char ** mArgv;
    int mArgc;
  public:
    ArgvProvider( const QValueVector<QCString> & args ) {
      mArgv = new char * [args.size()];
      for ( unsigned int i = 0 ; i < args.size() ; ++i )
        mArgv[i] = strdup( args[i].data() );
    }
    ~ArgvProvider() {
      std::for_each( mArgv, mArgv + mArgc, std::free );
      delete[] mArgv;
    }
    char ** argv() const { return mArgv; }
  };
}

int Kleo::ChiasmusLibrary::perform( const QValueVector<QCString> & args ) const {
  if ( main_func func = chiasmus() )
    return func( args.size(), ArgvProvider( args ).argv() );
  else
    return -1;
}
