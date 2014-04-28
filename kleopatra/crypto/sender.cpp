/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/sender.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#include "sender.h"

#include <models/predicates.h>
#include <models/keycache.h>

#include <utils/kleo_assert.h>
#include <utils/cached.h>

#include <KMime/kmime_header_parsing.h>

#include <gpgme++/key.h>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace KMime::Types;
using namespace GpgME;
using namespace boost;

namespace KMime {
namespace Types {
static bool operator==( const AddrSpec & lhs, const AddrSpec & rhs ) {
    return lhs.localPart == rhs.localPart
        && lhs.domain == rhs.domain ;
}

static bool operator==( const Mailbox & lhs, const Mailbox & rhs ) {
    return lhs.name() == rhs.name()
        && lhs.addrSpec() == rhs.addrSpec() ;
}

static bool determine_ambiguous( const Mailbox & mb, const std::vector<Key> & keys ) {
    Q_UNUSED( mb );
    // ### really do check when we don't only show matching keys
    return keys.size() != 1 ;
}
} // namespace Types
} // namespace KMime

class Sender::Private {
    friend class ::Kleo::Crypto::Sender;
public:
    explicit Private( const Mailbox & mb )
        : mailbox( mb )
    {
        // ### also fill up to a certain number of keys with those
        // ### that don't match, for the case where there's a low
        // ### total number of keys
        const std::vector<Key> signers = KeyCache::instance()->findSigningKeysByMailbox( mb );
        const std::vector<Key> encrypt = KeyCache::instance()->findEncryptionKeysByMailbox( mb );
        kdtools::separate_if( signers,
                              std::back_inserter( pgpSigners ), std::back_inserter( cmsSigners ),
                              boost::bind( &Key::protocol, _1 ) == OpenPGP );
        kdtools::separate_if( encrypt,
                              std::back_inserter( pgpEncryptToSelfKeys ), std::back_inserter( cmsEncryptToSelfKeys ),
                              boost::bind( &Key::protocol, _1 ) == OpenPGP );
    }

private:
    const Mailbox mailbox;
    std::vector<Key> pgpSigners, cmsSigners, pgpEncryptToSelfKeys, cmsEncryptToSelfKeys;
    cached<bool> signingAmbiguous[2], encryptionAmbiguous[2];
    Key signingKey[2], cmsEncryptionKey;
    UserID pgpEncryptionUid;
};

Sender::Sender( const Mailbox & mb )
    : d( new Private( mb ) )
{

}

void Sender::detach() {
    if ( d && !d.unique() )
        d.reset( new Private( *d ) );
}

bool Sender::deepEquals( const Sender & other ) const {
    static const _detail::ByFingerprint<std::equal_to> compare = {};
    return mailbox() == other.mailbox()
        && compare( d->signingKey[CMS],     other.d->signingKey[CMS]     )
        && compare( d->signingKey[OpenPGP], other.d->signingKey[OpenPGP] )
        && compare( d->cmsEncryptionKey, other.d->cmsEncryptionKey )
        && compare( d->pgpEncryptionUid.parent(), other.d->pgpEncryptionUid.parent() )
        && strcmp( d->pgpEncryptionUid.id(), other.d->pgpEncryptionUid.id() ) == 0
        && kdtools::equal( d->pgpSigners, other.d->pgpSigners, compare )
        && kdtools::equal( d->cmsSigners, other.d->cmsSigners, compare )
        && kdtools::equal( d->pgpEncryptToSelfKeys, other.d->pgpEncryptToSelfKeys, compare )
        && kdtools::equal( d->cmsEncryptToSelfKeys, other.d->cmsEncryptToSelfKeys, compare )
        ;
}

bool Sender::isSigningAmbiguous( GpgME::Protocol proto ) const {
    if ( d->signingAmbiguous[proto].dirty() )
        d->signingAmbiguous[proto] = determine_ambiguous( d->mailbox, signingCertificateCandidates( proto ) );
    return d->signingAmbiguous[proto];
}

bool Sender::isEncryptionAmbiguous( GpgME::Protocol proto ) const {
    if ( d->encryptionAmbiguous[proto].dirty() )
        d->encryptionAmbiguous[proto] = determine_ambiguous( d->mailbox, encryptToSelfCertificateCandidates( proto ) );
    return d->encryptionAmbiguous[proto];
}

const Mailbox & Sender::mailbox() const {
    return d->mailbox;
}

const std::vector<Key> & Sender::signingCertificateCandidates( GpgME::Protocol proto ) const {
    if ( proto == OpenPGP )
        return d->pgpSigners;
    if ( proto == CMS )
        return d->cmsSigners;
    kleo_assert_fail( proto == OpenPGP || proto == CMS );
#if 0
    return
        proto == OpenPGP ? d->pgpSigners :
        proto == CMS     ? d->cmsSigners :
        // even though gcc warns about this line, it's completely ok, promise:
        kleo_assert_fail( proto == OpenPGP || proto == CMS ) ;
#endif
}

const std::vector<Key> & Sender::encryptToSelfCertificateCandidates( GpgME::Protocol proto ) const {
    if ( proto == OpenPGP )
        return d->pgpEncryptToSelfKeys;
    if ( proto == CMS )
        return d->cmsEncryptToSelfKeys;
    kleo_assert_fail( proto == OpenPGP || proto == CMS );
#if 0
    return
        proto == OpenPGP ? d->pgpEncryptToSelfKeys :
        proto == CMS     ? d->cmsEncryptToSelfKeys :
        // even though gcc warns about this line, it's completely ok, promise:
        kleo_assert_fail( proto == OpenPGP || proto == CMS ) ;
#endif
}

void Sender::setResolvedSigningKey( const Key & key ) {
    if ( key.isNull() )
        return;
    const Protocol proto = key.protocol();
    kleo_assert( proto == OpenPGP || proto == CMS );
    detach();
    d->signingKey[proto] = key;
    d->signingAmbiguous[proto] = false;
}

Key Sender::resolvedSigningKey( GpgME::Protocol proto ) const {
    kleo_assert( proto == OpenPGP || proto == CMS );
    return d->signingKey[proto];
}

void Sender::setResolvedEncryptionKey( const Key & key ) {
    if ( key.isNull() )
        return;
    const Protocol proto = key.protocol();
    kleo_assert( proto == OpenPGP || proto == CMS );
    detach();
    if ( proto == OpenPGP )
        d->pgpEncryptionUid = key.userID( 0 );
    else
        d->cmsEncryptionKey = key;
    d->encryptionAmbiguous[proto] = false;
}

Key Sender::resolvedEncryptionKey( GpgME::Protocol proto ) const {
    kleo_assert( proto == OpenPGP || proto == CMS );
    if ( proto == OpenPGP )
        return d->pgpEncryptionUid.parent();
    else
        return d->cmsEncryptionKey;
}

void Sender::setResolvedOpenPGPEncryptionUserID( const UserID & uid ) {
    if ( uid.isNull() )
        return;
    detach();
    d->pgpEncryptionUid = uid;
}

UserID Sender::resolvedOpenPGPEncryptionUserID() const {
    return d->pgpEncryptionUid;
}
