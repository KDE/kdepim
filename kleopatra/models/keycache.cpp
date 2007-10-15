/* -*- mode: c++; c-basic-offset:4 -*-
    models/keycache.cpp

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

#include "keycache.h"

#include <kleo/dn.h>

#include <gpgme++/key.h>

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/weak_ptr.hpp>

#include <utility>
#include <algorithm>
#include <functional>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

shared_ptr<const KeyCache> KeyCache::instance() {
    return mutableInstance();
}

shared_ptr<KeyCache> KeyCache::mutableInstance() {
    static weak_ptr<KeyCache> self = shared_ptr<KeyCache>( new KeyCache );
    return shared_ptr<KeyCache>( self );
}

namespace {

#define make_comparator_str( Name, expr )                               \
    template <template <typename U> class Op>                           \
    struct Name {                                                       \
        typedef bool result_type;                                       \
                                                                        \
        bool operator()( const char * lhs, const char * rhs ) const {   \
            return Op<int>()( qstricmp( lhs, rhs ), 0 );                \
        }                                                               \
                                                                        \
        bool operator()( const std::string & lhs, const std::string & rhs ) const { \
            return operator()( lhs.c_str(), rhs.c_str() );              \
        }                                                               \
        bool operator()( const char * lhs, const std::string & rhs ) const { \
            return operator()( lhs, rhs.c_str() );                      \
        }                                                               \
        bool operator()( const std::string & lhs, const char * rhs ) const { \
            return operator()( lhs.c_str(), rhs );                      \
        }                                                               \
                                                                        \
        template <typename T>                                           \
        bool operator()( const T & lhs, const T & rhs ) const {         \
            return operator()( (lhs expr), (rhs expr) );                \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const char * rhs ) const {      \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const char * lhs, const T & rhs ) const {      \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const std::string & rhs ) const { \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const std::string & lhs, const T & rhs ) const {    \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
    }

    make_comparator_str( ByEMail, .first.c_str() );
    make_comparator_str( ByFingerprint, .primaryFingerprint() );
    make_comparator_str( ByKeyID, .keyID() );
    make_comparator_str( ByShortKeyID, .shortKeyID() );

}

class KeyCache::Private {
    friend class ::Kleo::KeyCache;
    KeyCache * const q;
public:
    explicit Private( KeyCache * qq ) : q( qq ) {}

    template < template <template <typename U> class Op> class Comp >
    std::vector<Key>::const_iterator find( const std::vector<Key> & keys, const char * key ) const {
        const std::vector<Key>::const_iterator it =
            std::lower_bound( keys.begin(), keys.end(), key, Comp<std::less>() );
        if ( it == keys.end() || Comp<std::equal_to>()( *it, key ) )
            return it;
        else
            return keys.end();
    }

    std::vector<Key>::const_iterator find_fpr( const char * fpr ) const {
        return find<ByFingerprint>( by.fpr, fpr );
    }

    std::vector< std::pair<std::string,Key> >::const_iterator find_email( const char * email ) const {
        const std::vector< std::pair<std::string,Key> >::const_iterator it =
            std::lower_bound( by.email.begin(), by.email.end(),
                              email, ByEMail<std::less>() );
        if ( it == by.email.end() || ByEMail<std::equal_to>()( *it, email ) )
            return it;
        else
            return by.email.end();
    }

    std::vector<Key>::const_iterator find_keyid( const char * keyid ) const {
        return find<ByKeyID>( by.keyid, keyid );
    }

    std::vector<Key>::const_iterator find_shortkeyid( const char * shortkeyid ) const {
        return find<ByShortKeyID>( by.shortkeyid, shortkeyid );
    }

private:
    struct By {
        std::vector<Key> fpr, keyid, shortkeyid;
        std::vector< std::pair<std::string,Key> > email;
    } by;
};



KeyCache::KeyCache( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

KeyCache::~KeyCache() {}


const Key & KeyCache::findByFingerprint( const char * fpr ) const {
    const std::vector<Key>::const_iterator it = d->find_fpr( fpr );
    if ( it == d->by.fpr.end() ) {
        static const Key null;
        return null;
    } else {
        return *it;
    }
}

const Key & KeyCache::findByEMailAddress( const char * email ) const {
    const std::vector< std::pair<std::string,Key> >::const_iterator it = d->find_email( email );
    if ( it == d->by.email.end() ) {
        static const Key null;
        return null;
    } else {
        return it->second;
    }
}

const Key & KeyCache::findByKeyIDOrFingerprint( const char * id ) const {
    {
        // try by.fpr first:
        const std::vector<Key>::const_iterator it = d->find_fpr( id );
        if ( it != d->by.fpr.end() )
            return *it;
    }{
        // try by.keyid next:
        const std::vector<Key>::const_iterator it = d->find_keyid( id );
        if ( it != d->by.keyid.end() )
            return *it;
    }{
        // try by.shortkeyid last:
        const std::vector<Key>::const_iterator it = d->find_shortkeyid( id );
        if ( it != d->by.shortkeyid.end() )
            return *it;
    }
    static Key null;
    return null;
}

static std::string email( const UserID & uid ) {
    const std::string email = uid.email();
    if ( email.empty() )
        return DN( uid.id() )["EMAIL"].trimmed().toUtf8().constData();
    if ( email[0] == '<' && email[email.size()-1] == '>' )
        return email.substr( 1, email.size() - 2 );
    else
        return email;
}

static std::vector<std::string> emails( const Key & key ) {
    std::vector<std::string> emails;
    Q_FOREACH( const UserID & uid, key.userIDs() ) {
        const std::string e = email( uid );
        if ( !e.empty() )
            emails.push_back( e );
    }
    std::sort( emails.begin(), emails.end() );
    emails.erase( std::unique( emails.begin(), emails.end() ), emails.end() );
    return emails;
}

void KeyCache::remove( const Key & key ) {
    if ( key.isNull() )
        return;

    const char * fpr = key.primaryFingerprint();
    if ( !fpr )
        return;

    {
        const std::pair<std::vector<Key>::iterator,std::vector<Key>::iterator> range
            = std::equal_range( d->by.fpr.begin(), d->by.fpr.end(), fpr,
                                ByFingerprint<std::less>() );
        d->by.fpr.erase( range.first, range.second );
    }

    if ( const char * keyid = key.keyID() ) {
        const std::pair<std::vector<Key>::iterator,std::vector<Key>::iterator> range
            = std::equal_range( d->by.keyid.begin(), d->by.keyid.end(), keyid,
                                ByKeyID<std::less>() );
        const std::vector<Key>::iterator it
            = std::remove_if( begin( range ), end( range ), bind( ByFingerprint<std::equal_to>(), fpr, _1 ) );
        d->by.keyid.erase( it, end( range ) );
    }
                
    if ( const char * shortkeyid = key.shortKeyID() ) {
        const std::pair<std::vector<Key>::iterator,std::vector<Key>::iterator> range
            = std::equal_range( d->by.shortkeyid.begin(), d->by.shortkeyid.end(), shortkeyid,
                                ByShortKeyID<std::less>() );
        const std::vector<Key>::iterator it
            = std::remove_if( begin( range ), end( range ), bind( ByFingerprint<std::equal_to>(), fpr, _1 ) );
        d->by.shortkeyid.erase( it, end( range ) );
    }

    Q_FOREACH( const std::string & email, emails( key ) ) {
        const std::pair<std::vector<std::pair<std::string,Key> >::iterator,std::vector<std::pair<std::string,Key> >::iterator> range
            = std::equal_range( d->by.email.begin(), d->by.email.end(), email, ByEMail<std::less>() );
        const std::vector< std::pair<std::string,Key> >::iterator it
            = std::remove_if( begin( range ), end( range ), bind( qstricmp, fpr, bind( &Key::primaryFingerprint, bind( &std::pair<std::string,Key>::second,_1 ) ) ) == 0 );
        d->by.email.erase( it, end( range ) );
    }
                
}

void KeyCache::insert( const Key & key ) {
    insert( std::vector<Key>( 1, key ) );
}

namespace {
    struct is_empty : std::unary_function<const char*,bool> {
        bool operator()( const char * s ) const { return !s || !*s; }
    };
}

void KeyCache::insert( const std::vector<Key> & keys ) {

    // 1. remove those with empty fingerprints:
    std::vector<Key> sorted;
    sorted.reserve( keys.size() );
    std::remove_copy_if( keys.begin(), keys.end(),
                         std::back_inserter( sorted ),
                         bind( is_empty(), bind( &Key::primaryFingerprint, _1 ) ) );

    Q_FOREACH( const Key & key, sorted )
        remove( key ); // this is sub-optimal, but makes implementation from here on much easier 

    // 2. sort by fingerprint:
    std::sort( sorted.begin(), sorted.end(), ByFingerprint<std::less>() );

    // 2a. insert into fpr index:
    std::vector<Key> by_fpr;
    by_fpr.reserve( sorted.size() + d->by.fpr.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.fpr.begin(), d->by.fpr.end(),
                std::back_inserter( by_fpr ),
                ByFingerprint<std::less>() );

    // 3. build email index:
    std::vector< std::pair<std::string,Key> > pairs;
    pairs.reserve( sorted.size() );
    Q_FOREACH( const Key & key, sorted ) {
        const std::vector<std::string> emails = ::emails( key );
        std::vector< std::pair<std::string,Key> > tmp, merge_tmp;
        tmp.reserve( emails.size() );
        Q_FOREACH( const std::string & e, emails )
            pairs.push_back( std::make_pair( e, key ) );
        merge_tmp.reserve( tmp.size() + pairs.size() );
        std::merge( pairs.begin(), pairs.end(),
                    tmp.begin(), tmp.end(),
                    std::back_inserter( merge_tmp ),
                    ByEMail<std::less>() );
        pairs.swap( merge_tmp );
    }

    // 3a. insert into email index:
    std::vector< std::pair<std::string,Key> > by_email;
    by_email.reserve( pairs.size() + d->by.email.size() );
    std::merge( pairs.begin(), pairs.end(),
                d->by.email.begin(), d->by.email.end(),
                std::back_inserter( by_email ),
                ByEMail<std::less>() );


    // 4. sort by key id:
    std::sort( sorted.begin(), sorted.end(), ByKeyID<std::less>() );

    // 4a. insert into keyid index:
    std::vector<Key> by_keyid;
    by_keyid.reserve( sorted.size() + d->by.keyid.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.keyid.begin(), d->by.keyid.end(),
                std::back_inserter( by_keyid ),
                ByKeyID<std::less>() );

    // 5. sort by short key id:
    std::sort( sorted.begin(), sorted.end(), ByShortKeyID<std::less>() );

    // 5a. insert into short keyid index:
    std::vector<Key> by_shortkeyid;
    by_shortkeyid.reserve( sorted.size() + d->by.shortkeyid.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.shortkeyid.begin(), d->by.shortkeyid.end(),
                std::back_inserter( by_shortkeyid ),
                ByShortKeyID<std::less>() );

    // now commit (well, we already removed keys...)
    by_fpr.swap( d->by.fpr );
    by_keyid.swap( d->by.keyid );
    by_shortkeyid.swap( d->by.shortkeyid );
    by_email.swap( d->by.email );
}

void KeyCache::clear() {
    d->by = Private::By();
}

//#include "moc_keycache.cpp"

