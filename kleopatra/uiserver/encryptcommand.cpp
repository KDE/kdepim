/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/encryptcommand.cpp

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

#include "encryptcommand.h"

#include <crypto/newsignencryptemailcontroller.h>

#include <utils/kleo_assert.h>
#include <utils/input.h>
#include <utils/output.h>

#include <kleo/exception.h>

#include <KLocalizedString>

#include <QTimer>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class EncryptCommand::Private : public QObject {
    Q_OBJECT
private:
    friend class ::Kleo::EncryptCommand;
    EncryptCommand * const q;
public:
    explicit Private( EncryptCommand * qq )
        : q( qq ),
          controller()
    {

    }

private:
    void checkForErrors() const;

private Q_SLOTS:
    void slotDone();
    void slotError( int, const QString & );
    void slotRecipientsResolved();

private:
    shared_ptr<NewSignEncryptEMailController> controller;
};

EncryptCommand::EncryptCommand()
    : AssuanCommandMixin<EncryptCommand>(), d( new Private( this ) )
{

}

EncryptCommand::~EncryptCommand() {}

void EncryptCommand::Private::checkForErrors() const {

    if ( q->numFiles() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "ENCRYPT is an email mode command, connection seems to be in filmanager mode" ) );

    if ( !q->senders().empty() && !q->informativeSenders() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "SENDER may not be given prior to ENCRYPT, except with --info" ) );

    if ( q->inputs().empty() )
        throw Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                         i18n( "At least one INPUT must be present" ) );

    if ( q->outputs().empty() )
        throw Exception( makeError( GPG_ERR_ASS_NO_OUTPUT ),
                         i18n( "At least one OUTPUT must be present" ) );

    if ( q->outputs().size() != q->inputs().size() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "INPUT/OUTPUT count mismatch" ) );

    if ( !q->messages().empty() )
        throw Exception( makeError( GPG_ERR_INV_VALUE ),
                         i18n( "MESSAGE command is not allowed before ENCRYPT" ) );

    const shared_ptr<NewSignEncryptEMailController> m = q->mementoContent< shared_ptr<NewSignEncryptEMailController> >( NewSignEncryptEMailController::mementoName() );
    kleo_assert( m );

    if ( m && m->isEncrypting() ) {

        if ( m->protocol() != q->checkProtocol( EMail ) )
            throw Exception( makeError( GPG_ERR_CONFLICT ),
                             i18n( "Protocol given conflicts with protocol determined by PREP_ENCRYPT" ) );

        if ( !q->recipients().empty() )
            throw Exception( makeError( GPG_ERR_CONFLICT ),
                             i18n( "New recipients added after PREP_ENCRYPT command" ) );
        if ( !q->senders().empty() )
            throw Exception( makeError( GPG_ERR_CONFLICT ),
                             i18n( "New senders added after PREP_ENCRYPT command" ) );

    } else {

        if ( q->recipients().empty() || q->informativeRecipients() )
            throw Exception( makeError( GPG_ERR_MISSING_VALUE ),
                             i18n( "No recipients given, or only with --info" ) );

    }

}

static void connectController( const QObject * controller, const QObject * d ) {

    QObject::connect( controller, SIGNAL(certificatesResolved()), d, SLOT(slotRecipientsResolved()) );
    QObject::connect( controller, SIGNAL(done()), d, SLOT(slotDone()) );
    QObject::connect( controller, SIGNAL(error(int,QString)), d, SLOT(slotError(int,QString)) );

}

int EncryptCommand::doStart() {

    d->checkForErrors();

    const shared_ptr<NewSignEncryptEMailController> seec = mementoContent< shared_ptr<NewSignEncryptEMailController> >( NewSignEncryptEMailController::mementoName() );

    if ( seec && seec->isEncrypting() ) {
        // reuse the controller from a previous PREP_ENCRYPT, if available:
        d->controller = seec;
        connectController( seec.get(), d.get() );
        removeMemento( NewSignEncryptEMailController::mementoName() );
        d->controller->setExecutionContext( shared_from_this() );
        if ( seec->areCertificatesResolved() )
            QTimer::singleShot( 0, d.get(), SLOT(slotRecipientsResolved()) );
        else
            kleo_assert( seec->isResolvingInProgress() );
    } else {
        // use a new controller
        d->controller.reset( new NewSignEncryptEMailController( shared_from_this() ) );

        const QString session = sessionTitle();
        if ( !session.isEmpty() )
            d->controller->setSubject( session );

        d->controller->setEncrypting( true );
        d->controller->setSigning( false );
        d->controller->setProtocol( checkProtocol( EMail ) );
        connectController( d->controller.get(), d.get() );
        d->controller->startResolveCertificates( recipients(), senders() );
    }


    return 0;
}

void EncryptCommand::Private::slotRecipientsResolved() {
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<NewSignEncryptEMailController> cont( controller );

    try {
        const QString sessionTitle = q->sessionTitle();
        if ( !sessionTitle.isEmpty() )
            Q_FOREACH ( const shared_ptr<Input> & i, q->inputs() )
                i->setLabel( sessionTitle );

        cont->startEncryption( q->inputs(), q->outputs() );

        return;

    } catch ( const Exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in EncryptCommand::Private::slotRecipientsResolved: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in EncryptCommand::Private::slotRecipientsResolved") );
    }
    cont->cancel();
}

void EncryptCommand::Private::slotDone() {
    q->done();
}

void EncryptCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void EncryptCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

#include "encryptcommand.moc"
