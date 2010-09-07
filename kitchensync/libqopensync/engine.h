/*
    This file is part of libqopensync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef QSYNC_ENGINE_H
#define QSYNC_ENGINE_H

class OSyncEngine;

namespace QSync {

class Group;
class Member;
class Result;

class Engine
{
  friend class CallbackHandler;

  public:
    /**
      Constructs an engine .
     */
    Engine( const Group &group );

    /**
      Destroys the engine.
     */
    ~Engine();

    /**
      Initializes the engine.
     */
    Result initialize();

    /**
      Finalizes the engine.
     */
    void finalize();

    /**
      Starts the synchronization process.
     */
    Result synchronize();

    /**
      Starts the discover process for a certain member.
     */
    Result discover( const Member &member );

    /**
      Stops the synchronization process.
     */
    void abort();

  private:
    OSyncEngine *mEngine;
};

}

#endif
