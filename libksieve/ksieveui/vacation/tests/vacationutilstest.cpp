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

void VacationUtilsTest::testParseEmptyScript()
{
    const QString script;
    QCOMPARE(VacationUtils::foundVacationScript(script), false);
}

void VacationUtilsTest::testParseOnlyComment()
{
    QString script(QLatin1String("#comment"));
    QCOMPARE(VacationUtils::foundVacationScript(script), false);
    script = QLatin1String("#comment\n\n#comment\n");
    QCOMPARE(VacationUtils::foundVacationScript(script), false);
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
    QCOMPARE(VacationUtils::foundVacationScript(script), found);

    QString messageText;
    QString subject;
    int notificationInterval;
    QStringList aliases;
    bool sendForSpam;
    QString domainName;
    QDate startDate;
    QDate endDate;
    bool scriptActive = !active;

    bool ret = VacationUtils::parseScript(script, scriptActive, messageText, subject, notificationInterval, aliases, sendForSpam, domainName, startDate, endDate);
    QCOMPARE(ret, found);
    QCOMPARE(scriptActive, active);
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

    QString messageTextA, messageTextD;
    QString subjectA, subjectD;
    int notificationIntervalA, notificationIntervalD;
    QStringList aliasesA, aliasesD;
    bool sendForSpamA, sendForSpamD;
    QString domainNameA, domainNameD;
    QDate startDateA, startDateD;
    QDate endDateA, endDateD;
    bool scriptActiveA, scriptActiveD;
    VacationUtils::parseScript(scriptA, scriptActiveA, messageTextA, subjectA, notificationIntervalA, aliasesA, sendForSpamA, domainNameA, startDateA, endDateA);
    VacationUtils::parseScript(scriptD, scriptActiveD, messageTextD, subjectD, notificationIntervalD, aliasesD, sendForSpamD, domainNameD, startDateD, endDateD);
    QCOMPARE(scriptActiveA, true);
    QCOMPARE(scriptActiveD, false);
    QCOMPARE(messageTextD, messageTextA);
    QCOMPARE(subjectD, subjectA);
    QCOMPARE(notificationIntervalD, notificationIntervalA);
    QCOMPARE(aliasesD, aliasesA);
    QCOMPARE(sendForSpamD, sendForSpamA);
    QCOMPARE(domainNameD, domainNameA);
    QCOMPARE(startDateD, startDateA);
    QCOMPARE(endDateD, endDateA);
}

void VacationUtilsTest::testParseScriptComplex()
{
    QFile file(QLatin1String(VACATIONTESTDATADIR "vacation-complex.siv"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString script = QString::fromUtf8(file.readAll());

    QString messageText;
    QString subject;
    int notificationInterval;
    QStringList aliases;
    bool sendForSpam;
    QString domainName;
    QDate startDate;
    QDate endDate;
    bool scriptActive;
    VacationUtils::parseScript(script, scriptActive, messageText, subject, notificationInterval, aliases, sendForSpam, domainName, startDate, endDate);
    QCOMPARE(scriptActive, true);
    QCOMPARE(messageText, QLatin1String("dsfgsdfgsdfg"));
    QCOMPARE(subject, QLatin1String("XXX"));
    QCOMPARE(notificationInterval, 7);
    QCOMPARE(aliases, QStringList() << QLatin1String("test@test.de"));
    QCOMPARE(sendForSpam, false);
    QCOMPARE(domainName, QString());
    QCOMPARE(startDate, QDate(2015, 01, 02));
    QCOMPARE(endDate, QDate(2015, 03, 04));
}

void VacationUtilsTest::testWriteScript()
{
    QString messageText(QLatin1String("dsfgsdfgsdfg"));
    QString subject(QLatin1String("XXX"));
    int notificationInterval(7);
    QStringList aliases = QStringList() << QLatin1String("test@test.de");
    QList<KMime::Types::AddrSpec> addesses;
    bool sendForSpam(false);
    QString domainName(QLatin1String("example.org"));
    QDate startDate(2015, 01, 02);
    QDate endDate(2015, 03, 04);
    bool scriptActive(true);

    QString messageTextA;
    QString subjectA;
    int notificationIntervalA;
    QStringList aliasesA;
    bool sendForSpamA;
    QString domainNameA;
    QDate startDateA;
    QDate endDateA;
    bool scriptActiveA;

    foreach(const QString &alias, aliases) {
        KMime::Types::Mailbox a;
        a.fromUnicodeString(alias);
        addesses.append(a.addrSpec());
    }

    QString script = VacationUtils::composeScript(messageText, scriptActive, subject, notificationInterval, addesses, sendForSpam, domainName, startDate, endDate);
    bool ret = VacationUtils::parseScript(script, scriptActiveA, messageTextA, subjectA, notificationIntervalA, aliasesA, sendForSpamA, domainNameA, startDateA, endDateA);
    QCOMPARE(ret, true);
    QCOMPARE(scriptActiveA, scriptActive);
    QCOMPARE(messageTextA, messageText);
    QCOMPARE(subjectA, subject);
    QCOMPARE(notificationIntervalA, notificationInterval);
    QCOMPARE(aliasesA, aliases);
    QCOMPARE(sendForSpamA, sendForSpam);
    QCOMPARE(domainNameA, domainName);
    QCOMPARE(startDateA, startDate);
    QCOMPARE(endDateA, endDate);

    scriptActive = false;
    script = VacationUtils::composeScript(messageText, scriptActive, subject, notificationInterval, addesses, sendForSpam, domainName, startDate, endDate);
    ret = VacationUtils::parseScript(script, scriptActiveA, messageTextA, subjectA, notificationIntervalA, aliasesA, sendForSpamA, domainNameA, startDateA, endDateA);
    QCOMPARE(ret, true);
    QCOMPARE(scriptActiveA, scriptActive);
    QCOMPARE(messageTextA, messageText);
    QCOMPARE(subjectA, subject);
    QCOMPARE(notificationIntervalA, notificationInterval);
    QCOMPARE(aliasesA, aliases);
    QCOMPARE(sendForSpamA, sendForSpam);
    QCOMPARE(domainNameA, domainName);
    QCOMPARE(startDateA, startDate);
    QCOMPARE(endDateA, endDate);
}


void VacationUtilsTest::testWriteSimpleScript()
{
    QString messageText(QLatin1String("dsfgsdfgsdfg"));
    QString subject(QLatin1String("XXX"));
    int notificationInterval(7);
    bool scriptActive(true);

    QString messageTextA;
    QString subjectA;
    int notificationIntervalA;
    QStringList aliasesA;
    bool sendForSpamA;
    QString domainNameA;
    QDate startDateA;
    QDate endDateA;
    bool scriptActiveA;

    QString script = VacationUtils::composeScript(messageText, scriptActive, subject, notificationInterval, QList<KMime::Types::AddrSpec>(), true, QString(), QDate(), QDate());
    bool ret = VacationUtils::parseScript(script, scriptActiveA, messageTextA, subjectA, notificationIntervalA, aliasesA, sendForSpamA, domainNameA, startDateA, endDateA);
    QCOMPARE(ret, true);
    QCOMPARE(scriptActiveA, scriptActive);
    QCOMPARE(messageTextA, messageText);
    QCOMPARE(subjectA, subject);
    QCOMPARE(notificationIntervalA, notificationInterval);

    scriptActive = false;
    script = VacationUtils::composeScript(messageText, scriptActive, subject, notificationInterval, QList<KMime::Types::AddrSpec>(), true, QString(), QDate(), QDate());
    ret = VacationUtils::parseScript(script, scriptActiveA, messageTextA, subjectA, notificationIntervalA, aliasesA, sendForSpamA, domainNameA, startDateA, endDateA);
    QCOMPARE(ret, true);
    QCOMPARE(scriptActiveA, scriptActive);
    QCOMPARE(messageTextA, messageText);
    QCOMPARE(subjectA, subject);
    QCOMPARE(notificationIntervalA, notificationInterval);
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
