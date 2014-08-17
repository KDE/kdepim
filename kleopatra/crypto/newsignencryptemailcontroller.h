/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/newsignencryptemailcontroller.h

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

#ifndef __KLEOPATRA_CRYPTO_NEWSIGNENCRYPTEMAILCONTROLLER_H__
#define __KLEOPATRA_CRYPTO_NEWSIGNENCRYPTEMAILCONTROLLER_H__

#include <crypto/controller.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <vector>
#include <utility>

namespace KMime {
namespace Types {
    class Mailbox;
}
}

namespace GpgME {
}

namespace boost {
    template <typename T> class shared_ptr;
}

namespace Kleo {

    class Input;
    class Output;

namespace Crypto {


    class NewSignEncryptEMailController : public Controller {
        Q_OBJECT
    public:
        explicit NewSignEncryptEMailController( QObject * parent=0 );
        explicit NewSignEncryptEMailController( const boost::shared_ptr<ExecutionContext> & xc, QObject * parent=0 );
        ~NewSignEncryptEMailController();

        static const char * mementoName() { return "NewSignEncryptEMailController"; }

        // 1st stage inputs

        void setSubject( const QString & subject );
        void setProtocol( GpgME::Protocol proto );
        const char * protocolAsString() const;
        GpgME::Protocol protocol() const;

        void setSigning( bool sign );
        bool isSigning() const;

        void setEncrypting( bool encrypt );
        bool isEncrypting() const;

        void startResolveCertificates( const std::vector<KMime::Types::Mailbox> & recipients, const std::vector<KMime::Types::Mailbox> & senders );

        bool isResolvingInProgress() const;
        bool areCertificatesResolved() const;

        // 2nd stage inputs

        void setDetachedSignature( bool detached );

        void startSigning( const std::vector< boost::shared_ptr<Kleo::Input> > & inputs,
                           const std::vector< boost::shared_ptr<Kleo::Output> > & outputs );

        void startEncryption( const std::vector< boost::shared_ptr<Kleo::Input> > & inputs,
                              const std::vector< boost::shared_ptr<Kleo::Output> > & outputs );

    public Q_SLOTS:
        void cancel();

    Q_SIGNALS:
        void certificatesResolved();
        void reportMicAlg( const QString & micAlg );

    private:

        /* reimp */ void doTaskDone( const Task * task, const boost::shared_ptr<const Kleo::Crypto::Task::Result> & );

        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void slotDialogAccepted() )
        Q_PRIVATE_SLOT( d, void slotDialogRejected() )
        Q_PRIVATE_SLOT( d, void schedule() )
    };

} // Crypto
} // Kleo

#endif /* __KLEOPATRA_CRYPTO_NEWSIGNENCRYPTEMAILCONTROLLER_H__ */

