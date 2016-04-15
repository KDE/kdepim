/*
    Copyright (c) 2010 Laurent Montel <montel@kde.org>

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

#ifndef LDAP_H
#define LDAP_H

#include "setupobject.h"
#include <KLDAP/LdapServer>

class LdapTest;
class KConfig;

namespace KLDAP
{
class LdapClientSearchConfig;
}

class Ldap : public SetupObject
{
    Q_OBJECT
public:
    explicit Ldap(QObject *parent = Q_NULLPTR);
    ~Ldap();
    void create() Q_DECL_OVERRIDE;
    void destroy() Q_DECL_OVERRIDE;
    void edit();
public Q_SLOTS:
    Q_SCRIPTABLE void setUser(const QString &name);
    Q_SCRIPTABLE void setServer(const QString &server);
    Q_SCRIPTABLE void setAuthenticationMethod(const QString &meth);
    Q_SCRIPTABLE void setBindDn(const QString &bindDn);
    Q_SCRIPTABLE void setBaseDn(const QString &baseDn);
    Q_SCRIPTABLE void setPassword(const QString &password);
    Q_SCRIPTABLE void setPort(const int port);
    Q_SCRIPTABLE void setSecurity(const KLDAP::LdapServer::Security security);
    Q_SCRIPTABLE void setSaslMech(const QString &saslmech);
    Q_SCRIPTABLE void setRealm(const QString &realm);
    Q_SCRIPTABLE void setVersion(const int version);
    Q_SCRIPTABLE void setPageSize(const int pageSize);
    Q_SCRIPTABLE void setTimeLimit(const int timeLimit);
    Q_SCRIPTABLE void setSizeLimit(const int sizeLimit);
    Q_SCRIPTABLE void setEditMode(const bool editMode);

protected:
    virtual KConfig *config() const;

    KLDAP::LdapClientSearchConfig *m_clientSearchConfig;
private:
    friend class LdapTest;
    QString securityString();

    QString m_user;
    QString m_server;
    QString m_bindDn;
    QString m_authMethod;
    QString m_password;
    QString m_mech;
    QString m_realm;
    QString m_baseDn;
    int m_port;
    KLDAP::LdapServer::Security m_security;
    int m_version;
    int m_pageSize;
    int m_timeLimit;
    int m_sizeLimit;
    int m_entry;
    bool m_editMode;
};

#endif
