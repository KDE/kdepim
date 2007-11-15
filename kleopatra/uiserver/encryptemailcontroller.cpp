/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/encryptemailcontroller.cpp

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

#include "encryptemailcontroller.h"

#include "kleo-assuan.h"
#include "assuancommand.h"
#include "certificateresolver.h"
#include "signencryptwizard.h"

#include <utils/stl_util.h>

#include <kmime/kmime_header_parsing.h>

#include <KLocale>

#include <QPointer>
#include <QTimer>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;
using namespace KMime::Types;

class EncryptEMailController::Private {
    friend class ::Kleo::EncryptEMailController;
    EncryptEMailController * const q;
public:
    explicit Private( EncryptEMailController * qq );

private:
    void slotWizardRecipientsResolved();
    void slotWizardCanceled();

private:
    void ensureWizardCreated();
    void ensureWizardVisible();
    void cancelAllJobs();

private:
    weak_ptr<AssuanCommand> command;
    QPointer<SignEncryptWizard> wizard;
    Protocol protocol;
};

EncryptEMailController::Private::Private( EncryptEMailController * qq )
    : q( qq ),
      command(),
      wizard(),
      protocol( UnknownProtocol )
{

}

EncryptEMailController::EncryptEMailController( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

EncryptEMailController::~EncryptEMailController() {
    if ( d->wizard && !d->wizard->isVisible() )
        delete d->wizard;
        //d->wizard->close(); ### ?
}

void EncryptEMailController::setProtocol( Protocol proto ) {
    assuan_assert( d->protocol == UnknownProtocol ||
                   d->protocol == proto );
    d->protocol = proto;
    if ( d->wizard )
        d->wizard->setProtocol( proto );
}

Protocol EncryptEMailController::protocol() const {
    return d->protocol;
}

const char * EncryptEMailController::protocolAsString() const {
    switch ( d->protocol ) {
    case OpenPGP: return "OpenPGP";
    case CMS:     return "CMS";
    default:
        throw assuan_exception( gpg_error( GPG_ERR_INTERNAL ),
                                i18n("Call to EncryptEMailController::protocolAsString() is ambiguous.") );
    }
}

void EncryptEMailController::setCommand( const shared_ptr<AssuanCommand> & cmd ) {
    d->command = cmd;
}

void EncryptEMailController::startResolveRecipients( const std::vector<Mailbox> & recipients ) {
    const std::vector< std::vector<Key> > keys = CertificateResolver::resolveRecipients( recipients, d->protocol );

    assuan_assert( keys.size() == static_cast<size_t>( recipients.size() ) );

    d->ensureWizardCreated();

    d->wizard->setRecipientsAndCandidates( recipients, keys );

    if ( d->wizard->canGoToNextPage() ) {
        d->wizard->next();
        QTimer::singleShot( 0, this, SIGNAL(recipientsResolved()) );
    } else {
        d->ensureWizardVisible();
    }
    
}

void EncryptEMailController::Private::slotWizardRecipientsResolved() {
    emit q->recipientsResolved();
}

void EncryptEMailController::Private::slotWizardCanceled() {
    emit q->error( gpg_error( GPG_ERR_CANCELED ), i18n("User cancel") );
}

void EncryptEMailController::importIO() {
    notImplemented();
}

void EncryptEMailController::start() {
    notImplemented();
}

void EncryptEMailController::cancel() {
    try {
        if ( d->wizard )
            d->wizard->close();
        d->cancelAllJobs();
    } catch ( const std::exception & e ) {
        qDebug( "Caught exception: %s", e.what() );
    }
}

void EncryptEMailController::Private::cancelAllJobs() {
    notImplemented();
}

void EncryptEMailController::Private::ensureWizardCreated() {
    if ( wizard )
        return;

    SignEncryptWizard * w = new SignEncryptWizard;
    if ( const shared_ptr<AssuanCommand> cmd = command.lock() )
        w = cmd->applyWindowID( w );
    w->setWindowTitle( i18n("Encrypt EMail Wizard") );
    w->setMode( SignEncryptWizard::EncryptEMail );
    w->setAttribute( Qt::WA_DeleteOnClose );
    connect( w, SIGNAL(recipientsResolved()), q, SLOT(slotWizardRecipientsResolved()) );
    connect( w, SIGNAL(canceled()), q, SLOT(slotWizardCanceled()) );

    w->setProtocol( protocol );

    wizard = w;
}

void EncryptEMailController::Private::ensureWizardVisible() {
    ensureWizardCreated();
    if ( wizard->isVisible() )
        wizard->raise();
    else
        wizard->show();
}

#include "moc_encryptemailcontroller.cpp"


