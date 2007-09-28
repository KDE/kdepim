/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuanserverconnection.cpp

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

#include "assuanserverconnection.h"
#include "assuancommand.h"

#include "detail_p.h"
#include "kleo-assuan.h"

#include <utils/kdpipeiodevice.h>

#include <gpgme++/data.h>

#include <QSocketNotifier>
#include <QVariant>
#include <QPointer>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include <boost/type_traits/remove_pointer.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <map>
#include <string>
#include <memory>

#include <errno.h>

#ifdef __GNUC__
# include <ext/algorithm> // for is_sorted
#endif

#ifdef Q_OS_WIN32
# include <io.h>
#endif

using namespace Kleo;
using namespace boost;

namespace {
    struct IO {
        QString file;
        QIODevice * iodev;
        GpgME::Data::Encoding encoding;
    };
}

static const unsigned int INIT_SOCKET_FLAGS = 3; // says info assuan...
//static int(*USE_DEFAULT_HANDLER)(assuan_context_t,char*) = 0;
static const int FOR_READING = 0;
static const unsigned int MAX_ACTIVE_FDS = 32;

// shared_ptr for assuan_context_t w/ deleter enforced to assuan_deinit_server:
typedef shared_ptr< remove_pointer<assuan_context_t>::type > AssuanContextBase;
struct AssuanContext : AssuanContextBase {
    AssuanContext() : AssuanContextBase() {}
    explicit AssuanContext( assuan_context_t ctx ) : AssuanContextBase( ctx, &assuan_deinit_server ) {}

    void reset( assuan_context_t ctx=0 ) { AssuanContextBase::reset( ctx, &assuan_deinit_server ); }
};

static unsigned char unhex( char ch ) {
    if ( ch >= '0' && ch <= '9' )
        return ch - '0';
    if ( ch >= 'A' && ch <= 'F' )
        return ch - 'A';
    if ( ch >= 'a' && ch <= 'f' )
        return ch - 'a';
    throw gpg_error( GPG_ERR_ASS_SYNTAX );
}

static std::string hexdecode( const std::string & in ) {
    std::string result;
    result.reserve( in.size() );
    for ( std::string::const_iterator it = in.begin(), end = in.end() ; it != end ; ++it )
        if ( *it == '%' ) {
            ++it;
            char ch = '\0';
            if ( it == end )
                throw gpg_error( GPG_ERR_ASS_SYNTAX );
            ch |= unhex( *it ) << 4;
            ++it;
            if ( it == end )
                throw gpg_error( GPG_ERR_ASS_SYNTAX );
            ch |= unhex( *it );
            result.push_back( ch );
        } else if ( *it == '+' ) {
            result += ' ';
        } else  {
            result.push_back( *it );
        }
    return result;
}

static std::map<std::string,std::string> parse_commandline( const char * line ) {
    std::map<std::string,std::string> result;
    if ( line ) {
        const char * begin = line;
        const char * lastEQ = 0;
        while ( *line ) {
            if ( *line == ' ' || *line == '\t' ) {
                if ( begin != line ) {
                    if ( begin[0] == '-' && begin[1] == '-' )
                        begin += 2; // skip initial "--"
                    if ( lastEQ && lastEQ > begin )
                        result[ std::string( begin, lastEQ - begin ) ] = hexdecode( std::string( lastEQ+1, line - (lastEQ+1) ) );
                    else
                        result[ std::string( begin,  line  - begin ) ] = std::string();
                }
                begin = line + 1;
            } else if ( *line == '=' ) {
                if ( line == begin )
                    throw gpg_error( GPG_ERR_ASS_SYNTAX );
                else
                    lastEQ = line;
            }
            ++line;
        }
        if ( begin != line ) {
            if ( begin[0] == '-' && begin[1] == '-' )
                begin += 2; // skip initial "--"
            if ( lastEQ && lastEQ > begin )
                result[ std::string( begin, lastEQ - begin ) ] = hexdecode( std::string( lastEQ+1, line - (lastEQ+1 ) ) );
            else
                result[ begin ] = std::string();
        }
    }

    return result;
}

//
//
// AssuanServerConnection:
//
//

class AssuanServerConnection::Private : public QObject {
    Q_OBJECT
    friend class ::Kleo::AssuanServerConnection;
    friend class ::Kleo::AssuanCommandFactory;
    friend class ::Kleo::AssuanCommand;
    AssuanServerConnection * const q;
public:
    Private( assuan_fd_t fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_, AssuanServerConnection * qq );
    ~Private();

public Q_SLOTS:
    void slotReadActivity( int ) {
        assert( ctx );
        if ( const int err = assuan_process_next( ctx.get() ) ) {
            if ( err == -1 || gpg_err_code(err) == GPG_ERR_EOF ) {
                if ( currentCommand )
                    currentCommand->canceled(); // ### respect --nohup
                cleanup();
                const QPointer<Private> that = this;
                emit q->closed( q );
                if ( that ) // still there
                    q->deleteLater();
            } else {
                //assuan_process_done( ctx.get(), err );
                return;
            }
        }
    }

private:
    static void reset_handler( assuan_context_t ctx_ ) {
        assert( assuan_get_pointer( ctx_ ) );

        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        conn.options.clear();
        conn.mementos.clear();
    }

    static int option_handler( assuan_context_t ctx_, const char * key, const char * value ) {
        assert( assuan_get_pointer( ctx_ ) );
        
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        if ( key && key[0] == '-' && key[1] == '-' )
            key += 2; // skip "--"
        conn.options[key] = QString::fromUtf8( value );

        return 0;
        //return gpg_error( GPG_ERR_UNKNOWN_OPTION );
    }

    // format: TAG (FD|FD=\d+|FILE=...)
    static int IO_handler( assuan_context_t ctx_, char * line_, bool in, const char * tag ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        try {

            /*const*/ std::map<std::string,std::string> options = parse_commandline( line_ );
            if ( options.size() < 1 || options.size() > 2 )
                throw gpg_error( GPG_ERR_ASS_SYNTAX );

            IO io;
            std::auto_ptr<QIODevice> iodev;

            if ( options.count( "FD" ) ) {

                if ( options.count( "FILE" ) )
                    throw gpg_error( GPG_ERR_CONFLICT );

                assuan_fd_t fd = ASSUAN_INVALID_FD;

                const std::string fdstr = options["FD"];

                if ( fdstr.empty() ) {
                    if ( const gpg_error_t err = assuan_receivefd( conn.ctx.get(), &fd ) )
                        throw err;
                } else {
#ifdef Q_OS_WIN32
                    fd = (assuan_fd_t)lexical_cast<intptr_t>( fdstr );
#else
                    fd = lexical_cast<assuan_fd_t>( fdstr );
#endif
                }

                io.iodev = new KDPipeIODevice( fd, in ? QIODevice::ReadOnly : QIODevice::WriteOnly );

                options.erase( "FD" );

            } else if ( options.count( "FILE" ) ) {
                
                if ( options.count( "FD" ) )
                    throw gpg_error( GPG_ERR_CONFLICT );

                io.file = QFile::decodeName( hexdecode( options["FILE"] ).c_str() );

                if ( io.file.isEmpty() ) {
                    if ( in )
                        throw gpg_error( GPG_ERR_ASS_SYNTAX );
                } else {

                    std::auto_ptr<QFile> f( new QFile( io.file ) );

                    if ( !f->open( in ? QIODevice::ReadOnly : QIODevice::ReadWrite ) )
                        throw gpg_error_from_errno( errno );

                    iodev = f;

                }

                options.erase( "FILE" );

            } else {

                throw gpg_error( GPG_ERR_ASS_PARAMETER );

            }

            io.encoding = GpgME::Data::AutoEncoding;
            if ( options.count( "binary" ) ) {
                if ( !options["binary"].empty() )
                    throw gpg_error( GPG_ERR_ASS_SYNTAX );
                io.encoding = GpgME::Data::BinaryEncoding;
                options.erase( "binary" );
            }
            if ( options.count( "armor" ) ) {
                if ( !options["armor"].empty() )
                    throw gpg_error( GPG_ERR_ASS_SYNTAX );
                if ( io.encoding )
                    throw gpg_error( GPG_ERR_CONFLICT ); // conflicting parameters
                io.encoding = GpgME::Data::ArmorEncoding;
                options.erase( "armor" );
            }
            if ( options.count( "base64" ) ) {
                if ( !options["base64"].empty() )
                    throw gpg_error( GPG_ERR_ASS_SYNTAX );
                if ( io.encoding )
                    throw gpg_error( GPG_ERR_CONFLICT ); // conflicting parameters
                io.encoding = GpgME::Data::Base64Encoding;
            }

            if ( options.size() )
                throw gpg_error( GPG_ERR_UNKNOWN_OPTION );

            io.iodev = iodev.release();

            ( in ? conn.inputs : conn.outputs )[tag].push_back( io );

            qDebug() << "AssuanServerConnection: added" << (in ? "input" : "output") << '('
                     << io.file << io.iodev << io.encoding << ')';

            return assuan_process_done( conn.ctx.get(), 0 );

        } catch ( const std::exception & e ) {
            return assuan_process_done( conn.ctx.get(), gpg_error( GPG_ERR_ASS_SYNTAX ) );
        } catch ( const gpg_error_t e ) {
            return assuan_process_done( conn.ctx.get(), e );
        } catch ( ... ) {
            return assuan_process_done( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ) );
        }

    }

    static int input_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, true, "INPUT" );
    }

    static int output_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, false, "OUTPUT" );
    }

    static int message_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, true, "MESSAGE" );
    }

    void cleanup();


    assuan_fd_t fd;
    AssuanContext ctx;
    std::vector< shared_ptr<QSocketNotifier> > notifiers;
    std::vector< shared_ptr<AssuanCommandFactory> > factories; // sorted: _detail::ByName<std::less>
    shared_ptr<AssuanCommand> currentCommand;
    std::map<std::string,QVariant> options;
    std::map< std::string, std::vector<IO> > inputs, outputs;
    std::map< QByteArray, shared_ptr<AssuanCommand::Memento> > mementos;
};

void AssuanServerConnection::Private::cleanup() {
    options.clear();
    currentCommand.reset();
    options.clear();
    mementos.clear();
    notifiers.clear();
    ctx.reset();
    fd = ASSUAN_INVALID_FD;
}

AssuanServerConnection::Private::Private( assuan_fd_t fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_, AssuanServerConnection * qq )
    : QObject(), q( qq ), fd( fd_ ), factories( factories_ )
{
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( factories_.begin(), factories_.end(), _detail::ByName<std::less>() ) );
#endif

    if ( fd == ASSUAN_INVALID_FD )
        throw assuan_exception( gpg_error( GPG_ERR_INV_ARG ), "pre-assuan_init_socket_server_ext" );

    assuan_context_t naked_ctx = 0;
    if ( const gpg_error_t err = assuan_init_socket_server_ext( &naked_ctx, fd, INIT_SOCKET_FLAGS ) )
        throw assuan_exception( err, "assuan_init_socket_server_ext" );
    
    ctx.reset( naked_ctx ); naked_ctx = 0;

    // for callbacks, associate the context with this connection:
    assuan_set_pointer( ctx.get(), this );

    assuan_set_log_stream( ctx.get(), stderr );

    // register FDs with the event loop:
    assuan_fd_t fds[MAX_ACTIVE_FDS];
    const int numFDs = assuan_get_active_fds( ctx.get(), FOR_READING, fds, MAX_ACTIVE_FDS );
    assert( numFDs != -1 ); // == 1

    if ( !numFDs || fds[0] != fd ) {
        const shared_ptr<QSocketNotifier> sn( new QSocketNotifier( (int)fd, QSocketNotifier::Read ) );
        connect( sn.get(), SIGNAL(activated(int)), this, SLOT(slotReadActivity(int)) );
        notifiers.push_back( sn );
    }

    notifiers.reserve( notifiers.size() + numFDs );
    for ( int i = 0 ; i < numFDs ; ++i ) {
        const shared_ptr<QSocketNotifier> sn( new QSocketNotifier( (int)fds[i], QSocketNotifier::Read ) );
        connect( sn.get(), SIGNAL(activated(int)), this, SLOT(slotReadActivity(int)) );
        notifiers.push_back( sn );
    }


    // register our INPUT/OUTPUT/MESSGAE handlers:
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "INPUT",  input_handler ) )
        throw assuan_exception( err, "register \"INPUT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "MESSAGE",  message_handler ) )
        throw assuan_exception( err, "register \"MESSAGE\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "OUTPUT", output_handler ) )
        throw assuan_exception( err, "register \"OUTPUT\" handler" );


    // register user-defined commands:
    Q_FOREACH( shared_ptr<AssuanCommandFactory> fac, factories )
        if ( const gpg_error_t err = assuan_register_command( ctx.get(), fac->name(), fac->_handler() ) )
            throw assuan_exception( err, std::string( "register \"" ) + fac->name() + "\" handler" );

    assuan_set_hello_line( ctx.get(), "GPG UI server (Kleopatra/1.9) ready to serve" );
    //assuan_set_hello_line( ctx.get(), GPG UI server (qApp->applicationName() + " v" + kapp->applicationVersion() + "ready to serve" )


    // some notifiers we're interested in:
    if ( const gpg_error_t err = assuan_register_reset_notify( ctx.get(), reset_handler ) )
        throw assuan_exception( err, "register reset notify" );
    if ( const gpg_error_t err = assuan_register_option_handler( ctx.get(), option_handler ) )
        throw assuan_exception( err, "register option handler" );

    // and last, we need to call assuan_accept, which doesn't block
    // (d/t INIT_SOCKET_FLAGS), but performs vital connection
    // establishing handling:
    if ( const gpg_error_t err = assuan_accept( ctx.get() ) )
        throw assuan_exception( err, "assuan_accept" );
}

AssuanServerConnection::Private::~Private() {
    cleanup();
}

AssuanServerConnection::AssuanServerConnection( assuan_fd_t fd, const std::vector< shared_ptr<AssuanCommandFactory> > & factories, QObject * p )
    : QObject( p ), d( new Private( fd, factories, this ) )
{

}

AssuanServerConnection::~AssuanServerConnection() {}


//
//
// AssuanCommand:
//
//

class InquiryHandler : public QObject {
    Q_OBJECT
public:

#ifdef HAVE_ASSUAN_INQUIRE_EXT
    explicit InquiryHandler( const char * keyword_, QObject * p=0 )
        : QObject( p ),
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
          buffer( 0 ),
          buflen( 0 ),
# endif
          keyword( keyword_ )
    {

    }

# ifdef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
    static int handler( void * cb_data, int rc, unsigned char * buffer, size_t buflen )
    {
        assert( cb_data );
        InquiryHandler * this_ = static_cast<InquiryHandler*>(cb_data);
        emit this_->signal( rc, QByteArray::fromRawData( reinterpret_cast<const char*>(buffer), buflen ), this_->keyword );
        std::free( buffer );
        delete this_;
        return 0;
    }
# else
    static int handler( void * cb_data, int rc )
    {
        assert( cb_data );
        InquiryHandler * this_ = static_cast<InquiryHandler*>(cb_data);
        emit this_->signal( rc, QByteArray::fromRawData( reinterpret_cast<const char*>(this_->buffer), this_->buflen ), this_->keyword );
        std::free( this_->buffer );
        delete this_;
        return 0;
    }
# endif

private:
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
    friend class ::Kleo::AssuanCommand;
    unsigned char * buffer;
    size_t buflen;
# endif
    const char * keyword;
#endif // HAVE_ASSUAN_INQUIRE_EXT

Q_SIGNALS:
    void signal( int rc, const QByteArray & data, const QByteArray & keyword );
};

class AssuanCommand::Private {
public:
    Private() : done( false ) {}

    std::map<std::string,QVariant> options;
    std::map< std::string, std::vector<IO> > inputs, outputs;
    QByteArray utf8ErrorKeepAlive;
    AssuanContext ctx;
    bool done;

public:
};

AssuanCommand::AssuanCommand()
    : d( new Private )
{

}

AssuanCommand::~AssuanCommand() {

}

int AssuanCommand::start() {
    if ( const int err = doStart() ) {
        if ( !d->done )
            done( err );
        return err;
    }
    return 0;
}

void AssuanCommand::canceled() {
    d->done = true;
    doCanceled();
}

// static
int AssuanCommand::makeError( int code ) {
    return gpg_error( static_cast<gpg_err_code_t>( code ) );
}

bool AssuanCommand::hasOption( const char * opt ) const {
    return d->options.count( opt );
}

QVariant AssuanCommand::option( const char * opt ) const {
    const std::map<std::string,QVariant>::const_iterator it = d->options.find( opt );
    if ( it == d->options.end() )
        return QVariant();
    else
        return it->second;
}

const std::map<std::string,QVariant> & AssuanCommand::options() const {
    return d->options;
}

namespace {
    template <typename Container>
    QString get_device_file_name( const Container & c, const char * tag, unsigned int idx ) {
        const typename Container::const_iterator it = c.find( tag );
        if ( it == c.end() || idx >= it->second.size() )
            return QString();
        else
            return it->second[idx].file;
    }

    template <typename Container>
    QIODevice * get_device( const Container & c, const char * tag, unsigned int idx ) {
        const typename Container::const_iterator it = c.find( tag );
        if ( it == c.end() || idx >= it->second.size() )
            return 0;
        else
            return it->second[idx].iodev;
    }

    template <typename Container>
    unsigned int get_device_encoding( const Container & c, const char * tag, unsigned int idx ) {
        const typename Container::const_iterator it = c.find( tag );
        if ( it == c.end() || idx >= it->second.size() )
            return 0;
        else
            return it->second[idx].encoding;
    }

    template <typename Container>
    unsigned int get_num_devices( const Container & c, const char * tag ) {
        const typename Container::const_iterator it = c.find( tag );
        if ( it == c.end() )
            return 0;
        else
            return it->second.size();
    }

    template <typename U, typename V>
    std::vector<U> keys( const std::map<U,V> & map ) {
        std::vector<U> result;
        result.resize( map.size() );
        for ( typename std::map<U,V>::const_iterator it = map.begin(), end = map.end() ; it != end ; ++it )
            result.push_back( it->first );
        return result;
    }
}

const std::map< QByteArray, shared_ptr<AssuanCommand::Memento> > & AssuanCommand::mementos() const {
    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    const AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );
    return conn.mementos;
}

bool AssuanCommand::hasMemento( const QByteArray & tag ) const {
    return mementos().count( tag );
}

shared_ptr<AssuanCommand::Memento> AssuanCommand::memento( const QByteArray & tag ) const {
    const std::map< QByteArray, shared_ptr<Memento> >::const_iterator it = mementos().find( tag );
    if ( it == mementos().end() )
        return shared_ptr<Memento>();
    else
        return it->second;
}

QByteArray AssuanCommand::registerMemento( const shared_ptr<Memento> & mem ) {
    const QByteArray tag = QByteArray::number( reinterpret_cast<qulonglong>( mem.get() ), 36 );
    // oh, hack :(
    assert( assuan_get_pointer( d->ctx.get() ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );
    
    conn.mementos[tag] = mem;
    return tag;
}

QString AssuanCommand::bulkInputDeviceFileName( const char * tag, unsigned int idx ) const {
    return get_device_file_name( d->inputs, tag, idx );
}

QIODevice * AssuanCommand::bulkInputDevice( const char * tag, unsigned int idx ) const {
    return get_device( d->inputs, tag, idx );
}

unsigned int AssuanCommand::bulkInputDataEncoding( const char * tag, unsigned int idx ) const {
    return get_device_encoding( d->inputs, tag, idx );
}

unsigned int AssuanCommand::numBulkInputDevices( const char * tag ) const {
    return get_num_devices( d->inputs, tag );
}

std::vector<std::string> AssuanCommand::bulkInputDeviceTags() const {
    return keys( d->inputs );
}

QString AssuanCommand::bulkOutputDeviceFileName( const char * tag, unsigned int idx ) const {
    return get_device_file_name( d->outputs, tag, idx );
}

QIODevice * AssuanCommand::bulkOutputDevice( const char * tag, unsigned int idx ) const {
    return get_device( d->outputs, tag, idx );
}

unsigned int AssuanCommand::bulkOutputDataEncoding( const char * tag, unsigned int idx ) const {
    return get_device_encoding( d->outputs, tag, idx );
}

unsigned int AssuanCommand::numBulkOutputDevices( const char * tag ) const {
    return get_num_devices( d->outputs, tag );
}

std::vector<std::string> AssuanCommand::bulkOutputDeviceTags() const {
    return keys( d->outputs );
}

int AssuanCommand::sendStatus( const char * keyword, const QString & text ) {
    return assuan_write_status( d->ctx.get(), keyword, text.toUtf8().constData() );
}

int AssuanCommand::sendData( const QByteArray & data, bool moreToCome ) {
    if ( const gpg_error_t err = assuan_send_data( d->ctx.get(), data.constData(), data.size() ) )
        return err;
    if ( moreToCome )
        return 0;
    else
        return assuan_send_data( d->ctx.get(), 0, 0 ); // flush
}

int AssuanCommand::inquire( const char * keyword, QObject * receiver, const char * slot, unsigned int maxSize ) {
    assert( keyword );
    assert( receiver );
    assert( slot );

#ifdef HAVE_ASSUAN_INQUIRE_EXT
    std::auto_ptr<InquiryHandler> ih( new InquiryHandler( keyword, receiver ) );
    receiver->connect( ih.get(), SIGNAL(signal(int,QByteArray,QByteArray)), slot );
    if ( const gpg_error_t err = assuan_inquire_ext( d->ctx.get(), keyword,
# ifndef HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT
                                                     &ih->buffer, &ih->buflen,
# endif
                                                     maxSize, InquiryHandler::handler, ih.get() ) )
         return err;
    ih.release();
    return 0;
#else
    return makeError( GPG_ERR_NOT_SUPPORTED ); // libassuan too old
#endif // HAVE_ASSUAN_INQUIRE_EXT
}

void AssuanCommand::done( int err, const QString & details ) {
    if ( d->ctx && !d->done && !details.isEmpty() ) {
        d->utf8ErrorKeepAlive = details.toUtf8();
        assuan_set_error( d->ctx.get(), err, d->utf8ErrorKeepAlive.constData() );
    }
    done( err );
}

void AssuanCommand::done( int err ) {
    if ( !d->ctx ) {
        qDebug( "AssuanCommand::done( %s ): called with NULL ctx.", gpg_strerror( err ) );
        return;
    }
    if ( d->done ) {
        qDebug( "AssuanCommand::done( %s ): called twice!", gpg_strerror( err ) );
        return;
    }

    d->done = true;

    for ( std::map< std::string, std::vector<IO> >::const_iterator it = d->inputs.begin(), end = d->inputs.end() ; it != end ; ++it )
        Q_FOREACH( const IO & io, it->second ) {
            if ( io.iodev )
                io.iodev->close();
            delete io.iodev;
        }

    for ( std::map< std::string, std::vector<IO> >::const_iterator it = d->outputs.begin(), end = d->outputs.end() ; it != end ; ++it )
        Q_FOREACH( const IO & io, it->second ) {
            if ( io.iodev )
                io.iodev->close();
            delete io.iodev;
        }

    const gpg_error_t rc = assuan_process_done( d->ctx.get(), err );
    if ( rc )
        qFatal( "AssuanCommand::done: assuan_process_done returned error %d (%s)",
                static_cast<int>(rc), gpg_strerror(rc) );
    d->utf8ErrorKeepAlive.clear();
    d->ctx.reset();
}


int AssuanCommandFactory::_handle( assuan_context_t ctx, char * line, const char * commandName ) {
    assert( assuan_get_pointer( ctx ) );

    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx ) );

    const std::vector< shared_ptr<AssuanCommandFactory> >::const_iterator it
        = std::lower_bound( conn.factories.begin(), conn.factories.end(), commandName, _detail::ByName<std::less>() );
    assert( it != conn.factories.end() );
    assert( *it );
    assert( qstricmp( (*it)->name(), commandName ) == 0 );

    const shared_ptr<AssuanCommand> cmd = (*it)->create();
    assert( cmd );

    cmd->d->ctx     = conn.ctx;
    cmd->d->options = conn.options;
    cmd->d->inputs.swap( conn.inputs );   assert( conn.inputs.empty() );
    cmd->d->outputs.swap( conn.outputs ); assert( conn.outputs.empty() );

    const std::map<std::string,std::string> cmdline_options = parse_commandline( line );
    for ( std::map<std::string,std::string>::const_iterator it = cmdline_options.begin(), end = cmdline_options.end() ; it != end ; ++it )
        cmd->d->options[it->first] = QString::fromUtf8( it->second.c_str() );
    
    if ( const int err = cmd->start() )
        return err;

    conn.currentCommand = cmd;
    return 0;
}


#include "assuanserverconnection.moc"
#include "moc_assuanserverconnection.cpp"
