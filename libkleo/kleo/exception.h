/* -*- mode: c++; c-basic-offset:4 -*-
    exception.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#ifndef __KLEO_EXCEPTION_H__
#define __KLEO_EXCEPTION_H__

#include "kleo_export.h"

#include <gpg-error.h>
#include <gpgme++/exception.h>

#include <QString>

namespace Kleo {

    class KLEO_EXPORT Exception : public GpgME::Exception {
    public:
        Exception( gpg_error_t e, const std::string & msg, Options opt=NoOptions )
            : GpgME::Exception( GpgME::Error( e ), msg, opt ) {}
        Exception( gpg_error_t e, const char* msg, Options opt=NoOptions )
            : GpgME::Exception( GpgME::Error( e ), msg, opt ) {}
        Exception( gpg_error_t e, const QString & msg, Options opt=NoOptions )
            : GpgME::Exception( GpgME::Error( e ), msg.toLocal8Bit().constData(), opt ) {}

        Exception( const GpgME::Error & e, const std::string & msg )
            : GpgME::Exception( e, msg ) {}
        Exception( const GpgME::Error & e, const char* msg )
            : GpgME::Exception( e, msg ) {}
        Exception( const GpgME::Error & e, const QString & msg )
            : GpgME::Exception( e, msg.toLocal8Bit().constData() ) {}

        ~Exception() throw ();

        const std::string & messageLocal8Bit() const { return GpgME::Exception::message(); }
        gpg_error_t error_code() const { return error().encodedError(); }

        QString message() const { return QString::fromLocal8Bit( GpgME::Exception::message().c_str() ); }
    };

}

#endif /* __KLEO_EXCEPTION_H__ */
