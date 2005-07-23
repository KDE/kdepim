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

#ifndef SERVERTYPE
#define SERVERTYPE

#include <kconfigpropagator.h>
#include <klibloader.h>

#include <qobject.h>

class ServerType : public QObject
{
  public:
    class ConnectionInfo
    {
      public:
        /**
          The unique identifier
         */
        QString uid;

        /**
          The user visible name
         */
        QString name;

        /**
          Whether the connection is active or passive
         */
        bool active;
    };

    /**
      This map contains the uids and the user visible names
      of a server type.
     */
    typedef QValueList<ConnectionInfo> ConnectionInfoList;

    ServerType( QObject *parent, const char *name ):QObject(parent, name) {}
    virtual ~ServerType() {}

    /**
      Returns the connection information of the server type.
     */
    virtual ConnectionInfoList connectionInfo() const = 0;

    /**
      This method is called whenever the user wants to add a new
      connection.
     */
    virtual void addConnection() = 0;

    /**
      This method is called whenever the user wants to edit an existing
      connection.

      @param uid The uid of the resource.
     */
    virtual void editConnection( const QString& uid ) = 0;

    /**
      This method is called whenever the user wants to remove an existing
      connection.

      @param uid The uid of the resource.
     */
    virtual void deleteConnection( const QString& uid ) = 0;

    /**
      This method is called whenever the user marks an existing
      connection as active or inactive.

      @param uid The uid of the resource.
      @param active Whether the connection shall be set active or not.
     */
    virtual void activateConnection( const QString& uid, bool active ) = 0;

    virtual KConfigPropagator::Change::List changes() = 0;
};


/**
  A factory class which loads/creates ServerType objects for us.
 */
class ServerTypeFactory : public KLibFactory
{
  public:
    virtual ServerType *serverType( QObject *parent, const char *name = 0 ) = 0;

    /**
      Returns the identifier.
     */
    virtual QString identifier() const = 0;

    /**
      Returns the i18n'ed name.
     */
    virtual QString title() const = 0;

  protected:
    virtual QObject* createObject( QObject*, const char*,
                                   const char*, const QStringList & )
    { return 0; }
};

#endif
