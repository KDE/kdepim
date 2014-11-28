/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/signemailcontroller.h

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

#ifndef __KLEOPATRA_CRYPTO_SIGNEMAILCONTROLLER_H__
#define __KLEOPATRA_CRYPTO_SIGNEMAILCONTROLLER_H__

#include <crypto/controller.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <vector>

namespace KMime
{
namespace Types
{
class Mailbox;
}
}

namespace boost
{
template <typename T> class shared_ptr;
}

namespace Kleo
{

class Input;
class Output;

namespace Crypto
{

class SignEMailController : public Controller
{
    Q_OBJECT
public:
    enum Mode {
        GpgOLMode,
        ClipboardMode,

        NumModes
    };

    explicit SignEMailController(Mode mode, QObject *parent = 0);
    explicit SignEMailController(const boost::shared_ptr<ExecutionContext> &xc, Mode mode, QObject *parent = 0);
    ~SignEMailController();

    Mode mode() const;

    void setProtocol(GpgME::Protocol proto);
    GpgME::Protocol protocol() const;
    //const char * protocolAsString() const;

    void startResolveSigners();
    void startResolveSigners(const std::vector<KMime::Types::Mailbox> &signers);

    void setDetachedSignature(bool detached);

    void setInputAndOutput(const boost::shared_ptr<Kleo::Input>   &input,
                           const boost::shared_ptr<Kleo::Output> &output);
    void setInputsAndOutputs(const std::vector< boost::shared_ptr<Kleo::Input> >   &inputs,
                             const std::vector< boost::shared_ptr<Kleo::Output> > &outputs);

    void start();

public Q_SLOTS:
    void cancel();

Q_SIGNALS:
    void signersResolved();
    void reportMicAlg(const QString &micalg);

private:
    /* reimp */ void doTaskDone(const Task *task, const boost::shared_ptr<const Task::Result> &result) Q_DECL_OVERRIDE;

    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotWizardSignersResolved())
    Q_PRIVATE_SLOT(d, void slotWizardCanceled())
    Q_PRIVATE_SLOT(d, void schedule())
};

} // Crypto
} // Kleo

#endif /* __KLEOPATRA_CRYPTO_SIGNEMAILCONTROLLER_H__ */

