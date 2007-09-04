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
#include <assuan.h>
#include <gpg-error.h>

#include <QtCore>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

static int dataFD, sigFD;

static int data( void * void_ctx, const void * buffer, size_t len ) {
    (void)void_ctx; (void)buffer; (void)len;
    return gpg_error( GPG_ERR_NOT_IMPLEMENTED );
}

static int status( void * void_ctx, const char * line ) {
    qDebug( "status[%p]:%s", void_ctx, line );
    return 0;
}

static int inquire( void * void_ctx, const char * keyword ) {
    assuan_context_t ctx = (assuan_context_t)void_ctx;
    assert( ctx );
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
}

int main( int argc, char * argv[] ) {
    
    if ( argc != 5 || qstrcmp( argv[2], "--verify-detached" ) != 0 )
        return 1;

    const QFileInfo fi[3] = {
        QFileInfo( QFile::decodeName( argv[1] ) ),
        QFileInfo( QFile::decodeName( argv[3] ) ),
        QFileInfo( QFile::decodeName( argv[4] ) ),
    };

    if ( !fi[0].exists() )
        return 2;

    if ( !fi[1].isReadable() )
        return 4;
    
    if ( !fi[2].isReadable() )
        return 5;

    assuan_context_t ctx = 0;

    if ( const gpg_error_t err = assuan_socket_connect_ext( &ctx, argv[1], -1, 1 ) ) {
        qDebug( "assuan_socket_connect_ext: %s", gpg_strerror( err ) );
        return 1;
    }

    if ( (dataFD = open( argv[3], O_RDONLY )) == -1 ) {
        perror( "signed data open()" );
        return 1;
    }

    if ( (sigFD = open( argv[4], O_RDONLY )) ==  -1 ) {
        perror( "signature open()" );
        return 1;
    }

    if ( const gpg_error_t err = assuan_sendfd( ctx, dataFD ) ) {
        qDebug( "assuan_sendfd: %s", gpg_strerror( err ) );
        return 1;
    }

    if ( const gpg_error_t err = assuan_write_line( ctx, "INPUT FD" ) ) {
        qDebug( "assuan_write_line(\"INPUT FD\"): %s", gpg_strerror( err ) );
        return 1;
    }

    if ( const gpg_error_t err = assuan_transact( ctx, "VERIFYDETACHED", data, ctx, inquire, ctx, status, ctx ) ) {
        qDebug( "assuan_transact(\"VERIFYDETACHED\"): %s", gpg_strerror( err ) );
        return 1;
    }

    close( sigFD );
    close( dataFD );

    assuan_disconnect( ctx );
    
    return 0;
}
