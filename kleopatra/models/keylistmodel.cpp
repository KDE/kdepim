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

using namespace GpgME;
using namespace Kleo;
using namespace boost;

AbstractKeyListModel::AbstractKeyListModel( QObject * p )
    : QAbstractItemModel( p )
{

}

AbstractKeyListModel::~AbstractKeyListModel() {}


Key AbstractKeyListModel::key( const QModelIndex & idx ) const {
    if ( idx.isValid() )
        return doMapToKey( idx );
    else
        return GpgME::Key::null;
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
        return doMapFromKey( key );
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
    return doAddKeys( keys );
}

int AbstractKeyListModel::numColumns() const {
    return NumColumns;
}

QVariant AbstractKeyListModel::headerData( int section, Qt::Orientation o, Qt::ItemDataRole role ) const {
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

QVariant AbstractKeyListModel::data( const QModelIndex & index, Qt::ItemDataRole role ) const {
    const GpgME::Key key = this->key( index );
    if ( key.isNull() )
        return QVariant();

    const int column = index.column();

    if ( role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole )
        switch ( column ) {
        case PrettyName:
	    if ( key.protocol() == GpgME::OpenPGP ) {
                const GpgME::UserID uid = key.userID( 0 );
                const QString name = QString::fromUtf8( uid.name() );
                if ( name.isEmpty() )
                    return QString::fromLatin1( key.primaryFingerprint() );
                const QString comment = QString::fromUtf8( uid.comment() );
                if ( comment.isEmpty() )
                    return name;
                return QString::fromLatin1( "%1 (%2)" ).arg( name, comment );
	    } else if ( key.protocol() == GpgME::CMS ) {
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
                const GpgME::Subkey subkey = key.subkey( 0 );
                time_t t;
                if ( column == ValidUntil )
                    if ( subkey.neverExpires() )
                        return tr("Never");
                    else
                        t = subkey.expirationTime();
                else
                    t = subkey.creationTime();
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
    else if ( role == Qt::DecorationRole || role == Qt::FontRole || role == Qt::BackgroundRole || role == Qt::ForegroundRole ) {
        if ( const KeyFilter * const filter = KeyFilterManager::instance()->filterMatching( key ) ) {
            switch ( role ) {
            case Qt::DecorationRole: return column == Icon ? QIcon( filter->icon() ) : QVariant() ;
            case Qt::FontRole:       return filter->font( qApp->font() ); // ### correct font?
            case Qt::BackgroundRole: return filter->bgColor();
            case Qt::ForegroundRole: return filter->fgColor();
            default: ; // silence compiler
            }
        }
    } else if ( role == Qt::TextAlignmentRole )
        ;
    return QVariant();
}


// static
AbstractKeyListModel * AbstractKeyListModel::createFlatKeyListModel( QObject * p ) {
    return 0;//new FlatKeyListModel( p );
}

AbstractKeyListModel * AbstractKeyListModel::createHierarchicalKeyListModel( QObject * p ) {
    return 0;//new HierarchicalKeyListModel( p );
}

#include "moc_keylistmodel.cpp"
