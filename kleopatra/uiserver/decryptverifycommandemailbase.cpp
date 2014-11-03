/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifycommandemailbase.cpp

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

#include <config-kleopatra.h>

#include "decryptverifycommandemailbase.h"

#include <crypto/decryptverifytask.h>
#include <crypto/decryptverifyemailcontroller.h>

#include <utils/formatting.h>

#include <models/keycache.h>

#include <utils/hex.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>

#include <kleo/exception.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>

#include <KLocalizedString>

#include <gpg-error.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Formatting;
using namespace GpgME;
using namespace boost;

class DecryptVerifyCommandEMailBase::Private : public QObject
{
    Q_OBJECT
    friend class ::Kleo::DecryptVerifyCommandEMailBase;
    DecryptVerifyCommandEMailBase *const q;
public:
    explicit Private(DecryptVerifyCommandEMailBase *qq)
        : QObject(),
          q(qq),
          controller()
    {
    }

    ~Private()
    {
    }

    void checkForErrors() const;

public Q_SLOTS:
    void slotProgress(const QString &what, int current, int total);
    void verificationResult(const GpgME::VerificationResult &);
    void slotDone()
    {
        q->done();
    }
    void slotError(int err, const QString &details)
    {
        q->done(err, details);
    }

public:

private:
    shared_ptr<DecryptVerifyEMailController> controller;
};

DecryptVerifyCommandEMailBase::DecryptVerifyCommandEMailBase()
    : AssuanCommandMixin<DecryptVerifyCommandEMailBase>(), d(new Private(this))
{

}

DecryptVerifyCommandEMailBase::~DecryptVerifyCommandEMailBase() {}

int DecryptVerifyCommandEMailBase::doStart()
{

    d->checkForErrors();

    d->controller.reset(new DecryptVerifyEMailController(shared_from_this()));

    const QString st = sessionTitle();
    if (!st.isEmpty())
        Q_FOREACH (const shared_ptr<Input> &i, inputs()) {
            i->setLabel(st);
        }

    d->controller->setSessionId(sessionId());
    d->controller->setOperation(operation());
    d->controller->setVerificationMode(messages().empty() ? Opaque : Detached);
    d->controller->setInputs(inputs());
    d->controller->setSignedData(messages());
    d->controller->setOutputs(outputs());
    d->controller->setWizardShown(!hasOption("silent"));
    d->controller->setProtocol(checkProtocol(mode()));
    if (informativeSenders()) {
        d->controller->setInformativeSenders(senders());
    }
    QObject::connect(d->controller.get(), SIGNAL(done()),
                     d.get(), SLOT(slotDone()), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(error(int,QString)),
                     d.get(), SLOT(slotError(int,QString)), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(verificationResult(GpgME::VerificationResult)),
                     d.get(), SLOT(verificationResult(GpgME::VerificationResult)), Qt::QueuedConnection);

    d->controller->start();

    return 0;
}

void DecryptVerifyCommandEMailBase::Private::checkForErrors() const
{
    if (!q->senders().empty() && !q->informativeSenders())
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                              i18n("Cannot use non-info SENDER"));

    if (!q->recipients().empty() && !q->informativeRecipients())
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                              i18n("Cannot use non-info RECIPIENT"));

    // ### use informative recipients and senders

    const unsigned int numInputs = q->inputs().size();
    const unsigned int numMessages = q->messages().size();
    const unsigned int numOutputs  = q->outputs().size();
    const unsigned int numInformativeSenders = q->informativeSenders() ? q->senders().size() : 0;

    const DecryptVerifyOperation op = q->operation();;
    const GpgME::Protocol proto = q->checkProtocol(q->mode());

    const unsigned int numFiles = q->numFiles();

    if (numFiles) {
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT), i18n("FILES present"));
    }

    if (!numInputs)
        throw Kleo::Exception(q->makeError(GPG_ERR_ASS_NO_INPUT),
                              i18n("At least one INPUT needs to be provided"));

    if (numInformativeSenders != 0)
        if (numInformativeSenders != numInputs)
            throw Kleo::Exception(q->makeError(GPG_ERR_ASS_NO_INPUT),     //TODO use better error code if possible
                                  i18n("INPUT/SENDER --info count mismatch"));

    if (numMessages) {
        if (numMessages != numInputs)
            throw Kleo::Exception(q->makeError(GPG_ERR_ASS_NO_INPUT),     //TODO use better error code if possible
                                  i18n("INPUT/MESSAGE count mismatch"));
        else if (op != Verify)
            throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                                  i18n("MESSAGE can only be given for detached signature verification"));
    }

    if (numOutputs) {
        if (numOutputs != numInputs)
            throw Kleo::Exception(q->makeError(GPG_ERR_ASS_NO_OUTPUT),    //TODO use better error code if possible
                                  i18n("INPUT/OUTPUT count mismatch"));
        else if (numMessages)
            throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                                  i18n("Cannot use OUTPUT and MESSAGE simultaneously"));
    }

    kleo_assert(proto != UnknownProtocol);

    const CryptoBackend::Protocol *const backend = CryptoBackendFactory::instance()->protocol(proto);
    if (!backend)
        throw Kleo::Exception(q->makeError(GPG_ERR_UNSUPPORTED_PROTOCOL),
                              proto == OpenPGP ? i18n("No backend support for OpenPGP") :
                              proto == CMS     ? i18n("No backend support for S/MIME") : QString());
}

void DecryptVerifyCommandEMailBase::doCanceled()
{
    if (d->controller) {
        d->controller->cancel();
    }
}

void DecryptVerifyCommandEMailBase::Private::slotProgress(const QString &what, int current, int total)
{
    Q_UNUSED(what);
    Q_UNUSED(current);
    Q_UNUSED(total);
    // ### FIXME report progress, via sendStatus()
}

void DecryptVerifyCommandEMailBase::Private::verificationResult(const VerificationResult &vResult)
{
    try {
        const std::vector<Signature> sigs = vResult.signatures();
        const std::vector<Key> signers = KeyCache::instance()->findSigners(vResult);
        Q_FOREACH (const Signature &sig, sigs) {
            const QString s = signatureToString(sig, DecryptVerifyResult::keyForSignature(sig, signers));
            const char *color = summaryToString(sig.summary());
            q->sendStatusEncoded("SIGSTATUS",
                                 color + (' ' + hexencode(s.toUtf8().constData())));
        }
    } catch (...) {}
}

#include "decryptverifycommandemailbase.moc"
