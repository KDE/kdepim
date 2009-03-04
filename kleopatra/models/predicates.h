/* -*- mode: c++; c-basic-offset:4 -*-
    models/predicates.h

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

#ifndef __KLEOPATRA_MODELS_PREDICATES_H__
#define __KLEOPATRA_MODELS_PREDICATES_H__

#include <QByteArray>
#include <string>

#include <cstring>

namespace Kleo {
namespace _detail {

    inline int mystrcmp( const char * s1, const char * s2 ) {
        using namespace std;
        return s1 ? s2 ? strcmp( s1, s2 ) : 1 : s2 ? -1 : 0 ;
    }

#define make_comparator_str_impl( Name, expr, cmp )                     \
    template <template <typename U> class Op>                           \
    struct Name {                                                       \
        typedef bool result_type;                                       \
                                                                        \
        bool operator()( const char * lhs, const char * rhs ) const {   \
            return Op<int>()( cmp, 0 );                                 \
        }                                                               \
                                                                        \
        bool operator()( const std::string & lhs, const std::string & rhs ) const { \
            return operator()( lhs.c_str(), rhs.c_str() );              \
        }                                                               \
        bool operator()( const char * lhs, const std::string & rhs ) const { \
            return operator()( lhs, rhs.c_str() );                      \
        }                                                               \
        bool operator()( const std::string & lhs, const char * rhs ) const { \
            return operator()( lhs.c_str(), rhs );                      \
        }                                                               \
                                                                        \
        template <typename T>                                           \
        bool operator()( const T & lhs, const T & rhs ) const {         \
            return operator()( (lhs expr), (rhs expr) );                \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const char * rhs ) const {      \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const char * lhs, const T & rhs ) const {      \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const T & lhs, const std::string & rhs ) const { \
            return operator()( (lhs expr), rhs );                       \
        }                                                               \
        template <typename T>                                           \
        bool operator()( const std::string & lhs, const T & rhs ) const {    \
            return operator()( lhs, (rhs expr) );                       \
        }                                                               \
    }

#define make_comparator_str_fast( Name, expr )                          \
    make_comparator_str_impl( Name, expr, _detail::mystrcmp( lhs, rhs ) )
#define make_comparator_str( Name, expr )                               \
    make_comparator_str_impl( Name, expr, qstricmp( lhs, rhs ) )

    make_comparator_str_fast( ByFingerprint, .primaryFingerprint() );
    make_comparator_str_fast( ByKeyID, .keyID() );
    make_comparator_str_fast( ByShortKeyID, .shortKeyID() );
    make_comparator_str_fast( ByChainID, .chainID() );

}
}

#endif /* __KLEOPATRA_MODELS_PREDICATES_H__ */
