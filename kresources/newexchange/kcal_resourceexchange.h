 /*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_RESOURCEEXCHANGE_H
#define KCAL_RESOURCEEXCHANGE_H

#include <kcal_resourcegroupwarebase.h>

namespace KCal {

/**
  This class provides a resource for accessing an Exchange server
*/
class KDE_EXPORT ResourceExchange : public ResourceGroupwareBase
{
    Q_OBJECT
  public:
    ResourceExchange();
    ResourceExchange( const KConfig * );

  protected:
    void init();
};

}

#endif
