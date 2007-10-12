/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/kleo-assuan.h

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

#ifndef __KLEOPATRA_UISERVER_KLEO_ASSUAN_H__
#define __KLEOPATRA_UISERVER_KLEO_ASSUAN_H__

#include <gpg-error.h>
#include <assuan.h>

#include <boost/preprocessor/stringize.hpp>

#include <stdexcept>
#include <assert.h>

class QString;

namespace Kleo {

    class assuan_exception : public std::runtime_error {
    public:
        assuan_exception( gpg_error_t e, const std::string & msg )
            : std::runtime_error( make_message( e, msg ) ), m_error( e ), m_message( msg ) {}
        assuan_exception( gpg_error_t e, const char* msg );
        assuan_exception( gpg_error_t e, const QString & msg );

        ~assuan_exception() throw () {}

        const std::string & message() const { return m_message; }
        gpg_error_t error_code() const { return m_error; }
    private:
        static std::string make_message( gpg_error_t, const std::string & );
    private:
        gpg_error_t m_error;
        std::string m_message;
    };

}

#define assuan_assert_impl( cond, file, line )                          \
    if ( cond ) {}                                                      \
    else throw Kleo::assuan_exception( gpg_error( GPG_ERR_INTERNAL ),   \
                                       "assertion \"" #cond "\" failed at " file ":" BOOST_PP_STRINGIZE( line ) )
#define assuan_assert_impl_func( cond, file, line, func )               \
    if ( cond ) {}                                                      \
    else throw Kleo::assuan_exception( gpg_error( GPG_ERR_INTERNAL ),   \
                                       std::string( "assertion \"" #cond "\" failed in " ) + func + " (" file ":" BOOST_PP_STRINGIZE( line ) ")" )
// from glibc's assert.h:
/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
#if defined __GNUC_PREREQ && __GNUC_PREREQ (2, 6)
# define assuan_assert( cond ) assuan_assert_impl_func( cond, __FILE__, __LINE__, __PRETTY_FUNCTION__ )
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define assuan_assert( cond ) assuan_assert_impl_func( cond, __FILE__, __LINE__, __func__ )
#endif

#ifndef assuan_assert
# define assuan_assert( cond ) assuan_assert_impl( cond, __FILE__, __LINE__ )
#endif

#endif /* __KLEOPATRA_UISERVER_KLEO_ASSUAN_H__ */
