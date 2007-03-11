/*
    This file is part of KDE.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include <qobject.h>

#include <dbus/qdbusobject.h>

class QDBusMessage;
class QDBusConnection;

class OpenSyncService : public QDBusObjectBase
{
  public:
    OpenSyncService();

    void setConnection( QDBusConnection *connection );

  protected:
    virtual bool handleMethodCall( const QDBusMessage &message );

    QDBusMessage hello( const QDBusMessage & );
    QDBusMessage randomNumber( const QDBusMessage & );

    QDBusMessage listGroups( const QDBusMessage &message );
    QDBusMessage listPlugins( const QDBusMessage &message );
    QDBusMessage showGroup( const QDBusMessage &message );
    QDBusMessage showMember( const QDBusMessage &message );

    QDBusMessage error( const QDBusMessage &, const QString &errorCode,
      const QString &errorMessage );

  private:
    QDBusConnection *mConnection;
};

#endif
