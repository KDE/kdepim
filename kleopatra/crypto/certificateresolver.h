/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/certificateresolver.h

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

#ifndef __KLEOPATRA_CRYPTO_CERTIFICATERESOLVER_H__
#define __KLEOPATRA_CRYPTO_CERTIFICATERESOLVER_H__

#include <utils/pimpl_ptr.h>

#include <gpgme++/key.h>
#include <KMime/kmime_header_parsing.h>

#include <KSharedConfig>

#include <vector>

class KConfig;

namespace GpgME {
    class Key;
}

namespace Kleo {
namespace Crypto {

    class SigningPreferences {
    public:
        virtual ~SigningPreferences() {}
        virtual GpgME::Key preferredCertificate( GpgME::Protocol protocol ) = 0;
        virtual void setPreferredCertificate( GpgME::Protocol protocol, const GpgME::Key& certificate ) = 0;
        
    };
    
    class RecipientPreferences {
    public:
        virtual ~RecipientPreferences() {}
        virtual GpgME::Key preferredCertificate( const KMime::Types::Mailbox& recipient, GpgME::Protocol protocol ) = 0;
        virtual void setPreferredCertificate( const KMime::Types::Mailbox& recipient, GpgME::Protocol protocol, const GpgME::Key& certificate ) = 0;
    };
    
    class KConfigBasedRecipientPreferences : public RecipientPreferences {
    public:
        explicit KConfigBasedRecipientPreferences( KSharedConfigPtr config );
        ~KConfigBasedRecipientPreferences();
        GpgME::Key preferredCertificate( const KMime::Types::Mailbox& recipient, GpgME::Protocol protocol );
        void setPreferredCertificate( const KMime::Types::Mailbox& recipient, GpgME::Protocol protocol, const GpgME::Key& certificate );
    private:
        Q_DISABLE_COPY( KConfigBasedRecipientPreferences )
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };
   
    class KConfigBasedSigningPreferences : public SigningPreferences {
    public:
        explicit KConfigBasedSigningPreferences( KSharedConfigPtr config );
        ~KConfigBasedSigningPreferences();
        GpgME::Key preferredCertificate( GpgME::Protocol protocol );
        void setPreferredCertificate(  GpgME::Protocol protocol, const GpgME::Key& certificate );
    private:
        Q_DISABLE_COPY( KConfigBasedSigningPreferences )
        class Private;
        kdtools::pimpl_ptr<Private> d;    
    };
    
    class CertificateResolver {
    public:
        static std::vector< std::vector<GpgME::Key> > resolveRecipients( const std::vector<KMime::Types::Mailbox> & recipients, GpgME::Protocol proto );
        static std::vector<GpgME::Key> resolveRecipient( const KMime::Types::Mailbox & recipient, GpgME::Protocol proto );

        
        static std::vector< std::vector<GpgME::Key> > resolveSigners( const std::vector<KMime::Types::Mailbox> & signers, GpgME::Protocol proto );
        static std::vector<GpgME::Key> resolveSigner( const KMime::Types::Mailbox & signer, GpgME::Protocol proto );
    };

}
}

#endif /* __KLEOPATRA_UISERVER_CERTIFICATERESOLVER_H__ */
