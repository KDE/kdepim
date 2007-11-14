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
#include "predicates.h"

#include <utils/stl_util.h>

#include <kleo/dn.h>

#include <gpgme++/key.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/weak_ptr.hpp>

#include <utility>
#include <algorithm>
#include <functional>
#include <iterator>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

shared_ptr<const KeyCache> KeyCache::instance() {
    return mutableInstance();
}

shared_ptr<KeyCache> KeyCache::mutableInstance() {
    static weak_ptr<KeyCache> self;
    try {
        return shared_ptr<KeyCache>( self );
    } catch ( const bad_weak_ptr & ) {
        const shared_ptr<KeyCache> s( new KeyCache );
        self = s;
        return s;
    }
}

namespace {

    make_comparator_str( ByEMail, .first.c_str() );

    struct is_empty : std::unary_function<const char*,bool> {
        bool operator()( const char * s ) const { return !s || !*s; }
    };

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
        return find<_detail::ByFingerprint>( by.fpr, fpr );
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
        return find<_detail::ByKeyID>( by.keyid, keyid );
    }

    std::vector<Key>::const_iterator find_shortkeyid( const char * shortkeyid ) const {
        return find<_detail::ByShortKeyID>( by.shortkeyid, shortkeyid );
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

std::vector<Key> KeyCache::findByFingerprint( const std::vector<std::string> & fprs ) const {
    std::vector<std::string> sorted;
    sorted.reserve( fprs.size() );
    std::remove_copy_if( fprs.begin(), fprs.end(), std::back_inserter( sorted ),
                         bind( is_empty(), bind( &std::string::c_str, _1 ) ) );

    std::sort( sorted.begin(), sorted.end(), _detail::ByFingerprint<std::less>() );

    std::vector<Key> result;
    kdtools::set_intersection( d->by.fpr.begin(), d->by.fpr.end(),
                               sorted.begin(), sorted.end(),
                               std::back_inserter( result ),
                               _detail::ByFingerprint<std::less>() );
    return result;
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

const Key & KeyCache::findByShortKeyID( const char * id ) const {
    const std::vector<Key>::const_iterator it = d->find_shortkeyid( id );
    if ( it != d->by.shortkeyid.end() )
        return *it;
    static const Key null;
    return null;
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
    }
    static const Key null;
    return null;
}

std::vector<Key> KeyCache::findByKeyIDOrFingerprint( const std::vector<std::string> & ids ) const {

    std::vector<std::string> keyids;
    std::remove_copy_if( ids.begin(), ids.end(), std::back_inserter( keyids ),
                         bind( is_empty(), bind( &std::string::c_str, _1 ) ) );

    // this is just case-insensitive string search:
    std::sort( keyids.begin(), keyids.end(), _detail::ByFingerprint<std::less>() );

    std::vector<Key> result;
    result.reserve( keyids.size() ); // dups shouldn't happen

    kdtools::set_intersection( d->by.fpr.begin(), d->by.fpr.end(),
                               keyids.begin(), keyids.end(),
                               std::back_inserter( result ),
                               _detail::ByFingerprint<std::less>() );
    if ( result.size() < keyids.size() )
        // note that By{Fingerprint,KeyID,ShortKeyID} define the same
        // order for _strings_
        kdtools::set_intersection( d->by.keyid.begin(), d->by.keyid.end(),
                                   keyids.begin(), keyids.end(),
                                   std::back_inserter( result ),
                                   _detail::ByKeyID<std::less>() );

    // duplicates shouldn't happen, but make sure nonetheless:
    std::sort( result.begin(), result.end(), _detail::ByFingerprint<std::less>() );
    result.erase( std::unique( result.begin(), result.end(), _detail::ByFingerprint<std::equal_to>() ), result.end() );

    // we skip looking into short key ids here, as it's highly
    // unlikely they're used for this purpose. We might need to revise
    // this decision, but only after testing.
    return result;
}

std::vector<Key> KeyCache::findRecipients( const DecryptionResult & res ) const {
    std::vector<std::string> keyids;
    Q_FOREACH( const DecryptionResult::Recipient & r, res.recipients() )
        keyids.push_back( r.keyID() );
    return findByKeyIDOrFingerprint( keyids );
}

std::vector<Key> KeyCache::findSigners( const VerificationResult & res ) const {
    std::vector<std::string> fprs;
    Q_FOREACH( const Signature & s, res.signatures() )
        fprs.push_back( s.fingerprint() );
    return findByKeyIDOrFingerprint( fprs );
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
                                _detail::ByFingerprint<std::less>() );
        d->by.fpr.erase( range.first, range.second );
    }

    if ( const char * keyid = key.keyID() ) {
        const std::pair<std::vector<Key>::iterator,std::vector<Key>::iterator> range
            = std::equal_range( d->by.keyid.begin(), d->by.keyid.end(), keyid,
                                _detail::ByKeyID<std::less>() );
        const std::vector<Key>::iterator it
            = std::remove_if( begin( range ), end( range ), bind( _detail::ByFingerprint<std::equal_to>(), fpr, _1 ) );
        d->by.keyid.erase( it, end( range ) );
    }
                
    if ( const char * shortkeyid = key.shortKeyID() ) {
        const std::pair<std::vector<Key>::iterator,std::vector<Key>::iterator> range
            = std::equal_range( d->by.shortkeyid.begin(), d->by.shortkeyid.end(), shortkeyid,
                                _detail::ByShortKeyID<std::less>() );
        const std::vector<Key>::iterator it
            = std::remove_if( begin( range ), end( range ), bind( _detail::ByFingerprint<std::equal_to>(), fpr, _1 ) );
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
    std::sort( sorted.begin(), sorted.end(), _detail::ByFingerprint<std::less>() );

    // 2a. insert into fpr index:
    std::vector<Key> by_fpr;
    by_fpr.reserve( sorted.size() + d->by.fpr.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.fpr.begin(), d->by.fpr.end(),
                std::back_inserter( by_fpr ),
                _detail::ByFingerprint<std::less>() );

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
    std::sort( sorted.begin(), sorted.end(), _detail::ByKeyID<std::less>() );

    // 4a. insert into keyid index:
    std::vector<Key> by_keyid;
    by_keyid.reserve( sorted.size() + d->by.keyid.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.keyid.begin(), d->by.keyid.end(),
                std::back_inserter( by_keyid ),
                _detail::ByKeyID<std::less>() );

    // 5. sort by short key id:
    std::sort( sorted.begin(), sorted.end(), _detail::ByShortKeyID<std::less>() );

    // 5a. insert into short keyid index:
    std::vector<Key> by_shortkeyid;
    by_shortkeyid.reserve( sorted.size() + d->by.shortkeyid.size() );
    std::merge( sorted.begin(), sorted.end(),
                d->by.shortkeyid.begin(), d->by.shortkeyid.end(),
                std::back_inserter( by_shortkeyid ),
                _detail::ByShortKeyID<std::less>() );

    // now commit (well, we already removed keys...)
    by_fpr.swap( d->by.fpr );
    by_keyid.swap( d->by.keyid );
    by_shortkeyid.swap( d->by.shortkeyid );
    by_email.swap( d->by.email );

    Q_FOREACH( const Key & key, sorted )
        emit added( key ); 
}

void KeyCache::clear() {
    d->by = Private::By();
}

#include "moc_keycache.cpp"

