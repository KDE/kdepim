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

#ifndef LIBSYNDICATION_CONSTANTS_H
#define LIBSYNDICATION_CONSTANTS_H

#include <kdepim_export.h>

class QString;

namespace Syndication {

SYNDICATION_EXPORT
QString xmlNamespace();

SYNDICATION_EXPORT
QString xhtmlNamespace();

SYNDICATION_EXPORT
QString dublinCoreNamespace();

SYNDICATION_EXPORT 
QString contentNameSpace();

SYNDICATION_EXPORT
QString itunesNamespace();

/**
 * wellformedweb.org's RSS namespace for comment functionality
 * "http://wellformedweb.org/CommentAPI/"
 */
SYNDICATION_EXPORT
QString commentApiNamespace();

/**
 * "slash" namespace
 * http://purl.org/rss/1.0/modules/slash/
 */
SYNDICATION_EXPORT 
QString slashNamespace();

} // namespace Syndication

#endif // LIBSYNDICATION_CONSTANTS_H
