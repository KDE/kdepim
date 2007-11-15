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

#include "encryptcommand.h"
#include "encryptemailcontroller.h"
#include "kleo-assuan.h"

#include <KLocale>

#include <QTimer>

using namespace Kleo;
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
    shared_ptr<EncryptEMailController> controller;
};

EncryptCommand::EncryptCommand()
    : AssuanCommandMixin<EncryptCommand>(), d( new Private( this ) )
{

}

EncryptCommand::~EncryptCommand() {}

void EncryptCommand::Private::checkForErrors() const {

    if ( q->numFiles() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "ENCRYPT is an email mode command, connection seems to be in filmanager mode" ) );

    if ( !q->senders().empty() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "SENDER may not be given prior to ENCRYPT" ) );

    if ( !q->numBulkInputDevices() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n( "At least one INPUT must be present" ) );

    if ( !q->numBulkOutputDevices() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_OUTPUT ),
                                i18n( "At least one OUTPUT must be present" ) );

    if ( q->numBulkOutputDevices() != q->numBulkInputDevices() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "INPUT/OUTPUT count mismatch" ) );

    if ( q->numBulkMessageDevices() )
        throw assuan_exception( makeError( GPG_ERR_INV_VALUE ),
                                i18n( "MESSAGE command is not allowed before ENCRYPT" ) );

    if ( q->hasMemento( EncryptEMailController::mementoName() ) ) {

        const shared_ptr<EncryptEMailController> m = q->mementoContent< shared_ptr<EncryptEMailController> >( EncryptEMailController::mementoName() );
        assuan_assert( m );

        if ( m->protocol() != q->checkProtocol( EMail ) )
            throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                    i18n( "Protocol given conflicts with protocol determined by PREP_ENCRYPT" ) );

        if ( !q->recipients().empty() )
            throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                    i18n( "New recipients added after PREP_ENCRYPT command" ) );
        if ( !q->senders().empty() )
            throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                    i18n( "New senders added after PREP_ENCRYPT command" ) );

    } else {

        if ( q->recipients().empty() )
            throw assuan_exception( makeError( GPG_ERR_MISSING_VALUE ),
                                    i18n( "No recipients given" ) );

    }

}

int EncryptCommand::doStart() {

    d->checkForErrors();

    const bool hasPreviousMemento = hasMemento( EncryptEMailController::mementoName() );

    if ( hasPreviousMemento ) {
        d->controller = mementoContent< shared_ptr<EncryptEMailController> >( EncryptEMailController::mementoName() );
        removeMemento( EncryptEMailController::mementoName() );
    } else {
        d->controller.reset( new EncryptEMailController );
        d->controller->setProtocol( checkProtocol( EMail ) );
    }

    assuan_assert( d->controller );

    d->controller->setCommand( shared_from_this() );

    QObject::connect( d->controller.get(), SIGNAL(recipientsResolved()), d.get(), SLOT(slotRecipientsResolved()), Qt::QueuedConnection );
    QObject::connect( d->controller.get(), SIGNAL(done()), d.get(), SLOT(slotDone()), Qt::QueuedConnection );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)), Qt::QueuedConnection );

    if ( hasPreviousMemento )
        QTimer::singleShot( 0, d.get(), SLOT(slotRecipientsResolved()) );
    else
        d->controller->startResolveRecipients( recipients() );

    return 0;
}

void EncryptCommand::Private::slotRecipientsResolved() {
    controller->importIO();
    controller->start();
}

void EncryptCommand::Private::slotDone() {
    q->done();
}

void EncryptCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void EncryptCommand::doCanceled() {
    d->controller->cancel();
}

#include "encryptcommand.moc"

