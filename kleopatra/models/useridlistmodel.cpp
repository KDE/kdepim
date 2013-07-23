/* -*- mode: c++; c-basic-offset:4 -*-
    models/useridlistmodel.cpp

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

#include "useridlistmodel.h"

#include <utils/formatting.h>

#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QVariant>
#include <QDate>

#include <boost/bind.hpp>

#include <algorithm>
#include <iterator>

using namespace GpgME;
using namespace Kleo;
using namespace boost;

namespace {

    static inline bool is_userid_level( const QModelIndex & idx ) {
        return idx.isValid() && idx.internalId() < 0 ;
    }

    static inline int extract_uid_number( const QModelIndex & idx ) {
        return idx.internalId();
    }

    static inline bool is_signature_level( const QModelIndex & idx ) {
        return idx.isValid() && idx.internalId() >= 0 ;
    }

}

class UserIDListModel::Private {
    friend class ::Kleo::UserIDListModel;
    UserIDListModel * const q;
public:
    explicit Private( UserIDListModel * qq )
        : q( qq ), key() {}

private:
    Key key;
};



UserIDListModel::UserIDListModel( QObject * p )
    : QAbstractItemModel( p ), d( new Private( this ) )
{

}

UserIDListModel::~UserIDListModel() {}


Key UserIDListModel::key() const {
    return d->key;
}

// slot
void UserIDListModel::setKey( const Key & key ) {

    const Key oldKey = d->key;

    d->key = key;

    if ( qstricmp( key.primaryFingerprint(), oldKey.primaryFingerprint() ) != 0 ) {
        // different key -> reset
        reset();
        return;
    }

    // ### diff them, and signal more fine-grained than this:

    if ( key.numUserIDs() > 0 && oldKey.numUserIDs() == key.numUserIDs() ) {
        bool identical = true;
        for ( unsigned int i = 0, end = key.numUserIDs() ; i != end ; ++i ) {
            if ( key.userID( i ).numSignatures() != oldKey.userID( i ).numSignatures() ) {
                identical = false;
                break;
            }
        }
        if ( identical ) {
            emit dataChanged( index( 0, 0 ), index( key.numUserIDs() - 1, NumColumns - 1 ) );
            return;
        }
    }
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

UserID UserIDListModel::userID( const QModelIndex & idx, bool strict ) const {
    if ( is_userid_level( idx ) )
        return d->key.userID( idx.row() );
    if ( !strict && is_signature_level( idx ) )
        return d->key.userID( extract_uid_number( idx ) );
    return UserID();
}

std::vector<UserID> UserIDListModel::userIDs( const QList<QModelIndex> & indexes, bool strict ) const {
    std::vector<UserID> result;
    result.reserve( indexes.size() );
    std::transform( indexes.begin(), indexes.end(),
                    std::back_inserter( result ),
                    boost::bind( &UserIDListModel::userID, this, _1, strict ) );
    return result;
}

UserID::Signature UserIDListModel::signature( const QModelIndex & idx ) const {
    if ( is_signature_level( idx ) )
        return d->key.userID( extract_uid_number( idx ) ).signature( idx.row() );
    else
        return UserID::Signature();
}

std::vector<UserID::Signature> UserIDListModel::signatures( const QList<QModelIndex> & indexes ) const {
    std::vector<UserID::Signature> result;
    result.reserve( indexes.size() );
    std::transform( indexes.begin(), indexes.end(),
                    std::back_inserter( result ),
                    boost::bind( &UserIDListModel::signature, this, _1 ) );
    return result;
}

QModelIndex UserIDListModel::index( const UserID & userID, int col ) const {
    // O(N), but not sorted, so no better way...
    for ( unsigned int row = 0, end = d->key.numUserIDs() ; row != end ; ++row )
        if ( qstricmp( userID.id(), d->key.userID( row ).id() ) == 0 )
            return createIndex( row, col, -1 );
    return QModelIndex();
}

QList<QModelIndex> UserIDListModel::indexes( const std::vector<UserID> & userIDs, int col ) const {
    // O(N*M), but who cares...?
    QList<QModelIndex> result;
    Q_FOREACH( const UserID & uid, userIDs )
        result.push_back( index( uid, col ) );
    return result;
}

QModelIndex UserIDListModel::index( const UserID::Signature & sig, int col ) const {
    const UserID uid = sig.parent();
    const QModelIndex pidx = index( uid );
    if ( !pidx.isValid() )
        return QModelIndex();
    const std::vector<UserID::Signature> sigs = uid.signatures();
    const std::vector<UserID::Signature>::const_iterator it
        = std::find_if( sigs.begin(), sigs.end(),
                        boost::bind( qstricmp, boost::bind( &UserID::Signature::signerKeyID, _1 ), sig.signerKeyID() ) == 0 );
    if ( it == sigs.end() )
        return QModelIndex();
    return createIndex( std::distance( sigs.begin(), it ), col, pidx.row() );
}

QList<QModelIndex> UserIDListModel::indexes( const std::vector<UserID::Signature> & signatures, int col ) const {
    QList<QModelIndex> result;
    Q_FOREACH( const UserID::Signature & sig, signatures )
        result.push_back( index( sig, col ) );
    return result;
}

void UserIDListModel::clear() {
    d->key = Key::null;
    reset();
}

int UserIDListModel::columnCount( const QModelIndex & ) const {
    return NumColumns;
}

int UserIDListModel::rowCount( const QModelIndex & pidx ) const {
    if ( !pidx.isValid() )
        return d->key.numUserIDs();
    if ( is_userid_level( pidx ) )
        return d->key.userID( pidx.row() ).numSignatures();
    return 0;
}

QModelIndex UserIDListModel::index( int row, int col, const QModelIndex & pidx ) const {
    if ( row < 0 || col < 0 || col >= NumColumns )
        return QModelIndex();

    if ( !pidx.isValid() ) {
        if ( static_cast<unsigned>( row ) < d->key.numUserIDs() )
            return createIndex( row, col, -1 );
        else
            return QModelIndex();
    }

    if ( !is_userid_level( pidx ) )
        return QModelIndex();

    const int numSigs = userID( pidx ).numSignatures();
    if ( row < numSigs )
        return createIndex( row, col, pidx.row() );

    return QModelIndex();
}

QModelIndex UserIDListModel::parent( const QModelIndex & idx ) const {
    if ( is_signature_level( idx ) )
        return createIndex( idx.internalId(), 0, -1 );
    else
        return QModelIndex();
}

QVariant UserIDListModel::headerData( int section, Qt::Orientation o, int role ) const {
    if ( o == Qt::Horizontal )
        if ( role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole )
            switch ( section ) {
            case ID:          return i18n( "ID" );
            case PrettyName:  return i18n( "Name" );
            case PrettyEMail: return i18n( "EMail" );
            case ValidFrom:   return i18n( "Valid From" );
            case ValidUntil:  return i18n( "Valid Until" );
            case Status:      return i18n( "Status" );
            case NumColumns:       ;
            }
    return QVariant();
}

QVariant UserIDListModel::data( const QModelIndex & idx, int role ) const {

    if ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ToolTipRole )
        return QVariant();

    if ( is_userid_level( idx ) ) {

        const UserID uid = this->userID( idx );
        if ( uid.isNull() )
            return QVariant();

        if ( idx.column() == 0 )
            // we assume that the top-level items are spanned
            return Formatting::prettyUserID( uid );
        else
            return QVariant();

    } else if ( is_signature_level( idx ) ) {

        const UserID::Signature signature = this->signature( idx );
        if ( signature.isNull() )
            return QVariant();

        switch ( idx.column() ) {

        case ID:
            return QString::fromLatin1( signature.signerKeyID() );
        case PrettyName:
            return Formatting::prettyName( signature );
        case PrettyEMail:
            return Formatting::prettyEMail( signature );
        case ValidFrom:
            if ( role == Qt::EditRole )
                return Formatting::creationDate( signature );
            else
                return Formatting::creationDateString( signature );
        case ValidUntil:
            if ( role == Qt::EditRole )
                return Formatting::expirationDate( signature );
            else
                return Formatting::expirationDateString( signature );
        case Status:
            return Formatting::validityShort( signature );
#if 0
            if ( userID.isRevoked() )
                return i18n("revoked");
            if ( userID.isExpired() )
                return i18n("expired");
            if ( userID.isDisabled() )
                return i18n("disabled");
            if ( userID.isInvalid() )
                return i18n("invalid");
            return i18n("good");
#endif
        }


    }

    return QVariant();
}

#include "moc_useridlistmodel.cpp"
