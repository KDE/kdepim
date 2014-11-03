/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptfilescommand.cpp

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

#include "signencryptfilescommand.h"

#include <crypto/signencryptfilescontroller.h>

#include <kleo/exception.h>

#include <KLocalizedString>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class SignEncryptFilesCommand::Private : public QObject
{
    Q_OBJECT
private:
    friend class ::Kleo::SignEncryptFilesCommand;
    SignEncryptFilesCommand *const q;
public:
    explicit Private(SignEncryptFilesCommand *qq)
        : q(qq),
          controller()
    {

    }

private:
    void checkForErrors() const;

private Q_SLOTS:
    void slotDone();
    void slotError(int, const QString &);

private:
    shared_ptr<SignEncryptFilesController> controller;
};

SignEncryptFilesCommand::SignEncryptFilesCommand()
    : AssuanCommandMixin<SignEncryptFilesCommand>(), d(new Private(this))
{

}

SignEncryptFilesCommand::~SignEncryptFilesCommand() {}

void SignEncryptFilesCommand::Private::checkForErrors() const
{

    if (!q->numFiles())
        throw Exception(makeError(GPG_ERR_ASS_NO_INPUT),
                        i18n("At least one FILE must be present"));

    if (!q->senders().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QLatin1String("SENDER")));
    if (!q->recipients().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QLatin1String("RECIPIENT")));

    if (!q->inputs().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QLatin1String("INPUT")));
    if (!q->outputs().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QLatin1String("OUTPUT")));
    if (!q->messages().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QLatin1String("MESSAGE")));
}

int SignEncryptFilesCommand::doStart()
{

    d->checkForErrors();

    d->controller.reset(new SignEncryptFilesController(shared_from_this()));

    d->controller->setProtocol(checkProtocol(FileManager));

    unsigned int op = operation();
    if (hasOption("archive")) {
        op |= SignEncryptFilesController::ArchiveForced;
    } else {
        op |= SignEncryptFilesController::ArchiveAllowed;
    }
    d->controller->setOperationMode(op);
    d->controller->setFiles(fileNames());

    QObject::connect(d->controller.get(), SIGNAL(done()), d.get(), SLOT(slotDone()), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)), Qt::QueuedConnection);

    d->controller->start();

    return 0;
}

void SignEncryptFilesCommand::Private::slotDone()
{
    q->done();
}

void SignEncryptFilesCommand::Private::slotError(int err, const QString &details)
{
    q->done(err, details);
}

void SignEncryptFilesCommand::doCanceled()
{
    if (d->controller) {
        d->controller->cancel();
    }
}

#include "signencryptfilescommand.moc"

