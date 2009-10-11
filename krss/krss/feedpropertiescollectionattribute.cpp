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
#include "helper_p.h"

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

QByteArray FeedPropertiesCollectionAttribute::serialized() const
{
    return encodeProperties( m_properties );
}

void FeedPropertiesCollectionAttribute::deserialize( const QByteArray &data )
{
    m_properties = decodeProperties( data );
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
