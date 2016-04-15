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
#include <QProcess>
#include <QTest>
#include <QTcpSocket>

#include "../autoconfigkolabldap.h"

class TAutoconfLdap : public AutoconfigKolabLdap
{
public:
    void startJob(const QUrl &url) Q_DECL_OVERRIDE {
        QCOMPARE(url, expectedUrls.takeFirst());
        if (replace.contains(url))
        {
            AutoconfigKolabLdap::startJob(replace[url]);
        } else {
            AutoconfigKolabLdap::startJob(url);
        }
    }

    QMap<QUrl, QUrl> replace;
    QList<QUrl> expectedUrls;
};

class AutoconfLdapTest : public QObject
{
    Q_OBJECT
public:
    AutoconfigKolabLdap *execIspdb(const QString &file)
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QLatin1Char('/') + file;
        [](const QString & file) {
            QVERIFY(QFile(file).exists());
        }(filePath);

        QEventLoop loop;
        TAutoconfLdap *ispdb = new TAutoconfLdap();

        connect(ispdb, &AutoconfigKolabLdap::finished, &loop, &QEventLoop::quit);

        const QUrl url = QUrl::fromLocalFile(filePath);
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->expectedUrls.append(url);
        ispdb->startJob(url);

        loop.exec();
        return ispdb;
    }

    void testLdapServer(const ldapServer &test, const ldapServer &expected) const
    {
        QCOMPARE(test.hostname, expected.hostname);
        QCOMPARE(test.port, expected.port);
        QCOMPARE(test.socketType, expected.socketType);
        QCOMPARE(test.authentication, expected.authentication);
        QCOMPARE(test.bindDn, expected.bindDn);
        QCOMPARE(test.password, expected.password);
        QCOMPARE(test.saslMech, expected.saslMech);
        QCOMPARE(test.username, expected.username);
        QCOMPARE(test.realm, expected.realm);
        QCOMPARE(test.dn, expected.dn);
        QCOMPARE(test.ldapVersion, expected.ldapVersion);
        QCOMPARE(test.filter, expected.filter);
        QCOMPARE(test.pageSize, expected.pageSize);
        QCOMPARE(test.timeLimit, expected.timeLimit);
        QCOMPARE(test.sizeLimit, expected.sizeLimit);
    }

    bool checkServerReady() const
    {
        QTcpSocket socket;
        socket.connectToHost(QStringLiteral("localhost"), 8000);
        return socket.waitForConnected(5000);
    }

private Q_SLOTS:
    void testLdapParsing()
    {
        AutoconfigKolabLdap *ispdb = execIspdb(QStringLiteral("ldap.xml"));

        ldapServer s;
        QCOMPARE(ispdb->ldapServers().count(), 2);

        s.hostname = QStringLiteral("ldap.example.com");
        s.port = 389;
        s.socketType = KLDAP::LdapServer::None;
        s.authentication = KLDAP::LdapServer::Simple;
        s.bindDn = QStringLiteral("cn=Directory Manager");
        s.password = QStringLiteral("Welcome2KolabSystems");
        s.saslMech = QString();
        s.username = QString();
        s.realm = QString();
        s.dn = QStringLiteral("dc=kolabsys,dc=com");
        s.ldapVersion = 3;
        s.filter = QString();
        s.pageSize = -1;
        s.timeLimit = -1;
        s.sizeLimit = -1;

        testLdapServer(ispdb->ldapServers()[QStringLiteral("ldap.example.com")], s);

        s.hostname = QStringLiteral("ldap2.example.com");
        s.port = 387;
        s.socketType = KLDAP::LdapServer::SSL;
        s.authentication = KLDAP::LdapServer::SASL;
        s.bindDn = QStringLiteral("cn=Directory");
        s.password = QStringLiteral("Welcome2KolabSystems");
        s.saslMech = QStringLiteral("XXX");
        s.username = QStringLiteral("john.doe");
        s.realm = QStringLiteral("realm.example.com");
        s.dn = QStringLiteral("dc=example,dc=com");
        s.ldapVersion = 3;
        s.filter = QString();
        s.pageSize = 10;
        s.timeLimit = -1;
        s.sizeLimit = 9999999;

        testLdapServer(ispdb->ldapServers()[QStringLiteral("ldap2.example.com")], s);
    }

    void testLdapCompleteFail()
    {
        QEventLoop loop;
        TAutoconfLdap *ispdb = new TAutoconfLdap();

        connect(ispdb, &AutoconfigKolabLdap::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabLdap::finished, this, &AutoconfLdapTest::expectedReturn);

        QUrl expected(QStringLiteral("http://autoconfig.example.com/ldap/config-v1.0.xml"));
        QUrl expected2(QStringLiteral("http://example.com/.well-known/autoconfig/ldap/config-v1.0.xml"));

        mReturn = false;
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->expectedUrls.append(expected);
        ispdb->expectedUrls.append(expected2);
        ispdb->replace[expected] = QStringLiteral("http://localhost:8000/500");
        ispdb->replace[expected2] = QStringLiteral("http://localhost:8000/404");
        ispdb->start();
        loop.exec();
        QCOMPARE(ispdb->expectedUrls.count(), 0);
    }

    void testLdapLogin()
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QStringLiteral("/ldap.xml");
        QVERIFY(QFile(filePath).exists());

        QEventLoop loop;
        TAutoconfLdap *ispdb = new TAutoconfLdap();

        connect(ispdb, &AutoconfigKolabLdap::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabLdap::finished, this, &AutoconfLdapTest::expectedReturn);

        QUrl expected(QStringLiteral("http://autoconfig.example.com/ldap/config-v1.0.xml"));
        QUrl expected2(QStringLiteral("https://john.doe%40example.com:xxx@autoconfig.example.com/ldap/config-v1.0.xml"));
        QUrl expected3(QStringLiteral("http://example.com/.well-known/autoconfig/ldap/config-v1.0.xml"));

        mReturn = true;
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->setPassword(QStringLiteral("xxx"));
        ispdb->expectedUrls.append(expected);
        ispdb->expectedUrls.append(expected2);
        ispdb->expectedUrls.append(expected3);
        ispdb->replace[expected] = QStringLiteral("http://localhost:8000/401");
        ispdb->replace[expected2] = QStringLiteral("http://localhost:8000/500");
        ispdb->replace[expected3] = QUrl::fromLocalFile(filePath);
        ispdb->start();
        loop.exec();
        QCOMPARE(ispdb->expectedUrls.count(), 0);
    }

    void expectedReturn(bool ret)
    {
        QCOMPARE(ret, mReturn);
    }

    void initTestCase()
    {
        const QString filePath = QStringLiteral(CURRENT_SOURCE_DIR "/errorserver.py");
        QVERIFY(QFile(filePath).exists());

        process.start(QStringLiteral("python"), QStringList() << filePath);
        process.waitForStarted();
        QCOMPARE(process.state(), QProcess::Running);

        // Wait for the server to start listening
        QTRY_VERIFY(checkServerReady());
    }

    void cleanupTestCase()
    {
        process.terminate();
        process.waitForFinished();
    }

public:
    bool mReturn;
    QProcess process;
};

QTEST_MAIN(AutoconfLdapTest)

#include "autoconfigkolabldaptest.moc"
