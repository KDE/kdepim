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

#include "keylistmodel.h"

#include <utils/formatting.h>

#include <kleo/keyfiltermanager.h>
#include <kleo/keyfilter.h>

#include <KLocale>

#include <QDateTime>
#include <QIcon>
#include <QFont>
#include <QColor>
#include <QApplication>

#include <gpgme++/key.h>

#include <boost/bind.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include <algorithm>
#include <vector>
#include <map>
#include <iterator>
#include <cassert>

#ifdef __GNUC__
#include <ext/algorithm> // for is_sorted
#endif

using namespace GpgME;
using namespace Kleo;
using namespace boost;

namespace {
    template <template <typename T> class Op>
    struct ByFingerprint {
        typedef bool result_type;

        bool operator()( const Key & lhs, const Key & rhs ) const {
            return Op<int>()( qstricmp( lhs.primaryFingerprint(), rhs.primaryFingerprint() ), 0 );
        }
        bool operator()( const Key & lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs.primaryFingerprint(), rhs ), 0 );
        }
        bool operator()( const char * lhs, const Key & rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs.primaryFingerprint() ), 0 );
        }
        bool operator()( const char * lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs ), 0 );
        }
    };

}

AbstractKeyListModel::AbstractKeyListModel( QObject * p )
    : QAbstractItemModel( p )
{

}

AbstractKeyListModel::~AbstractKeyListModel() {}


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
                    bind( &AbstractKeyListModel::key, this, _1 ) );
    result.erase( std::unique( result.begin(), result.end(), ByFingerprint<std::equal_to>() ), result.end() );
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
                    bind( &AbstractKeyListModel::index, this, _1, 0 ) );
    return result;
}

QModelIndex AbstractKeyListModel::addKey( const Key & key ) {
    const std::vector<Key> vec( 1, key );
    const QList<QModelIndex> l = doAddKeys( vec );
    return l.empty() ? QModelIndex() : l.front() ;
}

QList<QModelIndex> AbstractKeyListModel::addKeys( const std::vector<Key> & keys ) {
    std::vector<Key> sorted;
    sorted.reserve( keys.size() );
    std::remove_copy_if( keys.begin(), keys.end(),
			 std::back_inserter( sorted ),
			 bind( &Key::isNull, _1 ) );
    std::sort( sorted.begin(), sorted.end(), ByFingerprint<std::less>() );
    return doAddKeys( sorted );
}

void AbstractKeyListModel::clear() {
    doClear();
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
            case PrettyEMail:      return i18n( "E-Mail" );
            case ValidFrom:        return i18n( "Valid From" );
            case ValidUntil:       return i18n( "Valid Until" );
            case TechnicalDetails: return i18n( "Details" );
            case Fingerprint:      return i18n( "Fingerprint" );
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
            return Formatting::prettyName( key );
        case PrettyEMail:
            return Formatting::prettyEMail( key );
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
        case Fingerprint:
            return QString::fromLatin1( key.primaryFingerprint() );
        case NumColumns:
            break;
        }
    else if ( role == Qt::ToolTipRole )
        return Formatting::toolTip( key );
    else if ( role == Qt::FontRole ) {
        QFont font = qApp->font(); // ### correct font?
        if ( column == Fingerprint )
            font.setFamily( "courier" );
        if ( const shared_ptr<KeyFilter> & filter = KeyFilterManager::instance()->filterMatching( key, KeyFilter::Appearance ) )
            return filter->font( font );
        else
            return font;
    } else if ( role == Qt::DecorationRole || role == Qt::BackgroundRole || role == Qt::ForegroundRole ) {
        if ( const shared_ptr<KeyFilter> & filter = KeyFilterManager::instance()->filterMatching( key, KeyFilter::Appearance ) ) {
            switch ( role ) {
            case Qt::DecorationRole: return column == Icon ? returnIfValid( QIcon( filter->icon() ) ) : QVariant() ;
            case Qt::BackgroundRole: return returnIfValid( filter->bgColor() );
            case Qt::ForegroundRole: return returnIfValid( filter->fgColor() );
            default: ; // silence compiler
            }
        }
    } else if ( role == Qt::TextAlignmentRole ) { // needed?
    }
    return QVariant();
}


namespace {
    template <typename Base>
    class TableModelMixin : public Base {
    public:
        explicit TableModelMixin( QObject * p=0 ) : Base( p ) {}
        ~TableModelMixin() {}

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
        /* reimp */ void doClear() {
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
        = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                       key, ByFingerprint<std::less>() );
    if ( it == mKeysByFingerprint.end() )
        return QModelIndex();
    else
        return createIndex( it - mKeysByFingerprint.begin(), col );
}

QList<QModelIndex> FlatKeyListModel::doAddKeys( const std::vector<Key> & keys ) {
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( keys.begin(), keys.end(), ByFingerprint<std::less>() ) );
#endif
    if ( keys.empty() )
        return QList<QModelIndex>();

    for ( std::vector<Key>::const_iterator it = keys.begin(), end = keys.end() ; it != end ; ++it ) {

	// find an insertion point:
        const std::vector<Key>::iterator pos = std::upper_bound( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), *it, ByFingerprint<std::less>() );
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
    if ( !pidx.isValid() )
	if ( static_cast<unsigned>( row ) < mTopLevels.size() )
	    return index( mTopLevels[row], col );
	else
	    return QModelIndex();

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
		       cleanChainID( key ), ByFingerprint<std::less>() );
    if ( it == mKeysByFingerprint.end() )
	return QModelIndex();
    else
	return index( *it );
}

Key HierarchicalKeyListModel::doMapToKey( const QModelIndex & idx ) const {

    if ( !idx.isValid() )
	return Key::null;

    const char * const issuer_fpr = static_cast<const char*>( idx.internalPointer() );
    if ( !issuer_fpr || !*issuer_fpr )
	// top-level:
	if ( static_cast<unsigned>( idx.row() ) >= mTopLevels.size() )
	    return Key::null;
	else
	    return mTopLevels[idx.row()];

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
	= qBinaryFind( v->begin(), v->end(), key, ByFingerprint<std::less>() );
    if ( it == v->end() )
	return QModelIndex();

    const unsigned int row = std::distance( v->begin(), it );
    return createIndex( row, col, const_cast<char* /* thanks, Trolls :/ */ >( issuer_fpr ) ); 
}

void HierarchicalKeyListModel::addKeyWithParent( const char * issuer_fpr, const Key & key ) {

    assert( issuer_fpr ); assert( *issuer_fpr ); assert( !key.isNull() );

    std::vector<Key> & subjects = mKeysByExistingParent[issuer_fpr];

    // find insertion point:
    const std::vector<Key>::iterator it = std::lower_bound( subjects.begin(), subjects.end(), key, ByFingerprint<std::less>() );
    const int row = std::distance( subjects.begin(), it );

    if ( it != subjects.end() && qstricmp( it->primaryFingerprint(), key.primaryFingerprint() ) == 0 ) {
	// exists -> replace
	*it = key;
	emit dataChanged( createIndex( row, 0, const_cast<char*>( issuer_fpr ) ), createIndex( row, NumColumns-1, const_cast<char*>( issuer_fpr ) ) );
    } else {
	// doesn't exist -> insert
	const std::vector<Key>::const_iterator pos = qBinaryFind( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), issuer_fpr, ByFingerprint<std::less>() );
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
    const std::vector<Key>::iterator it = std::lower_bound( subjects.begin(), subjects.end(), key, ByFingerprint<std::less>() );

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
    const std::vector<Key>::iterator it = std::lower_bound( mTopLevels.begin(), mTopLevels.end(), key, ByFingerprint<std::less>() );
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

namespace {

    // sorts 'keys' such that parent always come before their children:
    std::vector<Key> topological_sort( const std::vector<Key> & keys ) {

        adjacency_list<> graph( keys.size() );

        // add edges from children to parents:
        for ( unsigned int i = 0, end = keys.size() ; i != end ; ++i ) {
            const char * const issuer_fpr = cleanChainID( keys[i] );
            if ( !issuer_fpr || !*issuer_fpr )
                continue;
            const std::vector<Key>::const_iterator it
                = qBinaryFind( keys.begin(), keys.end(), issuer_fpr, ByFingerprint<std::less>() );
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

}

QList<QModelIndex> HierarchicalKeyListModel::doAddKeys( const std::vector<Key> & keys ) {
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( keys.begin(), keys.end(), ByFingerprint<std::less>() ) );
#endif
    if ( keys.empty() )
        return QList<QModelIndex>();


    std::vector<Key> merged;
    merged.reserve( keys.size() + mKeysByFingerprint.size() );
    std::set_union( keys.begin(), keys.end(),
                    mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                    std::back_inserter( merged ), ByFingerprint<std::less>() );
    
    mKeysByFingerprint = merged;

    Q_FOREACH( const Key & key, topological_sort( keys ) ) {
	
	const char * const issuer_fpr = cleanChainID( key );

	if ( !issuer_fpr || !*issuer_fpr )
	    // root or something...
	    addTopLevelKey( key );
	else if ( std::binary_search( mKeysByFingerprint.begin(), mKeysByFingerprint.end(), issuer_fpr, ByFingerprint<std::less>() ) )
	    // parent exists...
	    addKeyWithParent( issuer_fpr, key );
	else
	    // parent does't exist yet...
	    addKeyWithoutParent( issuer_fpr, key );

        // check to see whether this key is a parent for a previously parent-less group:
        const char * const fpr = key.primaryFingerprint();
        if ( !fpr || !*fpr )
            continue;

        const Map::iterator it = mKeysByNonExistingParent.find( fpr );
        if ( it == mKeysByNonExistingParent.end() )
            continue;
        assert( !it->second.empty() );

        const std::vector<Key> children = it->second;

        // Step 1: Remove children from toplevel:
        std::vector<Key>::iterator last = mTopLevels.begin();
        Q_FOREACH( const Key & k, children ) {
            last = qBinaryFind( last, mTopLevels.end(), k, ByFingerprint<std::less>() );
            assert( last != mTopLevels.end() );
            const int row = std::distance( mTopLevels.begin(), last );

            emit rowAboutToBeMoved( QModelIndex(), row );
            beginRemoveRows( QModelIndex(), row, row );
            last = mTopLevels.erase( last );
            endRemoveRows();
        }

	mKeysByNonExistingParent.erase( it );

        // Step 2: Add children to new parent ( == key )
        const QModelIndex new_parent = index( key );
        beginInsertRows( index( key ), 0, children.size()-1 );
        assert( mKeysByExistingParent.find( fpr ) == mKeysByExistingParent.end() );
        mKeysByExistingParent[fpr] = children;
        endInsertRows();

        // emit the rowMoved() signals in reversed direction, so the
        // implementation can use a stack for mapping.
        for ( int i = children.size() - 1 ; i >= 0 ; --i )
            emit rowMoved( new_parent, i );

    }

    return indexes( keys );
}


// static
AbstractKeyListModel * AbstractKeyListModel::createFlatKeyListModel( QObject * p ) {
    return new FlatKeyListModel( p );
}

// static
AbstractKeyListModel * AbstractKeyListModel::createHierarchicalKeyListModel( QObject * p ) {
    return new HierarchicalKeyListModel( p );
}

#include "moc_keylistmodel.cpp"
#include "keylistmodel.moc"

/*!
  \fn AbstractKeyListModel::rowAboutToBeMoved( const QModelIndex & old_parent, int old_row )

  Emitted before the removal of a row from that model. It will later
  be added to the model again, in reponse to which rowMoved() will be
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
