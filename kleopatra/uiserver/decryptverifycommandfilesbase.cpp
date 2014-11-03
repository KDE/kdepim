/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifycommandfilesbase.cpp

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

#include "decryptverifycommandfilesbase.h"

#include <crypto/decryptverifytask.h>

#include <crypto/decryptverifyfilescontroller.h>

#include <models/keycache.h>

#include <utils/formatting.h>
#include <utils/hex.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>

#include <kleo/stl_util.h>
#include <kleo/exception.h>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <KLocalizedString>

#include <QFileInfo>

#include <gpg-error.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Formatting;
using namespace GpgME;
using namespace boost;

class DecryptVerifyCommandFilesBase::Private : public QObject
{
    Q_OBJECT
    friend class ::Kleo::DecryptVerifyCommandFilesBase;
    DecryptVerifyCommandFilesBase *const q;
public:
    explicit Private(DecryptVerifyCommandFilesBase *qq)
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
    shared_ptr<DecryptVerifyFilesController> controller;
};

DecryptVerifyCommandFilesBase::DecryptVerifyCommandFilesBase()
    : AssuanCommandMixin<DecryptVerifyCommandFilesBase>(), d(new Private(this))
{

}

DecryptVerifyCommandFilesBase::~DecryptVerifyCommandFilesBase() {}

int DecryptVerifyCommandFilesBase::doStart()
{

    d->checkForErrors();

    d->controller.reset(new DecryptVerifyFilesController(shared_from_this()));

    d->controller->setOperation(operation());
    d->controller->setFiles(fileNames());

    QObject::connect(d->controller.get(), SIGNAL(done()),
                     d.get(), SLOT(slotDone()), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(error(int,QString)),
                     d.get(), SLOT(slotError(int,QString)), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(verificationResult(GpgME::VerificationResult)),
                     d.get(), SLOT(verificationResult(GpgME::VerificationResult)), Qt::QueuedConnection);

    d->controller->start();

    return 0;
}

namespace
{

struct is_file : std::unary_function<QString, bool> {
    bool operator()(const QString &file) const
    {
        return QFileInfo(file).isFile();
    }
};

}

void DecryptVerifyCommandFilesBase::Private::checkForErrors() const
{
    if (!q->senders().empty())
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                              i18n("Cannot use SENDER"));

    if (!q->recipients().empty())
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT),
                              i18n("Cannot use RECIPIENT"));

    const unsigned int numInputs = q->inputs().size();
    const unsigned int numMessages = q->messages().size();
    const unsigned int numOutputs  = q->outputs().size();

    if (numInputs) {
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT), i18n("INPUT present"));
    }
    if (numMessages) {
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT), i18n("MESSAGE present"));
    }
    if (numOutputs) {
        throw Kleo::Exception(q->makeError(GPG_ERR_CONFLICT), i18n("OUTPUT present"));
    }
    const QStringList fileNames = q->fileNames();
    if (fileNames.empty())
        throw Exception(makeError(GPG_ERR_ASS_NO_INPUT),
                        i18n("At least one FILE must be present"));
    if (!kdtools::all(fileNames, is_file()))
        throw Exception(makeError(GPG_ERR_INV_ARG),
                        i18n("DECRYPT/VERIFY_FILES cannot use directories as input"));

}

void DecryptVerifyCommandFilesBase::doCanceled()
{
    if (d->controller) {
        d->controller->cancel();
    }
}

void DecryptVerifyCommandFilesBase::Private::slotProgress(const QString &what, int current, int total)
{
    Q_UNUSED(what);
    Q_UNUSED(current);
    Q_UNUSED(total);
    // ### FIXME report progress, via sendStatus()
}

void DecryptVerifyCommandFilesBase::Private::verificationResult(const VerificationResult &vResult)
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

#include "decryptverifycommandfilesbase.moc"
