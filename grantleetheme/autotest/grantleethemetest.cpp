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
#include "../grantleetheme.h"
#include <qtest.h>
#include <QDir>

GrantleeThemeTest::GrantleeThemeTest(QObject *parent)
    : QObject(parent)
{

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

QTEST_MAIN(GrantleeThemeTest)
