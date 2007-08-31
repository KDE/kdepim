/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuancommand.h

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

#ifndef __KLEOPATRA_UISERVER_ASSUANCOMMAND_H__
#define __KLEOPATRA_UISERVER_ASSUANCOMMAND_H__

#include <string>
#include <map>
#include <typeinfo>

class QVariant;

struct assuan_context_s;

namespace Kleo {

    class AssuanCommand {
    public:
        virtual ~AssuanCommand() {}

        virtual int start( const std::string & line, const std::map<std::string,QVariant> & options ) = 0;

        virtual const char * name() const = 0;

    protected:
        int input() const;
        int output() const;

        // forget the rest, it's internal!
    public:
        typedef int(*_Handler)( assuan_context_s*, char *);
        virtual _Handler _handler() const = 0;
    protected:
        // defined in assuanserverconnection.cpp!
        static int _handle( assuan_context_s*, char *, const std::type_info & );
    };

    template <typename Derived>
    class AssuanHandlerMixin : public AssuanCommand {
        /* reimp */ _Handler _handler() const { return &AssuanHandlerMixin::_handle; }
        static int _handle( assuan_context_s* _ctx, char * _line ) {
            return AssuanCommand::_handle( _ctx, _line, typeid(Derived) );
        }
    };

    // ### these are only temporary:
    class VerifyEmailCommand : public AssuanHandlerMixin<VerifyEmailCommand> {
        const char * name() const { return "VERIFYEMAIL"; }
        int start( const std::string &, const std::map<std::string,QVariant> & );
    };

    class DecryptEmailCommand : public AssuanHandlerMixin<DecryptEmailCommand> {
        const char * name() const { return "DECRYPTEMAIL"; }
        int start( const std::string &, const std::map<std::string,QVariant> & );
    };
}

#endif /* __KLEOPATRA_UISERVER_ASSUANCOMMAND_H__ */
