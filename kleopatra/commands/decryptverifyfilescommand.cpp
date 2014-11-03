/* -*- mode: c++; c-basic-offset:4 -*-
    commands/decryptverifyfilescommand.cpp

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

#include "decryptverifyfilescommand.h"

#include "command_p.h"

#include <crypto/decryptverifyfilescontroller.h>

#include <utils/filedialog.h>

#include <kleo/stl_util.h>

#include <KLocalizedString>
#include <qdebug.h>

#include <QStringList>

#include <exception>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Crypto;
using namespace boost;

class DecryptVerifyFilesCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::DecryptVerifyFilesCommand;
    DecryptVerifyFilesCommand *q_func() const
    {
        return static_cast<DecryptVerifyFilesCommand *>(q);
    }
public:
    explicit Private(DecryptVerifyFilesCommand *qq, KeyListController *c);
    ~Private();

    QStringList selectFiles() const;

    void init();

private:
    void slotControllerDone()
    {
        finished();
    }
    void slotControllerError(int, const QString &)
    {
        finished();
    }

private:
    QStringList files;
    shared_ptr<const ExecutionContext> shared_qq;
    DecryptVerifyFilesController controller;
};

DecryptVerifyFilesCommand::Private *DecryptVerifyFilesCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const DecryptVerifyFilesCommand::Private *DecryptVerifyFilesCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

DecryptVerifyFilesCommand::Private::Private(DecryptVerifyFilesCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      files(),
      shared_qq(qq, kdtools::nodelete()),
      controller()
{
}

DecryptVerifyFilesCommand::Private::~Private()
{
    qDebug();
}

DecryptVerifyFilesCommand::DecryptVerifyFilesCommand(KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
}

DecryptVerifyFilesCommand::DecryptVerifyFilesCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

DecryptVerifyFilesCommand::DecryptVerifyFilesCommand(const QStringList &files, KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
    d->files = files;
}

DecryptVerifyFilesCommand::DecryptVerifyFilesCommand(const QStringList &files, QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
    d->files = files;
}

void DecryptVerifyFilesCommand::Private::init()
{
    controller.setExecutionContext(shared_qq);
    connect(&controller, SIGNAL(done()), q, SLOT(slotControllerDone()));
    connect(&controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)));
}

DecryptVerifyFilesCommand::~DecryptVerifyFilesCommand()
{
    qDebug();
}

void DecryptVerifyFilesCommand::setFiles(const QStringList &files)
{
    d->files = files;
}

void DecryptVerifyFilesCommand::setOperation(DecryptVerifyOperation op)
{
    try {
        d->controller.setOperation(op);
    } catch (...) {}
}

DecryptVerifyOperation DecryptVerifyFilesCommand::operation() const
{
    return d->controller.operation();
}

void DecryptVerifyFilesCommand::doStart()
{

    try {

        if (d->files.empty()) {
            d->files = d->selectFiles();
        }
        if (d->files.empty()) {
            d->finished();
            return;
        }
        d->controller.setFiles(d->files);
        d->controller.start();

    } catch (const std::exception &e) {
        d->information(i18n("An error occurred: %1",
                            QString::fromLocal8Bit(e.what())),
                       i18n("Decrypt/Verify Files Error"));
        d->finished();
    }
}

void DecryptVerifyFilesCommand::doCancel()
{
    qDebug();
    d->controller.cancel();
}

QStringList DecryptVerifyFilesCommand::Private::selectFiles() const
{
    return FileDialog::getOpenFileNames(parentWidgetOrView(), i18n("Select One or More Files to Decrypt and/or Verify"), QLatin1String("enc"));
}

#undef d
#undef q

#include "moc_decryptverifyfilescommand.cpp"
