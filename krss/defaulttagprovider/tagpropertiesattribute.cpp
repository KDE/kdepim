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

#include "tagpropertiesattribute.h"
#include "helper_p.h"

#include <QtCore/QStringList>

using namespace KRss;

TagPropertiesAttribute::TagPropertiesAttribute( )
    : Akonadi::Attribute()
{
}

QByteArray TagPropertiesAttribute::type() const
{
    return "TagProperties";
}

TagPropertiesAttribute* TagPropertiesAttribute::clone() const
{
    TagPropertiesAttribute *attr = new TagPropertiesAttribute();
    attr->m_properties = m_properties;
    return attr;
}

QByteArray TagPropertiesAttribute::serialized() const
{
    return encodeProperties( m_properties );
}

void TagPropertiesAttribute::deserialize( const QByteArray& data )
{
    m_properties = decodeProperties( data );
}

QString TagPropertiesAttribute::label() const
{
    return m_properties.value( QLatin1String("Label") );
}

void TagPropertiesAttribute::setLabel( const QString& label )
{
    m_properties.insert( QLatin1String("Label"), label );
}

QString TagPropertiesAttribute::description() const
{
    return m_properties.value( QLatin1String("Description") );
}

void TagPropertiesAttribute::setDescription( const QString& description )
{
    m_properties.insert( QLatin1String("Description"), description );
}

QString TagPropertiesAttribute::icon() const
{
    return m_properties.value( QLatin1String("Icon") );
}

void TagPropertiesAttribute::setIcon( const QString& icon )
{
    m_properties.insert( QLatin1String("Icon"), icon );
}
