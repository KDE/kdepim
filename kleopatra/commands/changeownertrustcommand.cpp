/* -*- mode: c++; c-basic-offset:4 -*-
    commands/changeexpirycommand.cpp

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

#include <config-kleopatra.h>

#include "changeownertrustcommand.h"

#include "command_p.h"

#include <dialogs/ownertrustdialog.h>

#include <utils/formatting.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/changeownertrustjob.h>

#include <gpgme++/key.h>

#include <KLocalizedString>
#include <qdebug.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;

class ChangeOwnerTrustCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::ChangeOwnerTrustCommand;
    ChangeOwnerTrustCommand *q_func() const
    {
        return static_cast<ChangeOwnerTrustCommand *>(q);
    }
public:
    explicit Private(ChangeOwnerTrustCommand *qq, KeyListController *c);
    ~Private();

    void init();

private:
    void slotDialogAccepted();
    void slotDialogRejected();
    void slotResult(const Error &err);

private:
    void ensureDialogCreated();
    void createJob();
    void showErrorDialog(const Error &error);
    void showSuccessDialog();

private:
    QPointer<OwnerTrustDialog> dialog;
    QPointer<ChangeOwnerTrustJob> job;
};

ChangeOwnerTrustCommand::Private *ChangeOwnerTrustCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const ChangeOwnerTrustCommand::Private *ChangeOwnerTrustCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

ChangeOwnerTrustCommand::Private::Private(ChangeOwnerTrustCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      dialog(),
      job()
{

}

ChangeOwnerTrustCommand::Private::~Private()
{
    qDebug();
}

ChangeOwnerTrustCommand::ChangeOwnerTrustCommand(KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
}

ChangeOwnerTrustCommand::ChangeOwnerTrustCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

ChangeOwnerTrustCommand::ChangeOwnerTrustCommand(const Key &key)
    : Command(key, new Private(this, 0))
{
    d->init();
}

void ChangeOwnerTrustCommand::Private::init()
{

}

ChangeOwnerTrustCommand::~ChangeOwnerTrustCommand()
{
    qDebug();
}

void ChangeOwnerTrustCommand::doStart()
{

    if (d->keys().size() != 1) {
        d->finished();
        return;
    }

    const Key key = d->key();
    if (key.protocol() != GpgME::OpenPGP || (key.hasSecret() && key.ownerTrust() == Key::Ultimate)) {
        d->finished();
        return;
    }

    d->ensureDialogCreated();
    assert(d->dialog);

    d->dialog->setHasSecretKey(key.hasSecret());
    d->dialog->setFormattedCertificateName(Formatting::formatForComboBox(key));
    d->dialog->setOwnerTrust(key.ownerTrust());

    d->dialog->show();

}

void ChangeOwnerTrustCommand::Private::slotDialogAccepted()
{
    assert(dialog);

    const Key::OwnerTrust trust = dialog->ownerTrust();

    qDebug() << "trust " << trust;

    createJob();
    assert(job);

    if (const Error err = job->start(key(), trust)) {
        showErrorDialog(err);
        finished();
    }
}

void ChangeOwnerTrustCommand::Private::slotDialogRejected()
{
    emit q->canceled();
    finished();
}

void ChangeOwnerTrustCommand::Private::slotResult(const Error &err)
{
    if (err.isCanceled())
        ;
    else if (err) {
        showErrorDialog(err);
    } else {
        showSuccessDialog();
    }
    finished();
}

void ChangeOwnerTrustCommand::doCancel()
{
    qDebug();
    if (d->job) {
        d->job->slotCancel();
    }
}

void ChangeOwnerTrustCommand::Private::ensureDialogCreated()
{
    if (dialog) {
        return;
    }

    dialog = new OwnerTrustDialog;
    applyWindowID(dialog);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()));
    connect(dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()));
}

void ChangeOwnerTrustCommand::Private::createJob()
{
    assert(!job);

    const CryptoBackend::Protocol *const backend = CryptoBackendFactory::instance()->protocol(key().protocol());
    if (!backend) {
        return;
    }

    ChangeOwnerTrustJob *const j = backend->changeOwnerTrustJob();
    if (!j) {
        return;
    }

    connect(j, SIGNAL(progress(QString,int,int)),
            q, SIGNAL(progress(QString,int,int)));
    connect(j, SIGNAL(result(GpgME::Error)),
            q, SLOT(slotResult(GpgME::Error)));

    job = j;
}

void ChangeOwnerTrustCommand::Private::showErrorDialog(const Error &err)
{
    error(i18n("<p>An error occurred while trying to change "
               "the owner trust for <b>%1</b>:</p><p>%2</p>",
               Formatting::formatForComboBox(key()),
               QString::fromLocal8Bit(err.asString())),
          i18n("Owner Trust Change Error"));
}

void ChangeOwnerTrustCommand::Private::showSuccessDialog()
{
    information(i18n("Owner trust changed successfully."),
                i18n("Owner Trust Change Succeeded"));
}

#undef d
#undef q

#include "moc_changeownertrustcommand.cpp"
