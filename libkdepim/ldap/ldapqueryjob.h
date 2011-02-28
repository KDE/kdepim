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

#ifndef KLDAP_LDAPQUERYJOB_H
#define KLDAP_LDAPQUERYJOB_H

#include <KJob>
#include <kldap/ldapurl.h>
#include "ldapsession.h"
#include <kldap/ldapoperation.h>

namespace KLDAP {

class LdapQueryJob : public KJob
{
  Q_OBJECT
  public:
    LdapQueryJob( const LdapUrl &url, LdapSession * session );

    void start();

  public Q_SLOTS:
    void triggerStart();

  signals:
    void data( const QByteArray &data );

  private:
    LdapUrl m_url;
    LdapSession *m_session;
    LdapOperation m_op;
};

}

#endif
