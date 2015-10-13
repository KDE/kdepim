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

#include "../ispdb.h"
#include "../autoconfigkolabmail.h"

class TAutoconfMail : public AutoconfigKolabMail
{
public:
    void startJob(const QUrl &url) Q_DECL_OVERRIDE {
        QCOMPARE(url, expectedUrls.takeFirst());
        if (replace.contains(url))
        {
            AutoconfigKolabMail::startJob(replace[url]);
        } else {
            AutoconfigKolabMail::startJob(url);
        }
    }

    QMap<QUrl, QUrl> replace;
    QList<QUrl> expectedUrls;
};

class AutoconfMailTest : public QObject
{
    Q_OBJECT
public:
    TAutoconfMail *getAutoconf()
    {
        return new TAutoconfMail();
    }

    AutoconfigKolabMail *execIspdb(const QString &file)
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QLatin1Char('/') + file;
        [](const QString & file) {
            QVERIFY(QFile(file).exists());
        }(filePath);

        QEventLoop loop;
        TAutoconfMail *ispdb = getAutoconf();

        connect(ispdb, &AutoconfigKolabMail::finished, &loop, &QEventLoop::quit);

        const QUrl url = QUrl::fromLocalFile(filePath);
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->expectedUrls.append(url);
        ispdb->startJob(url);

        loop.exec();
        return ispdb;
    }

private Q_SLOTS:
    void testCompleteFail()
    {
        QEventLoop loop;
        TAutoconfMail *ispdb = getAutoconf();

        connect(ispdb, &AutoconfigKolabMail::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabMail::finished, this, &AutoconfMailTest::expectedReturn);

        QUrl expected0(QStringLiteral("https://autoconfig.thunderbird.net/v1.1/example.com"));
        QUrl expected1(QStringLiteral("http://autoconfig.example.com/mail/config-v1.1.xml"));
        QUrl expected2(QStringLiteral("http://example.com/.well-known/autoconfig/mail/config-v1.1.xml"));
        mReturn = false;
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->expectedUrls.append(expected0);
        ispdb->expectedUrls.append(expected1);
        ispdb->expectedUrls.append(expected2);
        ispdb->replace[expected0] = QStringLiteral("http://localhost:8000/500");
        ispdb->replace[expected1] = QStringLiteral("http://localhost:8000/500");
        ispdb->replace[expected2] = QStringLiteral("http://localhost:8000/404");
        ispdb->start();
        loop.exec();
        QCOMPARE(ispdb->expectedUrls.count(), 0);
    }

    void testLogin()
    {
        const QString filePath = QStringLiteral(AUTOCONFIG_DATA_DIR) + QStringLiteral("/autoconfig.xml");
        QVERIFY(QFile(filePath).exists());

        QEventLoop loop;
        TAutoconfMail *ispdb = getAutoconf();

        connect(ispdb, &AutoconfigKolabMail::finished, &loop, &QEventLoop::quit);
        connect(ispdb, &AutoconfigKolabMail::finished, this, &AutoconfMailTest::expectedReturn);

        QUrl expected0(QStringLiteral("https://autoconfig.thunderbird.net/v1.1/example.com"));
        QUrl expected1(QStringLiteral("http://autoconfig.example.com/mail/config-v1.1.xml"));
        QUrl expected2(QStringLiteral("https://john.doe%40example.com:xxx@autoconfig.example.com/mail/config-v1.1.xml"));
        QUrl expected3(QStringLiteral("http://example.com/.well-known/autoconfig/mail/config-v1.1.xml"));

        mReturn = true;
        ispdb->setEmail(QStringLiteral("john.doe@example.com"));
        ispdb->setPassword(QStringLiteral("xxx"));
        ispdb->expectedUrls.append(expected0);
        ispdb->expectedUrls.append(expected1);
        ispdb->expectedUrls.append(expected2);
        ispdb->expectedUrls.append(expected3);
        ispdb->replace[expected0] = QStringLiteral("http://localhost:8000/500");
        ispdb->replace[expected1] = QStringLiteral("http://localhost:8000/401");
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

QTEST_MAIN(AutoconfMailTest)

#include "autoconfigkolabmailtest.moc"
