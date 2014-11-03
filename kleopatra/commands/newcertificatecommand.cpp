/* -*- mode: c++; c-basic-offset:4 -*-
    commands/newcertificatecommand.cpp

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
    along with this program; if not, write to the Free Softwarls   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "newcertificatecommand.h"

#include "command_p.h"

#include <newcertificatewizard/newcertificatewizard.h>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;

class NewCertificateCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::NewCertificateCommand;
    NewCertificateCommand *q_func() const
    {
        return static_cast<NewCertificateCommand *>(q);
    }
public:
    explicit Private(NewCertificateCommand *qq, KeyListController *c);
    ~Private();

    void init();

private:
    void slotDialogRejected();
    void slotDialogAccepted();

private:
    void ensureDialogCreated();

private:
    Protocol protocol;
    QPointer<NewCertificateWizard> dialog;
};

NewCertificateCommand::Private *NewCertificateCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const NewCertificateCommand::Private *NewCertificateCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

NewCertificateCommand::Private::Private(NewCertificateCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      protocol(UnknownProtocol),
      dialog()
{

}

NewCertificateCommand::Private::~Private() {}

NewCertificateCommand::NewCertificateCommand(KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
}

NewCertificateCommand::NewCertificateCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

void NewCertificateCommand::Private::init()
{

}

NewCertificateCommand::~NewCertificateCommand() {}

void NewCertificateCommand::setProtocol(Protocol proto)
{
    d->protocol = proto;
    if (d->dialog) {
        d->dialog->setProtocol(proto);
    }
}

Protocol NewCertificateCommand::protocol() const
{
    if (d->dialog) {
        return d->dialog->protocol();
    } else {
        return d->protocol;
    }
}

void NewCertificateCommand::doStart()
{

    d->ensureDialogCreated();
    assert(d->dialog);

    if (d->protocol != UnknownProtocol) {
        d->dialog->setProtocol(d->protocol);
    }

    d->dialog->show();
}

void NewCertificateCommand::Private::slotDialogRejected()
{
    emit q->canceled();
    finished();
}

void NewCertificateCommand::Private::slotDialogAccepted()
{
    finished();
}

void NewCertificateCommand::doCancel()
{
    if (d->dialog) {
        d->dialog->close();
    }
}

void NewCertificateCommand::Private::ensureDialogCreated()
{
    if (dialog) {
        return;
    }

    dialog = new NewCertificateWizard;
    applyWindowID(dialog);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()));
    connect(dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()));
}

#undef d
#undef q

#include "moc_newcertificatecommand.cpp"
