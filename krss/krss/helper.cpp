/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#include "helper_p.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

static QByteArray encodeString( const QString& str ) {
    QByteArray ba = str.toUtf8();
    ba.replace( '\\', "\\\\" );
    ba.replace( ';', "\\;" );
    ba.replace( '=', "\\=" );
    return ba;
}

QByteArray KRss::encodeProperties( const QHash<QString, QString>& properties ) {
    QByteArray ba;
    Q_FOREACH( const QString& i, properties.keys() ) {
        if ( !ba.isEmpty() )
            ba += ";";
        ba += encodeString( i ) + "=" + encodeString( properties.value( i ) );
    }
    return ba;
}

QHash<QString, QString> KRss::decodeProperties( const QByteArray& data ) {
    QHash<QString,QString> properties;
    QByteArray key;
    QByteArray value;
    bool isEscaped = false;
    bool isKey = true;
    for ( int i=0; i < data.size(); ++i ) {
        const char ch = data[i];
        if ( isEscaped ) {
            ( isKey ? key : value ) += ch;
            isEscaped = false;
        } else {
            if ( ch == '\\' )
                isEscaped = true;
            else if ( ch == ';' ) {
                properties.insert( QString::fromUtf8( key ), QString::fromUtf8( value ) );
                key.clear();
                value.clear();
                isKey = true;
            }
            else if ( ch == '=' )
                isKey = false;
            else
                ( isKey ? key : value ) += ch;
        }
    }
    if ( !key.isEmpty() )
        properties.insert( QString::fromUtf8( key ), QString::fromUtf8( value ) );
    return properties;
}

QByteArray KRss::encodeStringList( const QStringList& list ) {
    QByteArray ba;
    Q_FOREACH( const QString& i, list ) {
        if ( !ba.isEmpty() )
            ba += ";";
        ba += encodeString( i );
    }
    return ba;

}

QStringList KRss::decodeStringList( const QByteArray& data ) {
    QStringList list;
    QByteArray str;
    bool isEscaped = false;
    for ( int i=0; i < data.size(); ++i ) {
        const char ch = data[i];
        if ( isEscaped ) {
            str += ch;
            isEscaped = false;
        } else {
            if ( ch == '\\' )
                isEscaped = true;
            else if ( ch == ';' ) {
                list << QString::fromUtf8( str );
                str.clear();
            }
            else
                str += ch;
        }
    }
    if ( !str.isEmpty() )
        list << QString::fromUtf8( str );
    return list;
}
