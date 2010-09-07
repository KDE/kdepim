/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2007 Daniel Gollub <dgollub@suse.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef OSYNC_PLUGINENV_H
#define OSYNC_PLUGINENV_H

#include <tqstring.h>

struct OSyncPluginEnv;

namespace QSync {

class Plugin;
class Result;

class PluginEnv
{
  public:
    PluginEnv();
    ~PluginEnv();

    /**
      Initializes the environment ( e.g. loads the groups and plugins ).
      Has to be called before the groups or plugins can be accessed.
     */
    Result initialize();

    /**
      Finalizes the environment ( e.g. unloads the groups and plugins ).
      Should be the last call before the object is deleted.
     */
    Result finalize();

    /**
      Returns the number of plugins.
     */
    int pluginCount() const;

    /**
      Returns the plugin at position @param pos.
     */
    Plugin pluginAt( int pos ) const;

    /**
      Returns a plugin by name or an invalid plugin when the plugin with this
      name doesn't exists.
     */
    Plugin pluginByName( const TQString &name ) const;

    /**
      Returns the conversion object of this environment.
     */
//    Conversion conversion() const;

  private:
    OSyncPluginEnv *mPluginEnv;
};

}

#endif
