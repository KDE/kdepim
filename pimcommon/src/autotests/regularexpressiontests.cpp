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
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch") << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch with several space") << (QStringList() << QStringLiteral("ArchiveMailCollection   12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
            << (QStringList() << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                                      << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("rchiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
            << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("lowercase") << (QStringList() << QStringLiteral("archiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                               << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("ArchiveMailCollection VV") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                             << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");

    QTest::newRow("missing space") << (QStringList() << QStringLiteral("ArchiveMailCollection55") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
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
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch") << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                           << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;
    QTest::newRow("catch with several space") << (QStringList() << QStringLiteral("ArchiveMailCollection   12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
            << (QStringList() << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << QStringLiteral("ArchiveMailCollection \\d+");
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                                      << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("rchiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
            << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("lowercase") << (QStringList() << QStringLiteral("archiveMailCollection 54") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                               << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("ArchiveMailCollection VV") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
                             << (QStringList() << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78")) << regExpStr;

    QTest::newRow("missing space") << (QStringList() << QStringLiteral("ArchiveMailCollection55") << QStringLiteral("ArchiveMailCollection 12") << QStringLiteral("ArchiveMailCollection 58") << QStringLiteral("ArchiveMailCollection 78"))
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
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8"))
                           << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8")) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8"))
                                      << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8"))
            << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8")) << regExpStr;

    QTest::newRow("uppercase") << (QStringList() << QStringLiteral("Mailbox-12") << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8"))
                               << (QStringList() << QStringLiteral("mailbox-12") << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Mailbox-AA") << QStringLiteral("mailbox-5") << QString() << QStringLiteral("mailbox-8"))
                             << (QStringList() << QStringLiteral("mailbox-5") << QStringLiteral("mailbox-8")) << regExpStr;
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
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8"))
                           << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8")) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8"))
                                      << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8"))
            << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8")) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("account: 12") << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8"))
                           << (QStringList() << QStringLiteral("Account: 12") << QStringLiteral("Account: 5") << QStringLiteral("Account: 8")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Account: AA") << QStringLiteral("Account: 5") << QString() << QStringLiteral("Account: 8"))
                             << (QStringList() << QStringLiteral("Account: 5") << QStringLiteral("Account: 8")) << regExpStr;
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

//QRegExp("^ServerSieve (.+)$") see sieveeditorutils
void RegularExpressionTests::shouldVerifyQStringListFilterWithStartCharAndEndConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("^ServerSieve (.+)$");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8"))
                           << (QStringList() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QString() << QStringLiteral("ServerSieve 8"))
                                      << (QStringList() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QString() << QStringLiteral("ServerSieve 8"))
            << (QStringList() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("serverSieve 12") << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QString() << QStringLiteral("ServerSieve 8"))
                           << (QStringList() << QStringLiteral("ServerSieve 12") << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("erverSieve AA") << QStringLiteral("ServerSieve 5") << QString() << QStringLiteral("ServerSieve 8"))
                             << (QStringList() << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;
    QTest::newRow("text before start") << (QStringList() << QStringLiteral("  ServerSieve AA") << QStringLiteral("ServerSieve 5") << QString() << QStringLiteral("ServerSieve 8"))
                                       << (QStringList() << QStringLiteral("ServerSieve 5") << QStringLiteral("ServerSieve 8")) << regExpStr;
}

void RegularExpressionTests::shouldVerifyQStringListFilterWithStartCharAndEndConversion()
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
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8"))
                           << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8")) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8"))
                                      << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8"))
            << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8")) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("filter #12") << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8"))
                           << (QStringList() << QStringLiteral("Filter #12") << QStringLiteral("Filter #5") << QStringLiteral("Filter #8")) << regExpStr;

    QTest::newRow("invalid") << (QStringList() << QStringLiteral("Filter #AA") << QStringLiteral("Filter #5") << QString() << QStringLiteral("Filter #8"))
                             << (QStringList() << QStringLiteral("Filter #5") << QStringLiteral("Filter #8")) << regExpStr;
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
    QTest::newRow("endline one") << QStringLiteral("\n") << QStringLiteral("<br/>") << QStringLiteral("<br/>") << QStringLiteral("\n+");
    QTest::newRow("endline multiple") << QStringLiteral("\n\n") << QStringLiteral("<br/>") << QStringLiteral("<br/>") << QStringLiteral("\n+");
    QTest::newRow("endline multiple with space") << QStringLiteral("\n  \n") << QStringLiteral("<br/>  <br/>") << QStringLiteral("<br/>") << QStringLiteral("\n+");
    QTest::newRow("replace end file") << QStringLiteral("foo.mbx") << QStringLiteral("foo.png") << QStringLiteral("png") << QStringLiteral("mbx$");
    QTest::newRow("replace end file invalid") << QStringLiteral("foo.mbx1") << QStringLiteral("foo.mbx1") << QStringLiteral("png") << QStringLiteral("mbx$");
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

//QRegExp("WinPMail Identity - *") see pmailsettings
void RegularExpressionTests::shouldVerifyQStringListFilterWithPmailSettingsConversion_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QString>("regexp");
    const QString regExpStr = QStringLiteral("WinPMail Identity - *");
    QTest::newRow("empty") <<  QStringList() << QStringList() << regExpStr;
    QTest::newRow("nocatcher") << (QStringList() << QStringLiteral("ArchiveMailCollection DD") << QStringLiteral("ArchiveMailCollection") << QStringLiteral("ArchiveMailCollection ") << QString()) << QStringList() << regExpStr;
    QTest::newRow("catch") << (QStringList() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QStringLiteral("WinPMail Identity - 8"))
                           << (QStringList() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QStringLiteral("WinPMail Identity - 8")) << regExpStr;
    QTest::newRow("catch with empty") << (QStringList() << QString() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QString() << QStringLiteral("WinPMail Identity - 8"))
                                      << (QStringList() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QStringLiteral("WinPMail Identity - 8")) << regExpStr;

    QTest::newRow("catch with not invalid string") << (QStringList() << QStringLiteral("mailbox12") << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QString() << QStringLiteral("WinPMail Identity - 8"))
            << (QStringList() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QStringLiteral("WinPMail Identity - 8")) << regExpStr;

    QTest::newRow("lower") << (QStringList() << QStringLiteral("winPMail Identity - 12") << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QString() << QStringLiteral("WinPMail Identity - 8"))
                           << (QStringList() << QStringLiteral("WinPMail Identity - 12") << QStringLiteral("WinPMail Identity - 5") << QStringLiteral("WinPMail Identity - 8")) << regExpStr;

}

void RegularExpressionTests::shouldContainsString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("contains");
    QTest::addColumn<QString>("regexp");
    QTest::newRow("valid ipv4") << QStringLiteral("123.125.44.12") << true << QStringLiteral("\\b[0-9]{1,3}\\.[0-9]{1,3}(?:\\.[0-9]{0,3})?(?:\\.[0-9]{0,3})?");
    QTest::newRow("not valid ipv4") << QStringLiteral("NOVALID") << false << QStringLiteral("\\b[0-9]{1,3}\\.[0-9]{1,3}(?:\\.[0-9]{0,3})?(?:\\.[0-9]{0,3})?");
}

void RegularExpressionTests::shouldContainsString()
{
    QFETCH(QString, input);
    QFETCH(bool, contains);
    QFETCH(QString, regexp);
    QCOMPARE(contains, input.contains(QRegExp(regexp)));
    QCOMPARE(contains, input.contains(QRegularExpression(regexp)));
}

void RegularExpressionTests::shouldCaptureValue_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("match");
    QTest::addColumn<QString>("regexp");
    QTest::addColumn<QStringList>("matchedElements");
    QTest::addColumn<bool>("insensitiveCase");
    QTest::newRow("cyrus") << QStringLiteral("Cyrus timsieved v2.2.12-ww")
                           << true
                           << QStringLiteral("Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)")
                           << (QStringList() << QStringLiteral("2") << QStringLiteral("2") << QStringLiteral("12") << QStringLiteral("-ww"))
                           << true;

    QTest::newRow("no match") << QStringLiteral("ww timsieved v2.2.12-ww")
                              << false
                              << QStringLiteral("Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)")
                              << (QStringList())
                              << true;

    QTest::newRow("without name") << QStringLiteral("Cyrus timsieved v2.2.12")
                                  << true
                                  << QStringLiteral("Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)")
                                  << (QStringList() << QStringLiteral("2") << QStringLiteral("2") << QStringLiteral("12") << QString())
                                  << true;

    QTest::newRow("insensitive case") << QStringLiteral("CYRUS timsieveD v2.2.12")
                                      << true
                                      << QStringLiteral("Cyrus\\stimsieved\\sv(\\d+)\\.(\\d+)\\.(\\d+)([-\\w]*)")
                                      << (QStringList() << QStringLiteral("2") << QStringLiteral("2") << QStringLiteral("12") << QString())
                                      << true;

}

void RegularExpressionTests::shouldCaptureValue()
{
    QFETCH(QString, input);
    QFETCH(bool, match);
    QFETCH(QString, regexp);
    QFETCH(QStringList, matchedElements);
    QFETCH(bool, insensitiveCase);

    QRegExp regExp(regexp, insensitiveCase ? Qt::CaseInsensitive : Qt::CaseSensitive);
    bool hasMatch = (regExp.indexIn(input) >= 0);
    QCOMPARE(hasMatch, match);
    if (match) {
        const int major = regExp.cap(1).toInt();
        const int minor = regExp.cap(2).toInt();
        const int patch = regExp.cap(3).toInt();
        const QString vendor = regExp.cap(4);
        QCOMPARE(major, matchedElements.at(0).toInt());
        QCOMPARE(minor, matchedElements.at(1).toInt());
        QCOMPARE(patch, matchedElements.at(2).toInt());
        QCOMPARE(vendor, matchedElements.at(3));
    }

    QRegularExpression expression(regexp, insensitiveCase ? QRegularExpression::CaseInsensitiveOption : QRegularExpression::NoPatternOption);
    QRegularExpressionMatch matchExpression = expression.match(input);
    hasMatch = matchExpression.hasMatch();
    QCOMPARE(hasMatch, match);
    if (match) {
        const int major = matchExpression.captured(1).toInt();
        const int minor = matchExpression.captured(2).toInt();
        const int patch = matchExpression.captured(3).toInt();
        const QString vendor = matchExpression.captured(4);
        QCOMPARE(major, matchedElements.at(0).toInt());
        QCOMPARE(minor, matchedElements.at(1).toInt());
        QCOMPARE(patch, matchedElements.at(2).toInt());
        QCOMPARE(vendor, matchedElements.at(3));
    }
}

void RegularExpressionTests::shouldVerifyQStringListFilterWithPmailSettingsConversion()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(QString, regexp);

    QStringList newList = input.filter(QRegExp(regexp));
    QCOMPARE(newList, expected);
    newList = input.filter(QRegularExpression(regexp));
    QCOMPARE(newList, expected);
}

void RegularExpressionTests::shouldRemoveString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("regexp");
    QTest::newRow("email at") << QStringLiteral("foo@kde.org") << QStringLiteral("foo") << QStringLiteral("@.*");
    QTest::newRow("email at not valid") << QStringLiteral("foo(at)kde.org") << QStringLiteral("foo(at)kde.org") << QStringLiteral("@.*");
    QTest::newRow("email with underscore") << QStringLiteral("foo_blable") << QStringLiteral("foo") << QStringLiteral("_.*");
}

void RegularExpressionTests::shouldRemoveString()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QFETCH(QString, regexp);

    QCOMPARE(input.remove(QRegExp(regexp)), expected);
    QCOMPARE(input.remove(QRegularExpression(regexp)), expected);
}

QTEST_MAIN(RegularExpressionTests)
