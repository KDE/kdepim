/* -*- mode: c++; c-basic-offset:4 -*-
    commands/changepassphrasecommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include "changepassphrasecommand.h"

#include "command_p.h"

#include <utils/formatting.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/changepasswdjob.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <kdebug.h>

#include <gpg-error.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;

class ChangePassphraseCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::ChangePassphraseCommand;
    ChangePassphraseCommand * q_func() const { return static_cast<ChangePassphraseCommand*>( q ); }
public:
    explicit Private( ChangePassphraseCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotResult( const Error & err );

private:
    void createJob();
    void startJob();
    void showErrorDialog( const Error & error );
    void showSuccessDialog();

private:
    GpgME::Key key;
    QPointer<ChangePasswdJob> job;
};


ChangePassphraseCommand::Private * ChangePassphraseCommand::d_func() { return static_cast<Private*>( d.get() ); }
const ChangePassphraseCommand::Private * ChangePassphraseCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

ChangePassphraseCommand::Private::Private( ChangePassphraseCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      key(),
      job()
{

}

ChangePassphraseCommand::Private::~Private() { kDebug(); }

ChangePassphraseCommand::ChangePassphraseCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

ChangePassphraseCommand::ChangePassphraseCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

ChangePassphraseCommand::ChangePassphraseCommand( const GpgME::Key & key )
    : Command( key, new Private( this, 0 ) )
{
    d->init();
}

void ChangePassphraseCommand::Private::init() {

}

ChangePassphraseCommand::~ChangePassphraseCommand() { kDebug(); }

void ChangePassphraseCommand::doStart() {

    const std::vector<Key> keys = d->keys();
    if ( keys.size() != 1 || !keys.front().hasSecret() ) {
        d->finished();
        return;
    }

    d->key = keys.front();

    d->createJob();
    d->startJob();

}

void ChangePassphraseCommand::Private::startJob() {
    const Error err = job
        ? job->start( key )
        : Error( gpg_error( GPG_ERR_NOT_SUPPORTED ) )
        ;
    if ( err ) {
        showErrorDialog( err );
        finished();
    }
}

void ChangePassphraseCommand::Private::slotResult( const Error & err ) {
    if ( err.isCanceled() )
        ;
    else if ( err )
        showErrorDialog( err );
    else
        showSuccessDialog();
    finished();
}

void ChangePassphraseCommand::doCancel() {
    kDebug();
    if ( d->job )
        d->job->slotCancel();
}

void ChangePassphraseCommand::Private::createJob() {
    assert( !job );

    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( key.protocol() );
    if ( !backend )
        return;

    ChangePasswdJob * const j = backend->changePasswdJob();
    if ( !j )
        return;

    connect( j, SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( j, SIGNAL(result(GpgME::Error)),
             q, SLOT(slotResult(GpgME::Error)) );

    job = j;
}

void ChangePassphraseCommand::Private::showErrorDialog( const Error & err ) {
    error( i18n("<p>An error occurred while trying to change "
                "the passphrase for <b>%1</b>:</p><p>%2</p>",
                Formatting::formatForComboBox( key ),
                QString::fromLocal8Bit( err.asString() ) ),
           i18n("Passphrase Change Error") );
}

void ChangePassphraseCommand::Private::showSuccessDialog() {
    information( i18n("Passphrase changed successfully."),
                 i18n("Passphrase Change Succeeded") );
}

#undef d
#undef q

#include "moc_changepassphrasecommand.cpp"
