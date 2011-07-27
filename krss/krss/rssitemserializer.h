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

#ifndef KRSS_RSSITEMSERIALIZER_H
#define KRSS_RSSITEMSERIALIZER_H

#include "krss_export.h"

class QByteArray;

namespace KRss
{

class RssItem;

class KRSS_EXPORT RssItemSerializer
{
public:

    enum ItemPart {
        Headers=0x01,
        Content=0x02,
        Full=Headers|Content
    };

    static void serialize( const KRss::RssItem& item, QByteArray& array, ItemPart part = Full );
    static bool deserialize( KRss::RssItem& item, const QByteArray& array, ItemPart part = Full );
};

}

#endif // KRSS_RSSITEMSERIALIZER_H
