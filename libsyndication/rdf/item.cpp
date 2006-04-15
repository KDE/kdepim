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

#include "contentvocab.h"
#include "dublincore.h"
#include "item.h"
#include "model.h"
#include "rssvocab.h"
#include "statement.h"

#include <specificitemvisitor.h>
#include <tools.h>

#include <QString>

namespace Syndication {
namespace RDF {

Item::Item() : ResourceWrapper()
{
}

Item::Item(ResourcePtr resource) : ResourceWrapper(resource)
{
}

Item::~Item()
{
}

QString Item::title() const
{
    QString str = resource()->property(RSSVocab::self()->title())->asString();
    return normalize(str);
}

QString Item::description() const
{
    QString str = resource()->property(RSSVocab::self()->description())->asString();
    return normalize(str);
}

QString Item::link() const
{
    return resource()->property(RSSVocab::self()->link())->asString();
}

DublinCore Item::dc() const
{
    return DublinCore(resource());
}

QString Item::encodedContent() const
{
    return resource()->property(ContentVocab::self()->encoded())->asString();
}

QString Item::debugInfo() const
{
    QString info;
    info += "### Item: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "content:encoded: #" + encodedContent() + "#\n";
    info += dc().debugInfo();
    info += "### Item end ################\n";
    return info;
}

bool Item::accept(SpecificItemVisitor* visitor)
{
    return visitor->visitRDFItem(this);
}

} // namespace RDF
} // namespace Syndication
