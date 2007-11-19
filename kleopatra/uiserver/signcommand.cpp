/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signcommand.cpp

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

#include "signcommand.h"
#include "signemailcontroller.h"
#include "kleo-assuan.h"

#include <KLocale>

using namespace Kleo;
using namespace boost;

class SignCommand::Private : public QObject {
    Q_OBJECT
private:
    friend class ::Kleo::SignCommand;
    SignCommand * const q;
public:
    explicit Private( SignCommand * qq )
        : q( qq ), controller()
    {

    }

private:    
    void checkForErrors() const;

private Q_SLOTS:
    void slotSignersResolved();
    void slotMicAlgDetermined( const QString & );
    void slotDone();
    void slotError( int, const QString & );

private:
    shared_ptr<SignEMailController> controller;
};

SignCommand::SignCommand()
    : AssuanCommandMixin<SignCommand>(), d( new Private( this ) )
{

}

SignCommand::~SignCommand() {}

void SignCommand::Private::checkForErrors() const {

    if ( q->numFiles() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "SIGN is an email mode command, connection seems to be in filemanager mode" ) );

    if ( q->senders().empty() )
        throw assuan_exception( makeError( GPG_ERR_NOT_IMPLEMENTED ),
                                i18n( "SIGN without SENDER" ) );

    if ( !q->recipients().empty() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "RECIPIENT may not be given prior to SIGN" ) );

    if ( q->inputs().empty() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n( "At least one INPUT must be present" ) );

    if ( q->outputs().size() != q->inputs().size() )
        throw assuan_exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                                i18n( "INPUT/OUTPUT count mismatch" ) );

    if ( !q->messages().empty() )
        throw assuan_exception( makeError( GPG_ERR_INV_VALUE ),
                                i18n( "MESSAGE command is not allowed before SIGN" ) );

}

#if 0
static QString collect_micalgs( const GpgME::SigningResult & result, GpgME::Protocol proto ) {
    const std::vector<GpgME::CreatedSignature> css = result.createdSignatures();
    QStringList micalgs;
    std::transform( css.begin(), css.end(),
                    std::back_inserter( micalgs ),
                    bind( &QString::toLower, bind( &QString::fromLatin1, bind( &GpgME::CreatedSignature::hashAlgorithmAsString, _1 ), -1 ) ) );
    if ( proto == GpgME::OpenPGP )
        for ( QStringList::iterator it = micalgs.begin(), end = micalgs.end() ; it != end ; ++it )
            it->prepend( "pgp-" );
    micalgs.sort();
    micalgs.erase( std::unique( micalgs.begin(), micalgs.end() ), micalgs.end() );
    return micalgs.join( QLatin1String(",") );
}
#endif

int SignCommand::doStart() {

    d->checkForErrors();

    d->controller.reset( new SignEMailController );
    d->controller->setProtocol( checkProtocol( EMail ) );

    d->controller->setCommand( shared_from_this() );

    QObject::connect( d->controller.get(), SIGNAL(signersResolved()), d.get(), SLOT(slotSignersResolved() ) );
    QObject::connect( d->controller.get(), SIGNAL(migAlgDetermined(QString)), d.get(), SLOT(slotMicAlgDetermined(QString)) );
    QObject::connect( d->controller.get(), SIGNAL(done()), d.get(), SLOT(slotDone()) );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)) );

    d->controller->startResolveSigners( senders() );
    
    return 0;
}

void SignCommand::Private::slotSignersResolved() {
    try {
        controller->setDetachedSignature( q->hasOption("detached" ) );
        controller->importIO();
        controller->start();
    } catch ( const assuan_exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in SignCommand::Private::slotRecipientsResolved: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
        controller->cancel();
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in SignCommand::Private::slotRecipientsResolved") );
        controller->cancel();
    }
}

void SignCommand::Private::slotMicAlgDetermined( const QString & micalg ) {
    if ( const int err = q->sendStatus( "MICALG", micalg ) ) {
        q->done( err );
        if ( controller )
            controller->cancel();
    }
}

void SignCommand::Private::slotDone() {
    q->done();
}

void SignCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void SignCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

#include "signcommand.moc"
