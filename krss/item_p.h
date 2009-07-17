/*
 * This file is part of the krss library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
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

#ifndef KRSS_ITEM_P_H
#define KRSS_ITEM_P_H

#include "item.h"
#include "rssitem.h"

#include <Akonadi/Item>

namespace KRss {

class Item::Private : public QSharedData
{
public:
    Private()
    {
        akonadiItem.setPayload<KRss::RssItem>( RssItem() );
        akonadiItem.setMimeType( Item::mimeType() );
    }

    Private( const Private& other )
        : QSharedData( other ), akonadiItem( other.akonadiItem )
    {
    }

    bool operator!=( const Private& other ) const
    {
        return !( *this == other );
    }

    bool operator==( const Private& other ) const
    {
        return akonadiItem.id() == other.akonadiItem.id();
    }

    bool operator<( const Private& other ) const
    {
        return akonadiItem.id() < other.akonadiItem.id();
    }

    Akonadi::Item akonadiItem;
    mutable QString titleAsPlainText;
};

} // namespace KRss

#endif // KRSS_ITEM_P_H
