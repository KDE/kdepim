/*
 * Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "vacationutilstest.h"
#include "ksieveui/vacation/vacationutils.h"
#include "ksieveui/vacation/legacy/vacationutils.h"

#include <kmime/kmime_header_parsing.h>

#include <QFile>
#include <QTest>

using namespace KSieveUi;

QTEST_MAIN(VacationUtilsTest)

void testAliases(const KMime::Types::AddrSpecList &l1, const KMime::Types::AddrSpecList &l2)
{
    const int l1count = l1.count();
    QCOMPARE(l1count, l2.count());
    for (int i = 0; i < l1count; ++i) {
        QCOMPARE(l1.at(i).asString(), l2.at(i).asString());
    }
}

void testAliases(const KMime::Types::AddrSpecList &l1, const QStringList &l2)
{
    const int l1count = l1.count();
    QCOMPARE(l1count, l2.count());
    for (int i = 0; i < l1count; ++i) {
        QCOMPARE(l1.at(i).asString(), l2.at(i));
    }
}

void VacationUtilsTest::testParseEmptyScript()
{
    const QString script;
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
}

void VacationUtilsTest::testParseOnlyComment()
{
    QString script(QStringLiteral("#comment"));
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
    script = QStringLiteral("#comment\n\n#comment\n");
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
}

void VacationUtilsTest::testParseActivate_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("found");
    QTest::addColumn<bool>("active");

    QTest::newRow("notfound")     << QStringLiteral("vacation-notfound.siv") << false << false;
    QTest::newRow("simple")     << QStringLiteral("vacation-simple.siv") << true << true;
    QTest::newRow("multile if")     << QStringLiteral("vacation-multiple.siv") << true << true;
    QTest::newRow("deactivate")     << QStringLiteral("vacation-deactivate.siv") << true << false;
    QTest::newRow("deactivate-multiple if")     << QStringLiteral("vacation-deactivate-multiple.siv") << true << false;
    QTest::newRow("deactivate-complex")     << QStringLiteral("vacation-deactivate-complex.siv") << true << false;
    QTest::newRow("old")     << QStringLiteral("vacation-old.siv") << true << true;
}

void VacationUtilsTest::testParseActivate()
{
    QFETCH(QString, filename);
    QFETCH(bool, found);
    QFETCH(bool, active);

    QFile file(QStringLiteral(VACATIONTESTDATADIR) + filename);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE(vacation.isValid(), found);
    QCOMPARE(vacation.active, active);
}

void VacationUtilsTest::testParseScript_data()
{
    QTest::addColumn<QString>("activate");
    QTest::addColumn<QString>("deactivate");

    QTest::newRow("simple")     << QStringLiteral("vacation-simple.siv") << QStringLiteral("vacation-deactivate.siv");
    QTest::newRow("complex")     << QStringLiteral("vacation-complex.siv") << QStringLiteral("vacation-deactivate-complex.siv");
    QTest::newRow("old")     << QStringLiteral("vacation-old.siv") << QStringLiteral("vacation-deactivate-complex-old.siv");
}

void VacationUtilsTest::testParseScript()
{
    QFETCH(QString, activate);
    QFETCH(QString, deactivate);
    QFile fileA(QStringLiteral(VACATIONTESTDATADIR) + activate);
    QVERIFY(fileA.open(QIODevice::ReadOnly));
    QString scriptA = QString::fromUtf8(fileA.readAll());
    QFile fileD(QStringLiteral(VACATIONTESTDATADIR) + deactivate);
    QVERIFY(fileD.open(QIODevice::ReadOnly));
    QString scriptD = QString::fromUtf8(fileD.readAll());

    VacationUtils::Vacation vacationA = VacationUtils::parseScript(scriptA);
    VacationUtils::Vacation vacationD = VacationUtils::parseScript(scriptD);
    QCOMPARE(vacationA.active, true);
    QCOMPARE(vacationD.active, false);
    QCOMPARE(vacationD.messageText, vacationA.messageText);
    QCOMPARE(vacationD.subject, vacationA.subject);
    QCOMPARE(vacationD.notificationInterval, vacationA.notificationInterval);
    testAliases(vacationD.aliases, vacationA.aliases);
    QCOMPARE(vacationD.sendForSpam, vacationA.sendForSpam);
    QCOMPARE(vacationD.excludeDomain, vacationA.excludeDomain);
    QCOMPARE(vacationD.startDate, vacationA.startDate);
    QCOMPARE(vacationD.endDate, vacationA.endDate);
    QCOMPARE(vacationD.startTime, QTime());
    QCOMPARE(vacationD.endTime, QTime());
}

void VacationUtilsTest::testMailAction_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("action");
    QTest::addColumn<QString>("recipient");

    QTest::newRow("keep")    << QStringLiteral("vacation-complex.siv")        << (int)VacationUtils::Keep << QString();
    QTest::newRow("discard") << QStringLiteral("vacation-active-discard.siv") << (int)VacationUtils::Discard << QString();
    QTest::newRow("send")    << QStringLiteral("vacation-deactive-send.siv")  << (int)VacationUtils::Sendto << QStringLiteral("redirect@example.org");
    QTest::newRow("copy")    << QStringLiteral("vacation-deactive-copy.siv")  << (int)VacationUtils::CopyTo << QStringLiteral("copy@example.org");
}

void VacationUtilsTest::testMailAction()
{
    QFETCH(QString, filename);
    QFETCH(int, action);
    QFETCH(QString, recipient);

    QFile file(QStringLiteral(VACATIONTESTDATADIR) + filename);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE((int)vacation.mailAction, action);
    QCOMPARE(vacation.mailActionRecipient, recipient);

    const QString composedScript = VacationUtils::composeScript(vacation);
    vacation = VacationUtils::parseScript(composedScript);
    QCOMPARE((int)vacation.mailAction, action);
    QCOMPARE(vacation.mailActionRecipient, recipient);
}

void VacationUtilsTest::testParseScriptComplex()
{
    QFile file(QStringLiteral(VACATIONTESTDATADIR "vacation-complex.siv"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE(vacation.active, true);
    QCOMPARE(vacation.messageText, QStringLiteral("dsfgsdfgsdfg"));
    QCOMPARE(vacation.subject, QStringLiteral("XXX"));
    QCOMPARE(vacation.notificationInterval, 7);
    testAliases(vacation.aliases, QStringList() << QStringLiteral("test@test.de"));
    QCOMPARE(vacation.sendForSpam, false);
    QCOMPARE(vacation.excludeDomain, QString());
    QCOMPARE(vacation.startDate, QDate(2015, 01, 02));
    QCOMPARE(vacation.endDate, QDate(2015, 03, 04));
    QCOMPARE(vacation.startTime, QTime());
    QCOMPARE(vacation.endTime, QTime());
}

void VacationUtilsTest::testParseScriptComplexTime()
{
    QFile file(QStringLiteral(VACATIONTESTDATADIR "vacation-complex-time.siv"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE(vacation.active, true);
    QCOMPARE(vacation.messageText, QStringLiteral("dsfgsdfgsdfg"));
    QCOMPARE(vacation.subject, QStringLiteral("XXX"));
    QCOMPARE(vacation.notificationInterval, 7);
    testAliases(vacation.aliases, QStringList() << QStringLiteral("test@test.de"));
    QCOMPARE(vacation.sendForSpam, false);
    QCOMPARE(vacation.excludeDomain, QString());
    QCOMPARE(vacation.startDate, QDate(2015, 01, 02));
    QCOMPARE(vacation.endDate, QDate(2015, 03, 04));
    QCOMPARE(vacation.startTime, QTime(2, 0));
    QCOMPARE(vacation.endTime, QTime());

    QString composedScript = VacationUtils::composeScript(vacation);
    vacation = VacationUtils::parseScript(composedScript);
    QCOMPARE(vacation.startTime, QTime(2, 0));
    QCOMPARE(vacation.endTime, QTime());
}

void VacationUtilsTest::testWriteScript()
{
    VacationUtils::Vacation vacation, vacationA;
    QStringList aliases = QStringList() << QStringLiteral("test@test.de");
    vacation.valid = true;

    vacation.messageText = QStringLiteral("dsfgsdfgsdfg");
    vacation.subject = QStringLiteral("XXX");
    vacation.notificationInterval = 7;
    vacation.sendForSpam = false;
    vacation.excludeDomain = QStringLiteral("example.org");
    vacation.startDate = QDate(2015, 01, 02);
    vacation.endDate = QDate(2015, 03, 04);
    vacation.active = true;

    foreach (const QString &alias, aliases) {
        KMime::Types::Mailbox a;
        a.fromUnicodeString(alias);
        vacation.aliases.append(a.addrSpec());
    }

    QString script = VacationUtils::composeScript(vacation);
    vacationA = VacationUtils::parseScript(script);
    QCOMPARE(vacationA.isValid(), true);
    QCOMPARE(vacationA.active, vacation.active);
    QCOMPARE(vacationA.messageText, vacation.messageText);
    QCOMPARE(vacationA.subject, vacation.subject);
    QCOMPARE(vacationA.notificationInterval, vacation.notificationInterval);
    testAliases(vacationA.aliases, vacation.aliases);
    QCOMPARE(vacationA.sendForSpam, vacation.sendForSpam);
    QCOMPARE(vacationA.excludeDomain, vacation.excludeDomain);
    QCOMPARE(vacationA.startDate, vacation.startDate);
    QCOMPARE(vacationA.endDate, vacation.endDate);
    QCOMPARE(vacationA.startTime, QTime());
    QCOMPARE(vacationA.endTime, QTime());

    vacation.active = false;
    script = VacationUtils::composeScript(vacation);
    vacationA = VacationUtils::parseScript(script);
    QCOMPARE(vacationA.isValid(), true);
    QCOMPARE(vacationA.active, vacation.active);
    QCOMPARE(vacationA.messageText, vacation.messageText);
    QCOMPARE(vacationA.subject, vacation.subject);
    QCOMPARE(vacationA.notificationInterval, vacation.notificationInterval);
    testAliases(vacationA.aliases, vacation.aliases);
    QCOMPARE(vacationA.sendForSpam, vacation.sendForSpam);
    QCOMPARE(vacationA.excludeDomain, vacation.excludeDomain);
    QCOMPARE(vacationA.startDate, vacation.startDate);
    QCOMPARE(vacationA.endDate, vacation.endDate);
    QCOMPARE(vacationA.startTime, QTime());
    QCOMPARE(vacationA.endTime, QTime());
}

void VacationUtilsTest::testWriteSimpleScript()
{
    VacationUtils::Vacation vacation;
    vacation.valid = true;
    vacation.messageText = QStringLiteral("dsfgsdfgsdfg");
    vacation.subject = QStringLiteral("XXX");
    vacation.notificationInterval = 7;
    vacation.active = true;
    vacation.sendForSpam = true;

    QString script = VacationUtils::composeScript(vacation);
    VacationUtils::Vacation vacationA = VacationUtils::parseScript(script);
    QCOMPARE(vacation.isValid(), true);
    QCOMPARE(vacationA.active, vacation.active);
    QCOMPARE(vacationA.messageText, vacation.messageText);
    QCOMPARE(vacationA.subject, vacation.subject);
    QCOMPARE(vacationA.notificationInterval, vacation.notificationInterval);

    vacation.active = false;
    script = VacationUtils::composeScript(vacation);
    vacationA = VacationUtils::parseScript(script);
    QCOMPARE(vacation.isValid(), true);
    QCOMPARE(vacationA.active, vacation.active);
    QCOMPARE(vacationA.messageText, vacation.messageText);
    QCOMPARE(vacationA.subject, vacation.subject);
    QCOMPARE(vacationA.notificationInterval, vacation.notificationInterval);

}

void VacationUtilsTest::testUpdateVacationBlock()
{
    QFile fileA(QStringLiteral(VACATIONTESTDATADIR "vacation-simple.siv"));
    QVERIFY(fileA.open(QIODevice::ReadOnly));
    QString scriptA = QString::fromUtf8(fileA.readAll());

    QFile fileB(QStringLiteral(VACATIONTESTDATADIR "vacation-deactivate.siv"));
    QVERIFY(fileB.open(QIODevice::ReadOnly));
    QString scriptB = QString::fromUtf8(fileB.readAll());

    const QString attend = QStringLiteral("if true\n{\ntestcmd;\n}\n");
    const QString require = QStringLiteral("require [\"date\", \"test\"];");
    const QString scriptAattend = scriptA + QStringLiteral("\n") + attend;
    const QString scriptBattend = scriptB + QStringLiteral("\n") + attend;

    QStringList linesA = scriptA.split(QLatin1Char('\n'));
    QStringList header;
    for (int i = 0; i < 5; ++i) {
        header.append(linesA.at(i));
    }

    QStringList vacation;
    for (int i = 5; i < linesA.count(); ++i) {
        vacation.append(linesA.at(i));
    }

    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, QString()), scriptA);
    QCOMPARE(VacationUtils::updateVacationBlock(QString(), scriptB), scriptB);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, scriptB), scriptB);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptB, scriptA), scriptA);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptAattend, scriptB), scriptBattend);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptBattend, scriptA), scriptAattend);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, attend), header.join(QStringLiteral("\n")));
    QStringList output = vacation;
    output << attend;
    QCOMPARE(VacationUtils::updateVacationBlock(attend, scriptA), output.join(QStringLiteral("\n")));
    output.insert(0, require);
    QCOMPARE(VacationUtils::updateVacationBlock(require + QStringLiteral("\n") + attend, scriptA), output.join(QStringLiteral("\n")));
}

void VacationUtilsTest::testMergeRequireLine()
{
    QString sEmpty = QStringLiteral("require;");
    QString sOne = QStringLiteral("require \"test\";");
    QString sList1 = QStringLiteral("require [\"test\"];");
    QString sList2 = QStringLiteral("require [\"test\", \"test2\"];");
    QString sList3 = QStringLiteral("require [\"test3\",\n \"test4\"];\ntestcmd;");

    QCOMPARE(VacationUtils::mergeRequireLine(sEmpty, sOne), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sEmpty), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList1), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList2), sList2);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList3), QStringLiteral("require [\"test\", \"test3\", \"test4\"];"));
    QCOMPARE(VacationUtils::mergeRequireLine(sList3, sOne), QStringLiteral("require [\"test\", \"test3\", \"test4\"];\ntestcmd;"));
}
