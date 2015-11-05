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
    QTest::newRow("catch with several space") << (QStringList() << QStringLiteral("ArchiveMailCollection   12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
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

// verify all pattern used as <word><space><a less one digit> => QStringLiteral("ArchiveMailCollection\\s\\d+");
void RegularExpressionTests::shouldVerifyQStringListFilterSpaceConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("ArchiveMailCollection\\s\\d+");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString() ) << QStringList() << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch") << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;
    QTest::newRow("catch with several space") << (QStringList() << QStringLiteral("ArchiveMailCollection   12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("rchiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("lowercase") << (QStringList() << QStringLiteral("archiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("ArchiveMailCollection VV") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("missing space") << (QStringList() << QStringLiteral("ArchiveMailCollection55") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78") )
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;
}

void RegularExpressionTests::shouldVerifyQStringListFilterSpaceConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}


//QRegExp("mailbox-") //balsasettings.cpp
void RegularExpressionTests::shouldVerifyQStringListFilterTwoConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("mailbox-");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString() ) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") )
                           << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") ) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8") )
                                      << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") ) << regExpStr;


    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8") )
                                      << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") ) << regExpStr;

    QTest::newRow("uppercase") << (QStringList() << QStringLiteral("Mailbox-12") << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8") )
                               << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") ) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Mailbox-AA") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8") )
                               << (QStringList() << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8") ) << regExpStr;
}

void RegularExpressionTests::shouldVerifyQStringListFilterTwoConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}

//QRegExp("Account: \\d+") see clawsmailsettings
void RegularExpressionTests::shouldVerifyQStringListFilterDoublePointConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("Account: \\d+");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString() ) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") )
                           << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") ) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8") )
                                      << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") ) << regExpStr;


    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8") )
                                      << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") ) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("account: 12") << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8") )
                               << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") ) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Account: AA") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8") )
                               << (QStringList() << QStringLiteral("Account: 5") << QStringLiteral("Account: 8") ) << regExpStr;
}

void RegularExpressionTests::shouldVerifyQStringListFilterDoublePointConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}

//QRegExp("Filter #\\d+") see filterimporterexporter.cpp
void RegularExpressionTests::shouldVerifyQStringListFilterWithSharpConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("Filter #\\d+");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString() ) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") )
                           << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") ) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8") )
                                      << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") ) << regExpStr;


    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8") )
                                      << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") ) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("filter #12") << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8") )
                               << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") ) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Filter #AA") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8") )
                             << (QStringList() << QStringLiteral("Filter #5") << QStringLiteral("Filter #8") ) << regExpStr;
}


void RegularExpressionTests::shouldVerifyQStringListFilterWithSharpConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}


void RegularExpressionTests::shouldReplaceString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("replacewith");
    QTest::addColumn<QString>("regexp");
    QTest::newRow("email with at") << QStringLiteral("foo (at) kde.org") << QStringLiteral("foo@kde.org") << QStringLiteral("@") << QStringLiteral("\\s*\\(at\\)\\s*");
}

void RegularExpressionTests::shouldReplaceString()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QFETCH(QString, replacewith);
    QFETCH(QString, regexp);

    QCOMPARE(input.replace(QRegExp(regexp), replacewith), expected);
    QCOMPARE(input.replace(QRegularExpression(regexp), replacewith), expected);
}


QTEST_MAIN(RegularExpressionTests)
