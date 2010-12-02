/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KLDAP_LDAPSESSION_H
#define KLDAP_LDAPSESSION_H

#include <QObject>
#include <kldap/ldapconnection.h>

class KJob;

namespace KLDAP {

class LdapServer;


class LdapSession : public QObject
{
  Q_OBJECT
  public:
    explicit LdapSession( QObject * parent = 0 );

    void connectToServer( const LdapServer &server );
    void disconnectFromServer();

    void addJob( KJob *job );

  private:
    void authenticate();

  private:
    enum State {
      Disconnected,
      Connected,
      Authenticated
    };
    State m_state;
    LdapConnection m_conn;
    LdapServer m_server;
};

}

#endif
