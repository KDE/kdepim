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
    attr->setLabel( m_label );
    attr->setDescription( m_description );
    attr->setIcon( m_icon );
    return attr;
}

QByteArray TagPropertiesAttribute::serialized() const
{
    QStringList props;
    props << QLatin1String("Label=") + m_label;
    props << QLatin1String("Description=") + m_description;
    props << QLatin1String("Icon=") + m_icon;
    return props.join( QLatin1String(";") ).toUtf8();
}

void TagPropertiesAttribute::deserialize( const QByteArray& data )
{
    if ( data.isEmpty() )
        return;

    // so ugly, am i missing something?
    const QStringList props = QString::fromUtf8( data.constData(), data.size() ).split( QLatin1Char(';') );
    m_label = props[0].split( QLatin1Char('=') )[1];
    m_description = props[1].split( QLatin1Char('=') )[1];
    m_icon = props[2].split( QLatin1Char('=') )[1];
}

QString TagPropertiesAttribute::label() const
{
    return m_label;
}

void TagPropertiesAttribute::setLabel( const QString& label )
{
    m_label = label;
}

QString TagPropertiesAttribute::description() const
{
    return m_description;
}

void TagPropertiesAttribute::setDescription( const QString& description )
{
    m_description = description;
}

QString TagPropertiesAttribute::icon() const
{
    return m_icon;
}

void TagPropertiesAttribute::setIcon( const QString& icon )
{
    m_icon = icon;
}
