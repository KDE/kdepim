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

#ifndef MESSAGEHELPERS_H
#define MESSAGEHELPERS_H

#include "messagecore_export.h"

#include <boost/shared_ptr.hpp>
#include <KMime/Message>

namespace Akonadi {
  class Item;
}

namespace MessageCore {
  namespace Util {
    MESSAGECORE_EXPORT KMime::Message::Ptr message( const Akonadi::Item & item );
  }
}

#endif // MESSAGEHELPERS_H
