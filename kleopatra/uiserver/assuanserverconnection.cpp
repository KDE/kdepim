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

#include "detail_p.h"

#include "assuanserverconnection.h"

#include "assuancommand.h"

#include "kleo-assuan.h"

#include <QSocketNotifier>
#include <QVariant>

#include <boost/type_traits/remove_pointer.hpp>

#include <vector>
#include <map>
#include <string>

#ifdef __GNUC__
# include <ext/algorithm> // for is_sorted
#endif

#ifdef Q_OS_WIN32
# include <io.h>
#endif

using namespace Kleo;
using namespace boost;

static const unsigned int INIT_SOCKET_FLAGS = 2; // says info assuan...
static int(*USE_DEFAULT_HANDLER)(assuan_context_t,char*) = 0;
static const int FOR_READING = 0;
static const unsigned int MAX_ACTIVE_FDS = 32;

// shared_ptr for assuan_context_t w/ deleter enforced to assuan_deinit_server:
typedef shared_ptr< remove_pointer<assuan_context_t>::type > AssuanContextBase;
struct AssuanContext : AssuanContextBase {
    AssuanContext() : AssuanContextBase() {}
    explicit AssuanContext( assuan_context_t ctx ) : AssuanContextBase( ctx, &assuan_deinit_server ) {}

    void reset( assuan_context_t ctx=0 ) { AssuanContextBase::reset( ctx, &assuan_deinit_server ); }
};

class AssuanServerConnection::Private : public QObject {
    Q_OBJECT
    friend class ::Kleo::AssuanServerConnection;
    friend class ::Kleo::AssuanCommandFactory;
public:
    Private( int fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_ );
    ~Private();

public Q_SLOTS:
    void slotReadActivity( int ) {
        assert( ctx );
        if ( const int err = assuan_process_next( ctx.get() ) ) {
            if ( err == -1 || gpg_err_code(err) == GPG_ERR_EOF ) {
                if ( currentCommand )
                    ;//currentCommand->canceled(); // ### error: is private
            } else {
                // ### what?
            }
            //###emit q->closed( );
            cleanup();
        }
    }

private:
    static int handler( assuan_context_t ctx_, char * line_, const char * commandName ) {
        assert( assuan_get_pointer( ctx_ ) );

        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        const std::vector< shared_ptr<AssuanCommandFactory> >::const_iterator it
            = std::lower_bound( conn.factories.begin(), conn.factories.end(), commandName, _detail::ByName<std::less>() );
        assert( it != conn.factories.end() );
        assert( *it );
        assert( qstricmp( (*it)->name(), commandName ) == 0 );

        const shared_ptr<AssuanCommand> cmd = (*it)->create();
        assert( cmd );

        //### cmd->d->options = conn.options;
        //### cmd->d->input device = new KDPipeIODevice
        //### cmd->d->putput device = new KDPipeIODevice
        //### cmd->d->ctx = ctx;
        if ( const int err = cmd->start( line_ ) )
            return err;

        conn.currentCommand = cmd;
        return 0;
    };

    static int option_handler( assuan_context_t ctx_, const char * key, const char * value ) {
        assert( assuan_get_pointer( ctx_ ) );
        
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        conn.options[key] = QString::fromUtf8( value );

        return 0;
        //return gpg_error( GPG_ERR_UNKNOWN_OPTION );
    }

    void cleanup();
    void setCommandFactories( const std::vector< shared_ptr<AssuanCommandFactory> > & factories );
    void setSocketDescriptor( int fd );    


    int fd;
    AssuanContext ctx;
    std::vector< shared_ptr<QSocketNotifier> > notifiers;
    std::vector< shared_ptr<AssuanCommandFactory> > factories; // sorted: _detail::ByName<std::less>
    shared_ptr<AssuanCommand> currentCommand;
    std::map<std::string,QVariant> options;
};

int AssuanCommandFactory::_handle( assuan_context_t ctx, char * line, const char * commandName ) {
    return AssuanServerConnection::Private::handler( ctx, line, commandName );
}

void AssuanServerConnection::Private::cleanup() {
    options.clear();
    currentCommand.reset();
    notifiers.clear();
    ctx.reset();
    fd = -1;
}

AssuanServerConnection::Private::Private( int fd_, const std::vector< shared_ptr<AssuanCommandFactory> > & factories_ )
    : fd( fd_ ), factories( factories_ )
{
#ifdef __GNUC__
    assert( __gnu_cxx::is_sorted( factories_.begin(), factories_.end(), _detail::ByName<std::less>() ) );
#endif

    if ( fd < 0 )
        throw assuan_exception( gpg_error( GPG_ERR_INV_ARG ), "pre-assuan_init_socket_server_ext" );

    assuan_context_t naked_ctx = 0;
    if ( const gpg_error_t err = assuan_init_socket_server_ext( &naked_ctx, fd, INIT_SOCKET_FLAGS ) )
        throw assuan_exception( err, "assuan_init_socket_server_ext" );
    
    ctx.reset( naked_ctx ); naked_ctx = 0;

    // for callbacks, associate the context with this connection:
    assuan_set_pointer( ctx.get(), this );


    // register FDs with the event loop:
    assuan_fd_t fds[MAX_ACTIVE_FDS];
    const int numFDs = assuan_get_active_fds( ctx.get(), FOR_READING, fds, MAX_ACTIVE_FDS );
    assert( numFDs > 0 ); // == 1

    notifiers.reserve( numFDs );
    for ( int i = 0 ; i < numFDs ; ++i ) {
        const shared_ptr<QSocketNotifier> sn( new QSocketNotifier( _detail::translate_sys2libc_fd( fds[i], false ), QSocketNotifier::Read ) );
        connect( sn.get(), SIGNAL(activated(int)), this, SLOT(slotReadActivity(int)) );
        notifiers.push_back( sn );
    }


    // enable INPUT/OUTPUT default handlers:
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "INPUT",  USE_DEFAULT_HANDLER ) )
        throw assuan_exception( err, "activate \"INPUT\" default handler" );
    if ( const gpg_error_t err = assuan_register_command( ctx.get(), "OUTPUT", USE_DEFAULT_HANDLER ) )
        throw assuan_exception( err, "activate \"OUTPUT\" default handler" );

    // register user-defined commands:
    Q_FOREACH( shared_ptr<AssuanCommandFactory> fac, factories )
        if ( const gpg_error_t err = assuan_register_command( ctx.get(), fac->name(), fac->_handler() ) )
            throw assuan_exception( err, std::string( "register \"" ) + fac->name() + "\" handler" );

    //assuan_set_hello_line( ctx.get(), GPG UI server (qApp->applicationName() + " v" + kapp->applicationVersion() + "ready to serve" )

    // register options:
    if ( const gpg_error_t err = assuan_register_option_handler( ctx.get(), option_handler ) )
        throw assuan_exception( err, "register option handler" );
}

AssuanServerConnection::Private::~Private() {
    cleanup();
}

AssuanServerConnection::AssuanServerConnection( int fd, const std::vector< shared_ptr<AssuanCommandFactory> > & factories )
    : d( new Private( fd, factories ) )
{

}

AssuanServerConnection::~AssuanServerConnection() {}

#include "assuanserverconnection.moc"
