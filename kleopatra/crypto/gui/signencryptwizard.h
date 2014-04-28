/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptwizard.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTWIZARD_H__
#define __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTWIZARD_H__

#include <crypto/gui/wizard.h>

#include <crypto/gui/signerresolvepage.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>
#include <KMime/kmime_header_parsing.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace GpgME {
    class Key;
}

class QFileInfo;
template <typename T> class QList;
typedef QList<QFileInfo> QFileInfoList;

namespace Kleo {
namespace Crypto {

    class Task;
    class TaskCollection;

namespace Gui {

    class ObjectsPage;
    class ResolveRecipientsPage;
    class ResultPage;
    class SignerResolvePage;

    class SignEncryptWizard : public Wizard {
        Q_OBJECT
    public:
        explicit SignEncryptWizard( QWidget * parent=0, Qt::WindowFlags f=0 );
        virtual ~SignEncryptWizard();

        enum Page {
            ResolveSignerPage=0,
            ObjectsPage,
            ResolveRecipientsPage,
            ResultPage
        };

        void setCommitPage( Page );

        GpgME::Protocol presetProtocol() const;
        void setPresetProtocol( GpgME::Protocol proto );

        GpgME::Protocol selectedProtocol() const;

        /// SignOrEncryptFiles mode subinterface
        //@{

        QFileInfoList resolvedFiles() const;        
        void setFiles( const QStringList & files );

        bool signingSelected() const;
        void setSigningSelected( bool selected );

        bool encryptionSelected() const;
        void setEncryptionSelected( bool selected );

        bool isSigningUserMutable() const;
        void setSigningUserMutable( bool isMutable );

        bool isEncryptionUserMutable() const;
        void setEncryptionUserMutable( bool isMutable );
       
        bool isMultipleProtocolsAllowed() const;
        void setMultipleProtocolsAllowed( bool allowed );

        //@}

        void setRecipients( const std::vector<KMime::Types::Mailbox> & recipients, const std::vector<KMime::Types::Mailbox> & encryptoToSelfRecipients );

        /** if true, the user is allowed to remove/add recipients via the UI.
         * Defaults to @p false.
         */
        bool recipientsUserMutable() const;
        void setRecipientsUserMutable( bool isMutable ); 

        void setSignersAndCandidates( const std::vector<KMime::Types::Mailbox> & signers, const std::vector< std::vector<GpgME::Key> > & keys );

        void setTaskCollection( const boost::shared_ptr<TaskCollection> & tasks );

        std::vector<GpgME::Key> resolvedCertificates() const;
        std::vector<GpgME::Key> resolvedSigners() const;

        bool isAsciiArmorEnabled() const;
        void setAsciiArmorEnabled( bool enabled );

        bool removeUnencryptedFile() const;
        void setRemoveUnencryptedFile( bool remove );

        bool keepResultPageOpenWhenDone() const;
        void setKeepResultPageOpenWhenDone( bool keep );

        /*reimp*/ void onNext( int currentId );

    Q_SIGNALS:
        void signersResolved();
        void objectsResolved();
        void recipientsResolved();
        void linkActivated( const QString & link );

    protected:
        Gui::SignerResolvePage* signerResolvePage();
        const Gui::SignerResolvePage* signerResolvePage() const;
        Gui::ObjectsPage* objectsPage();
        Gui::ResultPage* resultPage();
        Gui::ResolveRecipientsPage* resolveRecipientsPage();

        void setSignerResolvePageValidator( const boost::shared_ptr<SignerResolvePage::Validator>& validator );

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}
}
}

#endif /* __KLEOPATRA_CRYPTO_GUI_SIGNENCRYPTWIZARD_H__ */
