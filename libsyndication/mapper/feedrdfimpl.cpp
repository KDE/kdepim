/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "feedrdfimpl.h"
#include "imagerdfimpl.h"
#include "itemrdfimpl.h"

#include <rdf/dublincore.h>
#include <rdf/item.h>
#include <category.h>
#include <personimpl.h>
#include <tools.h>

#include <QString>
#include <QStringList>
#include <QList>

namespace Syndication {

FeedRDFImpl::FeedRDFImpl(Syndication::RDF::DocumentPtr doc) : m_doc(doc)
{
}

Syndication::SpecificDocumentPtr FeedRDFImpl::specificDocument() const
{
    return m_doc;
}

QList<Syndication::ItemPtr> FeedRDFImpl::items() const
{
    QList<ItemPtr> items;
    QList<Syndication::RDF::Item> entries = m_doc->items();
    QList<Syndication::RDF::Item>::ConstIterator it = entries.begin();
    QList<Syndication::RDF::Item>::ConstIterator end = entries.end();
    
    for ( ; it != end; ++it)
    {
        ItemRDFImplPtr item(new ItemRDFImpl(*it));
        items.append(item);
    }
    
    return items;
}

QList<Syndication::CategoryPtr> FeedRDFImpl::categories() const
{
    // TODO: check if it makes sense to map dc:subject to categories
    return QList<Syndication::CategoryPtr>();
}

QString FeedRDFImpl::title() const
{
    return m_doc->title();
}

QString FeedRDFImpl::link() const
{
    return m_doc->link();
}

QString FeedRDFImpl::description() const
{
    return m_doc->description();
}

QList<PersonPtr> FeedRDFImpl::authors() const
{
    QList<PersonPtr> list;
    
    QStringList people = m_doc->dc().creators();
    people += m_doc->dc().contributors();
    QStringList::ConstIterator it = people.begin();
    QStringList::ConstIterator end = people.end();
    
    for ( ; it != end; ++it)
    {
        PersonPtr ptr = personFromString(*it);
        if (!ptr->isNull())
        {
            list.append(ptr);
        }
    }

    return list;
}

QString FeedRDFImpl::language() const
{
    return m_doc->dc().language();
}

QString FeedRDFImpl::copyright() const
{
    return m_doc->dc().rights();
}

ImagePtr FeedRDFImpl::image() const
{
    ImageRDFImplPtr ptr(new ImageRDFImpl(m_doc->image()));
    return ptr;
}

} // namespace Syndication
