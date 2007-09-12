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

static std::vector<int> inFDs, outFDs, msgFDs;
static std::vector<std::string> inFiles, outFiles, msgFiles;
static std::map<std::string,std::string> inquireData;

static std::string hexencode( const std::string & in ) {
    std::string result;
    result.reserve( 3 * in.size() );

    static const char hex[] = "0123456789ABCDEF";

    for ( std::string::const_iterator it = in.begin(), end = in.end() ; it != end ; ++it )
        switch ( const unsigned char ch = *it ) {
        default:
            if ( ch >= '!' && ch <= '~' || ch > 0xA0 ) {
                result += ch;
                break;
            }
            // else fall through
        case ' ':
            result += '+';
            break;
        case '"':
        case '#':
        case '$':
        case '%':
        case '\'':
        case '+':
        case '=':
            result += '%';
            result += hex[ (ch & 0xF0) >> 4 ];
            result += hex[ (ch & 0x0F)      ];
            break;
        }
    
    return result;
}

static void usage( const std::string & msg=std::string() ) {
    std::cerr << msg << std::endl <<
        "\n"
        "Usage: test_uiserver <socket> [<io>] [<options>] [<inquire>] command [<args>]\n"
        "where:\n"
        "      <io>: [--input[-fd] <file>] [--output[-fd] <file>] [--message[-fd] <file>]\n"
        " <options>: *[--option name=value]\n"
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
        if ( qstrcmp( arg, "--input-fd" ) == 0 ) {
            int inFD;
            if ( (inFD = open( argv[++optind], O_RDONLY )) == -1 ) {
                perror( "--input-fd open()" );
                return 1;
            }
            inFDs.push_back( inFD );
        } else if ( qstrcmp( arg, "--output-fd" ) == 0 ) {
            int outFD;
            if ( (outFD = open( argv[++optind], O_WRONLY|O_CREAT )) ==  -1 ) {
                perror( "--output-fd open()" );
                return 1;
            }
            outFDs.push_back( outFD );
        } else if ( qstrcmp( arg, "--message-fd" ) == 0 ) {
            int msgFD;
            if ( (msgFD = open( argv[++optind], O_RDONLY )) ==  -1 ) {
                perror( "--message-fd open()" );
                return 1;
            }
            msgFDs.push_back( msgFD );
        } else if ( qstrcmp( arg, "--input" ) == 0 ) {
            const std::string file = argv[++optind];
            inFiles.push_back( file );
        } else if ( qstrcmp( arg, "--output" ) == 0 ) {
            const std::string file = argv[++optind];
            outFiles.push_back( file );
        } else if ( qstrcmp( arg, "--message" ) == 0 ) {
            const std::string file = argv[++optind];
            msgFiles.push_back( file );
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

    for ( std::vector<int>::const_iterator it = inFDs.begin(), end = inFDs.end() ; it != end ; ++it ) {
        if ( const gpg_error_t err = assuan_sendfd( ctx, *it ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_sendfd( inFD )" ).what() );
            return 1;
        }

        if ( const gpg_error_t err = assuan_transact( ctx, "INPUT FD", 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, "INPUT FD" ).what() );
            return 1;
        }
    }

    
    for ( std::vector<std::string>::const_iterator it = inFiles.begin(), end = inFiles.end() ; it != end ; ++it ) {
        char buffer[1024];
        sprintf( buffer, "INPUT FILE=%s", hexencode( *it ).c_str() );

        if ( const gpg_error_t err = assuan_transact( ctx, buffer, 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, buffer ).what() );
            return 1;
        }
    }

    
    for ( std::vector<int>::const_iterator it = msgFDs.begin(), end = msgFDs.end() ; it != end ; ++it ) {
        if ( const gpg_error_t err = assuan_sendfd( ctx, *it ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_sendfd( msgFD )" ).what() );
            return 1;
        }

        if ( const gpg_error_t err = assuan_transact( ctx, "MESSAGE FD", 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, "MESSAGE FD" ).what() );
            return 1;
        }
    }

    
    for ( std::vector<std::string>::const_iterator it = msgFiles.begin(), end = msgFiles.end() ; it != end ; ++it ) {
        char buffer[1024];
        sprintf( buffer, "MESSAGE FILE=%s", hexencode( *it ).c_str() );

        if ( const gpg_error_t err = assuan_transact( ctx, buffer, 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, buffer ).what() );
            return 1;
        }
    }

    
    for ( std::vector<int>::const_iterator it = outFDs.begin(), end = outFDs.end() ; it != end ; ++it ) {
        if ( const gpg_error_t err = assuan_sendfd( ctx, *it ) ) {
            qDebug( "%s", assuan_exception( err, "assuan_sendfd( outFD )" ).what() );
            return 1;
        }

        if ( const gpg_error_t err = assuan_transact( ctx, "OUTPUT FD", 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, "OUTPUT FD" ).what() );
            return 1;
        }
    }


    for ( std::vector<std::string>::const_iterator it = outFiles.begin(), end = outFiles.end() ; it != end ; ++it ) {
        char buffer[1024];
        sprintf( buffer, "OUTPUT FILE=%s", hexencode( *it ).c_str() );

        if ( const gpg_error_t err = assuan_transact( ctx, buffer, 0, 0, 0, 0, 0, 0 ) ) {
            qDebug( "%s", assuan_exception( err, buffer ).what() );
            return 1;
        }
    }

    
    
    Q_FOREACH( const char * opt, options ) {
        std::string line = "OPTION ";
        line += opt;
        if ( const gpg_error_t err = assuan_transact( ctx, line.c_str(), 0, 0, 0, 0, 0, 0 ) ) {
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
