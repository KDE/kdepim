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

#include "tagidsattribute.h"
#include <QStringList>

using namespace KRss;

TagIdsAttribute::TagIdsAttribute( const QList<TagId>& tagIds )
    : Attribute(), m_tagIds( tagIds )
{
}

QByteArray TagIdsAttribute::type() const
{
    return "TagIds";
}

TagIdsAttribute* TagIdsAttribute::clone() const
{
    TagIdsAttribute *attr = new TagIdsAttribute( m_tagIds );
    return attr;
}

QByteArray TagIdsAttribute::serialized() const
{
    QStringList rawIds;
    Q_FOREACH( const TagId& tagId, m_tagIds ) {
        rawIds.append( Tag::idToString( tagId ) );
    }

    return rawIds.join( ";" ).toUtf8();
}

void TagIdsAttribute::deserialize( const QByteArray& data )
{
    if ( data.isEmpty() )
        return;

    // so ugly, am i missing something?
    const QStringList rawIds = QString::fromUtf8( data.constData(), data.size() ).split( ';' );
    Q_FOREACH( const QString& rawId, rawIds ) {
        m_tagIds.append( Tag::idFromString( rawId ) );
    }
}

QList<TagId> TagIdsAttribute::tagIds() const
{
    return m_tagIds;
}

void TagIdsAttribute::setTagIds( const QList<TagId>& tagIds )
{
    m_tagIds = tagIds;
}

void TagIdsAttribute::addTagId( const TagId& tagId )
{
    if ( !m_tagIds.contains( tagId ) )
        m_tagIds.append( tagId );
}

void TagIdsAttribute::removeTagId( const TagId& tagId )
{
    // addTagId ensures the list contains no duplicates
    m_tagIds.removeOne( tagId );
}
