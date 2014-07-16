/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/prepencryptcommand.cpp

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

#include "prepencryptcommand.h"

#include <crypto/newsignencryptemailcontroller.h>

#include <kleo/exception.h>

#include <KLocalizedString>

#include <QPointer>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class PrepEncryptCommand::Private : public QObject {
    Q_OBJECT
private:
    friend class ::Kleo::PrepEncryptCommand;
    PrepEncryptCommand * const q;
public:
    explicit Private( PrepEncryptCommand * qq )
        : q( qq ), controller() {}

private:
    void checkForErrors() const;

public Q_SLOTS:
    void slotRecipientsResolved();
    void slotError( int, const QString & );

private:
    shared_ptr<NewSignEncryptEMailController> controller;
};

PrepEncryptCommand::PrepEncryptCommand()
    : AssuanCommandMixin<PrepEncryptCommand>(), d( new Private( this ) )
{

}

PrepEncryptCommand::~PrepEncryptCommand() {}

void PrepEncryptCommand::Private::checkForErrors() const {

    if ( !q->inputs().empty() || !q->outputs().empty() || !q->messages().empty() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "INPUT/OUTPUT/MESSAGE may only be given after PREP_ENCRYPT" ) );
    
    if ( q->numFiles() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "PREP_ENCRYPT is an email mode command, connection seems to be in filemanager mode" ) );

    if ( !q->senders().empty() && !q->informativeSenders() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "SENDER may not be given prior to PREP_ENCRYPT, except with --info" ) );

    if ( q->recipients().empty() || q->informativeRecipients() )
        throw Exception( makeError( GPG_ERR_MISSING_VALUE ),
                         i18n( "No recipients given, or only with --info" ) );

}

int PrepEncryptCommand::doStart() {

    removeMemento( NewSignEncryptEMailController::mementoName() );

    d->checkForErrors();

    d->controller.reset( new NewSignEncryptEMailController( shared_from_this() ) );
    d->controller->setEncrypting( true );

    const QString session = sessionTitle();
    if ( !session.isEmpty() )
        d->controller->setSubject( session );

    if ( hasOption( "protocol" ) )
        // --protocol is optional for PREP_ENCRYPT
        d->controller->setProtocol( checkProtocol( EMail ) );

    d->controller->setSigning( hasOption( "expect-sign" ) );

    QObject::connect( d->controller.get(), SIGNAL(certificatesResolved()), d.get(), SLOT(slotRecipientsResolved()) );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)) );

    d->controller->startResolveCertificates( recipients(), senders() );

    return 0;
}

void PrepEncryptCommand::Private::slotRecipientsResolved() {
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<NewSignEncryptEMailController> cont = controller;
    QPointer<Private> that( this );

    try {

        q->sendStatus( "PROTOCOL", QLatin1String(controller->protocolAsString()) );
        q->registerMemento( NewSignEncryptEMailController::mementoName(),
                            make_typed_memento( controller ) );
        q->done();
        return;

    } catch ( const Exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in PrepEncryptCommand::Private::slotRecipientsResolved: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in PrepEncryptCommand::Private::slotRecipientsResolved") );
    }
    if ( that ) // isn't this always deleted here and thus unnecessary?
        q->removeMemento( NewSignEncryptEMailController::mementoName() );
    cont->cancel();
}

void PrepEncryptCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void PrepEncryptCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

#include "prepencryptcommand.moc"
