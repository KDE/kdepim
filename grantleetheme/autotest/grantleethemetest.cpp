/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grantleethemetest.h"
#include "grantleetheme.h"

#include <qtest.h>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

GrantleeThemeTest::GrantleeThemeTest(QObject *parent)
    : QObject(parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

GrantleeThemeTest::~GrantleeThemeTest()
{

}

void GrantleeThemeTest::shouldHaveDefaultValue()
{
    GrantleeTheme::Theme theme;
    QVERIFY(theme.description().isEmpty());
    QVERIFY(theme.themeFilename().isEmpty());
    QVERIFY(theme.name().isEmpty());
    QVERIFY(theme.displayExtraVariables().isEmpty());
    QVERIFY(theme.dirName().isEmpty());
    QVERIFY(theme.absolutePath().isEmpty());
    QVERIFY(theme.author().isEmpty());
    QVERIFY(theme.authorEmail().isEmpty());
    QVERIFY(!theme.isValid());
}

void GrantleeThemeTest::shouldInvalidWhenPathIsNotValid()
{
    const QString themePath(QStringLiteral("/foo"));
    const QString dirName(QStringLiteral("name"));
    const QString defaultDesktopFileName(QStringLiteral("bla"));
    GrantleeTheme::Theme theme(themePath, dirName, defaultDesktopFileName);
    QVERIFY(theme.description().isEmpty());
    QVERIFY(theme.themeFilename().isEmpty());
    QVERIFY(theme.name().isEmpty());
    QVERIFY(theme.displayExtraVariables().isEmpty());
    QVERIFY(!theme.dirName().isEmpty());
    QVERIFY(!theme.absolutePath().isEmpty());
    QVERIFY(theme.author().isEmpty());
    QVERIFY(theme.authorEmail().isEmpty());
    QVERIFY(!theme.isValid());
}

void GrantleeThemeTest::shouldLoadTheme_data()
{
    QTest::addColumn<QString>("dirname");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("isvalid");
    QTest::addColumn<QStringList>("displayExtraVariables");

    QTest::newRow("valid theme") <<  QStringLiteral("valid") << QStringLiteral("filename.testdesktop") << true << QStringList();
    QTest::newRow("not existing theme") <<  QStringLiteral("notvalid") << QStringLiteral("filename.testdesktop") << false << QStringList();
    QStringList extraVariables;
    extraVariables << QStringLiteral("foo") << QStringLiteral("bla");
    QTest::newRow("valid with extra variable") <<  QStringLiteral("valid-with-extravariables") << QStringLiteral("filename.testdesktop") << true << extraVariables;
}

void GrantleeThemeTest::shouldLoadTheme()
{
    QFETCH(QString, dirname);
    QFETCH(QString, filename);
    QFETCH(bool, isvalid);
    QFETCH(QStringList, displayExtraVariables);

    GrantleeTheme::Theme theme(QStringLiteral(GRANTLEETHEME_DATA_DIR) + QDir::separator() + dirname, dirname, filename);
    QCOMPARE(theme.isValid(), isvalid);
    QCOMPARE(theme.displayExtraVariables(), displayExtraVariables);
    QCOMPARE(theme.dirName(), dirname);
}

bool GrantleeThemeTest::validateHtml(const QString &themePath, const QString &name, const QString &html)
{
    const QString outFileName = themePath + QStringLiteral("/%1.out").arg(name);
    const QString htmlFileName = themePath + QStringLiteral("/%1.out.html").arg(name);
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    outFile.write(html.toUtf8());
    outFile.close();

    // validate xml and pretty-print for comparisson
    // TODO add proper cmake check for xmllint and diff
    const QStringList args = {
        QStringLiteral("--format"),
        QStringLiteral("--encode"),
        QStringLiteral("UTF8"),
        QStringLiteral("--output"),
        htmlFileName,
        outFileName
    };

    const int result = QProcess::execute(QStringLiteral("xmllint"), args);
    return result == 0;
}

bool GrantleeThemeTest::compareHtml(const QString &themePath, const QString &name)
{
    const QString htmlFileName = themePath + QStringLiteral("/%1.out.html").arg(name);
    const QString referenceFileName = themePath + QStringLiteral("/%1_expected.html").arg(name);

    // get rid of system dependent or random paths
    {
        QFile f(htmlFileName);
        if (!f.open(QIODevice::ReadOnly)) {
            return false;
        }
        QString content = QString::fromUtf8(f.readAll());
        f.close();
        content.replace(QRegExp(QLatin1String("\"file:[^\"]*[/(?:%2F)]([^\"/(?:%2F)]*)\"")), QStringLiteral("\"file:\\1\""));
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        f.write(content.toUtf8());
        f.close();
    }

    // compare to reference file
    const QStringList args = {
        QStringLiteral("-u"),
        referenceFileName,
        htmlFileName
    };

    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start(QStringLiteral("diff"), args);
    if (!proc.waitForFinished()) {
        return false;
    }

    return proc.exitCode() == 0;
}

void GrantleeThemeTest::testRenderTemplate_data()
{
    QTest::addColumn<QString>("dirname");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("templateBasename");

    QTest::newRow("valid theme") << QStringLiteral("valid") << true << QStringLiteral("filename.testdesktop") << QStringLiteral("header");
    QTest::newRow("invalid theme") << QStringLiteral("invalid") << false << QStringLiteral("filename.testdesktop") << QString();
}

void GrantleeThemeTest::testRenderTemplate()
{
    QFETCH(QString, dirname);
    QFETCH(bool, isValid);
    QFETCH(QString, filename);
    QFETCH(QString, templateBasename);

    const QString themePath = QStringLiteral(GRANTLEETHEME_DATA_DIR) + QDir::separator() + dirname;

    QVariantHash data;
    data[QStringLiteral("icon")] = QStringLiteral("kmail");
    data[QStringLiteral("name")] = QStringLiteral("KMail");
    data[QStringLiteral("subtitle")] = QStringLiteral("...just rocks!");
    data[QStringLiteral("title")] = QStringLiteral("Something's going on");
    data[QStringLiteral("subtext")] = QStringLiteral("Please wait, it will be over soon.");

    GrantleeTheme::Theme theme(themePath, dirname, filename);
    QCOMPARE(theme.isValid(), isValid);

    if (isValid) {
        const QString result = theme.render(templateBasename + QStringLiteral(".html"), data);

        QVERIFY(validateHtml(themePath, templateBasename, result));
        QVERIFY(compareHtml(themePath, templateBasename));

        QFile::remove(themePath + QDir::separator() + templateBasename + QStringLiteral(".out"));
        QFile::remove(themePath + QDir::separator() + templateBasename + QStringLiteral(".out.html"));
    }
}

QTEST_MAIN(GrantleeThemeTest)
