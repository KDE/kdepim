/*
    Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include <QObject>
#include <QDir>
#include <qtest.h>

#include <KConfig>
#include <KConfigGroup>
#include <QTemporaryFile>

#include <Libkdepim/LdapClientSearchConfig>
#include "../ldap.h"

class TLdap : public Ldap
{
public:
    explicit TLdap(QObject *parent = Q_NULLPTR)
        : Ldap(parent)
    {
        mTempFile.open();
        mTempFile.close();
        mConfig = new KConfig(mTempFile.fileName(), KConfig::SimpleConfig);
        m_clientSearchConfig->askForWallet(false);
    }

    virtual ~TLdap()
    {
        mTempFile.close();
        delete mConfig;
    }

    virtual KConfig *config() const
    {
        mConfig->reparseConfiguration();
        return mConfig;
    }

    QTemporaryFile mTempFile;
    KConfig *mConfig;
};

class LdapTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDestroy()
    {
        TLdap l;
        QFile::remove(l.mTempFile.fileName());
        QFile::copy(QStringLiteral(TEST_DATA_DIR) + QStringLiteral("/ldap.cfg"), l.mTempFile.fileName());
        KLDAP::LdapClientSearchConfig csc;
        csc.askForWallet(false);

        KConfigGroup group = l.config()->group("LDAP");
        int cSelHosts = group.readEntry("NumSelectedHosts", 0);
        int cHosts = group.readEntry("NumHosts", 0);
        QCOMPARE(cSelHosts, 3);
        QCOMPARE(cHosts, 1);
        QVector<KLDAP::LdapServer> selHosts;
        selHosts.reserve(cSelHosts);

        for (int i = 0; i < cSelHosts; ++i) {
            KLDAP::LdapServer server;
            csc.readConfig(server, group, i, true);
            selHosts.append(server);
        }
        QVector<KLDAP::LdapServer> hosts;
        hosts.reserve(cHosts);
        for (int i = 0; i < cHosts; ++i) {
            KLDAP::LdapServer server;
            csc.readConfig(server, group, i, false);
            hosts.append(server);
        }

        l.m_entry = 0;
        l.destroy();
        group = l.config()->group("LDAP");

        QCOMPARE(group.readEntry("NumSelectedHosts", 0), 2);
        QCOMPARE(group.readEntry("NumHosts", 0), 1);
        KLDAP::LdapServer server;
        csc.readConfig(server, group, 0, false);
        QCOMPARE(server.host(), hosts.at(0).host());

        csc.readConfig(server, group, 0, true);
        QCOMPARE(server.host(), selHosts.at(1).host());
        csc.readConfig(server, group, 1, true);
        QCOMPARE(server.host(), selHosts.at(2).host());
    }
};

QTEST_MAIN(LdapTest)

#include "ldaptest.moc"
