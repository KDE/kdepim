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

#ifndef IDENTITY_H
#define IDENTITY_H

#include "setupobject.h"

class Transport;

namespace GpgME {
class Key;
}

namespace KIdentityManagement
{
class Identity;
class IdentityManager;
}

class Identity : public SetupObject
{
    Q_OBJECT
public:
    explicit Identity(QObject *parent = Q_NULLPTR);
    ~Identity();
    void create() Q_DECL_OVERRIDE;
    void destroy() Q_DECL_OVERRIDE;

public Q_SLOTS:
    Q_SCRIPTABLE void setIdentityName(const QString &name);
    Q_SCRIPTABLE void setRealName(const QString &name);
    Q_SCRIPTABLE void setEmail(const QString &email);
    Q_SCRIPTABLE void setOrganization(const QString &org);
    Q_SCRIPTABLE void setSignature(const QString &sig);
    Q_SCRIPTABLE uint uoid() const;
    Q_SCRIPTABLE void setTransport(QObject *transport);
    Q_SCRIPTABLE void setPreferredCryptoMessageFormat(const QString &format);
    Q_SCRIPTABLE void setXFace(const QString &xface);
    Q_SCRIPTABLE void setPgpAutoSign(bool autosign);
    Q_SCRIPTABLE void setPgpAutoEncrypt(bool autoencrypt);
    Q_SCRIPTABLE void setKey(const GpgME::Key &key);

protected:
    QString identityName() const;

private:
    QString m_identityName;
    KIdentityManagement::IdentityManager *m_manager;
    KIdentityManagement::Identity *m_identity;
};

#endif
