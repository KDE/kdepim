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

#include "ldapsession.h"

#include <kldap/ldapoperation.h>
#include <kldap/ldif.h>
#include <kldap/ldapcontrol.h>
#include <kldap/ldapdefs.h>

#include <KDebug>

using namespace KLDAP;

LdapSession::LdapSession(QObject* parent) :
  QObject(parent),
  m_state( Disconnected )
{
  kDebug();
}

void LdapSession::connectToServer(const KLDAP::LdapServer& server)
{
  kDebug();
  if ( m_state != Disconnected )
    return;
  m_server = server;
  m_conn.setServer( server );
  if ( m_conn.connect() != 0 ) {
    kWarning() << "failed to connect: " << m_conn.connectionError();
    return;
  }
  m_state = Connected;
  authenticate();
}

void LdapSession::disconnectFromServer()
{
  m_conn.close();
  m_state = Disconnected;
}

void LdapSession::authenticate()
{
  kDebug();
  LdapOperation op( m_conn );
  while ( true ) {
    int retval = op.bind_s();
    if ( retval == 0 ) {
      kDebug() << "connected!";
      m_state = Authenticated;
      return;
    }
    if ( retval == KLDAP_INVALID_CREDENTIALS ||
         retval == KLDAP_INSUFFICIENT_ACCESS ||
         retval == KLDAP_INAPPROPRIATE_AUTH  ||
         retval == KLDAP_UNWILLING_TO_PERFORM ) {

      if ( m_server.auth() != LdapServer::SASL ) {
        m_server.setBindDn( m_server.user() );
      }
      m_conn.setServer( m_server );
    } else {
//       LDAPErr( retval );
      disconnectFromServer();
      kDebug() << "error" << retval;
      return;
    }
  }
}

void LdapSession::addJob(KJob* job)
{
  kDebug() << "TODO" << job;
}


#include "ldapsession.moc"
