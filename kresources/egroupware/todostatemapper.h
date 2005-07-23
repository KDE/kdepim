/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef TODOSTATEMAPPER_H
#define TODOSTATEMAPPER_H

#include <qdatastream.h>
#include <qmap.h>

class TodoStateMapper
{
  public:
    /**
      Create Id mapper. You have to set path and identifier before you can call
      load() or save().
    */
    TodoStateMapper();
    ~TodoStateMapper();

    void setPath( const QString &path );
    void setIdentifier( const QString &identifier );

    bool load();
    bool save();

    void clear();

    void addTodoState( const QString &uid, int localState, const QString &remoteState );

    QString remoteState( const QString &uid, int localState );

    void remove( const QString &uid );

    static int toLocal( const QString &remoteState );
    static QString toRemote( int localState );

  protected:
    QString filename();

  private:
    QString mPath;
    QString mIdentifier;

    typedef struct {
      QString uid;
      int localState;
      QString remoteState;
    } TodoStateMapEntry;

    typedef QMap<QString, TodoStateMapEntry> TodoStateMap;
    TodoStateMap mTodoStateMap;

    friend QDataStream &operator<<( QDataStream&, const TodoStateMapEntry& );
    friend QDataStream &operator>>( QDataStream&, TodoStateMapEntry& );
};

#endif
