/* -*- mode: c++; c-basic-offset:4 -*-
    signerresolvepage.h

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

#ifndef __KLEOPATRA_SIGNERRESOLVEPAGE_H__
#define __KLEOPATRA_SIGNERRESOLVEPAGE_H__ 

#include "wizardpage.h"

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>
#include <kmime/kmime_header_parsing.h>

#include <boost/shared_ptr.hpp>

namespace GpgME {
    class Key;
}

namespace Kleo {

    class SignerResolvePage : public WizardPage {
        Q_OBJECT
    public:
        explicit SignerResolvePage( QWidget * parent=0, Qt::WFlags f=0 );
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

        void setProtocol( GpgME::Protocol protocol );
        GpgME::Protocol protocol() const;
 
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
        };

        void setValidator( const boost::shared_ptr<Validator>& );
        boost::shared_ptr<Validator> validator() const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;

        Q_PRIVATE_SLOT( d, void setOperation( int ) )
        Q_PRIVATE_SLOT( d, void selectCertificates() )
        Q_PRIVATE_SLOT( d, void updateUi() )
    };
}

#endif // __KLEOPATRA_SIGNERRESOLVEPAGE_H__

