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

#include "prepencryptcommand.h"
#include "encryptemailcontroller.h"
#include "kleo-assuan.h"

#include <KLocale>

using namespace Kleo;
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
    shared_ptr<EncryptEMailController> controller;
};

PrepEncryptCommand::PrepEncryptCommand()
    : AssuanCommandMixin<PrepEncryptCommand>(), d( new Private( this ) )
{

}

PrepEncryptCommand::~PrepEncryptCommand() {}

void PrepEncryptCommand::Private::checkForErrors() const {

    if ( q->numBulkInputDevices() || q->numBulkOutputDevices() || q->numBulkMessageDevices() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "INPUT/OUTPUT/MESSAGE may only be given after PREP_ENCRYPT" ) );
    
    if ( q->numFiles() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "PREP_ENCRYPT is an email mode command, connection seems to be in filemanager mode" ) );

    if ( !q->senders().empty() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "SENDER may not be given prior to PREP_ENCRYPT" ) );

    if ( q->recipients().empty() )
        throw assuan_exception( makeError( GPG_ERR_MISSING_VALUE ),
                                i18n( "No recipients given" ) );

}

int PrepEncryptCommand::doStart() {

    removeMemento( EncryptEMailController::mementoName() );

    d->checkForErrors();

    d->controller.reset( new EncryptEMailController );

    d->controller->setCommand( shared_from_this() );

    if ( hasOption( "protocol" ) )
        // --protocol is optional for PREP_ENCRYPT
        d->controller->setProtocol( checkProtocol( EMail ) );

    QObject::connect( d->controller.get(), SIGNAL(recipientsResolved()), d.get(), SLOT(slotRecipientsResolved()) );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)) );

    d->controller->startResolveRecipients( recipients() );

    return 0;
}

void PrepEncryptCommand::Private::slotRecipientsResolved() {
    if ( const int err = q->sendStatus( "PROTOCOL", controller->protocolAsString() ) )
        q->done( err, i18n( "Failed to send PROTOCOL status" ) );
    q->registerMemento( EncryptEMailController::mementoName(),
                        make_typed_memento( controller ) );
    q->done();
}

void PrepEncryptCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void PrepEncryptCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

#include "prepencryptcommand.moc"
