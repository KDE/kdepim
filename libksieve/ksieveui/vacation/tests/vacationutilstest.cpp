/*
  Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

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
#include "vacationutilstest.h"
#include "vacation/vacationutils.h"

#include <kmime/kmime_header_parsing.h>

#include <QFile>
#include <qtest_kde.h>
#include <KDebug>

using namespace KSieveUi;

QTEST_KDEMAIN( VacationUtilsTest, NoGUI )

void testAliases(KMime::Types::AddrSpecList l1, KMime::Types::AddrSpecList l2)
{
    QCOMPARE(l1.count(),l2.count());
    for (int i=0; i < l1.count(); i++) {
        QCOMPARE(l1.at(i).asString(),l2.at(i).asString());
    }
}

void testAliases(KMime::Types::AddrSpecList l1, QStringList l2)
{
    QCOMPARE(l1.count(),l2.count());
    for (int i=0;i < l1.count(); i++) {
        QCOMPARE(l1.at(i).asString(),l2.at(i));
    }
}


void VacationUtilsTest::testParseEmptyScript()
{
    const QString script;
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
}

void VacationUtilsTest::testParseOnlyComment()
{
    QString script(QLatin1String("#comment"));
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
    script = QLatin1String("#comment\n\n#comment\n");
    QCOMPARE(VacationUtils::parseScript(script).isValid(), false);
}

void VacationUtilsTest::testParseActivate_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("found");
    QTest::addColumn<bool>("active");

     QTest::newRow("notfound")     << QString::fromLatin1("vacation-notfound.siv") << false << false;
     QTest::newRow("simple")     << QString::fromLatin1("vacation-simple.siv") << true << true;
     QTest::newRow("multile if")     << QString::fromLatin1("vacation-multiple.siv") << true << true;
     QTest::newRow("deactivate")     << QString::fromLatin1("vacation-deactivate.siv") << true << false;
     QTest::newRow("deactivate-multiple if")     << QString::fromLatin1("vacation-deactivate-multiple.siv") << true << false;
     QTest::newRow("deactivate-complex")     << QString::fromLatin1("vacation-deactivate-complex.siv") << true << false;
}


void VacationUtilsTest::testParseActivate()
{
    QFETCH(QString, filename);
    QFETCH(bool, found);
    QFETCH(bool, active);

    QFile file(QLatin1String(VACATIONTESTDATADIR)+filename);
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

    QTest::newRow("simple")     << QString::fromLatin1("vacation-simple.siv") << QString::fromLatin1("vacation-deactivate.siv");
    QTest::newRow("complex")     << QString::fromLatin1("vacation-complex.siv") << QString::fromLatin1("vacation-deactivate-complex.siv");
}


void VacationUtilsTest::testParseScript()
{
    QFETCH(QString, activate);
    QFETCH(QString, deactivate);
    QFile fileA(QLatin1String(VACATIONTESTDATADIR) + activate);
    QVERIFY(fileA.open(QIODevice::ReadOnly));
    QString scriptA = QString::fromUtf8(fileA.readAll());
    QFile fileD(QLatin1String(VACATIONTESTDATADIR) + deactivate);
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

    QTest::newRow("keep")    << QString::fromLatin1("vacation-complex.siv")        << (int)VacationUtils::Keep << QString();
    QTest::newRow("discard") << QString::fromLatin1("vacation-active-discard.siv") << (int)VacationUtils::Discard << QString();
    QTest::newRow("send")    << QString::fromLatin1("vacation-deactive-send.siv")  << (int)VacationUtils::Sendto << QString::fromLatin1("redirect@example.org");
    QTest::newRow("copy")    << QString::fromLatin1("vacation-deactive-copy.siv")  << (int)VacationUtils::CopyTo << QString::fromLatin1("copy@example.org");
}

void VacationUtilsTest::testMailAction()
{
    QFETCH(QString, filename);
    QFETCH(int, action);
    QFETCH(QString, recipient);

    QFile file(QLatin1String(VACATIONTESTDATADIR) + filename);
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
    QFile file(QLatin1String(VACATIONTESTDATADIR "vacation-complex.siv"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE(vacation.active, true);
    QCOMPARE(vacation.messageText, QLatin1String("dsfgsdfgsdfg"));
    QCOMPARE(vacation.subject, QLatin1String("XXX"));
    QCOMPARE(vacation.notificationInterval, 7);
    testAliases(vacation.aliases, QStringList() << QLatin1String("test@test.de"));
    QCOMPARE(vacation.sendForSpam, false);
    QCOMPARE(vacation.excludeDomain, QString());
    QCOMPARE(vacation.startDate, QDate(2015, 01, 02));
    QCOMPARE(vacation.endDate, QDate(2015, 03, 04));
    QCOMPARE(vacation.startTime, QTime());
    QCOMPARE(vacation.endTime, QTime());
}

void VacationUtilsTest::testParseScriptComplexTime()
{
    QFile file(QLatin1String(VACATIONTESTDATADIR "vacation-complex-time.siv"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    VacationUtils::Vacation vacation = VacationUtils::parseScript(script);
    QCOMPARE(vacation.active, true);
    QCOMPARE(vacation.messageText, QLatin1String("dsfgsdfgsdfg"));
    QCOMPARE(vacation.subject, QLatin1String("XXX"));
    QCOMPARE(vacation.notificationInterval, 7);
    testAliases(vacation.aliases, QStringList() << QLatin1String("test@test.de"));
    QCOMPARE(vacation.sendForSpam, false);
    QCOMPARE(vacation.excludeDomain, QString());
    QCOMPARE(vacation.startDate, QDate(2015, 01, 02));
    QCOMPARE(vacation.endDate, QDate(2015, 03, 04));
    QCOMPARE(vacation.startTime, QTime(2,0));
    QCOMPARE(vacation.endTime, QTime());

    QString composedScript = VacationUtils::composeScript(vacation);
    vacation = VacationUtils::parseScript(composedScript);
    QCOMPARE(vacation.startTime, QTime(2,0));
    QCOMPARE(vacation.endTime, QTime());
}

void VacationUtilsTest::testWriteScript()
{
    VacationUtils::Vacation vacation, vacationA;
    QStringList aliases = QStringList() << QLatin1String("test@test.de");
    vacation.valid = true;

    vacation.messageText = QLatin1String("dsfgsdfgsdfg");
    vacation.subject = QLatin1String("XXX");
    vacation.notificationInterval = 7;
    vacation.sendForSpam = false;
    vacation.excludeDomain = QLatin1String("example.org");
    vacation.startDate = QDate(2015, 01, 02);
    vacation.endDate = QDate(2015, 03, 04);
    vacation.active = true;

    foreach(const QString &alias, aliases) {
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
    vacation.messageText = QLatin1String("dsfgsdfgsdfg");
    vacation.subject = QLatin1String("XXX");
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
    QFile fileA(QLatin1String(VACATIONTESTDATADIR "vacation-simple.siv"));
    QVERIFY(fileA.open(QIODevice::ReadOnly));
    QString scriptA = QString::fromUtf8(fileA.readAll());

    QFile fileB(QLatin1String(VACATIONTESTDATADIR "vacation-deactivate.siv"));
    QVERIFY(fileB.open(QIODevice::ReadOnly));
    QString scriptB = QString::fromUtf8(fileB.readAll());

    const QString attend = QLatin1String("if true\n{\ntestcmd;\n}\n");
    const QString require = QLatin1String("require [\"date\", \"test\"];");
    const QString scriptAattend = scriptA + QLatin1String("\n") + attend;
    const QString scriptBattend = scriptB + QLatin1String("\n") + attend;

    QStringList linesA = scriptA.split(QLatin1Char('\n'));
    QStringList header;
    for(int i=0; i<5;i++ ){
        header.append(linesA.at(i));
    }

    QStringList vacation;
    for(int i=5; i<linesA.count(); i++ ){
        vacation.append(linesA.at(i));
    }

    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, QString()), scriptA);
    QCOMPARE(VacationUtils::updateVacationBlock(QString(), scriptB), scriptB);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, scriptB), scriptB);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptB, scriptA), scriptA);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptAattend, scriptB), scriptBattend);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptBattend, scriptA), scriptAattend);
    QCOMPARE(VacationUtils::updateVacationBlock(scriptA, attend), header.join(QLatin1String("\n")));
    QStringList output = vacation;
    output << attend;
    QCOMPARE(VacationUtils::updateVacationBlock(attend, scriptA), output.join(QLatin1String("\n")));
    output.insert(0,require);
    QCOMPARE(VacationUtils::updateVacationBlock(require+ QLatin1String("\n") + attend, scriptA), output.join(QLatin1String("\n")));
}

void VacationUtilsTest::testMergeRequireLine()
{
    QString sEmpty=QLatin1String("require;");
    QString sOne=QLatin1String("require \"test\";");
    QString sList1=QLatin1String("require [\"test\"];");
    QString sList2=QLatin1String("require [\"test\", \"test2\"];");
    QString sList3=QLatin1String("require [\"test3\",\n \"test4\"];\ntestcmd;");

    QCOMPARE(VacationUtils::mergeRequireLine(sEmpty, sOne), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sEmpty), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList1), sOne);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList2), sList2);
    QCOMPARE(VacationUtils::mergeRequireLine(sOne, sList3), QLatin1String("require [\"test\", \"test3\", \"test4\"];") );
    QCOMPARE(VacationUtils::mergeRequireLine(sList3, sOne), QLatin1String("require [\"test\", \"test3\", \"test4\"];\ntestcmd;") );
}
