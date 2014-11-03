/* -*- mode: c++; c-basic-offset:4 -*-
    ./crypto/sender.h

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

#ifndef __KLEOPATRA_CRYPTO_SENDER_H__
#define __KLEOPATRA_CRYPTO_SENDER_H__

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace KMime
{
namespace Types
{
class Mailbox;
}
}

namespace GpgME
{
class Key;
class UserID;
}

namespace Kleo
{
namespace Crypto
{

class Sender
{
public:
    Sender() : d() {}
    explicit Sender(const KMime::Types::Mailbox &mailbox);

    void swap(Sender &other)
    {
        d.swap(other.d);
    }

    bool isNull() const
    {
        return !d;
    }

    bool isSigningAmbiguous(GpgME::Protocol proto) const;
    bool isEncryptionAmbiguous(GpgME::Protocol proto) const;

    const KMime::Types::Mailbox &mailbox() const;

    const std::vector<GpgME::Key> &signingCertificateCandidates(GpgME::Protocol proto) const;
    const std::vector<GpgME::Key> &encryptToSelfCertificateCandidates(GpgME::Protocol proto) const;

    void setResolvedSigningKey(const GpgME::Key &key);
    GpgME::Key resolvedSigningKey(GpgME::Protocol proto) const;

    void setResolvedEncryptionKey(const GpgME::Key &key);
    GpgME::Key resolvedEncryptionKey(GpgME::Protocol proto) const;

    void setResolvedOpenPGPEncryptionUserID(const GpgME::UserID &uid);
    GpgME::UserID resolvedOpenPGPEncryptionUserID() const;

    friend inline bool operator==(const Sender &lhs, const Sender &rhs)
    {
        return rhs.d == lhs.d || lhs.deepEquals(rhs);
    }

private:
    void detach();
    bool deepEquals(const Sender &other) const;

private:
    class Private;
    boost::shared_ptr<Private> d;
};

inline bool operator!=(const Sender &lhs, const Sender &rhs)
{
    return !operator==(lhs, rhs);
}

} // namespace Crypto
} // namespace Kleo

#endif /* __KLEOPATRA_CRYPTO_SENDER_H__ */
