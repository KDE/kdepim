/* -*- mode: c++; c-basic-offset:4 -*-
    utils/detail_p.h

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

#ifndef __KLEOPATRA_UTILS_DETAIL_P_H__
#define __KLEOPATRA_UTILS_DETAIL_P_H__

#include <kleo-assuan.h>

#include <boost/shared_ptr.hpp>

#include <QByteArray>

#ifdef _WIN32
#include <io.h>
#endif

class QString;

namespace Kleo {
namespace _detail {

    template <template <typename U> class Op>
    struct ByName {
        typedef bool result_type;

        template <typename T>
        bool operator()( const T & lhs, const T & rhs ) const {
            return Op<int>()( qstricmp( lhs->name(), rhs->name() ), 0 );
        }
        template <typename T>
        bool operator()( const T & lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs->name(), rhs ), 0 );
        }
        template <typename T>
        bool operator()( const char * lhs, const T & rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs->name() ), 0 );
        }
        bool operator()( const char * lhs, const char * rhs ) const {
            return Op<int>()( qstricmp( lhs, rhs ), 0 );
        }
    };

    // inspired by GnuPG's translate_sys2libc_fd, this converts a HANDLE
    // to int fd on Windows, and is a NO-OP on POSIX:
    static inline int translate_sys2libc_fd( assuan_fd_t fd, bool for_write ) {
        if ( fd == ASSUAN_INVALID_FD )
            return -1;
#if defined(_WIN32)
        return _open_osfhandle( (intptr_t)fd, for_write );
#else
        (void)for_write;
        return fd;
#endif
    }

    static inline assuan_fd_t translate_libc2sys_fd( int fd ) {
        if ( fd == -1 )
            return ASSUAN_INVALID_FD;
#if defined(_WIN32)
        return (assuan_fd_t)_get_osfhandle( fd );
#else
        return fd;
#endif
    }

    //returns an integer representation of the assuan_fd_t,
    //suitable for debug output
    static inline qulonglong assuanFD2int( assuan_fd_t fd )
    {
#ifdef _WIN32
        return reinterpret_cast<qulonglong>( fd );
#else
        return static_cast<qulonglong>( fd );
#endif
    }
}
}

#endif /* __KLEOPATRA_UTILS_DETAIL_P_H__ */
