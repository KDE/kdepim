/* -*- mode: c++; c-basic-offset:4 -*-
    commands/signencryptfilescommand.cpp

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

#include "signencryptfilescommand.h"

#include "command_p.h"

#include <crypto/signencryptfilescontroller.h>

#include <KLocale>
#include <KMessageBox>
#include <kdebug.h>

#include <QFileDialog>
#include <QStringList>

#include <exception>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Crypto;
using namespace boost;

namespace {
    struct nodelete {
        template <typename T>
        void operator()( const T * ) const {}
    };
}

class SignEncryptFilesCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::SignEncryptFilesCommand;
    SignEncryptFilesCommand * q_func() const { return static_cast<SignEncryptFilesCommand*>( q ); }
public:
    explicit Private( SignEncryptFilesCommand * qq, KeyListController * c );
    ~Private();

    QStringList selectFiles() const;

    void init();

private:
    void slotControllerDone() {
        finished();
    }
    void slotControllerError( int, const QString & ) {
        finished();
    }

private:
    QStringList files;
    shared_ptr<const ExecutionContext> shared_qq;
    SignEncryptFilesController controller;
};


SignEncryptFilesCommand::Private * SignEncryptFilesCommand::d_func() { return static_cast<Private*>( d.get() ); }
const SignEncryptFilesCommand::Private * SignEncryptFilesCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

SignEncryptFilesCommand::Private::Private( SignEncryptFilesCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      files(),
      shared_qq( qq, nodelete() ),
      controller()
{
    controller.setOperationMode( SignEncryptFilesController::SignAllowed | SignEncryptFilesController::EncryptAllowed );
}

SignEncryptFilesCommand::Private::~Private() { kDebug(); }

SignEncryptFilesCommand::SignEncryptFilesCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

SignEncryptFilesCommand::SignEncryptFilesCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void SignEncryptFilesCommand::Private::init() {
    controller.setExecutionContext( shared_qq );
    connect( &controller, SIGNAL(done()), q, SLOT(slotControllerDone()) );
    connect( &controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)) );
}

SignEncryptFilesCommand::~SignEncryptFilesCommand() { kDebug(); }

void SignEncryptFilesCommand::setFiles( const QStringList & files ) {
    d->files = files;
}

void SignEncryptFilesCommand::doStart() {

    try {

        if ( d->files.empty() )
            d->files = d->selectFiles();
        if ( d->files.empty() ) {
            d->finished();
            return;
        }

        d->controller.setFiles( d->files );
        d->controller.start();

    } catch ( const std::exception & e ) {
        KMessageBox::information( d->view(),
                                  i18n("An error occurred: %1",
                                       QString::fromLocal8Bit( e.what() ) ),
                                  i18n("Sign/Encrypt Files Error") );
        d->finished();
    }
}

void SignEncryptFilesCommand::doCancel() {
    kDebug();
    d->controller.cancel();
}

void SignEncryptFilesCommand::applyWindowID( QDialog * dlg ) const {
    if ( dlg )
        dlg->setParent( d->view(), dlg->windowFlags() );
}

QStringList SignEncryptFilesCommand::Private::selectFiles() const {
    return QFileDialog::getOpenFileNames( view(), i18n( "Select one of more files to sign and/or encrypt" ) );
}

#undef d
#undef q

#include "moc_signencryptfilescommand.cpp"
