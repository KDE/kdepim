 /*
    This file is part of kdepim.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_RESOURCEBLOGGING_H
#define KCAL_RESOURCEBLOGGING_H

#include <kcal_resourcegroupwarebase.h>
#include <kdepimmacros.h>

namespace KBlog {
class APIBlog;
}

namespace KCal {

/**
  This class provides a resource for accessing blogs on a blogging server as journals
*/
class KDE_EXPORT ResourceBlogging : public ResourceGroupwareBase
{
    Q_OBJECT
  public:
    ResourceBlogging();
    ResourceBlogging( const KConfig * );

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    bool addEvent( Event* ) { return false; }
    bool addTodo( Todo * ) { return false; }
    void deleteEvent( Event* ) {}
    void deleteTodo( Todo * ) {}
    static KBlog::APIBlog *api() { return mAPI; }

  protected:
    void init();
    static KBlog::APIBlog *mAPI;
};

}

#endif
