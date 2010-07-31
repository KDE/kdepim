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

#include <tqdatastream.h>
#include <tqmap.h>

class TodoStateMapper
{
  public:
    /**
      Create Id mapper. You have to set path and identifier before you can call
      load() or save().
    */
    TodoStateMapper();
    ~TodoStateMapper();

    void setPath( const TQString &path );
    void setIdentifier( const TQString &identifier );

    bool load();
    bool save();

    void clear();

    void addTodoState( const TQString &uid, int localState, const TQString &remoteState );

    TQString remoteState( const TQString &uid, int localState );

    void remove( const TQString &uid );

    static int toLocal( const TQString &remoteState );
    static TQString toRemote( int localState );

  protected:
    TQString filename();

  private:
    TQString mPath;
    TQString mIdentifier;

    typedef struct {
      TQString uid;
      int localState;
      TQString remoteState;
    } TodoStateMapEntry;

    typedef TQMap<TQString, TodoStateMapEntry> TodoStateMap;
    TodoStateMap mTodoStateMap;

    friend TQDataStream &operator<<( TQDataStream&, const TodoStateMapEntry& );
    friend TQDataStream &operator>>( TQDataStream&, TodoStateMapEntry& );
};

#endif
