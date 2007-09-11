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

#include <boost/bind.hpp>

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

int EchoCommand::start( const std::string & line ) {

    if ( bulkInputDevice( "IN" ) && !bulkOutputDevice( "OUT" ) )
        return makeError( GPG_ERR_NOT_SUPPORTED );

    if ( hasOption( option_prefix ) && !option( option_prefix ).toByteArray().isEmpty() )
        return makeError( GPG_ERR_NOT_IMPLEMENTED );

    QList<QByteArray> tokens = QByteArray( line.c_str() ).split( ' ' );
    tokens.erase( std::remove_if( tokens.begin(), tokens.end(),
                                  bind( &QByteArray::isEmpty, _1 ) ),
                  tokens.end() );

    std::string keyword;
    if ( !tokens.empty() && tokens.front() == "--inquire" ) {
        tokens.pop_front();
        if ( tokens.empty() )
            return makeError( GPG_ERR_MISSING_VALUE );
        keyword = tokens.front().constData();
        tokens.pop_front();
    }

    bool optionsExpected = true;
    QByteArray output;
    Q_FOREACH( QByteArray token, tokens ) {
        if ( token == "--" )
            optionsExpected = false;
        else if ( optionsExpected && token.startsWith( "--" ) )
            if ( token == "--inquire" )
                return makeError( GPG_ERR_DUP_VALUE ); // duplicate
            else
                return makeError( GPG_ERR_UNKNOWN_OPTION );
        else
            if ( output.isEmpty() )
                output = token;
            else
                output += ' ' + token;
    }

    // aaand ACTION:

    // 1. echo the command line though the status channel
    if ( const int err = sendStatus( "ECHO", output.constData() ) )
        return err;

    // 2. if --inquire was given, inquire more data from the client:
    if ( !keyword.empty() )
        if ( const int err = inquire( keyword.c_str(), this,
                                      SLOT(slotInquireData(int,QByteArray)) ) )
            return err;
        else
            ++d->operationsInFlight;

    // 3. if INPUT was given, start the data pump for input->output
    if ( QIODevice * const in = bulkInputDevice( "IN" ) ) {
        QIODevice * const out = bulkOutputDevice( "OUT" );

        ++d->operationsInFlight;

        connect( in, SIGNAL(readyRead()), this, SLOT(slotInputReadyRead()) );
        connect( out, SIGNAL(bytesWritten(qint64)), this, SLOT(slotOutputBytesWritten()) );

        if ( in->bytesAvailable() )
            slotInputReadyRead();
    }

    if ( !d->operationsInFlight )
        done();
    return 0;
}

void EchoCommand::canceled() {

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
    QIODevice * const in = bulkInputDevice( "IN" );
    assert( in );

    if ( !in->atEnd() )
        d->buffer += in->readAll();

    slotOutputBytesWritten();
}


void EchoCommand::slotOutputBytesWritten() {
    QIODevice * const out = bulkOutputDevice( "OUT" );
    assert( out );

    if ( d->buffer.isEmpty() ) {
        if ( bulkInputDevice( "IN" )->atEnd() && out->isOpen() ) {
            out->close();
            if ( !--d->operationsInFlight )
                done();
        }
        return;
    }

    if ( out->bytesToWrite() )
        return;

    const qint64 written = out->write( d->buffer );
    if ( written == -1 ) {
        done( makeError( GPG_ERR_EIO ) );
        return;
    }
    d->buffer.remove( 0, written );

}

#include "moc_echocommand.cpp"
