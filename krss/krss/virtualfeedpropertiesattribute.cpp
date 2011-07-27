/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "virtualfeedpropertiesattribute.h"

#include <QtCore/QStringList>

using namespace KRss;

QByteArray VirtualFeedPropertiesAttribute::type() const
{
    return "VirtualFeedProperties";
}

VirtualFeedPropertiesAttribute* VirtualFeedPropertiesAttribute::clone() const
{
    return new VirtualFeedPropertiesAttribute( *this );
}

namespace {
    static QByteArray encode( const QString& str ) {
        QByteArray ba = str.toUtf8();
        ba.replace( '\\', "\\\\" );
        ba.replace( ';', "\\;" );
        ba.replace( '=', "\\=" );
        return ba;
    }
}

QByteArray VirtualFeedPropertiesAttribute::serialized() const
{
    QByteArray ba;
    Q_FOREACH( const QString& i, m_properties.keys() ) {
        if ( !ba.isEmpty() )
            ba += ";";
        ba += encode( i ) + "=" + encode( m_properties.value( i ) );
    }
    return ba;
}

void VirtualFeedPropertiesAttribute::deserialize( const QByteArray &data )
{
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
                m_properties.insert( QString::fromUtf8( key ), QString::fromUtf8( value ) );
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
        m_properties.insert( QString::fromUtf8( key ), QString::fromUtf8( value ) );
}

QString VirtualFeedPropertiesAttribute::title() const
{
    return m_properties.value( QString::fromLatin1( "Title" ) );
}

void VirtualFeedPropertiesAttribute::setTitle( const QString& name )
{
    m_properties.insert( QString::fromLatin1( "Title" ), name );
}

QString VirtualFeedPropertiesAttribute::description() const
{
    return m_properties.value( QString::fromLatin1( "Description" ) );
}

void VirtualFeedPropertiesAttribute::setDescription( const QString& name )
{
    m_properties.insert( QString::fromLatin1( "Description" ), name );
}
