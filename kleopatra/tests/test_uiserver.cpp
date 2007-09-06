/* -*- mode: c++; c-basic-offset:4 -*-
    tests/test_uiserver.cpp

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

//
// Usage: test_uiserver <socket> --verify-detached <signed data> <signature>
//
#ifndef _ASSUAN_ONLY_GPG_ERRORS
#define _ASSUAN_ONLY_GPG_ERRORS
#endif
#include "../uiserver/kleo-assuan.h"
#include <gpg-error.h>

#include <QtCore>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace Kleo;

static int inFD = -1, outFD = -1;
static std::map<std::string,std::string> inquireData;

static void usage( const std::string & msg=std::string() ) {
    std::cerr << msg << std::endl <<
        "\n"
        "Usage: test_uiserver <socket> [<io>] [<inquire>] command [<args>]\n"
        "where:\n"
        "      <io>: [--input <file>] [--output <file>] *[--option name=value]\n"
        " <inquire>: [--inquire keyword=<file>]\n";
    exit( 1 );
}

static int data( void * void_ctx, const void * buffer, size_t len ) {
    (void)void_ctx; (void)buffer; (void)len;
    return 0; // ### implement me
}

static int status( void * void_ctx, const char * line ) {
    (void)void_ctx; (void)line;
    return 0;
}

static int inquire( void * void_ctx, const char * keyword ) {
    assuan_context_t ctx = (assuan_context_t)void_ctx;
    assert( ctx );
    const std::map<std::string,std::string>::const_iterator it = inquireData.find( keyword );
    if ( it == inquireData.end() )
        return gpg_error( GPG_ERR_UNKNOWN_COMMAND );

    if ( !it->second.empty() && it->second[0] == '@' )
        return gpg_error( GPG_ERR_NOT_IMPLEMENTED );

    if ( const gpg_error_t err = assuan_send_data( ctx, it->second.c_str(), it->second.size() ) ) {
        qDebug( "assuan_write_data: %s", gpg_strerror( err ) );
        return err;
    }

    return 0;

#if 0
    if ( qstrcmp( keyword, "DETACHEDSIGNATURE" ) == 0 ) {
        // copy data from sigFD into assuan
        for ( ;; ) {
            char buffer[4096];
            const ssize_t read = ::read( sigFD, buffer, sizeof buffer );
            if ( read == -1 )
                if ( errno == EAGAIN )
                    continue;
                else
                    return gpg_err_code_from_errno( errno );
            if ( read == 0 ) { // EOF
                // don't, assuan_transact does that itself! if ( const gpg_error_t = assuan_write_line( ctx, 0, 0 ) )
                return 0;
            }
            if ( const gpg_error_t err = assuan_send_data( ctx, buffer, read ) ) {
                qDebug( "assuan_write_data: %s", gpg_strerror( err ) );
                return err;
            }
        }
    } else {
        return gpg_error( GPG_ERR_UNKNOWN_COMMAND );
    }
#endif
}

int main( int argc, char * argv[] ) {

    assuan_set_assuan_err_source( GPG_ERR_SOURCE_DEFAULT );

    if ( argc < 3 )
        usage(); // need socket and command, at least

    const char * socket = argv[1];

    std::vector<const char*> options;

    std::string command;
    for ( int optind = 2 ; optind < argc ; ++optind ) {
        const char * const arg = argv[optind];
        if ( qstrcmp( arg, "--input" ) == 0 ) {
            if ( inFD != -1 )
                usage( "more than one --input given" );
            if ( (inFD = open( argv[++optind], O_RDONLY )) == -1 ) {
                perror( "--input open()" );
                return 1;
            }
        } else if ( qstrcmp( arg, "--output" ) == 0 ) {
            if ( outFD != -1 )
                usage( "more than one --output given" );
            if ( (outFD = open( argv[++optind], O_WRONLY|O_CREAT )) ==  -1 ) {
                perror( "--output open()" );
                return 1;
            }
        } else if ( qstrcmp( arg, "--option" ) == 0 ) {
            options.push_back( argv[++optind] );
        } else if ( qstrcmp( arg, "--inquire" ) == 0 ) {
            const std::string inqval = argv[++optind];
            const size_t pos = inqval.find( '=' );
            // ### implement indirection with "@file"...
            inquireData[inqval.substr( 0, pos )] = inqval.substr( pos+1 );
        } else {
            while ( optind < argc ) {
                if ( !command.empty() )
                    command += ' ';
                command += argv[optind++];
            }
        }
    }
    if ( command.empty() )
        usage( "Command expected, but only options found" );

    assuan_context_t ctx = 0;

    if ( const gpg_error_t err = assuan_socket_connect_ext( &ctx, socket, -1, 1 ) ) {
        qDebug( "%s", assuan_exception( err, "assuan_socket_connect_ext" ).what() );
        return 1;
    }

    assuan_set_log_stream( ctx, stderr );

    if ( inFD != -1 ) {
        if ( const gpg_error_t err = assuan_sendfd( ctx, inFD ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_sendfd( inFD )" ).what() );
            return 1;
        }

        if ( const gpg_error_t err = assuan_write_line( ctx, "INPUT FD" ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_write_line(\"INPUT FD\")" ).what() );
            return 1;
        }
    }

    
    if ( outFD != -1 ) {
        if ( const gpg_error_t err = assuan_sendfd( ctx, outFD ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_sendfd( outFD )" ).what() );
            return 1;
        }

        if ( const gpg_error_t err = assuan_write_line( ctx, "OUTPUT FD" ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_write_line(\"OUTPUT FD\")" ).what() );
            return 1;
        }
    }

    Q_FOREACH( const char * opt, options ) {
        std::string line = "OPTION ";
        line += opt;
        if ( const gpg_error_t err = assuan_write_line( ctx, line.c_str() ) ) {
            qDebug( "%s", assuan_exception( err, line ).what() );
            return 1;
        }
    }

    if ( const gpg_error_t err = assuan_transact( ctx, command.c_str(), data, ctx, inquire, ctx, status, ctx ) ) {
        qDebug( "%s", assuan_exception( err, command ).what() );
        return 1;
    }

    assuan_disconnect( ctx );
    
    return 0;
}
