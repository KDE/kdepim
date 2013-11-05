/* -*- mode: c++; c-basic-offset:4 -*-
    models/keylistsortfilterproxymodel.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#include <config-kleopatra.h>

#include "keylistsortfilterproxymodel.h"

#include "keylistmodel.h"

#include <kleo/keyfilter.h>

#include <gpgme++/key.h>

#include <kleo/stl_util.h>

#include <boost/bind.hpp>

#include <cassert>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

AbstractKeyListSortFilterProxyModel::AbstractKeyListSortFilterProxyModel( QObject * p )
    : QSortFilterProxyModel( p ), KeyListModelInterface()
{
    init();
}

AbstractKeyListSortFilterProxyModel::AbstractKeyListSortFilterProxyModel( const AbstractKeyListSortFilterProxyModel & other )
    : QSortFilterProxyModel(), KeyListModelInterface()
{
    Q_UNUSED( other );
    init();
}

void AbstractKeyListSortFilterProxyModel::init() {
    setDynamicSortFilter( true );
    setSortRole( Qt::EditRole );  // EditRole can be expected to be in a less formatted way, better for sorting
    setFilterRole( Qt::DisplayRole );
    setFilterCaseSensitivity( Qt::CaseInsensitive );
}

AbstractKeyListSortFilterProxyModel::~AbstractKeyListSortFilterProxyModel() {}

Key AbstractKeyListSortFilterProxyModel::key( const QModelIndex & idx ) const {
    const KeyListModelInterface * const klmi = dynamic_cast<KeyListModelInterface*>( sourceModel() );
    if ( !klmi ) {
        static Key null;
        return null;
    }
    return klmi->key( mapToSource( idx ) );
}

std::vector<Key> AbstractKeyListSortFilterProxyModel::keys( const QList<QModelIndex> & indexes ) const {
    const KeyListModelInterface * const klmi = dynamic_cast<KeyListModelInterface*>( sourceModel() );
    if ( !klmi )
        return std::vector<Key>();
    QList<QModelIndex> mapped;
    std::transform( indexes.begin(), indexes.end(),
                    std::back_inserter( mapped ),
                    boost::bind( &QAbstractProxyModel::mapToSource, this, _1 ) );
    return klmi->keys( mapped );
}

QModelIndex AbstractKeyListSortFilterProxyModel::index( const Key & key ) const {
    if ( const KeyListModelInterface * const klmi = dynamic_cast<KeyListModelInterface*>( sourceModel() ) )
        return mapFromSource( klmi->index( key ) );
    else
        return QModelIndex();
}

QList<QModelIndex> AbstractKeyListSortFilterProxyModel::indexes( const std::vector<Key> & keys ) const {
    if ( const KeyListModelInterface * const klmi = dynamic_cast<KeyListModelInterface*>( sourceModel() ) ) {
        const QList<QModelIndex> source = klmi->indexes( keys );
        QList<QModelIndex> mapped;
        std::transform( source.begin(), source.end(),
                        std::back_inserter( mapped ),
                        boost::bind( &QAbstractProxyModel::mapFromSource, this, _1 ) );
        return mapped;
    } else {
        return QList<QModelIndex>();
    }
}


class KeyListSortFilterProxyModel::Private {
    friend class ::Kleo::KeyListSortFilterProxyModel;
public:
    explicit Private()
        : keyFilter() {}
    ~Private() {}

private:
    shared_ptr<const KeyFilter> keyFilter;
};


KeyListSortFilterProxyModel::KeyListSortFilterProxyModel( QObject * p )
    : AbstractKeyListSortFilterProxyModel( p ), d( new Private )
{

}

KeyListSortFilterProxyModel::KeyListSortFilterProxyModel( const KeyListSortFilterProxyModel & other )
    : AbstractKeyListSortFilterProxyModel( other ), d( new Private( *other.d ) )
{

}

KeyListSortFilterProxyModel::~KeyListSortFilterProxyModel() {}

KeyListSortFilterProxyModel * KeyListSortFilterProxyModel::clone() const {
    return new KeyListSortFilterProxyModel( *this );
}

shared_ptr<const KeyFilter> KeyListSortFilterProxyModel::keyFilter() const {
    return d->keyFilter;
}

void KeyListSortFilterProxyModel::setKeyFilter( const shared_ptr<const KeyFilter> & kf ) {
    if ( kf == d->keyFilter )
        return;
    d->keyFilter = kf;
    invalidateFilter();
}

bool KeyListSortFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const {

    //
    // 0. Keep parents of matching children:
    //
    const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
    for ( int i = 0, end = sourceModel()->rowCount( index ) ; i != end ; ++i )
        if ( filterAcceptsRow( i, index ) )
            return true;

    //
    // 1. Check that name or email matches filterRegExp
    //
    const int role = filterRole();

    const QModelIndex nameIndex = sourceModel()->index( source_row, PrettyName, source_parent );
    const QString name = nameIndex.data( role ).toString();

#ifndef KDEPIM_MOBILE_UI
    const QModelIndex emailIndex = sourceModel()->index( source_row, PrettyEMail, source_parent );
    const QString email = emailIndex.data( role ).toString();
#endif

    const QRegExp rx = filterRegExp();
    if ( !name.contains( rx ) )
#ifndef KDEPIM_MOBILE_UI
        if ( !email.contains( rx ) )
#endif
            return false;

    //
    // 2. Check that key filters match (if any are defined)
    //
    if ( d->keyFilter ) { // avoid artifacts when no filters are defined

        const KeyListModelInterface * const klm = dynamic_cast<KeyListModelInterface*>( sourceModel() );
        assert( klm );
        const Key key = klm->key( nameIndex );

        return d->keyFilter->matches( key, KeyFilter::Filtering );
    }

    // 3. match by default:
    return true;
}

