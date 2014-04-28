/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signerresolvepage.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_H__
#define __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_H__

#include <crypto/gui/wizardpage.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>
#include <KMime/kmime_header_parsing.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace GpgME {
    class Key;
}

namespace Kleo {
namespace Crypto {

    class SigningPreferences;

namespace Gui {
    class SignerResolvePage : public WizardPage {
        Q_OBJECT
    public:
        explicit SignerResolvePage( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~SignerResolvePage();

        void setSignersAndCandidates( const std::vector<KMime::Types::Mailbox> & signers,
                                      const std::vector< std::vector<GpgME::Key> > & keys );

        std::vector<GpgME::Key> resolvedSigners() const;
        std::vector<GpgME::Key> signingCertificates( GpgME::Protocol protocol = GpgME::UnknownProtocol ) const;


        /*reimpl*/ bool isComplete() const;

        bool encryptionSelected() const;
        void setEncryptionSelected( bool selected );

        bool signingSelected() const;
        void setSigningSelected( bool selected );

        bool isEncryptionUserMutable() const;
        void setEncryptionUserMutable( bool ismutable );

        bool isSigningUserMutable() const;
        void setSigningUserMutable( bool ismutable );

        bool isAsciiArmorEnabled() const;
        void setAsciiArmorEnabled( bool enabled );

        bool removeUnencryptedFile() const;
        void setRemoveUnencryptedFile( bool remove );

        void setPresetProtocol( GpgME::Protocol protocol );
        void setPresetProtocols( const std::vector<GpgME::Protocol>& protocols );

        std::vector<GpgME::Protocol> selectedProtocols() const;

        std::vector<GpgME::Protocol> selectedProtocolsWithoutSigningCertificate() const;

        void setMultipleProtocolsAllowed( bool allowed );
        bool multipleProtocolsAllowed() const;

        void setProtocolSelectionUserMutable( bool ismutable );
        bool protocolSelectionUserMutable() const;

        enum Operation {
            SignAndEncrypt=0,
            SignOnly,
            EncryptOnly
        };

        Operation operation() const;

        class Validator
        {
        public:
            virtual ~Validator() {}
            virtual bool isComplete() const = 0;
            virtual QString explanation() const = 0;
            /**
             * returns a custom window title, or a null string if no custom
             * title is required.
             * (use this if the title needs dynamic adaption
             * depending on the user's selection)
             */
            virtual QString customWindowTitle() const = 0;
        };

        void setValidator( const boost::shared_ptr<Validator>& );
        boost::shared_ptr<Validator> validator() const;

        void setSigningPreferences( const boost::shared_ptr<SigningPreferences>& prefs );
        boost::shared_ptr<SigningPreferences> signingPreferences() const;

    private:
        /*reimpl*/ void onNext();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;

        Q_PRIVATE_SLOT( d, void operationButtonClicked( int ) )
        Q_PRIVATE_SLOT( d, void selectCertificates() )
        Q_PRIVATE_SLOT( d, void updateUi() )
    };
}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_SIGNERRESOLVEPAGE_H__

