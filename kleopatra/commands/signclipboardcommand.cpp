/* -*- mode: c++; c-basic-offset:4 -*-
    commands/signclipboardcommand.cpp

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

#include "signclipboardcommand.h"

#ifndef QT_NO_CLIPBOARD

#include "command_p.h"

#include <crypto/signemailcontroller.h>

#include <utils/input.h>
#include <utils/output.h>

#include <kleo/stl_util.h>

#include <KLocalizedString>
#include <qdebug.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include <exception>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Crypto;
using namespace boost;

class SignClipboardCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::SignClipboardCommand;
    SignClipboardCommand *q_func() const
    {
        return static_cast<SignClipboardCommand *>(q);
    }
public:
    explicit Private(SignClipboardCommand *qq, KeyListController *c);
    ~Private();

    void init();

private:
    void slotSignersResolved();
    void slotControllerDone()
    {
        finished();
    }
    void slotControllerError(int, const QString &)
    {
        finished();
    }

private:
    shared_ptr<const ExecutionContext> shared_qq;
    shared_ptr<Input> input;
    SignEMailController controller;
};

SignClipboardCommand::Private *SignClipboardCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const SignClipboardCommand::Private *SignClipboardCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

SignClipboardCommand::Private::Private(SignClipboardCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      shared_qq(qq, kdtools::nodelete()),
      input(),
      controller(SignEMailController::ClipboardMode)
{

}

SignClipboardCommand::Private::~Private()
{
    qDebug();
}

SignClipboardCommand::SignClipboardCommand(GpgME::Protocol protocol, KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
    d->controller.setProtocol(protocol);
}

SignClipboardCommand::SignClipboardCommand(GpgME::Protocol protocol, QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
    d->controller.setProtocol(protocol);
}

void SignClipboardCommand::Private::init()
{
    controller.setExecutionContext(shared_qq);
    controller.setDetachedSignature(false);
    connect(&controller, SIGNAL(done()), q, SLOT(slotControllerDone()));
    connect(&controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)));
}

SignClipboardCommand::~SignClipboardCommand()
{
    qDebug();
}

// static
bool SignClipboardCommand::canSignCurrentClipboard()
{
    if (const QClipboard *clip = QApplication::clipboard())
        if (const QMimeData *mime = clip->mimeData()) {
            return mime->hasText();
        }
    return false;
}

void SignClipboardCommand::doStart()
{

    try {

        // snapshot clipboard content here, in case it's being changed...
        d->input = Input::createFromClipboard();

        connect(&d->controller, SIGNAL(signersResolved()),
                this, SLOT(slotSignersResolved()));

        d->controller.startResolveSigners();

    } catch (const std::exception &e) {
        d->information(i18n("An error occurred: %1",
                            QString::fromLocal8Bit(e.what())),
                       i18n("Sign Clipboard Error"));
        d->finished();
    }
}

void SignClipboardCommand::Private::slotSignersResolved()
{
    try {
        controller.setInputAndOutput(input, Output::createFromClipboard());
        input.reset(); // no longer needed, so don't keep a reference
        controller.start();
    } catch (const std::exception &e) {
        information(i18n("An error occurred: %1",
                         QString::fromLocal8Bit(e.what())),
                    i18n("Sign Clipboard Error"));
        finished();
    }
}

void SignClipboardCommand::doCancel()
{
    qDebug();
    d->controller.cancel();
}

#undef d
#undef q

#include "moc_signclipboardcommand.cpp"

#endif // QT_NO_CLIPBOARD
