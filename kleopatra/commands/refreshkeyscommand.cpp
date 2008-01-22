/* -*- mode: c++; c-basic-offset:4 -*-
    refreshkeyscommand.cpp

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

#include "refreshkeyscommand.h"
#include "command_p.h"

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/stl_util.h>

#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/keylistjob.h>

#include <QStringList>

#include <boost/bind.hpp>

#include <deque>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

class RefreshKeysCommand::Private : public Command::Private {
    friend class ::Kleo::RefreshKeysCommand;
public:
    Private( RefreshKeysCommand * qq, Mode mode, KeyListController* controller );
    ~Private();

    enum KeyType {
        PublicKeys,
        SecretKeys
    };
    void startKeyListing( const char* backend, KeyType type );

    void publicKeyListingDone( const KeyListResult & result );
    void secretKeyListingDone( const KeyListResult & result );

    void addKey( const Key & key );

    void mergeKeysAndUpdateKeyCache();

private:
    const Mode mode;
    uint m_pubKeysJobs;
    uint m_secKeysJobs;
    bool canceled;

    std::deque<Key> m_pubkeys, m_seckeys;
};

RefreshKeysCommand::Private * RefreshKeysCommand::d_func() { return static_cast<Private*>( d.get() ); }
const RefreshKeysCommand::Private * RefreshKeysCommand::d_func() const { return static_cast<const Private*>( d.get() ); }


RefreshKeysCommand::RefreshKeysCommand( Mode mode, KeyListController * p )
    : Command( new Private( this, mode, p ) )
{

}

RefreshKeysCommand::RefreshKeysCommand( Mode mode, QAbstractItemView * v, KeyListController * p )
    : Command( v, new Private( this, mode, p ) )
{

}

RefreshKeysCommand::~RefreshKeysCommand() {}

RefreshKeysCommand::Private::Private( RefreshKeysCommand * qq, Mode m, KeyListController * controller )
    : Command::Private( qq, controller ),
      mode( m ),
      m_pubKeysJobs( 0 ),
      m_secKeysJobs( 0 ),
      canceled( false ),
      m_pubkeys(),
      m_seckeys()
{

}

RefreshKeysCommand::Private::~Private() {}


void RefreshKeysCommand::Private::startKeyListing( const char* backend, KeyType type )
{
    const Kleo::CryptoBackend::Protocol * const protocol = Kleo::CryptoBackendFactory::instance()->protocol( backend );
    if ( !protocol )
        return;
    Kleo::KeyListJob * const job = protocol->keyListJob( /*remote*/false, /*includeSigs*/false, mode == Validate );
    if ( !job )
        return;
    if ( type == PublicKeys ) {
        connect( job, SIGNAL(result(GpgME::KeyListResult)),
                 q, SLOT(publicKeyListingDone(GpgME::KeyListResult)) );
    } else {
        connect( job, SIGNAL(result(GpgME::KeyListResult)),
                 q, SLOT(secretKeyListingDone(GpgME::KeyListResult)) );
    }
    connect( job, SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( job, SIGNAL(nextKey(GpgME::Key)),
             q, SLOT(addKey(GpgME::Key)) );
    connect( q, SIGNAL(canceled()),
             job, SLOT(slotCancel()) );
    const GpgME::Error error = job->start( QStringList(), type == SecretKeys );
    ++( type == PublicKeys ? m_pubKeysJobs : m_secKeysJobs );
    if ( error || error.isCanceled() )
        if ( type == PublicKeys )
            publicKeyListingDone( KeyListResult( error ) );
        else
            secretKeyListingDone( KeyListResult( error ) );
}

void RefreshKeysCommand::Private::publicKeyListingDone( const KeyListResult & result )
{
    assert( m_pubKeysJobs > 0 );
    --m_pubKeysJobs;
    if ( result.error().isCanceled() )
        finished();
    else if ( m_pubKeysJobs == 0 ) {
        startKeyListing( "openpgp", Private::SecretKeys );
        startKeyListing( "smime", Private::SecretKeys );
    }
}


void RefreshKeysCommand::Private::secretKeyListingDone( const KeyListResult & result )
{
    assert( m_secKeysJobs > 0 );
    --m_secKeysJobs;
    if ( result.error().isCanceled() || m_secKeysJobs == 0 )
        finished();
    if ( m_secKeysJobs == 0 )
        mergeKeysAndUpdateKeyCache();
}

namespace {
    template <typename ForwardIterator, typename BinaryPredicate>
    ForwardIterator unique_by_merge( ForwardIterator first, ForwardIterator last, BinaryPredicate pred ) {
        first = std::adjacent_find( first, last, pred );
        if ( first == last )
            return last;

        ForwardIterator dest = first;
        dest->mergeWith( *++first );
        while ( ++first != last )
            if ( pred( *dest, *first ) )
                dest->mergeWith( *first );
            else
                *++dest = *first;
        return ++dest;
    }
}

void RefreshKeysCommand::Private::mergeKeysAndUpdateKeyCache()
{
    std::sort( m_pubkeys.begin(), m_pubkeys.end(), _detail::ByFingerprint<std::less>() );
    std::sort( m_seckeys.begin(), m_seckeys.end(), _detail::ByFingerprint<std::less>() );

    std::vector<Key> keys;
    keys.reserve( m_pubkeys.size() + m_seckeys.size() );

    std::merge( m_pubkeys.begin(), m_pubkeys.end(),
                m_seckeys.begin(), m_seckeys.end(),
                std::back_inserter( keys ),
                _detail::ByFingerprint<std::less>() );

    keys.erase( unique_by_merge( keys.begin(), keys.end(), _detail::ByFingerprint<std::equal_to>() ),
                keys.end() );

    std::vector<Key> sec;
    sec.reserve( m_seckeys.size() );
    kdtools::copy_if( keys.begin(), keys.end(), std::back_inserter( sec ),
                      bind( &Key::hasSecret, _1 ) );

    PublicKeyCache::mutableInstance()->refresh( keys );
    SecretKeyCache::mutableInstance()->refresh( sec );
}

void RefreshKeysCommand::Private::addKey( const Key & key )
{
    if ( key.hasSecret() )
        m_seckeys.push_back( key );
    else
        m_seckeys.push_back( key );
}

#define d d_func()

void RefreshKeysCommand::doStart() {
    /* NOTE: first fetch public keys. when done, fetch secret keys. hasSecret() works only
       correctly when the key was retrieved with --list-secret-keys (secretOnly flag
       in gpgme keylist operations) so we overwrite the key from --list-keys (secret
       not set) with the one from --list-secret-keys (with secret set).
    */
    d->startKeyListing( "openpgp", Private::PublicKeys );
    d->startKeyListing( "smime", Private::PublicKeys );
}

void RefreshKeysCommand::doCancel() {
    d->canceled = true;
}

#undef d

#include "moc_refreshkeyscommand.cpp"
