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

#include "changeexpirycommand.h"

#include "command_p.h"

#include <dialogs/expirydialog.h>

#include <utils/formatting.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>
#include <kleo/changeexpiryjob.h>

#include <gpgme++/key.h>

#include <KLocalizedString>
#include <kdebug.h>

#include <QDateTime>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace GpgME;

class ChangeExpiryCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::ChangeExpiryCommand;
    ChangeExpiryCommand * q_func() const { return static_cast<ChangeExpiryCommand*>( q ); }
public:
    explicit Private( ChangeExpiryCommand * qq, KeyListController * c );
    ~Private();

    void init();

private:
    void slotDialogAccepted();
    void slotDialogRejected();
    void slotResult( const Error & err );

private:
    void ensureDialogCreated();
    void createJob();
    void showErrorDialog( const Error & error );
    void showSuccessDialog();

private:
    GpgME::Key key;
    QPointer<ExpiryDialog> dialog;
    QPointer<ChangeExpiryJob> job;
};


ChangeExpiryCommand::Private * ChangeExpiryCommand::d_func() { return static_cast<Private*>( d.get() ); }
const ChangeExpiryCommand::Private * ChangeExpiryCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

ChangeExpiryCommand::Private::Private( ChangeExpiryCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      key(),
      dialog(),
      job()
{

}

ChangeExpiryCommand::Private::~Private() { kDebug(); }

ChangeExpiryCommand::ChangeExpiryCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

ChangeExpiryCommand::ChangeExpiryCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

ChangeExpiryCommand::ChangeExpiryCommand( const GpgME::Key & key )
    : Command( key, new Private( this, 0 ) )
{
    d->init();
}

void ChangeExpiryCommand::Private::init() {

}

ChangeExpiryCommand::~ChangeExpiryCommand() { kDebug(); }

void ChangeExpiryCommand::doStart() {

    const std::vector<Key> keys = d->keys();
    if ( keys.size() != 1 ||
         keys.front().protocol() != GpgME::OpenPGP ||
         !keys.front().hasSecret() ||
         keys.front().subkey(0).isNull() ) {
        d->finished();
        return;
    }

    d->key = keys.front();

    d->ensureDialogCreated();
    assert( d->dialog );
    const Subkey subkey = d->key.subkey(0);
    d->dialog->setDateOfExpiry( subkey.neverExpires() ? QDate() : QDateTime::fromTime_t( d->key.subkey(0).expirationTime() ).date() );
    d->dialog->show();
    
}

void ChangeExpiryCommand::Private::slotDialogAccepted() {
    assert( dialog );

    static const QTime END_OF_DAY( 23, 59, 59 ); // not used, so as good as any

    const QDateTime expiry( dialog->dateOfExpiry(), END_OF_DAY );

    kDebug() << "expiry" << expiry;

    createJob();
    assert( job );

    if ( const Error err = job->start( key, expiry ) ) {
        showErrorDialog( err );
        finished();
    }
}

void ChangeExpiryCommand::Private::slotDialogRejected() {
    emit q->canceled();
    finished();
}

void ChangeExpiryCommand::Private::slotResult( const Error & err ) {
    if ( err.isCanceled() )
        ;
    else if ( err )
        showErrorDialog( err );
    else
        showSuccessDialog();
    finished();
}

void ChangeExpiryCommand::doCancel() {
    kDebug();
    if ( d->job )
        d->job->slotCancel();
}

void ChangeExpiryCommand::Private::ensureDialogCreated() {
    if ( dialog )
        return;

    dialog = new ExpiryDialog;
    applyWindowID( dialog );
    dialog->setAttribute( Qt::WA_DeleteOnClose );

    connect( dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()) );
    connect( dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()) );
}

void ChangeExpiryCommand::Private::createJob() {
    assert( !job );

    const CryptoBackend::Protocol * const backend = CryptoBackendFactory::instance()->protocol( key.protocol() );
    if ( !backend )
        return;

    ChangeExpiryJob * const j = backend->changeExpiryJob();
    if ( !j )
        return;

    connect( j, SIGNAL(progress(QString,int,int)),
             q, SIGNAL(progress(QString,int,int)) );
    connect( j, SIGNAL(result(GpgME::Error)),
             q, SLOT(slotResult(GpgME::Error)) );

    job = j;
}

void ChangeExpiryCommand::Private::showErrorDialog( const Error & err ) {
    error( i18n("<p>An error occurred while trying to change "
                "the expiry date for <b>%1</b>:</p><p>%2</p>",
                Formatting::formatForComboBox( key ),
                QString::fromLocal8Bit( err.asString() ) ),
           i18n("Expiry Date Change Error") );
}

void ChangeExpiryCommand::Private::showSuccessDialog() {
    information( i18n("Expiry date changed successfully."),
                 i18n("Expiry Date Change Succeeded") );
}

#undef d
#undef q

#include "moc_changeexpirycommand.cpp"
