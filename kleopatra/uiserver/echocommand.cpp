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

    QByteArray buffer;
};

EchoCommand::EchoCommand()
    : QObject(), AssuanCommandMixin<EchoCommand>(), d( new Private ) {}

EchoCommand::~EchoCommand() {}

int EchoCommand::start( const std::string & line ) {

    if ( bulkInputDevice() && !bulkOutputDevice() )
        return makeError( GPG_ERR_NOT_SUPPORTED );

    if ( hasOption( option_prefix ) && !option( option_prefix ).toByteArray().isEmpty() )
        return makeError( GPG_ERR_NOT_IMPLEMENTED );

    QList<QByteArray> tokens = QByteArray( line.c_str() ).split( ' ' );
    tokens.erase( std::remove_if( tokens.begin(), tokens.end(),
                                  bind( &QByteArray::isEmpty, _1 ) ),
                  tokens.end() );

    if ( tokens.empty() || tokens.front() != "ECHO" )
        return makeError( GPG_ERR_INTERNAL );

    tokens.pop_front();

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

    // 3. if INPUT was given, start the data pump for input->output
    if ( QIODevice * const in = bulkInputDevice() ) {
        QIODevice * const out = bulkOutputDevice();

        connect( in, SIGNAL(readyRead()), this, SLOT(slotInputReadyRead()) );
        connect( out, SIGNAL(bytesWritten(qint64)), this, SLOT(slotOutputBytesWritten()) );

        if ( in->bytesAvailable() )
            slotInputReadyRead();
    }

    return 0;
}

void EchoCommand::canceled() {
    done( makeError( GPG_ERR_CANCELED ) );
}

void EchoCommand::slotInquireData( int rc, const QByteArray & data ) {

    if ( rc )
        done( rc );

    //else if ( const int err = sendData( data ) )
    else if ( const int err = sendStatus( "ECHOINQ", data ) )
        done( err );

}

void EchoCommand::slotInputReadyRead() {
    QIODevice * const in = bulkInputDevice();
    assert( in );

    if ( !in->atEnd() )
        d->buffer += in->readAll();

    slotOutputBytesWritten();
}


void EchoCommand::slotOutputBytesWritten() {
    QIODevice * const out = bulkOutputDevice();
    assert( out );

    if ( d->buffer.isEmpty() ) {
        if ( bulkInputDevice()->atEnd() && out->isOpen() )
            out->close();
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
