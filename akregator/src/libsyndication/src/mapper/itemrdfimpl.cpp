/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "itemrdfimpl.h"
#include "personimpl.h"

#include "../rdf/dublincore.h"
#include "../rdf/resource.h"
#include "../rdf/resourcewrapper.h"
#include "../category.h"
#include "../enclosure.h"
#include "../tools.h"

#include <QList>
#include <QString>

namespace LibSyndication {

ItemRDFImpl::ItemRDFImpl(const LibSyndication::RDF::Item& item) : m_item(item)
{
}

QString ItemRDFImpl::title() const
{
    return m_item.title();
}

QString ItemRDFImpl::link() const
{
    return m_item.link();
}

QString ItemRDFImpl::description() const
{
    return m_item.description();
}

QString ItemRDFImpl::content() const
{
    return m_item.encodedContent();
}

QList<PersonPtr> ItemRDFImpl::authors() const
{
    QList<PersonPtr> list;
    
    PersonPtr ptr = PersonImpl::fromString(m_item.dc().creator());
    
    if (!ptr->isNull())
    {
        list.append(ptr);
    }

    return list;
}

QString ItemRDFImpl::language() const
{
    return m_item.dc().language();
}

QString ItemRDFImpl::id() const
{
    if (!m_item.resource()->isAnon())
        return m_item.resource()->uri();
    else
        return "hash:" + calcMD5Sum(title() + description() + content());
}

time_t ItemRDFImpl::datePublished() const 
{
    return m_item.dc().date();
}

time_t ItemRDFImpl::dateUpdated() const 
{
    return m_item.dc().date();
}

QList<LibSyndication::EnclosurePtr> ItemRDFImpl::enclosures() const
{
    // return empty list
    return QList<LibSyndication::EnclosurePtr>();
}

QList<LibSyndication::CategoryPtr> ItemRDFImpl::categories() const
{
    // return empty list
    return QList<LibSyndication::CategoryPtr>();
}

LibSyndication::SpecificItemPtr ItemRDFImpl::specificItem() const
{
    return LibSyndication::SpecificItemPtr(new LibSyndication::RDF::SpecificItem(m_item));
}

} // namespace LibSyndication
