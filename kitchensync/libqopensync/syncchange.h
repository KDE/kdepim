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

#ifndef QSYNC_SYNCCHANGE_H
#define QSYNC_SYNCCHANGE_H

#include <libqopensync/member.h>

class OSyncChange;

namespace QSync {

class SyncChange
{
  friend class SyncMapping;

  public:
    enum Type
    {
      UnknownChange,
      AddedChange,
      UnmodifiedChange,
      DeletedChange,
      ModifiedChange
    };

    SyncChange();
    SyncChange( OSyncChange* );
    ~SyncChange();

    /**
      Returns whether it's a valid SyncChange.
     */
    bool isValid() const;

    /**
      Sets the uid of this change.
     */
    void setUid( const QString &uid );

    /**
      Returns the uid of this change.
     */
    QString uid() const;

    /**
      Sets the hash of this change.
     */
    void setHash( const QString &hash );

    /**
      Returns the hash of this change.
     */
    QString hash() const;

    /**
      Sets the data provided by the plugin.
     */
    void setData( const QString &data );

    /**
      Returns the data provided by the plugin.
     */
    QString data() const;

    /**
      Returns whether the change contains data.
     */
    bool hasData() const;

    /**
      Returns the object format name.
     */
    QString objectFormatName() const;

    /**
      Returns the parent member of this change.
     */
    Member member() const;

    /**
      Sets the change type.
     */
    void setChangeType( Type changeType );

    /**
      Returns the change type.
     */
    Type changeType() const;

  private:
    OSyncChange *mSyncChange;
};

}

#endif

