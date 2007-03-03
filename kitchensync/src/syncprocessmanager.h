/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/
#ifndef SYNCPROCESSMANAGER_H
#define SYNCPROCESSMANAGER_H

#include <qobject.h>

#include <libqopensync/group.h>

namespace QSync {
class Environment;
}

class SyncProcess;

class SyncProcessManager : public QObject
{
    Q_OBJECT
  public:
    static SyncProcessManager *self();

    /**
      Destroys the SyncProcessList.
     */
    ~SyncProcessManager();

    /**
      Return OpenSync Environment.
    */
    QSync::Environment *environment() const { return mEnvironment; }

    /**
      Returns the number of SyncProcesses.
     */
    int count() const;

    /**
      Returns the SyncProcess at position @param pos.
     */
    SyncProcess* at( int pos ) const;

    /**
      Returns the SyncProcess with the given @param group.
     */
    SyncProcess* byGroup( const QSync::Group &group );

    /**
      Returns the SyncProcess with the given group @param name.
     */
    SyncProcess* byGroupName( const QString &name );

    /**
      Adds a group with given @param name.
     */
    void addGroup( const QString &name );

    /**
      Removes the given @param sync process.
     */
    void remove( SyncProcess *syncProcess );

    /**
      Adds @param plugin instance as member to the group of @param process.
     */
    QSync::Result addMember( SyncProcess *process, const QSync::Plugin &plugin );

  signals:
    void changed();
    void syncProcessChanged( SyncProcess *process );

  private:
    SyncProcessManager();

    void init( QSync::Environment *environment );

    QValueList<SyncProcess*> mProcesses;
    QSync::Environment *mEnvironment;

    static SyncProcessManager *mSelf;
};

#endif
