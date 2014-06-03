/* -*- mode: c++; c-basic-offset:4 -*-
    commands/encryptclipboardcommand.cpp

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

#include "encryptclipboardcommand.h"

#ifndef QT_NO_CLIPBOARD

#include "command_p.h"

#include <crypto/encryptemailcontroller.h>

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

class EncryptClipboardCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::EncryptClipboardCommand;
    EncryptClipboardCommand * q_func() const { return static_cast<EncryptClipboardCommand*>( q ); }
public:
    explicit Private( EncryptClipboardCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotRecipientsResolved();
    void slotControllerDone() {
        finished();
    }
    void slotControllerError( int, const QString & ) {
        finished();
    }

private:
    shared_ptr<const ExecutionContext> shared_qq;
    shared_ptr<Input> input;
    EncryptEMailController controller;
};


EncryptClipboardCommand::Private * EncryptClipboardCommand::d_func() { return static_cast<Private*>( d.get() ); }
const EncryptClipboardCommand::Private * EncryptClipboardCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

EncryptClipboardCommand::Private::Private( EncryptClipboardCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      shared_qq( qq, kdtools::nodelete() ),
      input(),
      controller( EncryptEMailController::ClipboardMode )
{

}

EncryptClipboardCommand::Private::~Private() { qDebug(); }

EncryptClipboardCommand::EncryptClipboardCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

EncryptClipboardCommand::EncryptClipboardCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void EncryptClipboardCommand::Private::init() {
    controller.setExecutionContext( shared_qq );
    connect( &controller, SIGNAL(done()), q, SLOT(slotControllerDone()) );
    connect( &controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)) );
}

EncryptClipboardCommand::~EncryptClipboardCommand() { qDebug(); }

// static
bool EncryptClipboardCommand::canEncryptCurrentClipboard() {
    if ( const QClipboard * clip = QApplication::clipboard() )
        if ( const QMimeData * mime = clip->mimeData() )
            return mime->hasText();
    return false;
}

void EncryptClipboardCommand::doStart() {

    try {

        // snapshot clipboard content here, in case it's being changed...
        d->input = Input::createFromClipboard();

        connect( &d->controller, SIGNAL(recipientsResolved()),
                 this, SLOT(slotRecipientsResolved()) );

        d->controller.startResolveRecipients();

    } catch ( const std::exception & e ) {
        d->information( i18n("An error occurred: %1",
                             QString::fromLocal8Bit( e.what() ) ),
                        i18n("Encrypt Clipboard Error") );
        d->finished();
    }
}

void EncryptClipboardCommand::Private::slotRecipientsResolved() {
    try {
        controller.setInputAndOutput( input, Output::createFromClipboard() );
        input.reset(); // no longer needed, so don't keep a reference
        controller.start();
    } catch ( const std::exception & e ) {
        information( i18n("An error occurred: %1",
                          QString::fromLocal8Bit( e.what() ) ),
                     i18n("Encrypt Clipboard Error") );
        finished();
    }
}

void EncryptClipboardCommand::doCancel() {
    qDebug();
    d->controller.cancel();
}

#undef d
#undef q

#include "moc_encryptclipboardcommand.cpp"

#endif // QT_NO_CLIPBOARD
