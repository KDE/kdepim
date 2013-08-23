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

#include "response.h"
#include <KDebug>

void KManageSieve::Response::clear()
{
    m_type = None;
    m_key.clear();
    m_value.clear();
    m_extra.clear();
    m_quantity = 0;
}

QByteArray KManageSieve::Response::action() const
{
    return m_key;
}

QByteArray KManageSieve::Response::extra() const
{
    return m_extra;
}

QByteArray KManageSieve::Response::key() const
{
    return m_key;
}

QByteArray KManageSieve::Response::value() const
{
    return m_value;
}

uint KManageSieve::Response::quantity() const
{
    return m_quantity;
}

KManageSieve::Response::Type KManageSieve::Response::type() const
{
    return m_type;
}

bool KManageSieve::Response::parseResponse(const QByteArray &line)
{
    clear();

    switch( line.at( 0 ) ) {
    case '{':
    {
        // expecting {quantity}
        int start = 0;
        int end = line.indexOf( "+}", start + 1 );
        // some older versions of Cyrus enclose the literal size just in { } instead of { +}
        if ( end == -1 )
            end = line.indexOf( '}', start + 1 );

        bool ok = false;
        m_type = Quantity;
        m_quantity = line.mid( start + 1, end - start - 1 ).toUInt( &ok );
        if (!ok) {
            //         disconnect();
            //         error(ERR_INTERNAL_SERVER, i18n("A protocol error occurred."));
            return false;
        }

        return true;
    }
    case '"':
        // expecting "key" "value" pairs
        m_type = KeyValuePair;
        break;
    default:
        // expecting single string
        m_type = Action;
        m_key = line;
        return true;
    }

    int start = 0;
    int end = line.indexOf( '"', start + 1 );
    if ( end == -1 ) {
        kDebug() << "Invalid protocol in:" << line;
        m_key = line.right( line.length() - start );
        return true;
    }
    m_key = line.mid( start + 1, end - start - 1 );

    start = line.indexOf( '"', end + 1 );
    if (start == -1) {
        if ( line.length() > end )
            // skip " and space
            m_extra = line.right( line.length() - end - 2 );
        return true;
    }

    end = line.indexOf( '"', start + 1 );
    if ( end == -1 ) {
        kDebug() << "Invalid protocol in:" << line;
        m_value = line.right( line.length() - start );
        return true;
    }

    m_value = line.mid( start + 1, end - start - 1 );
    return true;
}

KManageSieve::Response::Result KManageSieve::Response::operationResult() const
{
    if ( m_type == Action ) {
        const QByteArray response = m_key.left( 2 );
        if ( response == "OK" ) {
            return Ok;
        } else if ( response == "NO" ) {
            return No;
        } else if ( response == "BY"/*E*/ ) {
            return Bye;
        }
    }
    return Other;
}

bool KManageSieve::Response::operationSuccessful() const
{
    return operationResult() == Ok;
}
