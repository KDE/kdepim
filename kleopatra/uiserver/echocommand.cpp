/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/echocommand.cpp

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

#include "echocommand.h"

#include <gpg-error.h>

#include <QVariant>
#include <QByteArray>
#include <QIODevice>
#include <QList>

#include <string>
#include <algorithm>

using namespace Kleo;
using namespace boost;

static const char option_prefix[] = "prefix";

class EchoCommand::Private {
public:
    Private() : operationsInFlight( 0 ), buffer() {}

    int operationsInFlight;
    QByteArray buffer;
};

EchoCommand::EchoCommand()
    : QObject(), AssuanCommandMixin<EchoCommand>(), d( new Private ) {}

EchoCommand::~EchoCommand() {}

int EchoCommand::doStart() {

    if ( bulkInputDevice() && !bulkOutputDevice() )
        return makeError( GPG_ERR_NOT_SUPPORTED );

    if ( bulkMessageDevice() )
        return makeError( GPG_ERR_NOT_SUPPORTED );

    if ( hasOption( option_prefix ) && !option( option_prefix ).toByteArray().isEmpty() )
        return makeError( GPG_ERR_NOT_IMPLEMENTED );

    std::string keyword;
    if ( hasOption( "inquire" ) ) {
        keyword = option("inquire").toString().toStdString();
        if ( keyword.empty() )
            return makeError( GPG_ERR_INV_ARG );
    }

    const std::string output = option("text").toString().toStdString();

    // aaand ACTION:

    // 1. echo the command line though the status channel
    if ( const int err = sendStatus( "ECHO", output.empty() ? "" : output.c_str() ) )
        return err;

    // 2. if --inquire was given, inquire more data from the client:
    if ( !keyword.empty() )
        if ( const int err = inquire( keyword.c_str(), this,
                                      SLOT(slotInquireData(int,QByteArray)) ) )
            return err;
        else
            ++d->operationsInFlight;

    // 3. if INPUT was given, start the data pump for input->output
    if ( const shared_ptr<QIODevice> in = bulkInputDevice() ) {
        const shared_ptr<QIODevice> out = bulkOutputDevice();

        ++d->operationsInFlight;

        connect( in.get(), SIGNAL(readyRead()), this, SLOT(slotInputReadyRead()) );
        connect( out.get(), SIGNAL(bytesWritten(qint64)), this, SLOT(slotOutputBytesWritten()) );

        if ( in->bytesAvailable() )
            slotInputReadyRead();
    }

    if ( !d->operationsInFlight )
        done();
    return 0;
}

void EchoCommand::doCanceled() {

}

void EchoCommand::slotInquireData( int rc, const QByteArray & data ) {

    --d->operationsInFlight;

    if ( rc )
        done( rc );

    //else if ( const int err = sendData( data ) )
    else if ( const int err = sendStatus( "ECHOINQ", data ) )
        done( err );

    else if ( !d->operationsInFlight )
        done();

}

void EchoCommand::slotInputReadyRead() {
    const shared_ptr<QIODevice> in = bulkInputDevice();
    assert( in );

    QByteArray buffer;
    buffer.resize( in->bytesAvailable() );
    const qint64 read = in->read( buffer.data(), buffer.size() );
    if ( read == - 1 ) {
        done( makeError( GPG_ERR_EIO ) );
        return;
    }
    if ( read == 0 || !in->isSequential() && read == in->size() )
        in->close();

    buffer.resize( read );
    d->buffer += buffer;

    slotOutputBytesWritten();
}


void EchoCommand::slotOutputBytesWritten() {
    const shared_ptr<QIODevice> out = bulkOutputDevice();
    assert( out );

    if ( !d->buffer.isEmpty() ) {

        if ( out->bytesToWrite() )
            return;

        const qint64 written = out->write( d->buffer );
        if ( written == -1 ) {
            done( makeError( GPG_ERR_EIO ) );
            return;
        }
        d->buffer.remove( 0, written );

    }

    if ( out->isOpen() && d->buffer.isEmpty() && !bulkInputDevice()->isOpen() ) {
        out->close();
        if ( !--d->operationsInFlight )
            done();
    }
}

#include "moc_echocommand.cpp"
