/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptwizard.h

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

#ifndef __KLEOPATRA_UISERVER_SIGNENCRYPTWIZARD_H__
#define __KLEOPATRA_UISERVER_SIGNENCRYPTWIZARD_H__

#include <KDialog>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace GpgME {
    class Key;
}

namespace KMime {
namespace Types {
    class Mailbox;
}
}

class QFileInfo;
template <typename T> class QList;
typedef QList<QFileInfo> QFileInfoList;

namespace Kleo {

    class Task;

    class SignEncryptWizard : public KDialog {
        Q_OBJECT
    public:
        explicit SignEncryptWizard( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~SignEncryptWizard();

        enum Page {
            NoPage=0,
            PreparationPage,
            ObjectsPage,
            ResolveRecipientsPage,
            ResolveSignerPage,
            ResultPage
        };

        enum Mode {
            EncryptEMail,
            SignEMail,
            EncryptOrSignFiles,
            SignOrEncryptFiles = EncryptOrSignFiles
        };
        void setMode( Mode mode );

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

        void setRecipients( const std::vector<KMime::Types::Mailbox> & recipients );
        void setSignersAndCandidates( const std::vector<KMime::Types::Mailbox> & signers, const std::vector< std::vector<GpgME::Key> > & keys );

        bool canGoToNextPage() const;

        void connectTask( const boost::shared_ptr<Task> & task, unsigned int idx );

        std::vector<GpgME::Key> resolvedCertificates() const;
        std::vector<GpgME::Key> resolvedSigners() const;

        Page currentPage() const;

    public Q_SLOTS:
        void next();

    Q_SIGNALS:
        //void operationResolved();
        void signersResolved();
        void objectsResolved();
        void recipientsResolved();
        void canceled();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void updateButtonStates() )
        Q_PRIVATE_SLOT( d, void selectedProtocolChanged() );
    };

}

#endif /* __KLEOPATRA_UISERVER_SIGNENCRYPTWIZARD_H__ */
