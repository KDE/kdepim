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

#include <kleo/keyfiltermanager.h>
#include <kleo/keyfilter.h>
#include <kleo/dn.h>

#include <QDateTime>
#include <QIcon>
#include <QFont>
#include <QColor>
#include <QApplication>

#include <gpgme++/key.h>

#include <boost/bind.hpp>

#include <algorithm>
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
    return result;
}

QModelIndex AbstractKeyListModel::index( const Key & key ) const {
    if ( key.isNull() )
        return QModelIndex();
    else
        return doMapFromKey( key, 0 );
}

QList<QModelIndex> AbstractKeyListModel::indexes( const std::vector<Key> & keys ) const {
    QList<QModelIndex> result;
    std::transform( keys.begin(), keys.end(),
                    std::back_inserter( result ),
                    // if some compilers are complaining about ambigious overloads, use this line instead:
                    //bind( static_cast<QModelIndex(AbstractKeyListModel::*)(const Key&)const>( &AbstractKeyListModel::index ), this, _1 ) );
                    bind( &AbstractKeyListModel::index, this, _1 ) );
    return result;
}

QModelIndex AbstractKeyListModel::addKey( const Key & key ) {
    const std::vector<Key> vec( 1, key );
    const QList<QModelIndex> l = doAddKeys( vec );
    return l.empty() ? QModelIndex() : l.front() ;
}

QList<QModelIndex> AbstractKeyListModel::addKeys( const std::vector<Key> & keys ) {
    std::vector<Key> sorted = keys;
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
            case PrettyName:       return tr( "Name" );
            case PrettyEMail:      return tr( "E-Mail" );
            case ValidFrom:        return tr( "Valid From" );
            case ValidUntil:       return tr( "Valid Until" );
            case TechnicalDetails: return tr( "Details" );
            case Fingerprint:      return tr( "Fingerprint" );
            case NumColumns:       ;
            }
    return QVariant();
}

QVariant AbstractKeyListModel::data( const QModelIndex & index, int role ) const {
    const Key key = this->key( index );
    if ( key.isNull() )
        return QVariant();

    const int column = index.column();

    if ( role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole )
        switch ( column ) {
        case PrettyName:
	    if ( key.protocol() == OpenPGP ) {
                const UserID uid = key.userID( 0 );
                const QString name = QString::fromUtf8( uid.name() );
                if ( name.isEmpty() )
                    return QString::fromLatin1( key.primaryFingerprint() );
                const QString comment = QString::fromUtf8( uid.comment() );
                if ( comment.isEmpty() )
                    return name;
                return QString::fromLatin1( "%1 (%2)" ).arg( name, comment );
	    } else if ( key.protocol() == CMS ) {
                const DN subject( key.userID( 0 ).id() );
                const QString cn = subject["CN"];
                if ( cn.isEmpty() )
                    return subject.prettyDN();
                return cn;
            } else {
                return tr( "Unknown Key Type" );
            }
        case PrettyEMail:
            return QString::fromUtf8( key.userID( 0 ).email() );
        case ValidFrom:
        case ValidUntil:
            {
                const Subkey subkey = key.subkey( 0 );
		if ( column == ValidUntil && subkey.neverExpires() )
		    return QVariant();//tr("Indefinitely");
                const time_t t = column == ValidUntil ? subkey.expirationTime() : subkey.creationTime() ;
                QDateTime dt;
                dt.setTime_t( t );
                return dt.date().toString();
            }
        case TechnicalDetails:
            return QString::fromUtf8( key.protocolAsString() );
        case Fingerprint:
            return QString::fromLatin1( key.primaryFingerprint() );
        case NumColumns:
            break;
        }
    else if ( role == Qt::FontRole ) {
	QFont font = qApp->font(); // ### correct font?
	if ( column == Fingerprint )
	    font.setFamily( "courier" );
	if ( const KeyFilter * const filter = KeyFilterManager::instance()->filterMatching( key ) )
	    return filter->font( font );
	else
	    return font;
    } else if ( role == Qt::DecorationRole || role == Qt::BackgroundRole || role == Qt::ForegroundRole ) {
        if ( const KeyFilter * const filter = KeyFilterManager::instance()->filterMatching( key ) ) {
            switch ( role ) {
            case Qt::DecorationRole: return column == Icon ? QIcon( filter->icon() ) : QVariant() ;
            case Qt::BackgroundRole: return filter->bgColor();
            case Qt::ForegroundRole: return filter->fgColor();
            default: ; // silence compiler
            }
        }
    } else if ( role == Qt::TextAlignmentRole ) // needed?
        ;
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
    std::vector<Key> merged;
    merged.reserve( keys.size() + mKeysByFingerprint.size() );
    std::merge( mKeysByFingerprint.begin(), mKeysByFingerprint.end(),
                keys.begin(), keys.end(),
                std::back_inserter( merged ), ByFingerprint<std::less>() );
    merged.erase( std::unique( merged.begin(), merged.end(), ByFingerprint<std::equal_to>() ), merged.end() );
    
    mKeysByFingerprint.swap( merged );
    reset(); // ### be better here...
    return QList<QModelIndex>(); // ### FIXME
}

// static
AbstractKeyListModel * AbstractKeyListModel::createFlatKeyListModel( QObject * p ) {
    return new FlatKeyListModel( p );
}

// static
AbstractKeyListModel * AbstractKeyListModel::createHierarchicalKeyListModel( QObject * p ) {
    return 0;//new HierarchicalKeyListModel( p );
}

#include "moc_keylistmodel.cpp"
#include "keylistmodel.moc"
