/*
    aboutdata.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <kaboutdata.h>
#include <kdepimmacros.h>

#ifndef KNODE_ABOUTDATA_H
#define KNODE_ABOUTDATA_H

namespace KNode
{
  class KDE_EXPORT AboutData : public KAboutData
  {
  public:
    AboutData();
    ~AboutData();
  };

} // namespace KNode

#endif
