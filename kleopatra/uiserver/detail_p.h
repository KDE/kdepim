/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/detail_p.h

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

#ifndef __KLEOPATRA_UISERVER_DETAIL_P_H__
#define __KLEOPATRA_UISERVER_DETAIL_P_H__

#include "kleo-assuan.h"

#include <boost/shared_ptr.hpp>

#include <typeinfo>

namespace Kleo {
namespace _detail {

    // Predicate for sorting by typeid (works for containers of
    // pointers and shared_ptr's)
    struct ByTypeId {
        typedef bool result_type;
        bool operator()( const std::type_info & lhs, const std::type_info & rhs ) const {
            return lhs.before( rhs );
        }
        template <typename T>
        bool operator()( const T * lhs, const T * rhs ) const {
            return operator()( typeid(*lhs), typeid(*rhs) );
        }
        template <typename T>
        bool operator()( const std::type_info & lhs, const T * rhs ) const {
            return operator()( lhs, typeid(*rhs) );
        }
        template <typename T>
        bool operator()( const T * lhs, const std::type_info & rhs ) const {
            return operator()( typeid(*lhs), rhs );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<T> & lhs, const boost::shared_ptr<T> & rhs ) const {
            return operator()( lhs.get(), rhs.get() );
        }
        template <typename T>
        bool operator()( const std::type_info & lhs, const boost::shared_ptr<T> & rhs ) const {
            return operator()( lhs, rhs.get() );
        }
        template <typename T>
        bool operator()( const T * lhs, const boost::shared_ptr<T> & rhs ) const {
            return operator()( lhs, rhs.get() );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<T> & lhs, const std::type_info & rhs ) const {
            return operator()( lhs.get(), rhs );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<T> & lhs, const T * rhs ) const {
            return operator()( lhs.get(), rhs );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<const T> & lhs, const boost::shared_ptr<const T> & rhs ) const {
            return operator()( lhs.get(), rhs.get() );
        }
        template <typename T>
        bool operator()( const std::type_info & lhs, const boost::shared_ptr<const T> & rhs ) const {
            return operator()( lhs, rhs.get() );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<const T> & lhs, const std::type_info & rhs ) const {
            return operator()( lhs.get(), rhs );
        }
        template <typename T>
        bool operator()( const T * lhs, const boost::shared_ptr<const T> & rhs ) const {
            return operator()( lhs, rhs.get() );
        }
        template <typename T>
        bool operator()( const boost::shared_ptr<const T> & lhs, const T * rhs ) const {
            return operator()( lhs.get(), rhs );
        }
    };

    // inspired by GnuPG's translate_sys2libc_fd, this converts a HANDLE
    // to int fd on Windows, and is a NO-OP on POSIX:
    static inline int translate_sys2libc_fd( assuan_fd_t fd, bool for_write ) {
        if ( fd == ASSUAN_INVALID_FD )
            return -1;
#ifdef Q_OS_WIN32
        return _open_osfhandle( fd, for_write );
#else
        (void)for_write;
        return fd;
#endif
    }

}
}

#endif /* __KLEOPATRA_UISERVER_DETAIL_P_H__ */
