/*
 * This file is part of KDEPIM.
 * Copyright (c) 2010 Till Adam <adam@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "messagehelpers.h"

#include <Akonadi/Item>

using namespace MessageCore;
using namespace MessageCore::Util;

KMime::Message::Ptr MessageCore::Util::message( const Akonadi::Item & item )
{
  if ( !item.hasPayload<KMime::Message::Ptr>() ) {
    kWarning() << "Payload is not a MessagePtr!";
    return KMime::Message::Ptr();
  }
  return item.payload<boost::shared_ptr<KMime::Message> >();
}

bool MessageCore::Util::isStandaloneMessage( const Akonadi::Item& item )
{
  // standalone message have a valid payload, but are not, themselves valid items
  return item.hasPayload<KMime::Message::Ptr>() && !item.isValid();
}
