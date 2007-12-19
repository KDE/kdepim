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

#include "keyfiltermanager.h"
#include "kconfigbasedkeyfilter.h"

#include "cryptobackendfactory.h"

#include <kconfig.h>
#include <kconfiggroup.h>

#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QAbstractListModel>
#include <QModelIndex>

#include <boost/bind.hpp>

#include <algorithm>
#include <vector>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

static std::vector< shared_ptr<KeyFilter> > defaultFilters() {
    std::vector<shared_ptr<KeyFilter> > result;
    
    return result;
}


namespace {

    class Model : public QAbstractListModel {
        KeyFilterManager::Private * m_keyFilterManagerPrivate;
    public:
        explicit Model( KeyFilterManager::Private * p )
            : QAbstractListModel( 0 ), m_keyFilterManagerPrivate( p ) {}

        /* reimp */ int rowCount( const QModelIndex & ) const;
        /* reimp */ QVariant data( const QModelIndex & idx, int role ) const;
        /* upgrade to public */ using QAbstractListModel::reset;
    };
}

class KeyFilterManager::Private {
public:
    Private() : filters(), model( this ) {}
    void clear() {
        filters.clear();
        model.reset();
    }

    std::vector< shared_ptr<KeyFilter> > filters;
    Model model;
};


KeyFilterManager * KeyFilterManager::mSelf = 0;

KeyFilterManager::KeyFilterManager( QObject * parent )
    : QObject( parent ), d( new Private )
{
  mSelf = this;
  // ### DF: doesn't a KStaticDeleter work more reliably?
  if ( QCoreApplication * app = QCoreApplication::instance() )
    connect( app, SIGNAL(aboutToQuit()), SLOT(deleteLater()) );
  reload();
}

KeyFilterManager::~KeyFilterManager() {
  mSelf = 0;
  if ( d )
    d->clear();
  delete d; d = 0;
}

KeyFilterManager * KeyFilterManager::instance() {
  if ( !mSelf )
    mSelf = new KeyFilterManager();
  return mSelf;
}

const KeyFilter * KeyFilterManager::filterMatching( const Key & key ) const {
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = std::find_if( d->filters.begin(), d->filters.end(),
                        bind( &KeyFilter::matches, _1, cref( key ) ) );
    return it == d->filters.end() ? 0 : it->get();
}

namespace {
    struct BySpecificity : std::binary_function<shared_ptr<KeyFilter>,shared_ptr<KeyFilter>,bool> {
        bool operator()( const shared_ptr<KeyFilter> & lhs, const shared_ptr<KeyFilter> & rhs ) const {
            return lhs->specificity() > rhs->specificity();
        }
    };
}

void KeyFilterManager::reload() {
  d->clear();

  KConfig * config = CryptoBackendFactory::instance()->configObject();
  if ( !config )
    return;
  const QStringList groups = config->groupList().filter( QRegExp( "^Key Filter #\\d+$" ) );
  for ( QStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it ) {
    const KConfigGroup cfg( config, *it );
    d->filters.push_back( shared_ptr<KeyFilter>( new KConfigBasedKeyFilter( cfg ) ) );
  }
  qDebug( "KeyFilterManager::reload: loaded %lu filters", (unsigned long)d->filters.size() );
  if ( d->filters.empty() )
      d->filters = defaultFilters();
  std::stable_sort( d->filters.begin(), d->filters.end(), BySpecificity() );
  qDebug( "KeyFilterManager::reload: final filter count is %lu", (unsigned long)d->filters.size() );
}

QAbstractItemModel * KeyFilterManager::model() const {
    return &d->model;
}

const shared_ptr<KeyFilter> & KeyFilterManager::keyFilterByID( const QString & id ) const {
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = std::find_if( d->filters.begin(), d->filters.end(),
                        bind( &KeyFilter::id, _1 ) == id );
    if ( it != d->filters.end() )
        return *it;
    static const shared_ptr<KeyFilter> empty;
    return empty;
}

const shared_ptr<KeyFilter> & KeyFilterManager::fromModelIndex( const QModelIndex & idx ) const {
    if ( !idx.isValid() || idx.model() != &d->model || idx.row() < 0 ||
         static_cast<unsigned>(idx.row()) >= d->filters.size() ) {
        static const shared_ptr<KeyFilter> empty;
        return empty;
    }
    return d->filters[idx.row()];
}

QModelIndex KeyFilterManager::toModelIndex( const shared_ptr<KeyFilter> & kf ) const {
    if ( !kf )
        return QModelIndex();
    const std::pair<
      std::vector<shared_ptr<KeyFilter> >::const_iterator,
      std::vector<shared_ptr<KeyFilter> >::const_iterator
    > pair = std::equal_range( d->filters.begin(), d->filters.end(), kf, BySpecificity() );
    const std::vector<shared_ptr<KeyFilter> >::const_iterator it
        = std::find( pair.first, pair.second, kf );
    if ( it != pair.second )
        return d->model.index( it - d->filters.begin() );
    else
        return QModelIndex();
}

int Model::rowCount( const QModelIndex & ) const {
    return m_keyFilterManagerPrivate->filters.size();
}

QVariant Model::data( const QModelIndex & idx, int role ) const {
    if ( role != Qt::DisplayRole && role != Qt::EditRole &&
         role != Qt::ToolTipRole && role != Qt::DecorationRole ||
         !idx.isValid() || idx.model() != this ||
         idx.row() < 0 || static_cast<unsigned>(idx.row()) >  m_keyFilterManagerPrivate->filters.size() )
        return QVariant();
    if ( role == Qt::DecorationRole )
        return m_keyFilterManagerPrivate->filters[idx.row()]->icon();
    else
        return m_keyFilterManagerPrivate->filters[idx.row()]->name();
}

#include "keyfiltermanager.moc"
