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

#ifndef KMAILNODEHELPER_H
#define KMAILNODEHELPER_H

#include <QList>

namespace KMime {
  class Content;
}

namespace KMail {

/**
	@author
*/
class NodeHelper{
public:
    static NodeHelper * instance();

    ~NodeHelper();

    void setNodeProcessed( KMime::Content* node, bool recurse );
    bool nodeProcessed( KMime::Content* node );
    void clear();

private:
    NodeHelper();
    static NodeHelper * mSelf;

    QList<KMime::Content*> mProcessedNodes;
};

}

#endif
