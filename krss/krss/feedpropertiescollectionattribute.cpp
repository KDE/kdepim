/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "feedpropertiescollectionattribute.h"

#include <QtCore/QStringList>

using namespace KRss;

FeedPropertiesCollectionAttribute::FeedPropertiesCollectionAttribute( )
        : Akonadi::Attribute()
{
}

QByteArray FeedPropertiesCollectionAttribute::type() const
{
    return "FeedProperties";
}

FeedPropertiesCollectionAttribute* FeedPropertiesCollectionAttribute::clone() const
{
    return new FeedPropertiesCollectionAttribute( *this );
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

QByteArray FeedPropertiesCollectionAttribute::serialized() const
{
    QByteArray ba;
    Q_FOREACH( const QString& i, m_properties.keys() ) {
        if ( !ba.isEmpty() )
            ba += ";";
        ba += encode( i ) + "=" + encode( m_properties.value( i ) );
    }
    return ba;

}

void FeedPropertiesCollectionAttribute::deserialize( const QByteArray &data )
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

QString FeedPropertiesCollectionAttribute::name() const
{
    return m_properties.value( QString::fromLatin1("Name" ) );
}

void FeedPropertiesCollectionAttribute::setName( const QString &name )
{
    m_properties.insert( QString::fromLatin1("Name" ), name );
}

bool FeedPropertiesCollectionAttribute::preferItemLinkForDisplay() const
{
    return m_properties.value( QLatin1String("PreferItemLinkForDisplay") ) == QLatin1String( "true" );
}

void FeedPropertiesCollectionAttribute::setPreferItemLinkForDisplay( bool b )
{
    m_properties.insert( QLatin1String("PreferItemLinkForDisplay"), b ? QString::fromLatin1("true") : QString() );
}

QString FeedPropertiesCollectionAttribute::xmlUrl() const
{
    return m_properties.value( QString::fromLatin1("XmlUrl" ) );
}

void FeedPropertiesCollectionAttribute::setXmlUrl( const QString &xmlUrl )
{
    m_properties.insert( QString::fromLatin1("XmlUrl" ), xmlUrl );
}

QString FeedPropertiesCollectionAttribute::htmlUrl() const
{
    return m_properties.value( QString::fromLatin1("HtmlUrl" ) );
}

void FeedPropertiesCollectionAttribute::setHtmlUrl( const QString &htmlUrl )
{
    m_properties.insert( QString::fromLatin1("HtmlUrl" ), htmlUrl );
}

QString FeedPropertiesCollectionAttribute::feedType() const
{
    return m_properties.value( QString::fromLatin1("FeedType" ) );
}

void FeedPropertiesCollectionAttribute::setFeedType( const QString &feedType )
{
    m_properties.insert( QString::fromLatin1("FeedType" ), feedType );
}

QString FeedPropertiesCollectionAttribute::description() const
{
    return m_properties.value( QString::fromLatin1("Description" ) );
}

void FeedPropertiesCollectionAttribute::setDescription( const QString &description )
{
    m_properties.insert( QString::fromLatin1("Description" ), description );
}
