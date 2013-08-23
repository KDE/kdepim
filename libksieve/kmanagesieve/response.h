/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KSIEVE_KMANAGESIEVE_RESPONSE_H
#define KSIEVE_KMANAGESIEVE_RESPONSE_H

#include <QByteArray>

namespace KManageSieve {

/** A response from a managesieve server.
 * @internal
 */
class Response
{
public:
    enum Type {
        None,
        KeyValuePair,
        Action,
        Quantity
    };

    Response()
        : m_type( None ),
          m_quantity( 0 )
    {
    }
    Type type() const;
    QByteArray action() const;
    uint quantity() const;
    QByteArray key() const;
    QByteArray value() const;
    QByteArray extra() const;

    enum Result {
        Ok,
        No,
        Bye,
        Other
    };

    Result operationResult() const;
    bool operationSuccessful() const;

    void clear();
    bool parseResponse( const QByteArray &line );

private:
    Type m_type;
    uint m_quantity;
    QByteArray m_key;
    QByteArray m_value;
    QByteArray m_extra;
};

}

#endif
