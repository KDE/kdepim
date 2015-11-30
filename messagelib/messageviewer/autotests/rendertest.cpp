/*
  Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "htmlwriter/filehtmlwriter.h"
#include "viewer/objecttreeparser.h"
#include "testcsshelper.h"
#include "setupenv.h"

#include <KMime/Message>
#include <qtest.h>
#include <QDir>
#include <QObject>
#include <QProcess>

using namespace MessageViewer;

class RenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        MessageViewer::Test::setupEnv();
    }

    void testRender_data()
    {
        QTest::addColumn<QString>("mailFileName");
        QTest::addColumn<QString>("referenceFileName");
        QTest::addColumn<QString>("outFileName");

        QDir dir(QStringLiteral(MAIL_DATA_DIR));
        foreach (const QString &file, dir.entryList(QStringList(QLatin1String("*.mbox")), QDir::Files | QDir::Readable | QDir::NoSymLinks)) {
            if (!QFile::exists(dir.path() + QLatin1Char('/') + file + QLatin1String(".html"))) {
                continue;
            }
            QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') +  file) << QString(dir.path() + QLatin1Char('/') + file + QLatin1String(".html")) << QString(file + QLatin1String(".out"));
        }
    }

    void testRender()
    {
        QFETCH(QString, mailFileName);
        QFETCH(QString, referenceFileName);
        QFETCH(QString, outFileName);

        const QString htmlFileName = outFileName + QLatin1String(".html");

        // load input mail
        QFile mailFile(mailFileName);
        QVERIFY(mailFile.open(QIODevice::ReadOnly));
        const QByteArray mailData = KMime::CRLFtoLF(mailFile.readAll());
        QVERIFY(!mailData.isEmpty());
        KMime::Message::Ptr msg(new KMime::Message);
        msg->setContent(mailData);
        msg->parse();

        // render the mail
        FileHtmlWriter fileWriter(outFileName);
        QImage paintDevice;
        TestCSSHelper cssHelper(&paintDevice);
        NodeHelper nodeHelper;
        MessageViewer::Test::TestObjectTreeSource testSource(&fileWriter, &cssHelper);
        testSource.setAllowDecryption(true);
        ObjectTreeParser otp(&testSource, &nodeHelper);

        fileWriter.begin(QString());
        fileWriter.queue(cssHelper.htmlHead(false));

        otp.parseObjectTree(msg.data());

        fileWriter.queue(QStringLiteral("</body></html>"));
        fileWriter.flush();
        fileWriter.end();

        QVERIFY(QFile::exists(outFileName));

        // validate xml and pretty-print for comparisson
        // TODO add proper cmake check for xmllint and diff
        QStringList args = QStringList()
                           << QStringLiteral("--format")
                           << QStringLiteral("--encode")
                           << QStringLiteral("UTF8")
                           << QStringLiteral("--output")
                           << htmlFileName
                           << outFileName;
        QCOMPARE(QProcess::execute(QLatin1String("xmllint"), args),  0);

        // get rid of system dependent or random paths
        {
            QFile f(htmlFileName);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QString content = QString::fromUtf8(f.readAll());
            f.close();
            content.replace(QRegExp(QLatin1String("\"file:[^\"]*[/(?:%2F)]([^\"/(?:%2F)]*)\"")), QStringLiteral("\"file:\\1\""));
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(content.toUtf8());
            f.close();
        }

        // compare to reference file
        args = QStringList()
               << QStringLiteral("-u")
               << referenceFileName
               << htmlFileName;
        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start(QStringLiteral("diff"), args);
        QVERIFY(proc.waitForFinished());

        QCOMPARE(proc.exitCode(), 0);
    }
};

QTEST_MAIN(RenderTest)

#include "rendertest.moc"
