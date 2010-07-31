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

#ifndef SERVERTYPEMANAGER_H
#define SERVERTYPEMANAGER_H

#include <tqobject.h>

#include "servertype.h"

class ServerTypeManager : public QObject
{
  Q_OBJECT

  public:
    static ServerTypeManager* self();

    ~ServerTypeManager();

    /**
      Returns the list of identifiers of all available
      server types.
     */
    TQStringList identifiers() const;

    /**
      Returns the i18n'ed name of a server type.

      @param The identifier of the server type.
     */
    TQString title( const TQString& ) const;

    /**
      Returns a pointer to the server type with the given
      identifier or 0 when it doesn't exist.
     */
    ServerType* serverType( const TQString& );

  private:
    typedef TQMap<TQString, ServerType*> ServerTypeMap;
    typedef TQMap<TQString, ServerTypeFactory*> ServerTypeFactoryMap;

    void loadPlugins();

    ServerTypeManager( TQObject *parent, const char *name = 0 );

    static ServerTypeManager* mSelf;

    ServerTypeMap mServerTypeMap;
    ServerTypeFactoryMap mServerTypeFactoryMap;
};

#endif
