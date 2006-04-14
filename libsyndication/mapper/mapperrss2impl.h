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


#ifndef LIBSYNDICATION_MAPPERRSS2IMPL_H
#define LIBSYNDICATION_MAPPERRSS2IMPL_H

#include "feedrss2impl.h"

#include <rss2/document.h>
#include <specificdocument.h>
#include <feed.h>
#include <mapper.h>



namespace LibSyndication {

/** @internal */
class RSS2Mapper : public Mapper<Feed>
{
    SharedPtr<Feed> map(SpecificDocumentPtr doc) const
    {
        return SharedPtr<Feed>(new FeedRSS2Impl(doc));
    }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPERRSS2IMPL_H
