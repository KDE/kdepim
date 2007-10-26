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
#ifndef QT_NO_CAST_TO_ASCII
# define QT_NO_CAST_TO_ASCII
#endif
#ifndef QT_NO_CAST_FROM_ASCII
# define QT_NO_CAST_FROM_ASCII
#endif

#include <config-kleopatra.h>

#include "assuanserverconnection.h"
#include "assuancommand.h"

#include "detail_p.h"
#include "kleo-assuan.h"

#include <utils/kdpipeiodevice.h>

#include <gpgme++/data.h>

#include <KLocale>

#include <QSocketNotifier>
#include <QVariant>
#include <QPointer>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDebug>
#include <QStringList>

#include <boost/type_traits/remove_pointer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>

#include <errno.h>

#ifdef __GNUC__
# include <ext/algorithm> // for is_sorted
#endif

#ifdef Q_OS_WIN32
# include <io.h>
# include <process.h>
#else
# include <sys/types.h>
# include <unistd.h>
#endif

using namespace Kleo;
using namespace boost;

namespace {
    struct IO {
        QString fileName;
        shared_ptr<QIODevice> iodev;
    };

    struct IOF {
        QString fileName;
        shared_ptr<QFile> file;
    };

    static inline qint64 mygetpid() {
#ifdef Q_OS_WIN32
        return (qint64)_getpid();
#else
        return (qint64)getpid();
#endif
    }
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

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const char * err_msg ) {
    return assuan_process_done( ctx, assuan_set_error( ctx, err, err_msg ) );
}

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const std::string & err_msg ) {
    return assuan_process_done_msg( ctx, err, err_msg.c_str() );
}

static inline gpg_error_t assuan_process_done_msg( assuan_context_t ctx, gpg_error_t err, const QString & err_msg ) {
    return assuan_process_done_msg( ctx, err, err_msg.toUtf8().constData() );
}

static unsigned char unhex( unsigned char ch ) {
    if ( ch >= '0' && ch <= '9' )
        return ch - '0';
    if ( ch >= 'A' && ch <= 'F' )
        return ch - 'A' + 10;
    if ( ch >= 'a' && ch <= 'f' )
        return ch - 'a' + 10;
    throw gpg_error( GPG_ERR_ASS_SYNTAX );
}

static std::string hexdecode( const std::string & in ) {
    std::string result;
    result.reserve( in.size() );
    for ( std::string::const_iterator it = in.begin(), end = in.end() ; it != end ; ++it )
        if ( *it == '%' ) {
            ++it;
            unsigned char ch = '\0';
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

static std::map<std::string,std::string> upcase_option( const char * option, std::map<std::string,std::string> options ) {
    std::string value;
    bool value_found = false;
    std::map<std::string,std::string>::iterator it = options.begin();
    while ( it != options.end() )
        if ( qstricmp( it->first.c_str(), option ) == 0 ) {
            value = it->second;
            options.erase( it++ );
            value_found = true;
        } else {
            ++it;
        }
    if ( value_found )
        options[option] = value;
    return options;
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
            //if ( err == -1 || gpg_err_code(err) == GPG_ERR_EOF ) {
                topHalfDeletion();
                if ( nohupedCommands.empty() )
                    bottomHalfDeletion();
            //} else {
                //assuan_process_done( ctx.get(), err );
                //return;
            //}
        }
    }

private:

    void nohupDone( AssuanCommand * cmd ) {
        const std::vector< shared_ptr<AssuanCommand> >::iterator it
            = std::find_if( nohupedCommands.begin(), nohupedCommands.end(),
                            bind( &shared_ptr<AssuanCommand>::get, _1 ) == cmd );
        assert( it != nohupedCommands.end() );
        nohupedCommands.erase( it );
        if ( nohupedCommands.empty() && closed )
            bottomHalfDeletion();
    }

    void topHalfDeletion() {
        if ( currentCommand )
            currentCommand->canceled();
        notifiers.clear();
        closed = true;
    }

    void bottomHalfDeletion() {
        cleanup();
        const QPointer<Private> that = this;
        emit q->closed( q );
        if ( that ) // still there
            q->deleteLater();
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

    static int getinfo_handler( assuan_context_t ctx_, char * line ) {
        if ( qstrcmp( line, "version" ) == 0 ) {
            static const char version[] = "Kleopatra " KLEOPATRA_VERSION_STRING ;
            return assuan_process_done( ctx_, assuan_send_data( ctx_, version, sizeof version - 1 ) );
        }
        if ( qstrcmp( line, "pid" ) == 0 ) {
            static const QByteArray pid = QByteArray::number( mygetpid() );
            return assuan_process_done( ctx_, assuan_send_data( ctx_, pid.constData(), pid.size() ) );
        }
        static const QString errorString = i18n("Unknown value for WHAT");
        return assuan_process_done_msg( ctx_, gpg_error( GPG_ERR_ASS_PARAMETER ), errorString );
    }

    // format: TAG (FD|FD=\d+|FILE=...)
    template <typename T_memptr>
    static int IO_handler( assuan_context_t ctx_, char * line_, bool in, T_memptr which ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        try {

            /*const*/ std::map<std::string,std::string> options = upcase_option( "FD", upcase_option( "FILE", parse_commandline( line_ ) ) );
            if ( options.size() < 1 || options.size() > 2 )
                throw gpg_error( GPG_ERR_ASS_SYNTAX );

            IO io;

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

                std::auto_ptr<KDPipeIODevice> pio( new KDPipeIODevice );
                if ( !pio->open( fd, in ? QIODevice::ReadOnly : QIODevice::WriteOnly ) )
                    throw gpg_error_from_errno( errno );

                io.iodev = pio;

                options.erase( "FD" );

            } else if ( options.count( "FILE" ) ) {
                
                if ( options.count( "FD" ) )
                    throw gpg_error( GPG_ERR_CONFLICT );

                const QString filePath = QFile::decodeName( options["FILE"].c_str() );
                if ( filePath.isEmpty() )
                    throw assuan_exception( gpg_error( GPG_ERR_ASS_SYNTAX ), i18n("Empty file path") );
                const QFileInfo fi( filePath );
                if ( !fi.isAbsolute() )
                    throw assuan_exception( gpg_error( GPG_ERR_INV_ARG ), i18n("Only absolute file paths are allowed") );

                io.fileName = fi.absoluteFilePath();

                if ( in ) {
                    std::auto_ptr<QFile> f( new QFile( io.fileName ) );
                    if ( !f->open( QIODevice::ReadOnly ) )
                        throw assuan_exception( gpg_error_from_errno( errno ),
                                                i18n( "Couldn't open file \"%1\" for reading", io.fileName ) );
                    io.iodev = f;
                } else {
                    std::auto_ptr<QTemporaryFile> f( new QTemporaryFile( io.fileName ) );
                    if ( !f->open() )
                        throw assuan_exception( gpg_error_from_errno( errno ),
                                                i18n( "Couldn't create temporary file \"%1\"", f->fileName() ) );
                    io.iodev = f;
                }

                options.erase( "FILE" );

            } else {

                throw gpg_error( GPG_ERR_ASS_PARAMETER );

            }

            if ( options.size() )
                throw gpg_error( GPG_ERR_UNKNOWN_OPTION );

            (conn.*which).push_back( io );

            qDebug() << "AssuanServerConnection: added" << (in ? "input" : "output") << '('
                     << io.fileName << io.iodev << ')';

            return assuan_process_done( conn.ctx.get(), 0 );
        } catch ( const GpgME::Exception & e ) {
            return assuan_process_done_msg( conn.ctx.get(), e.error(), e.message().c_str() );
        } catch ( const std::exception & e ) {
            return assuan_process_done( conn.ctx.get(), gpg_error( GPG_ERR_ASS_SYNTAX ) );
        } catch ( const gpg_error_t e ) {
            return assuan_process_done( conn.ctx.get(), e );
        } catch ( ... ) {
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), "unknown exception caught" );
        }

    }

    static int input_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, true, &Private::inputs );
    }

    static int output_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, false, &Private::outputs );
    }

    static int message_handler( assuan_context_t ctx, char * line ) {
        return IO_handler( ctx, line, true, &Private::messages );
    }

    static int file_handler( assuan_context_t ctx_, char * line ) {
        assert( assuan_get_pointer( ctx_ ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        try {
            const QFileInfo fi( QFile::decodeName( line ) );
            if ( !fi.isAbsolute() )
                throw assuan_exception( gpg_error( GPG_ERR_INV_ARG ), i18n("Only absolute file paths are allowed") );
            if ( !fi.isFile() )
                throw assuan_exception( gpg_error( GPG_ERR_NOT_IMPLEMENTED ), i18n("Directory traversal is not yet implemented") );
            const QString filePath = fi.absoluteFilePath();
            const shared_ptr<QFile> file( new QFile( filePath ) );
            if ( file->open( QIODevice::ReadOnly ) )
                throw assuan_exception( gpg_error_from_errno( errno ), i18n("Could not open file \"%1\" for reading", filePath) );
            const IOF io = {
                filePath, file
            };
            conn.files.push_back( io );

            return 0;
        } catch ( const assuan_exception & e ) {
            return assuan_process_done_msg( conn.ctx.get(), e.error(), e.message().toUtf8().constData() );
        } catch ( ... ) {
            return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), i18n("unknown exception caught").toUtf8().constData() );
        }
    }

    template <typename T_memptr>
    static int recipient_sender_handler( T_memptr mp, assuan_context_t ctx, char * line ) {
        assert( assuan_get_pointer( ctx ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx ) );

        (conn.*mp).push_back( QString::fromUtf8( hexdecode( line ).c_str() ) );
        return assuan_process_done( ctx, 0 );
    }

    static int recipient_handler( assuan_context_t ctx, char * line ) {
        return recipient_sender_handler( &Private::recipients, ctx, line );
    }

    static int sender_handler( assuan_context_t ctx, char * line ) {
        return recipient_sender_handler( &Private::senders, ctx, line );
    }

    void cleanup();


    assuan_fd_t fd;
    AssuanContext ctx;
    bool closed;
    std::vector< shared_ptr<QSocketNotifier> > notifiers;
    std::vector< shared_ptr<AssuanCommandFactory> > factories; // sorted: _detail::ByName<std::less>
    shared_ptr<AssuanCommand> currentCommand;
    std::vector< shared_ptr<AssuanCommand> > nohupedCommands;
    std::map<std::string,QVariant> options;
    std::vector<QString> senders, recipients;
    std::vector<IO> inputs, outputs, messages;
    std::vector<IOF> files;
    std::map< QByteArray, shared_ptr<AssuanCommand::Memento> > mementos;
};

void AssuanServerConnection::Private::cleanup() {
    assert( nohupedCommands.empty() );
    options.clear();
    currentCommand.reset();
    options.clear();
    mementos.clear();
    notifiers.clear();
    ctx.reset();
    fd = ASSUAN_INVALID_FD;
}

AssuanServerConnection::Private::Private( assuan_fd_t fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_, AssuanServerConnection * qq )
    : QObject(), q( qq ), fd( fd_ ), closed( false ), factories( factories_ )
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


    // register our INPUT/OUTPUT/MESSGAE/FILE handlers:
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "INPUT",  input_handler ) )
        throw assuan_exception( err, "register \"INPUT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "MESSAGE",  message_handler ) )
        throw assuan_exception( err, "register \"MESSAGE\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "OUTPUT", output_handler ) )
        throw assuan_exception( err, "register \"OUTPUT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "FILE", file_handler ) )
        throw assuan_exception( err, "register \"FILE\" handler" );


    // register user-defined commands:
    Q_FOREACH( shared_ptr<AssuanCommandFactory> fac, factories )
        if ( const gpg_error_t err = assuan_register_command( ctx.get(), fac->name(), fac->_handler() ) )
            throw assuan_exception( err, std::string( "register \"" ) + fac->name() + "\" handler" );

    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "GETINFO", getinfo_handler ) )
        throw assuan_exception( err, "register \"GETINFO\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "RECIPIENT", recipient_handler ) )
        throw assuan_exception( err, "register \"RECIPIENT\" handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "SENDER", sender_handler ) )
        throw assuan_exception( err, "register \"SENDER\" handler" );

    assuan_set_hello_line( ctx.get(), "GPG UI server (Kleopatra/" KLEOPATRA_VERSION_STRING ") ready to serve" );
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
    Private() : done( false ), nohup( false ) {}

    std::map<std::string,QVariant> options;
    std::vector<IO> inputs, messages, outputs;
    std::vector<IOF> files;
    std::vector<QString> recipients, senders;
    QByteArray utf8ErrorKeepAlive;
    AssuanContext ctx;
    bool done;
    bool nohup;
public:
};

AssuanCommand::AssuanCommand()
    : d( new Private )
{

}

AssuanCommand::~AssuanCommand() {

}

int AssuanCommand::start() {
    try {
        if ( const int err = doStart() ) {
            if ( !d->done )
                done( err );
            return err;
        }
        return 0;
    } catch ( const assuan_exception & e ) {
        if ( !d->done )
            done( e.error_code(), e.message() );
        return e.error_code();
    } catch ( const GpgME::Exception & e ) {
        if ( !d->done )
            done( e.error(), QString::fromLocal8Bit( e.message().c_str() ) );
        return e.error();
    } catch ( const std::exception & e ) {
        if ( !d->done )
            done( makeError( GPG_ERR_INTERNAL ), i18n("Caught unexpected exception: %1", QString::fromLocal8Bit( e.what() ) ) );
        return makeError( GPG_ERR_INTERNAL );
    } catch ( ... ) {
        if ( !d->done )
            done( makeError( GPG_ERR_INTERNAL ), i18n("Caught unknown exception - fix the program!" ) );
        return makeError( GPG_ERR_INTERNAL );
    }
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

QStringList AssuanCommand::fileNames() const {
    QStringList result;
    Q_FOREACH( const IOF & io, d->files )
        result.push_back( io.fileName );
    return result;
}

std::vector< shared_ptr<QFile> > AssuanCommand::files() const {
    std::vector< shared_ptr<QFile> > result;
    Q_FOREACH( const IOF & io, d->files )
        result.push_back( io.file );
    return result;
}

unsigned int AssuanCommand::numFiles() const {
    return d->files.size();
}

QString AssuanCommand::bulkInputDeviceFileName( unsigned int idx ) const {
    return d->inputs.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkInputDevice( unsigned int idx ) const {
    return d->inputs.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkInputDevices() const {
    return d->inputs.size();
}


QString AssuanCommand::bulkMessageDeviceFileName( unsigned int idx ) const {
    return d->messages.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkMessageDevice( unsigned int idx ) const {
    return d->messages.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkMessageDevices() const {
    return d->messages.size();
}


QString AssuanCommand::bulkOutputDeviceFileName( unsigned int idx ) const {
    return d->outputs.at( idx ).fileName;
}

shared_ptr<QIODevice> AssuanCommand::bulkOutputDevice( unsigned int idx ) const {
    return d->outputs.at( idx ).iodev;
}

unsigned int AssuanCommand::numBulkOutputDevices() const {
    return d->outputs.size();
}

int AssuanCommand::sendStatus( const char * keyword, const QString & text ) {
    if ( d->nohup )
        return 0;
    return assuan_write_status( d->ctx.get(), keyword, text.toUtf8().constData() );
}

int AssuanCommand::sendData( const QByteArray & data, bool moreToCome ) {
    if ( d->nohup )
        return 0;
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

    if ( d->nohup )
        return makeError( GPG_ERR_INV_OP );

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
        if ( !d->nohup )
            assuan_set_error( d->ctx.get(), err, d->utf8ErrorKeepAlive.constData() );
    }
    done( err );
}

static void close_all( const std::vector<IO> & ios ) {
    Q_FOREACH( const IO & io, ios )
        if ( io.iodev && io.iodev->isOpen() )
            io.iodev->close();
}

static void close_all( const std::vector<IOF> & ios ) {
    Q_FOREACH( const IOF & io, ios )
        if ( io.file && io.file->isOpen() )
            io.file->close();
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

    close_all( d->inputs );
    close_all( d->messages );
    close_all( d->outputs );
    close_all( d->files ); // ### ???

    if ( d->nohup ) {

        // oh, hack :(
        assert( assuan_get_pointer( d->ctx.get() ) );
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( d->ctx.get() ) );

        conn.nohupDone( this );

        return;

    } else {

        const gpg_error_t rc = assuan_process_done( d->ctx.get(), err );
        if ( gpg_err_code( rc ) != GPG_ERR_NO_ERROR )
            qFatal( "AssuanCommand::done: assuan_process_done returned error %d (%s)",
                    static_cast<int>(rc), gpg_strerror(rc) );

    }

    d->utf8ErrorKeepAlive.clear();
    d->ctx.reset();
}


bool AssuanCommand::isNohup() const {
    return d->nohup;
}

QStringList AssuanCommand::recipients() const {
    QStringList result;
    std::copy( d->recipients.begin(), d->recipients.end(), std::back_inserter( result ) );
    return result;
}

QStringList AssuanCommand::senders() const {
    QStringList result;
    std::copy( d->senders.begin(), d->senders.end(), std::back_inserter( result ) );
    return result;
}

int AssuanCommandFactory::_handle( assuan_context_t ctx, char * line, const char * commandName ) {
    assert( assuan_get_pointer( ctx ) );
    AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx ) );

    try {

        const std::vector< shared_ptr<AssuanCommandFactory> >::const_iterator it
            = std::lower_bound( conn.factories.begin(), conn.factories.end(), commandName, _detail::ByName<std::less>() );
        assuan_assert( it != conn.factories.end() );
        assuan_assert( *it );
        assuan_assert( qstricmp( (*it)->name(), commandName ) == 0 );

        const shared_ptr<AssuanCommand> cmd = (*it)->create();
        assuan_assert( cmd );

        cmd->d->ctx     = conn.ctx;
        cmd->d->options = conn.options;
        cmd->d->inputs.swap( conn.inputs );     assuan_assert( conn.inputs.empty() );
        cmd->d->messages.swap( conn.messages ); assuan_assert( conn.messages.empty() );
        cmd->d->outputs.swap( conn.outputs );   assuan_assert( conn.outputs.empty() );
        cmd->d->senders.swap( conn.senders );   assuan_assert( conn.senders.empty() );
        cmd->d->recipients.swap( conn.recipients ); assuan_assert( conn.recipients.empty() );

        const std::map<std::string,std::string> cmdline_options = parse_commandline( line );
        for ( std::map<std::string,std::string>::const_iterator it = cmdline_options.begin(), end = cmdline_options.end() ; it != end ; ++it )
            cmd->d->options[it->first] = QString::fromUtf8( it->second.c_str() );

        bool nohup = false;
        if ( cmd->d->options.count( "nohup" ) ) {
            if ( !cmd->d->options["nohup"].toString().isEmpty() )
                return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_ASS_PARAMETER ), "--nohup takes no argument" );
            nohup = true;
            cmd->d->options.erase( "nohup" );
        }

        if ( const int err = cmd->start() )
            if ( cmd->d->done )
                return err;
            else
                return assuan_process_done( conn.ctx.get(), err );

        if ( cmd->d->done )
            return 0;

        if ( nohup ) {
            cmd->d->nohup = true;
            conn.nohupedCommands.push_back( cmd );
            return assuan_process_done_msg( conn.ctx.get(), 0, "Command put in the background to continue executing after connection end." );
        } else {
            conn.currentCommand = cmd;
            return 0;
        }

    } catch ( const assuan_exception & e ) {
        return assuan_process_done_msg( conn.ctx.get(), e.error_code(), e.message() );
    } catch ( const std::exception & e ) {
        return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), e.what() );
    } catch ( ... ) {
        return assuan_process_done_msg( conn.ctx.get(), gpg_error( GPG_ERR_UNEXPECTED ), i18n("Caught unknown exception") );
    }

}

//
//
// AssuanCommand convenience methods
//
//

/*!
  Checks the \c --mode parameter.

  \returns The parameter as an AssuanCommand::Mode enum value.

  If no \c --mode was given, or it's value wasn't recognized, throws
  an assuan_exception.
*/
AssuanCommand::Mode AssuanCommand::checkMode() const {
    if ( !hasOption( "mode" ) )
        throw assuan_exception( makeError( GPG_ERR_MISSING_VALUE ), i18n( "Required --mode option missing" ) );

    const QString modeString = option("mode").toString().toLower();
    if ( modeString == QLatin1String( "filemanager" ) )
        return FileManager;
    if ( modeString == QLatin1String( "email" ) )
        return EMail;
    throw assuan_exception( makeError( GPG_ERR_INV_ARG ), i18n( "invalid mode: \"%1\"", modeString ) );
}

/*!
  Checks the \c --protocol parameter.

  \returns The parameter as a GpgME::Protocol enum value.

  If \c --protocol was given, but has an invalid value, throws an
  assuan_exception.

  If no \c --protocol was given, in FileManager mode, returns
  GpgME::UnknownProtocol, but if \a mode == \c EMail, throws an
  assuan_exception instead.
*/
GpgME::Protocol AssuanCommand::checkProtocol( Mode mode ) const {
    if ( !hasOption("protocol") )
        if ( mode == AssuanCommand::EMail )
            throw assuan_exception( makeError( GPG_ERR_MISSING_VALUE ), i18n( "Required --protocol option missing" ) );
        else
            return GpgME::UnknownProtocol;
    else
        if ( mode == AssuanCommand::FileManager )
            throw assuan_exception( makeError( GPG_ERR_INV_FLAG ), i18n("--protocol is not allowed here") );

    const QString protocolString = option("protocol").toString().toLower();
    if ( protocolString == QLatin1String( "openpgp" ) )
        return GpgME::OpenPGP;
    if ( protocolString == QLatin1String( "cms" ) )
        return GpgME::CMS;
    throw assuan_exception( makeError( GPG_ERR_INV_ARG ), i18n( "invalid protocol \"%1\"", protocolString ) );
}        

void AssuanCommand::doApplyWindowID( QDialog * dlg ) const {
    if ( !dlg || !hasOption( "window-id" ) )
        return;
    // ### TODO: parse window-id, and apply it to dlg...
}

static QString commonPrefix( const QString & s1, const QString & s2 ) {
    return QString( s1.data(), std::mismatch( s1.data(), s1.data() + std::min( s1.size(), s2.size() ), s2.data() ).first - s1.data() );
}

static QString longestCommonPrefix( const QStringList & sl ) {
    if ( sl.empty() )
        return QString();
    QString result = sl.front();
    Q_FOREACH( const QString & s, sl )
        result = commonPrefix( s, result );
    return result;
}

QString AssuanCommand::heuristicBaseDirectory() const {
    QStringList inputs;
    const unsigned int numInputs = numBulkInputDevices();
    for ( unsigned int i = 0 ; i < numInputs ; ++i ) {
        const QString fname = bulkInputDeviceFileName( i );
        if ( !fname.isEmpty() )
            inputs.push_back( fname );
    }
    const QString candidate = longestCommonPrefix( inputs );
    const QFileInfo fi( candidate );
    if ( fi.isDir() )
        return candidate;
    else
        return fi.absolutePath();
}

#include "assuanserverconnection.moc"
#include "moc_assuanserverconnection.cpp"
