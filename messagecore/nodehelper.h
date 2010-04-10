/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MESSAGECORE_NODE_HELPER_H
#define MESSAGECORE_NODE_HELPER_H

#include "messagecore_export.h"

namespace KMime {
  class Content;
}

/**
 * A few static functions for nagivating in KMime::Node trees
 */

namespace MessageCore {

namespace NodeHelper {


  MESSAGECORE_EXPORT KMime::Content *nextSibling( const KMime::Content* node );
  MESSAGECORE_EXPORT KMime::Content *next( KMime::Content *node, bool allowChildren = true );

  MESSAGECORE_EXPORT KMime::Content *firstChild( const KMime::Content* node );

  // The node parameters here should be const, be there is no const version of
  // functions like contentDisposition() yet
  MESSAGECORE_EXPORT bool isAttachment( KMime::Content* node );
  MESSAGECORE_EXPORT bool isHeuristicalAttachment( KMime::Content* node );


}

}

#endif
