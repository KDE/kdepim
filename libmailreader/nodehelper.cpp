/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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
#include "nodehelper.h"

#include <kmime/kmime_content.h>

KMail::NodeHelper * KMail::NodeHelper::mSelf = 0;

namespace KMail {

NodeHelper * KMail::NodeHelper::instance()
{
  if ( !mSelf )
    mSelf = new NodeHelper();
  return mSelf;
}

NodeHelper::NodeHelper()
{
  mSelf = this;
}

NodeHelper::~NodeHelper()
{
  mSelf = 0;
}

void NodeHelper::setNodeProcessed(KMime::Content* node, bool recurse )
{
  mProcessedNodes.append( node );
  if ( recurse ) {
    KMime::Content::List contents = node->contents();
    Q_FOREACH( KMime::Content *c, contents )
    {
      setNodeProcessed( c, true );
    }
  }
}

bool NodeHelper::nodeProcessed( KMime::Content* node )
{
  return mProcessedNodes.contains( node );
}

void NodeHelper::clear()
{
  mProcessedNodes.clear();
}

}
