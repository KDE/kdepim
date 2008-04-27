/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifyemailcontroller.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_UISERVER_DECRYPTVERIFYEMAILCONTROLLER_H__
#define __KLEOPATRA_UISERVER_DECRYPTVERIFYEMAILCONTROLLER_H__

#include <crypto/controller.h>

#include <utils/types.h>

#include <gpgme++/global.h>

#include <QMetaType>

#include <boost/shared_ptr.hpp>

#include <vector>

class QFile;

namespace GpgME {
    class VerificationResult;
}

namespace Kleo {

class AssuanCommand;
class Input;
class Output;

namespace Crypto {

class DecryptVerifyResult;

class DecryptVerifyEMailController : public Controller {
    Q_OBJECT
public:

    explicit DecryptVerifyEMailController( QObject * parent=0 );
    explicit DecryptVerifyEMailController( const boost::shared_ptr<const ExecutionContext> & cmd, QObject * parent=0 );
 
    ~DecryptVerifyEMailController();
    void setInputs( const std::vector<boost::shared_ptr<Input> > & inputs );
    void setSignedData( const std::vector<boost::shared_ptr<Input> > & data );
    void setOutputs( const std::vector<boost::shared_ptr<Output> > & outputs );
    void setWizardShown( bool shown );

    void setOperation( DecryptVerifyOperation operation );
    void setVerificationMode( VerificationMode vm );
    void setProtocol( GpgME::Protocol protocol );

    void start();

public Q_SLOTS:
    void cancel();

Q_SIGNALS:
    void verificationResult( const GpgME::VerificationResult & );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT( d, void slotWizardCanceled() )
    Q_PRIVATE_SLOT( d, void slotTaskDone( boost::shared_ptr<const Kleo::Crypto::DecryptVerifyResult> ) )
    Q_PRIVATE_SLOT( d, void schedule() )
};

} //namespace Crypto
} //namespace Kleo

Q_DECLARE_METATYPE( GpgME::VerificationResult )

#endif // __KLEOPATRA_UISERVER_DECRYPTVERIFYEMAILCONTROLLER_H__
