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
#include <QTest>
#include <QProcess>

#include "../autoconfigkolabfreebusy.h"

class TAutoconfFreebusy : public AutoconfigKolabFreebusy
{
public:
    void startJob(const QUrl &url) Q_DECL_OVERRIDE {
        QCOMPARE(url, expectedUrls.takeFirst());
        if (replace.contains(url))
        {
            AutoconfigKolabFreebusy::startJob(replace[url]);
        } else {
            AutoconfigKolabFreebusy::startJob(url);
        }
    }

    QMap<QUrl, QUrl> replace;
    QList<QUrl> expectedUrls;
};

class AutoconfFreebusyTest : public QObject
{
    Q_OBJECT
public:
    AutoconfigKolabFreebusy *execIspdb(const QString &file)
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QLatin1Char('/') + file;
        [](const QString & file) {
            QVERIFY(QFile(file).exists());
        }(filePath);

        QEventLoop loop;
        TAutoconfFreebusy *ispdb = new TAutoconfFreebusy();

        connect(ispdb, &AutoconfigKolabFreebusy::finished, &loop, &QEventLoop::quit);

        const QUrl url = QUrl::fromLocalFile(filePath);
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->expectedUrls.append(url);
        ispdb->startJob(url);

        loop.exec();
        return ispdb;
    }

    void testFreebusy(const freebusy &test, const freebusy &expected) const
    {
        QVERIFY(test.isValid());
        QCOMPARE(test.hostname, expected.hostname);
        QCOMPARE(test.port, expected.port);
        QCOMPARE(test.socketType, expected.socketType);
        QCOMPARE(test.authentication, expected.authentication);
        QCOMPARE(test.username, expected.username);
        QCOMPARE(test.password, expected.password);
        QCOMPARE(test.path, expected.path);
    }
private Q_SLOTS:
    void testFreebusyParsing()
    {
        AutoconfigKolabFreebusy *ispdb = execIspdb(QStringLiteral("freebusy.xml"));

        freebusy s;

        s.hostname = QStringLiteral("example.com");
        s.port = 80;
        s.socketType = Ispdb::None;
        s.authentication = Ispdb::Basic;
        s.username = QStringLiteral("user");
        s.password = QStringLiteral("pass");
        s.path = QStringLiteral("/freebusy/$EMAIL$.ifb");

        QCOMPARE(ispdb->freebusyServers().count(), 1);
        testFreebusy(ispdb->freebusyServers()[QStringLiteral("freebusy.example.com")], s);
    }

    void testFreebusyCompleteFail()
    {
        QEventLoop loop;
        TAutoconfFreebusy *ispdb = new TAutoconfFreebusy();

        connect(ispdb, &AutoconfigKolabFreebusy::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabFreebusy::finished, this, &AutoconfFreebusyTest::expectedReturn);

        QUrl expected(QStringLiteral("http://autoconfig.example.com/freebusy/config-v1.0.xml"));
        QUrl expected2(QStringLiteral("http://example.com/.well-known/autoconfig/freebusy/config-v1.0.xml"));
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

    void testFreebusyLogin()
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QStringLiteral("/freebusy.xml");
        QVERIFY(QFile(filePath).exists());

        QEventLoop loop;
        TAutoconfFreebusy *ispdb = new TAutoconfFreebusy();

        connect(ispdb, &AutoconfigKolabFreebusy::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabFreebusy::finished, this, &AutoconfFreebusyTest::expectedReturn);

        QUrl expected(QStringLiteral("http://autoconfig.example.com/freebusy/config-v1.0.xml"));
        QUrl expected2(QStringLiteral("https://john.doe%40example.com:xxx@autoconfig.example.com/freebusy/config-v1.0.xml"));
        QUrl expected3(QStringLiteral("http://example.com/.well-known/autoconfig/freebusy/config-v1.0.xml"));

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

QTEST_MAIN(AutoconfFreebusyTest)

#include "autoconfigkolabfreebusy.moc"
