/*
    keyfiltermanager.cpp

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

#include "keyfiltermanager.h"
#include "kconfigbasedkeyfilter.h"

#include "cryptobackendfactory.h"

#include <kconfig.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include <algorithm>

namespace {
  template <typename T>
  struct Delete {
    void operator()( T * item ) { delete item; }
  };
}

struct Kleo::KeyFilterManager::Private {
  void clear() {
    std::for_each( filters.begin(), filters.end(), Delete<KeyFilter>() );
    filters.clear();
  }

  QValueVector<KeyFilter*> filters;
};

Kleo::KeyFilterManager * Kleo::KeyFilterManager::mSelf = 0;

Kleo::KeyFilterManager::KeyFilterManager( QObject * parent, const char * name )
  : QObject( parent, name ), d( 0 )
{
  mSelf = this;
  d = new Private();
  // ### DF: doesn't a KStaticDeleter work more reliably?
  if ( qApp )
    connect( qApp, SIGNAL(aboutToQuit()), SLOT(deleteLater()) );
  reload();
}

Kleo::KeyFilterManager::~KeyFilterManager() {
  mSelf = 0;
  if ( d )
    d->clear();
  delete d; d = 0;
}

Kleo::KeyFilterManager * Kleo::KeyFilterManager::instance() {
  if ( !mSelf )
    mSelf = new Kleo::KeyFilterManager();
  return mSelf;
}

const Kleo::KeyFilter * Kleo::KeyFilterManager::filterMatching( const GpgME::Key & key ) const {
  for ( QValueVector<KeyFilter*>::const_iterator it = d->filters.begin() ; it != d->filters.end() ; ++it )
    if ( (*it)->matches( key ) )
      return *it;
  return 0;
}

static inline bool by_increasing_specificity( const Kleo::KeyFilter * left, const Kleo::KeyFilter * right ) {
  return left->specificity() > right->specificity();
}

void Kleo::KeyFilterManager::reload() {
  d->clear();

  KConfig * config = Kleo::CryptoBackendFactory::instance()->configObject();
  if ( !config )
    return;
  const QStringList groups = config->groupList().grep( QRegExp( "^Key Filter #\\d+$" ) );
  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it ) {
    const KConfigGroup cfg( config, *it );
    d->filters.push_back( new KConfigBasedKeyFilter( cfg ) );
  }
  std::stable_sort( d->filters.begin(), d->filters.end(), by_increasing_specificity );
}

#include "keyfiltermanager.moc"
