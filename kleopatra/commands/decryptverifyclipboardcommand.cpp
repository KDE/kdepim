/* -*- mode: c++; c-basic-offset:4 -*-
    commands/decryptverifyclipboardcommand.cpp

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

#include "decryptverifyclipboardcommand.h"

#ifndef QT_NO_CLIPBOARD

#include "command_p.h"

#include <crypto/decryptverifyemailcontroller.h>

#include <utils/input.h>
#include <utils/output.h>

#include <utils/classify.h>
#include <utils/types.h>

#include <kleo/stl_util.h>

#include <KLocale>
#include <kdebug.h>

#include <exception>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Crypto;
using namespace boost;

class DecryptVerifyClipboardCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::DecryptVerifyClipboardCommand;
    DecryptVerifyClipboardCommand * q_func() const { return static_cast<DecryptVerifyClipboardCommand*>( q ); }
public:
    explicit Private( DecryptVerifyClipboardCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotControllerDone() {
        finished();
    }
    void slotControllerError( int, const QString & ) {
        finished();
    }

private:
    shared_ptr<const ExecutionContext> shared_qq;
    shared_ptr<Input> input;
    DecryptVerifyEMailController controller;
};


DecryptVerifyClipboardCommand::Private * DecryptVerifyClipboardCommand::d_func() { return static_cast<Private*>( d.get() ); }
const DecryptVerifyClipboardCommand::Private * DecryptVerifyClipboardCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

DecryptVerifyClipboardCommand::Private::Private( DecryptVerifyClipboardCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      shared_qq( qq, kdtools::nodelete() ),
      input(),
      controller()
{

}

DecryptVerifyClipboardCommand::Private::~Private() { kDebug(); }

DecryptVerifyClipboardCommand::DecryptVerifyClipboardCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

DecryptVerifyClipboardCommand::DecryptVerifyClipboardCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void DecryptVerifyClipboardCommand::Private::init() {
    controller.setExecutionContext( shared_qq );
    connect( &controller, SIGNAL(done()), q, SLOT(slotControllerDone()) );
    connect( &controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)) );
}

DecryptVerifyClipboardCommand::~DecryptVerifyClipboardCommand() { kDebug(); }

// static
bool DecryptVerifyClipboardCommand::canDecryptVerifyCurrentClipboard() {
    try {
        return Input::createFromClipboard()->classification()
            & (Class::CipherText|Class::ClearsignedMessage|Class::OpaqueSignature) ;
    } catch ( ... ) {}
    return false;
}

void DecryptVerifyClipboardCommand::doStart() {

    try {

        const shared_ptr<Input> input = Input::createFromClipboard();

        const unsigned int classification = input->classification();

        if ( classification & (Class::ClearsignedMessage|Class::OpaqueSignature) ) {
            d->controller.setOperation( Verify );
            d->controller.setVerificationMode( Opaque );
        } else if ( classification & Class::CipherText ) {
            d->controller.setOperation( DecryptVerify );
        } else {
            d->information( i18n("The clipboard does not appear to "
                                 "contain a signature or encrypted text."),
                            i18n("Decrypt/Verify Clipboard Error") );
            d->finished();
            return;
        }

        d->controller.setProtocol( findProtocol( classification ) );
        d->controller.setInput( input );
        d->controller.setOutput( Output::createFromClipboard() );

        d->controller.start();

    } catch ( const std::exception & e ) {
        d->information( i18n("An error occurred: %1",
                             QString::fromLocal8Bit( e.what() ) ),
                        i18n("Decrypt/Verify Clipboard Error") );
        d->finished();
    }
}

void DecryptVerifyClipboardCommand::doCancel() {
    kDebug();
    d->controller.cancel();
}

#undef d
#undef q

#include "moc_decryptverifyclipboardcommand.cpp"

#endif // QT_NO_CLIPBOARD
