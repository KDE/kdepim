/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signcommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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
    you do not wish to do so, delete this exception statement from
    your version of the file, but you are not obligated to do so.  If
    your version.
*/

#include <config-kleopatra.h>

#include "signcommand.h"

#include <crypto/newsignencryptemailcontroller.h>

#include <utils/kleo_assert.h>
#include <utils/input.h>
#include <utils/output.h>

#include <kleo/exception.h>

#include <KLocalizedString>

#include <QTimer>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class SignCommand::Private : public QObject
{
    Q_OBJECT
private:
    friend class ::Kleo::SignCommand;
    SignCommand *const q;
public:
    explicit Private(SignCommand *qq)
        : q(qq), controller()
    {

    }

private:
    void checkForErrors() const;

private Q_SLOTS:
    void slotSignersResolved();
    void slotMicAlgDetermined(const QString &);
    void slotDone();
    void slotError(int, const QString &);

private:
    shared_ptr<NewSignEncryptEMailController> controller;
};

SignCommand::SignCommand()
    : AssuanCommandMixin<SignCommand>(), d(new Private(this))
{

}

SignCommand::~SignCommand() {}

void SignCommand::Private::checkForErrors() const
{

    if (q->numFiles())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("SIGN is an email mode command, connection seems to be in filemanager mode"));

    if (!q->recipients().empty() && !q->informativeRecipients())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("RECIPIENT may not be given prior to SIGN, except with --info"));

    if (q->inputs().empty())
        throw Exception(makeError(GPG_ERR_ASS_NO_INPUT),
                        i18n("At least one INPUT must be present"));

    if (q->outputs().size() != q->inputs().size())
        throw Exception(makeError(GPG_ERR_ASS_NO_INPUT),
                        i18n("INPUT/OUTPUT count mismatch"));

    if (!q->messages().empty())
        throw Exception(makeError(GPG_ERR_INV_VALUE),
                        i18n("MESSAGE command is not allowed before SIGN"));

    const shared_ptr<NewSignEncryptEMailController> m = q->mementoContent< shared_ptr<NewSignEncryptEMailController> >(NewSignEncryptEMailController::mementoName());

    if (m && m->isSigning()) {

        if (m->protocol() != q->checkProtocol(EMail))
            throw Exception(makeError(GPG_ERR_CONFLICT),
                            i18n("Protocol given conflicts with protocol determined by PREP_ENCRYPT in this session"));

        // ### check that any SENDER here is the same as the one for PREP_ENCRYPT

        // ### ditto RECIPIENT

    } else {

        // ### support the stupid "default signer" semantics of GpgOL
        // ### where SENDER is missing
        if (false)
            if (q->senders().empty() || q->informativeSenders())
                throw Exception(makeError(GPG_ERR_MISSING_VALUE),
                                i18n("No senders given, or only with --info"));

    }

}

static void connectController(const QObject *controller, const QObject *d)
{
    QObject::connect(controller, SIGNAL(certificatesResolved()), d, SLOT(slotSignersResolved()));
    QObject::connect(controller, SIGNAL(reportMicAlg(QString)), d, SLOT(slotMicAlgDetermined(QString)));
    QObject::connect(controller, SIGNAL(done()), d, SLOT(slotDone()));
    QObject::connect(controller, SIGNAL(error(int,QString)), d, SLOT(slotError(int,QString)));
}

int SignCommand::doStart()
{

    d->checkForErrors();

    const shared_ptr<NewSignEncryptEMailController> seec = mementoContent< shared_ptr<NewSignEncryptEMailController> >(NewSignEncryptEMailController::mementoName());

    if (seec && seec->isSigning()) {
        // reuse the controller from a previous PREP_ENCRYPT --expect-sign, if available:
        d->controller = seec;
        connectController(seec.get(), d.get());
        if (!seec->isEncrypting()) {
            removeMemento(NewSignEncryptEMailController::mementoName());
        }
        seec->setExecutionContext(shared_from_this());
        if (seec->areCertificatesResolved()) {
            QTimer::singleShot(0, d.get(), SLOT(slotSignersResolved()));
        } else {
            kleo_assert(seec->isResolvingInProgress());
        }
    } else {
        // use a new controller
        d->controller.reset(new NewSignEncryptEMailController(shared_from_this()));

        const QString session = sessionTitle();
        if (!session.isEmpty()) {
            d->controller->setSubject(session);
        }

        d->controller->setSigning(true);
        d->controller->setEncrypting(false);
        d->controller->setProtocol(checkProtocol(EMail, AssuanCommand::AllowProtocolMissing));
        connectController(d->controller.get(), d.get());
        d->controller->startResolveCertificates(recipients(), senders());
    }

    return 0;
}

void SignCommand::Private::slotSignersResolved()
{
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<NewSignEncryptEMailController> cont(controller);

    try {
        const QString sessionTitle = q->sessionTitle();
        if (!sessionTitle.isEmpty())
            Q_FOREACH (const shared_ptr<Input> &i, q->inputs()) {
                i->setLabel(sessionTitle);
            }

        cont->setDetachedSignature(q->hasOption("detached"));
        cont->startSigning(q->inputs(), q->outputs());

        return;

    } catch (const Exception &e) {
        q->done(e.error(), e.message());
    } catch (const std::exception &e) {
        q->done(makeError(GPG_ERR_UNEXPECTED),
                i18n("Caught unexpected exception in SignCommand::Private::slotRecipientsResolved: %1",
                     QString::fromLocal8Bit(e.what())));
    } catch (...) {
        q->done(makeError(GPG_ERR_UNEXPECTED),
                i18n("Caught unknown exception in SignCommand::Private::slotRecipientsResolved"));
    }
    cont->cancel();
}

void SignCommand::Private::slotMicAlgDetermined(const QString &micalg)
{
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<NewSignEncryptEMailController> cont(controller);

    try {

        q->sendStatus("MICALG", micalg);
        return;

    } catch (const Exception &e) {
        q->done(e.error(), e.message());
    } catch (const std::exception &e) {
        q->done(makeError(GPG_ERR_UNEXPECTED),
                i18n("Caught unexpected exception in SignCommand::Private::slotMicAlgDetermined: %1",
                     QString::fromLocal8Bit(e.what())));
    } catch (...) {
        q->done(makeError(GPG_ERR_UNEXPECTED),
                i18n("Caught unknown exception in SignCommand::Private::slotMicAlgDetermined"));
    }
    cont->cancel();
}

void SignCommand::Private::slotDone()
{
    q->done();
}

void SignCommand::Private::slotError(int err, const QString &details)
{
    q->done(err, details);
}

void SignCommand::doCanceled()
{
    if (d->controller) {
        d->controller->cancel();
    }
}

#include "signcommand.moc"
