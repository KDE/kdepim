/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/recipient.cpp

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

#include "recipient.h"

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

class Recipient::Private {
    friend class ::Kleo::Crypto::Recipient;
public:
    explicit Private( const Mailbox & mb )
        : mailbox( mb )
    {
        // ### also fill up to a certain number of keys with those
        // ### that don't match, for the case where there's a low
        // ### total number of keys
        const std::vector<Key> encrypt = KeyCache::instance()->findEncryptionKeysByMailbox( mb );
        kdtools::separate_if( encrypt,
                              std::back_inserter( pgpEncryptionKeys ), std::back_inserter( cmsEncryptionKeys ),
                              boost::bind( &Key::protocol, _1 ) == OpenPGP );
    }

private:
    const Mailbox mailbox;
    std::vector<Key> pgpEncryptionKeys, cmsEncryptionKeys;
    Key cmsEncryptionKey;
    UserID pgpEncryptionUid;
    cached<bool> encryptionAmbiguous[2];
};

Recipient::Recipient( const Mailbox & mb )
    : d( new Private( mb ) )
{

}

void Recipient::detach() {
    if ( d && !d.unique() )
        d.reset( new Private( *d ) );
}

bool Recipient::deepEquals( const Recipient & other ) const {
    static const _detail::ByFingerprint<std::equal_to> compare = {};
    return mailbox() == other.mailbox()
        && compare( d->cmsEncryptionKey, other.d->cmsEncryptionKey )
        && compare( d->pgpEncryptionUid.parent(), other.d->pgpEncryptionUid.parent() )
        && strcmp( d->pgpEncryptionUid.id(), other.d->pgpEncryptionUid.id() )
        && kdtools::equal( d->pgpEncryptionKeys, other.d->pgpEncryptionKeys, compare )
        && kdtools::equal( d->cmsEncryptionKeys, other.d->cmsEncryptionKeys, compare )
        ;
}

bool Recipient::isEncryptionAmbiguous( GpgME::Protocol proto ) const {
    if ( d->encryptionAmbiguous[proto].dirty() )
        d->encryptionAmbiguous[proto] = determine_ambiguous( d->mailbox, encryptionCertificateCandidates( proto ) );
    return d->encryptionAmbiguous[proto];
}

const Mailbox & Recipient::mailbox() const {
    return d->mailbox;
}

const std::vector<Key> & Recipient::encryptionCertificateCandidates( GpgME::Protocol proto ) const {
    if ( proto == OpenPGP )
        return d->pgpEncryptionKeys;
    if ( proto == CMS )
        return d->cmsEncryptionKeys;
    kleo_assert_fail( proto == OpenPGP || proto == CMS );
#if 0
    return
        proto == OpenPGP ? d->pgpEncryptionKeys :
        proto == CMS     ? d->cmsEncryptionKeys :
        // even though gcc warns about this line, it's completely ok, promise:
        kleo_assert_fail( proto == OpenPGP || proto == CMS ) ;
#endif
}

void Recipient::setResolvedEncryptionKey( const Key & key ) {
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

Key Recipient::resolvedEncryptionKey( GpgME::Protocol proto ) const {
    kleo_assert( proto == OpenPGP || proto == CMS );
    if ( proto == OpenPGP )
        return d->pgpEncryptionUid.parent();
    else
        return d->cmsEncryptionKey;
}

void Recipient::setResolvedOpenPGPEncryptionUserID( const UserID & uid ) {
    if ( uid.isNull() )
        return;
    detach();
    d->pgpEncryptionUid = uid;
}

UserID Recipient::resolvedOpenPGPEncryptionUserID() const {
    return d->pgpEncryptionUid;
}
