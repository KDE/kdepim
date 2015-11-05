/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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


#include "regularexpressiontests.h"

#include <QTest>
#include <QRegExp>
#include <QRegularExpression>

RegularExpressionTests::RegularExpressionTests(QObject *parent)
    : QObject(parent)
{

}

RegularExpressionTests::~RegularExpressionTests()
{

}

// verify all pattern used as <word><space><a less one digit> => QStringLiteral("ArchiveMailCollection \\d+");
void RegularExpressionTests::shouldVerifyQStringListFilterConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    QTest::newRow("empty") <<  QStringList() << QStringList() << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString() ) << QStringList() << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch") << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("rchiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("lowercase") << (QStringList() << QStringLiteral("archiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("ArchiveMailCollection VV") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("missing space") << (QStringList() << QStringLiteral("ArchiveMailCollection55") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
}

void RegularExpressionTests::shouldVerifyQStringListFilterConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}

QTEST_MAIN(RegularExpressionTests)
