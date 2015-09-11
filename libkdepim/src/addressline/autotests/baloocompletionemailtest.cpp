/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "baloocompletionemailtest.h"
#include <qtest.h>
#include "libkdepim_debug.h"
#include "../addresslineedit/baloocompletionemail.h"
BalooCompletionEmailTest::BalooCompletionEmailTest(QObject *parent)
    : QObject(parent)
{

}

BalooCompletionEmailTest::~BalooCompletionEmailTest()
{

}

void BalooCompletionEmailTest::returnEmptyListWhenEmailListIsEmpty()
{
    KPIM::BalooCompletionEmail completion;
    QVERIFY(completion.cleanupEmailList().isEmpty());
}

void BalooCompletionEmailTest::shouldReturnSameListWhenNotExclude()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo2");
    emailList << QStringLiteral("foo3");
    emailList << QStringLiteral("foo4");
    emailList << QStringLiteral("foo5");
    emailList << QStringLiteral("foo6");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldReturnSameListIfBlackListDoesntInterceptEmail()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo2");
    emailList << QStringLiteral("foo3");
    emailList << QStringLiteral("foo4");
    emailList << QStringLiteral("foo5");
    emailList << QStringLiteral("foo6");
    completion.setEmailList(emailList);

    QStringList blackList;
    blackList << QStringLiteral("bla");
    blackList << QStringLiteral("bla2");
    blackList << QStringLiteral("bla3");
    blackList << QStringLiteral("bla4");
    completion.setBlackList(blackList);
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldReturnUniqueEmail()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo1");
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo1");
    emailList << QStringLiteral("foo2");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), (QStringList() << QStringLiteral("foo") << QStringLiteral("foo1") << QStringLiteral("foo2")));
}

void BalooCompletionEmailTest::shouldReturnEmptyListWhenAllBlackListed()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo2");
    emailList << QStringLiteral("foo3");
    emailList << QStringLiteral("foo4");
    emailList << QStringLiteral("foo5");
    emailList << QStringLiteral("foo6");
    completion.setEmailList(emailList);
    completion.setBlackList(emailList);
    QVERIFY(completion.cleanupEmailList().isEmpty());
}

void BalooCompletionEmailTest::shouldExcludeDomain()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo@kde.org");
    emailList << QStringLiteral("foo2@kde.org");
    emailList << QStringLiteral("foo3@kde.org");
    emailList << QStringLiteral("foo4@kde.org");
    emailList << QStringLiteral("foo5@kde.org");
    emailList << QStringLiteral("foo6@kde.org");
    completion.setEmailList(emailList);
    completion.setExcludeDomain(QStringList() << QStringLiteral("kde.org"));
    QVERIFY(completion.cleanupEmailList().isEmpty());

    const QString newAddress = QStringLiteral("foo6@linux.org");
    emailList << newAddress;
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), (QStringList() << newAddress));

    completion.setExcludeDomain(QStringList() << QStringLiteral("kde.org") << QStringLiteral("linux.org"));
    QVERIFY(completion.cleanupEmailList().isEmpty());
}

void BalooCompletionEmailTest::shouldReturnEmailListWhenDomainListIsNotNull()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo@kde.org");
    emailList << QStringLiteral("foo2@kde.org");
    emailList << QStringLiteral("foo3@kde.org");
    emailList << QStringLiteral("foo4@kde.org");
    emailList << QStringLiteral("foo5@kde.org");
    emailList << QStringLiteral("foo6@kde.org");
    emailList.sort();
    completion.setEmailList(emailList);
    completion.setExcludeDomain(QStringList() << QStringLiteral(""));
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldDontDuplicateEmailWhenUseCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("foo");
    emailList << QStringLiteral("foo2");
    emailList << QStringLiteral("foo3");
    emailList << QStringLiteral("foo4");
    emailList << QStringLiteral("foo5");
    emailList << QStringLiteral("foo6");

    QStringList caseEmailList;
    caseEmailList << QStringLiteral("Foo");
    caseEmailList << QStringLiteral("fOo2");
    caseEmailList << QStringLiteral("FOo3");
    completion.setEmailList((QStringList() << emailList << caseEmailList));
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDisplayName()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("John Doe <doe@example.com>");
    emailList << QStringLiteral("\"John Doe\" <doe@example.com>");
    emailList << QStringLiteral("\"\'John Doe\'\" <doe@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);
}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDisplayNameAddAddressWithDifferentCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("John Doe <doe@example.com>");
    emailList << QStringLiteral("\"John Doe\" <doe@example.com>");
    emailList << QStringLiteral("\"\'John Doe\'\" <doe@example.com>");
    emailList << QStringLiteral("John Doe <Doe@example.com>");
    emailList << QStringLiteral("John Doe <DOE@example.com>");
    emailList << QStringLiteral("John Doe <dOE@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);

}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDifferentDisplayNameAddAddressWithDifferentCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QStringLiteral("John Doe <doe@example.com>");
    emailList << QStringLiteral("\"John Doe\" <doe@example.com>");
    emailList << QStringLiteral("\"\'John Doe\'\" <doe@example.com>");
    emailList << QStringLiteral("John Doe <doe@example.com>");
    emailList << QStringLiteral("Doe John <Doe@example.com>");
    emailList << QStringLiteral("John <DOE@example.com>");
    emailList << QStringLiteral("Doe <dOE@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);
}

QTEST_MAIN(BalooCompletionEmailTest)
