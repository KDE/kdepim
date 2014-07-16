/* -*- mode: c++; c-basic-offset:4 -*-
    models/keylistmodel.cpp

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

#include "keylistmodel.h"
#include "predicates.h"

#ifdef KLEO_MODEL_TEST
# include "modeltest.h"
#endif

#include <utils/formatting.h>

#include <kleo/keyfiltermanager.h>
#include <kleo/keyfilter.h>

#include <KLocalizedString>

#include <QFont>
#include <QColor>
#include <QHash>
#include <QIcon>
#include <QDate>
#include <gpgme++/key.h>


#ifndef Q_MOC_RUN // QTBUG-22829
#include <boost/bind.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/adjacency_list.hpp>
#endif

#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <cassert>

#ifdef __GLIBCXX__
#include <ext/algorithm> // for is_sorted
#endif

using namespace GpgME;
using namespace Kleo;

/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

/*
    These functions are based on Peter J. Weinberger's hash function
    (from the Dragon Book). The constant 24 in the original function
    was replaced with 23 to produce fewer collisions on input such as
    "a", "aa", "aaa", "aaaa", ...
*/

// adjustment to null-terminated strings
// (c) 2008 Klarälvdalens Datakonsult AB
static uint hash(const uchar *p)
{
    uint h = 0;
    uint g;

    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}

//
// end Nokia-copyrighted code
//

static inline uint qHash( const char * data ) {
    if ( !data )
        return 1; // something != 0
    return ::hash( reinterpret_cast<const uchar*>( data ) );
}

class AbstractKeyListModel::Private {
public:
    Private() : m_toolTipOptions( Formatting::Validity ) {}
    int m_toolTipOptions;
    mutable QHash<const char*,QVariant> prettyEMailCache;
};
AbstractKeyListModel::AbstractKeyListModel( QObject * p )
    : QAbstractItemModel( p ), KeyListModelInterface(), d( new Private )
{

}

AbstractKeyListModel::~AbstractKeyListModel() {}

void AbstractKeyListModel::setToolTipOptions( int opts )
{
    d->m_toolTipOptions = opts;
}

int AbstractKeyListModel::toolTipOptions() const
{
    return d->m_toolTipOptions;
}

Key AbstractKeyListModel::key( const QModelIndex & idx ) const {
    if ( idx.isValid() )
        return doMapToKey( idx );
    else
        return Key::null;
}

std::vector<Key> AbstractKeyListModel::keys( const QList<QModelIndex> & indexes ) const {
    std::vector<Key> result;
    result.reserve( indexes.size() );
    std::transform( indexes.begin(), indexes.end(),
                    std::back_inserter( result ),
                    boost::bind( &AbstractKeyListModel::key, this, _1 ) );
    result.erase( std::unique( result.begin(), result.end(), _detail::ByFingerprint<std::equal_to>() ), result.end() );
    return result;
}

QModelIndex AbstractKeyListModel::index( const Key & key, int col ) const {
    if ( key.isNull() || col < 0 || col >= NumColumns )
        return QModelIndex();
    else
        return doMapFromKey( key, col );
}

QList<QModelIndex> AbstractKeyListModel::indexes( const std::vector<Key> & keys ) const {
    QList<QModelIndex> result;
    std::transform( keys.begin(), keys.end(),
                    std::back_inserter( result ),
                    // if some compilers are complaining about ambigious overloads, use this line instead:
                    //bind( static_cast<QModelIndex(AbstractKeyListModel::*)(const Key&,int)const>( &AbstractKeyListModel::index ), this, _1, 0 ) );
                    boost::bind( &AbstractKeyListModel::index, this, _1, 0 ) );
    return result;
}

void AbstractKeyListModel::setKeys( const std::vector<Key> & keys ) {
    clear();
    addKeys( keys );
}

QModelIndex AbstractKeyListModel::addKey( const Key & key ) {
    const std::vector<Key> vec( 1, key );
    const QList<QModelIndex> l = doAddKeys( vec );
    return l.empty() ? QModelIndex() : l.front() ;
}

void AbstractKeyListModel::removeKey( const Key & key ) {
    if ( key.isNull() )
        return;
    doRemoveKey( key );
    d->prettyEMailCache.remove( key.primaryFingerprint() );
}

QList<QModelIndex> AbstractKeyListModel::addKeys( const std::vector<Key> & keys ) {
    std::vector<Key> sorted;
    sorted.reserve( keys.size() );
    std::remove_copy_if( keys.begin(), keys.end(),
                         std::back_inserter( sorted ),
                         boost::bind( &Key::isNull, _1 ) );
    std::sort( sorted.begin(), sorted.end(), _detail::ByFingerprint<std::less>() );
    return doAddKeys( sorted );
}

void AbstractKeyListModel::clear() {
    doClear();
    d->prettyEMailCache.clear();
    reset();
}

int AbstractKeyListModel::columnCount( const QModelIndex & ) const {
    return NumColumns;
}

QVariant AbstractKeyListModel::headerData( int section, Qt::Orientation o, int role ) const {
    if ( o == Qt::Horizontal )
        if ( role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole )
            switch ( section ) {
            case PrettyName:       return i18n( "Name" );
#ifndef KDEPIM_MOBILE_UI
            case PrettyEMail:      return i18n( "E-Mail" );
            case ValidFrom:        return i18n( "Valid From" );
            case ValidUntil:       return i18n( "Valid Until" );
            case TechnicalDetails: return i18n( "Details" );
            case ShortKeyID:       return i18n( "Key-ID" );
#endif
            case NumColumns:       ;
            }
    return QVariant();
}

static QVariant returnIfValid( const QColor & t ) {
    if ( t.isValid() )
        return t;
    else
        return QVariant();
}

static QVariant returnIfValid( const QIcon & t ) {
    if ( !t.isNull() )
        return t;
    else
        return QVariant();
}

QVariant AbstractKeyListModel::data( const QModelIndex & index, int role ) const {
    const Key key = this->key( index );
    if ( key.isNull() )
        return QVariant();

    const int column = index.column();

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
        switch ( column ) {
        case PrettyName:
#ifdef KDEPIM_MOBILE_UI
            return Formatting::formatForComboBox( key );
#else
            return Formatting::prettyName( key );
        case PrettyEMail:
            if ( const char * const fpr = key.primaryFingerprint() ) {
                const QHash<const char*,QVariant>::const_iterator it = d->prettyEMailCache.constFind( fpr );
                if ( it != d->prettyEMailCache.constEnd() )
                    return *it;
                else
                    return d->prettyEMailCache[fpr] = Formatting::prettyEMail( key );
            } else {
                return QVariant();
            }
        case ValidFrom:
            if ( role == Qt::EditRole )
                return Formatting::creationDate( key );
            else
                return Formatting::creationDateString( key );
        case ValidUntil:
            if ( role == Qt::EditRole )
                return Formatting::expirationDate( key );
            else
                return Formatting::expirationDateString( key );
        case TechnicalDetails:
            return Formatting::type( key );
        case ShortKeyID:
            return QString::fromLatin1( key.shortKeyID() );
#endif // KDEPIM_MOBILE_UI
        case NumColumns:
            break;
        }
#ifndef KDEPIM_MOBILE_UI
    else if ( role == Qt::ToolTipRole )
        return Formatting::toolTip( key, toolTipOptions() );
    else if ( role == Qt::FontRole )
        return KeyFilterManager::instance()->font( key, ( column == ShortKeyID ) ? QFont( QLatin1String("courier") ) : QFont() );
#endif
    else if ( role == Qt::DecorationRole )
        return column == Icon ? returnIfValid( KeyFilterManager::instance()->icon( key ) ) : QVariant() ;
    else if ( role == Qt::BackgroundRole )
        return returnIfValid( KeyFilterManager::instance()->bgColor( key ) );
    else if ( role == Qt::ForegroundRole )
        return returnIfValid( KeyFilterManager::instance()->fgColor( key ) );
    else if ( role == FingerprintRole )
        return QString::fromLatin1( key.primaryFingerprint() );
    return QVariant();
}


namespace {
    template <typename Base>
    class TableModelMixin : public Base {
    public:
        explicit TableModelMixin( QObject * p=0 ) : Base( p ) {}
        ~TableModelMixin() {}

        using Base::index;
        /* reimp */ QModelIndex index( int row, int column, const QModelIndex & pidx=QModelIndex() ) const {
            return this->hasIndex( row, column, pidx ) ? this->createIndex( row, column, 0 ) : QModelIndex() ;
        }

    private:
        /* reimp */ QModelIndex parent( const QModelIndex & ) const { return QModelIndex(); }
        /* reimp */ bool hasChildren( const QModelIndex & pidx ) const {
            return ( pidx.model() == this || !pidx.isValid() ) && this->rowCount( pidx ) > 0 && this->columnCount( pidx ) > 0 ;
        }
    };

    class FlatKeyListModel
#ifndef Q_MOC_RUN
        : public TableModelMixin<AbstractKeyListModel>
#else
        : public AbstractKeyListModel
#endif
    {
        Q_OBJECT
    public:
        explicit FlatKeyListModel( QObject * parent=0 );
        ~FlatKeyListModel();

        /* reimp */ int rowCount( const QModelIndex & pidx ) const { return pidx.isValid() ? 0 : mKeysByFingerprint.size() ; }

    private:
        /* reimp */ Key doMapToKey( const QModelIndex & index ) const;
        /* reimp */ QModelIndex doMapFromKey( const Key & key, int col ) const;
        /* reimp */ QList<QModelIndex> doAddKeys( const std::vector<Key> & keys );
        /* reimp */ void doRemoveKey( const Key & key );
        /* reimp */ void doClear() {
            mKeysByFingerprint.clear();
        }

    private:
        std::vector<Key> mKeysByFingerprint;
    };

    class HierarchicalKeyListModel : public AbstractKeyListModel {
        Q_OBJECT
    public:
        explicit HierarchicalKeyListModel( QObject * parent=0 );
        ~HierarchicalKeyListModel();

        /* reimp */ int rowCount( const QModelIndex & pidx ) const;
        using AbstractKeyListModel::index;
        /* reimp */ QModelIndex index( int row, int col, const QModelIndex & pidx ) const;
        /* reimp */ QModelIndex parent( const QModelIndex & idx ) const;

        bool hasChildren( const QModelIndex & pidx ) const { return rowCount( pidx ) > 0 ; }

    private:
        /* reimp */ Key doMapToKey( const QModelIndex & index ) const;
        /* reimp */ QModelIndex doMapFromKey( const Key & key, int col ) const;
        /* reimp */ QList<QModelIndex> doAddKeys( const std::vector<Key> & keys );
        /* reimp */ void doRemoveKey( const Key & key );
        /* reimp */ void doClear() {
            mTopLevels.clear();
            mKeysByFingerprint.clear();
            mKeysByExistingParent.clear();
            mKeysByNonExistingParent.clear();
        }

    private:
        void addTopLevelKey( const Key & key );
        void addKeyWithParent( const char * issuer_fpr, const Key & key );
        void addKeyWithoutParent( const char * issuer_fpr, const Key & key );

    private:
        typedef std::map< std::string, std::vector<Key> > Map;
        std::vector<Key> mKeysByFingerprint; // all keys
        Map mKeysByExistingParent, mKeysByNonExistingParent; // parent->child map
        std::vector<Key> mTopLevels; // all roots + parent-less
    };

    static const char * cleanChainID( const Key & key ) {
        if ( key.isRoot() )
            return "";
        if ( const char * chid = key.chainID() )
            return chid;
        return "";
    }

}


FlatKeyListModel::FlatKeyListModel( QObject * p )
    : TableModelMixin<AbstractKeyListModel>( p ),
      mKeysByFingerprint()
{

}

FlatKeyListModel::~FlatKeyListModel() {}

Key FlatKeyListModel::doMapToKey( const QModelIndex & idx ) const {
    assert( idx.isValid() );
    if ( static_cast<unsigned>( idx.row() ) < mKeysByFingerprint.size() && idx.column() < NumColumns )
        return mKeysByFingerprint[ idx.row() ];
    else
        return Key::null;
}

QModelIndex FlatKeyListModel::doMapFromKey( const Key & key, int col ) const {
    assert( !key.isNull() );
    const std::vector<Key>::const_iterator it
        = std::lower_bound( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                       key, _detail::ByFingerprint<std::less>() );
    if ( it == mKeysByFingerprint.end() || !_detail::ByFingerprint<std::equal_to>()( *it, key ) )
        return QModelIndex();
    else
        return createIndex( it - mKeysByFingerprint.begin(), col );
}

QList<QModelIndex> FlatKeyListModel::doAddKeys( const std::vector<Key> & keys ) {
#ifdef __GLIBCXX__
    assert( __gnu_cxx::is_sorted( keys.begin(), keys.end(), _detail::ByFingerprint<std::less>() ) );
#endif
    if ( keys.empty() )
        return QList<QModelIndex>();

    for ( std::vector<Key>::const_iterator it = keys.begin(), end = keys.end() ; it != end ; ++it ) {

        // find an insertion point:
        const std::vector<Key>::iterator pos = std::upper_bound( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), *it, _detail::ByFingerprint<std::less>() );
        const unsigned int idx = std::distance( mKeysByFingerprint.begin(), pos );

        if ( idx > 0 && qstrcmp( mKeysByFingerprint[idx-1].primaryFingerprint(), it->primaryFingerprint() ) == 0 ) {
            // key existed before - replace with new one:
            mKeysByFingerprint[idx-1] = *it;
            emit dataChanged( createIndex( idx-1, 0 ), createIndex( idx-1, NumColumns-1 ) );
        } else {
            // new key - insert:
            beginInsertRows( QModelIndex(), idx, idx );
            mKeysByFingerprint.insert( pos, *it );
            endInsertRows();
        }
    }

    return indexes( keys );
}


void FlatKeyListModel::doRemoveKey( const Key & key ) {
    const std::vector<Key>::iterator it
        = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                       key, _detail::ByFingerprint<std::less>() );
    if ( it == mKeysByFingerprint.end() )
        return;

    const unsigned int row = std::distance( mKeysByFingerprint.begin(), it );
    beginRemoveRows( QModelIndex(), row, row );
    mKeysByFingerprint.erase( it );
    endRemoveRows();
}








HierarchicalKeyListModel::HierarchicalKeyListModel( QObject * p )
    : AbstractKeyListModel( p ),
      mKeysByFingerprint(),
      mKeysByExistingParent(),
      mKeysByNonExistingParent(),
      mTopLevels()
{

}

HierarchicalKeyListModel::~HierarchicalKeyListModel() {}

int HierarchicalKeyListModel::rowCount( const QModelIndex & pidx ) const {

    // toplevel item:
    if ( !pidx.isValid() )
        return mTopLevels.size();

    if ( pidx.column() != 0 )
        return 0;

    // non-toplevel item - find the number of subjects for this issuer:
    const Key issuer = this->key( pidx );
    const char * const fpr = issuer.primaryFingerprint();
    if ( !fpr || !*fpr )
        return 0;
    const Map::const_iterator it = mKeysByExistingParent.find( fpr );
    if ( it == mKeysByExistingParent.end() )
        return 0;
    return it->second.size();
}

QModelIndex HierarchicalKeyListModel::index( int row, int col, const QModelIndex & pidx ) const {

    if ( row < 0 || col < 0 || col >= NumColumns )
        return QModelIndex();

    // toplevel item:
    if ( !pidx.isValid() ) {
        if ( static_cast<unsigned>( row ) < mTopLevels.size() )
            return index( mTopLevels[row], col );
        else
            return QModelIndex();
    }

    // non-toplevel item - find the row'th subject of this key:
    const Key issuer = this->key( pidx );
    const char * const fpr = issuer.primaryFingerprint();
    if ( !fpr || !*fpr )
        return QModelIndex();
    const Map::const_iterator it = mKeysByExistingParent.find( fpr );
    if ( it == mKeysByExistingParent.end() || static_cast<unsigned>( row ) >= it->second.size() )
        return QModelIndex();
    return index( it->second[row], col );
}

QModelIndex HierarchicalKeyListModel::parent( const QModelIndex & idx ) const {
    const Key key = this->key( idx );
    if ( key.isNull() || key.isRoot() )
        return QModelIndex();
    const std::vector<Key>::const_iterator it
        = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                       cleanChainID( key ), _detail::ByFingerprint<std::less>() );
    return it != mKeysByFingerprint.end() ? index( *it ) : QModelIndex();
}

Key HierarchicalKeyListModel::doMapToKey( const QModelIndex & idx ) const {

    if ( !idx.isValid() )
        return Key::null;

    const char * const issuer_fpr = static_cast<const char*>( idx.internalPointer() );
    if ( !issuer_fpr || !*issuer_fpr ) {
        // top-level:
        if ( static_cast<unsigned>( idx.row() ) >= mTopLevels.size() )
            return Key::null;
        else
            return mTopLevels[idx.row()];
    }

    // non-toplevel:
    const Map::const_iterator it
        = mKeysByExistingParent.find( issuer_fpr );
    if ( it == mKeysByExistingParent.end() || static_cast<unsigned>( idx.row() ) >= it->second.size() )
        return Key::null;
    return it->second[idx.row()];
}

QModelIndex HierarchicalKeyListModel::doMapFromKey( const Key & key, int col ) const {

    if ( key.isNull() )
        return QModelIndex();

    const char * issuer_fpr = cleanChainID( key );

    // we need to look in the toplevels list,...
    const std::vector<Key> * v = &mTopLevels;
    if ( issuer_fpr && *issuer_fpr ) {
        const std::map< std::string, std::vector<Key> >::const_iterator it
            = mKeysByExistingParent.find( issuer_fpr );
        // ...unless we find an existing parent:
        if ( it != mKeysByExistingParent.end() )
            v = &it->second;
        else
            issuer_fpr = 0; // force internalPointer to zero for toplevels
    }

    const std::vector<Key>::const_iterator it
        = std::lower_bound( v->begin(), v->end(), key, _detail::ByFingerprint<std::less>() );
    if ( it == v->end() || !_detail::ByFingerprint<std::equal_to>()( *it, key ) )
        return QModelIndex();

    const unsigned int row = std::distance( v->begin(), it );
    return createIndex( row, col, const_cast<char* /* thanks, Trolls :/ */ >( issuer_fpr ) );
}

void HierarchicalKeyListModel::addKeyWithParent( const char * issuer_fpr, const Key & key ) {

    assert( issuer_fpr ); assert( *issuer_fpr ); assert( !key.isNull() );

    std::vector<Key> & subjects = mKeysByExistingParent[issuer_fpr];

    // find insertion point:
    const std::vector<Key>::iterator it = std::lower_bound( subjects.begin(), subjects.end(), key, _detail::ByFingerprint<std::less>() );
    const int row = std::distance( subjects.begin(), it );

    if ( it != subjects.end() && qstricmp( it->primaryFingerprint(), key.primaryFingerprint() ) == 0 ) {
        // exists -> replace
        *it = key;
        emit dataChanged( createIndex( row, 0, const_cast<char*>( issuer_fpr ) ), createIndex( row, NumColumns-1, const_cast<char*>( issuer_fpr ) ) );
    } else {
        // doesn't exist -> insert
        const std::vector<Key>::const_iterator pos = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), issuer_fpr, _detail::ByFingerprint<std::less>() );
        assert( pos != mKeysByFingerprint.end() );
        beginInsertRows( index( *pos ), row, row );
        subjects.insert( it, key );
        endInsertRows();
    }
}

void HierarchicalKeyListModel::addKeyWithoutParent( const char * issuer_fpr, const Key & key ) {

    assert( issuer_fpr ); assert( *issuer_fpr ); assert( !key.isNull() );

    std::vector<Key> & subjects = mKeysByNonExistingParent[issuer_fpr];

    // find insertion point:
    const std::vector<Key>::iterator it = std::lower_bound( subjects.begin(), subjects.end(), key, _detail::ByFingerprint<std::less>() );

    if ( it != subjects.end() && qstricmp( it->primaryFingerprint(), key.primaryFingerprint() ) == 0 )
        // exists -> replace
        *it = key;
    else
        // doesn't exist -> insert
        subjects.insert( it, key );

    addTopLevelKey( key );
}

void HierarchicalKeyListModel::addTopLevelKey( const Key & key ) {

    // find insertion point:
    const std::vector<Key>::iterator it = std::lower_bound( mTopLevels.begin(), mTopLevels.end(), key, _detail::ByFingerprint<std::less>() );
    const int row = std::distance( mTopLevels.begin(), it );

    if ( it != mTopLevels.end() && qstricmp( it->primaryFingerprint(), key.primaryFingerprint() ) == 0 ) {
        // exists -> replace
        *it = key;
        emit dataChanged( createIndex( row, 0 ), createIndex( row, NumColumns-1 ) );
    } else {
        // doesn't exist -> insert
        beginInsertRows( QModelIndex(), row, row );
        mTopLevels.insert( it, key );
        endInsertRows();
    }

}

// sorts 'keys' such that parent always come before their children:
static std::vector<Key> topological_sort( const std::vector<Key> & keys ) {

    boost::adjacency_list<> graph( keys.size() );

    // add edges from children to parents:
    for ( unsigned int i = 0, end = keys.size() ; i != end ; ++i ) {
        const char * const issuer_fpr = cleanChainID( keys[i] );
        if ( !issuer_fpr || !*issuer_fpr )
            continue;
        const std::vector<Key>::const_iterator it
            = qBinaryFind( keys.begin(), keys.end(), issuer_fpr, _detail::ByFingerprint<std::less>() );
        if ( it == keys.end() )
            continue;
        add_edge( i, std::distance( keys.begin(), it ), graph );
    }

    std::vector<int> order;
    order.reserve( keys.size() );
    topological_sort( graph, std::back_inserter( order ) );

    assert( order.size() == keys.size() );

    std::vector<Key> result;
    result.reserve( keys.size() );
    Q_FOREACH( int i, order )
        result.push_back( keys[i] );
    return result;
}

QList<QModelIndex> HierarchicalKeyListModel::doAddKeys( const std::vector<Key> & keys ) {
#ifdef __GLIBCXX__
    assert( __gnu_cxx::is_sorted( keys.begin(), keys.end(), _detail::ByFingerprint<std::less>() ) );
#endif
    if ( keys.empty() )
        return QList<QModelIndex>();


    const std::vector<Key> oldKeys = mKeysByFingerprint;

    std::vector<Key> merged;
    merged.reserve( keys.size() + mKeysByFingerprint.size() );
    std::set_union( keys.begin(), keys.end(),
                    mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                    std::back_inserter( merged ), _detail::ByFingerprint<std::less>() );

    mKeysByFingerprint = merged;

    std::set<Key, _detail::ByFingerprint<std::less> > changedParents;

    Q_FOREACH( const Key & key, topological_sort( keys ) ) {

        // check to see whether this key is a parent for a previously parent-less group:
        const char * const fpr = key.primaryFingerprint();
        if ( !fpr || !*fpr )
            continue;

        const bool keyAlreadyExisted = qBinaryFind( oldKeys.begin(), oldKeys.end(), key, _detail::ByFingerprint<std::less>() ) != oldKeys.end();

        const Map::iterator it = mKeysByNonExistingParent.find( fpr );
        const std::vector<Key> children = it != mKeysByNonExistingParent.end() ? it->second : std::vector<Key>();
        if ( it != mKeysByNonExistingParent.end() )
            mKeysByNonExistingParent.erase( it );

        // Step 1: For new keys, remove children from toplevel:

        if ( !keyAlreadyExisted ) {
            std::vector<Key>::iterator last = mTopLevels.begin();
            std::vector<Key>::iterator lastFP = mKeysByFingerprint.begin();

            Q_FOREACH( const Key & k, children ) {
                last = qBinaryFind( last, mTopLevels.end(), k, _detail::ByFingerprint<std::less>() );
                assert( last != mTopLevels.end() );
                const int row = std::distance( mTopLevels.begin(), last );

                lastFP = qBinaryFind( lastFP, mKeysByFingerprint.end(), k, _detail::ByFingerprint<std::less>() );
                assert( lastFP != mKeysByFingerprint.end() );

                emit rowAboutToBeMoved( QModelIndex(), row );
                beginRemoveRows( QModelIndex(), row, row );
                last = mTopLevels.erase( last );
                lastFP = mKeysByFingerprint.erase( lastFP );
                endRemoveRows();
            }
        }
        // Step 2: add/update key

        const char * const issuer_fpr = cleanChainID( key );
        if ( !issuer_fpr || !*issuer_fpr )
            // root or something...
            addTopLevelKey( key );
        else if ( std::binary_search( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), issuer_fpr, _detail::ByFingerprint<std::less>() ) )
            // parent exists...
            addKeyWithParent( issuer_fpr, key );
        else
            // parent does't exist yet...
            addKeyWithoutParent( issuer_fpr, key );

        const QModelIndex key_idx = index( key );
        QModelIndex key_parent = key_idx.parent();
        while ( key_parent.isValid() ) {
            changedParents.insert( doMapToKey( key_parent ) );
            key_parent = key_parent.parent();
        }

        // Step 3: Add children to new parent ( == key )

        if ( !keyAlreadyExisted && !children.empty() ) {
            addKeys( children );
            const QModelIndex new_parent = index( key );
            // emit the rowMoved() signals in reversed direction, so the
            // implementation can use a stack for mapping.
            for ( int i = children.size() - 1 ; i >= 0 ; --i )
                emit rowMoved( new_parent, i );
        }
    }
    //emit dataChanged for all parents with new children. This triggers KeyListSortFilterProxyModel to
    //show a parent node if it just got children matching the proxy's filter
    Q_FOREACH( const Key & i, changedParents ) {
        const QModelIndex idx = index( i );
        if ( idx.isValid() )
            emit dataChanged( idx.sibling( idx.row(), 0 ), idx.sibling( idx.row(), NumColumns - 1 ) );
    }
    return indexes( keys );
}

void HierarchicalKeyListModel::doRemoveKey( const Key & key ) {
    const QModelIndex idx = index( key );
    if ( !idx.isValid() )
        return;

    const char * const fpr = key.primaryFingerprint();
    if ( mKeysByExistingParent.find( fpr ) != mKeysByExistingParent.end() ) {
        //handle non-leave nodes:
        std::vector<Key> keys = mKeysByFingerprint;
        const std::vector<Key>::iterator it = qBinaryFind( keys.begin(), keys.end(),
                                                           key, _detail::ByFingerprint<std::less>() );
        if ( it == keys.end() )
            return;
        keys.erase( it );
        // FIXME for simplicity, we just clear the model and re-add all keys minus the removed one. This is suboptimal,
        // but acceptable given that deletion of non-leave nodes is rather rare.
        clear();
        addKeys( keys );
        return;
    }

    //handle leave nodes:

    const std::vector<Key>::iterator it = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                                                       key, _detail::ByFingerprint<std::less>() );

    assert( it != mKeysByFingerprint.end() );
    assert( mKeysByNonExistingParent.find( fpr ) == mKeysByNonExistingParent.end() );
    assert( mKeysByExistingParent.find( fpr ) == mKeysByExistingParent.end() );

    beginRemoveRows( parent( idx ), idx.row(), idx.row() );
    mKeysByFingerprint.erase( it );

    const char * const issuer_fpr = cleanChainID( key );

    const std::vector<Key>::iterator tlIt = qBinaryFind( mTopLevels.begin(), mTopLevels.end(), key, _detail::ByFingerprint<std::less>() );
    if( tlIt != mTopLevels.end() )
        mTopLevels.erase( tlIt );

    if ( issuer_fpr && *issuer_fpr ) {
        const Map::iterator nexIt = mKeysByNonExistingParent.find( issuer_fpr );
        if ( nexIt != mKeysByNonExistingParent.end() ) {
            const std::vector<Key>::iterator eit = qBinaryFind( nexIt->second.begin(), nexIt->second.end(), key, _detail::ByFingerprint<std::less>() );
            if ( eit != nexIt->second.end() )
                nexIt->second.erase( eit );
            if ( nexIt->second.empty() )
                mKeysByNonExistingParent.erase( nexIt );
        }

        const Map::iterator exIt = mKeysByExistingParent.find( issuer_fpr );
        if ( exIt != mKeysByExistingParent.end() ) {
            const std::vector<Key>::iterator eit = qBinaryFind( exIt->second.begin(), exIt->second.end(), key, _detail::ByFingerprint<std::less>() );
            if ( eit != exIt->second.end() )
                exIt->second.erase( eit );
            if ( exIt->second.empty() )
                mKeysByExistingParent.erase( exIt );
        }
    }
    endRemoveRows();
}

// static
AbstractKeyListModel * AbstractKeyListModel::createFlatKeyListModel( QObject * p ) {
    AbstractKeyListModel * const m = new FlatKeyListModel( p );
#ifdef KLEO_MODEL_TEST
    new ModelTest( m, p );
#endif
    return m;
}

// static
AbstractKeyListModel * AbstractKeyListModel::createHierarchicalKeyListModel( QObject * p ) {
    AbstractKeyListModel * const m = new HierarchicalKeyListModel( p );
#ifdef KLEO_MODEL_TEST
    new ModelTest( m, p );
#endif
    return m;
}

#include "keylistmodel.moc"

/*!
  \fn AbstractKeyListModel::rowAboutToBeMoved( const QModelIndex & old_parent, int old_row )

  Emitted before the removal of a row from that model. It will later
  be added to the model again, in response to which rowMoved() will be
  emitted. If multiple rows are moved in one go, multiple
  rowAboutToBeMoved() signals are emitted before the corresponding
  number of rowMoved() signals is emitted - in reverse order.

  This works around the absence of move semantics in
  QAbstractItemModel. Clients can maintain a stack to perform the
  QModelIndex-mapping themselves, or, e.g., to preserve the selection
  status of the row:

  \code
  std::vector<bool> mMovingRowWasSelected; // transient, used when rows are moved
  // ...
  void slotRowAboutToBeMoved( const QModelIndex & p, int row ) {
      mMovingRowWasSelected.push_back( selectionModel()->isSelected( model()->index( row, 0, p ) ) );
  }
  void slotRowMoved( const QModelIndex & p, int row ) {
      const bool wasSelected = mMovingRowWasSelected.back();
      mMovingRowWasSelected.pop_back();
      if ( wasSelected )
          selectionModel()->select( model()->index( row, 0, p ), Select|Rows );
  }
  \endcode

  A similar mechanism could be used to preserve the current item during moves.
*/

/*!
  \fn AbstractKeyListModel::rowMoved( const QModelIndex & new_parent, int new_parent )

  See rowAboutToBeMoved()
*/
