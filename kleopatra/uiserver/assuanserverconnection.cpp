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
#include <boost/ref.hpp>

#include <typeinfo>
#include <vector>
#include <map>
#include <string>

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
    friend class ::Kleo::AssuanCommand;
public:
    Private( int fd_, const std::vector< shared_ptr<AssuanCommand> > & commands_ );
    ~Private();

public Q_SLOTS:
    void slotReadActivity( int ) {
        if ( ctx )
            assuan_process_next( ctx.get() );
    }

private:
    static int handler( assuan_context_t ctx_, char * line_, const std::type_info & ti_ ) {
        assert( assuan_get_pointer( ctx_ ) );

        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        const std::vector< shared_ptr<AssuanCommand> >::const_iterator it = std::lower_bound( conn.commands.begin(), conn.commands.end(), ti_, _detail::ByTypeId() );
        assert( it != conn.commands.end() );
        assert( typeid(*it->get()) == ti_ );

        if ( const int err = (*it)->start( line_, conn.options ) )
            return err;

        conn.currentCommand = *it;
        return 0;
    };

    static int option_handler( assuan_context_t ctx_, const char * key, const char * value ) {
        assert( assuan_get_pointer( ctx_ ) );
        
        AssuanServerConnection::Private & conn = *static_cast<AssuanServerConnection::Private*>( assuan_get_pointer( ctx_ ) );

        conn.options.insert( std::make_pair( key, QString::fromUtf8( value ) ) );

        return 0;
        //return gpg_error( GPG_ERR_UNKNOWN_OPTION );
    }

    void cleanup();
    void setCommands( const std::vector< shared_ptr<AssuanCommand> > & commands );
    void setSocketDescriptor( int fd );    


    int fd;
    AssuanContext ctx;
    std::vector< shared_ptr<QSocketNotifier> > notifiers;
    std::vector< shared_ptr<AssuanCommand> > commands; // sorted: _detail::ByTypeId
    shared_ptr<AssuanCommand> currentCommand;
    std::map<std::string,QVariant> options;
};

int AssuanCommand::_handle( assuan_context_t ctx, char * line, const std::type_info & ti ) {
    return AssuanServerConnection::Private::handler( ctx, line, ti );
}

void AssuanServerConnection::Private::cleanup() {
    options.clear();
    currentCommand.reset();
    notifiers.clear();
    ctx.reset();
    fd = -1;
}

AssuanServerConnection::Private::Private( int fd_, const std::vector< shared_ptr<AssuanCommand> > & commands_ )
    : fd( fd_ ), commands( commands_ )
{
    std::sort( commands.begin(), commands.end(), _detail::ByTypeId() );

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
    Q_FOREACH( shared_ptr<AssuanCommand> cmd, commands )
        if ( const gpg_error_t err = assuan_register_command( ctx.get(), cmd->name(), cmd->_handler() ) )
            throw assuan_exception( err, std::string( "register \"" ) + cmd->name() + "\" handler" );

    //assuan_set_hello_line( ctx.get(), GPG UI server (qApp->applicationName() + " v" + kapp->applicationVersion() + "ready to serve" )

    // register options:
    if ( const gpg_error_t err = assuan_register_option_handler( ctx.get(), option_handler ) )
        throw assuan_exception( err, "register option handler" );
}

AssuanServerConnection::Private::~Private() {
    cleanup();
}

AssuanServerConnection::AssuanServerConnection( int fd, const std::vector< shared_ptr<AssuanCommand> > & cmds )
    : d( new Private( fd, cmds ) )
{

}

AssuanServerConnection::~AssuanServerConnection() {}

#include "assuanserverconnection.moc"
