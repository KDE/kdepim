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
#include "../baloocompletionemail.h"
#include <qtest_kde.h>
#include <QDebug>

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
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo2");
    emailList << QLatin1String("foo3");
    emailList << QLatin1String("foo4");
    emailList << QLatin1String("foo5");
    emailList << QLatin1String("foo6");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldReturnSameListIfBlackListDoesntInterceptEmail()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo2");
    emailList << QLatin1String("foo3");
    emailList << QLatin1String("foo4");
    emailList << QLatin1String("foo5");
    emailList << QLatin1String("foo6");
    completion.setEmailList(emailList);

    QStringList blackList;
    blackList << QLatin1String("bla");
    blackList << QLatin1String("bla2");
    blackList << QLatin1String("bla3");
    blackList << QLatin1String("bla4");
    completion.setBlackList(blackList);
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldReturnUniqueEmail()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo1");
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo1");
    emailList << QLatin1String("foo2");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), (QStringList() << QLatin1String("foo") << QLatin1String("foo1") << QLatin1String("foo2") ) );
}

void BalooCompletionEmailTest::shouldReturnEmptyListWhenAllBlackListed()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo2");
    emailList << QLatin1String("foo3");
    emailList << QLatin1String("foo4");
    emailList << QLatin1String("foo5");
    emailList << QLatin1String("foo6");
    completion.setEmailList(emailList);
    completion.setBlackList(emailList);
    QVERIFY(completion.cleanupEmailList().isEmpty());
}

void BalooCompletionEmailTest::shouldExcludeDomain()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo@kde.org");
    emailList << QLatin1String("foo2@kde.org");
    emailList << QLatin1String("foo3@kde.org");
    emailList << QLatin1String("foo4@kde.org");
    emailList << QLatin1String("foo5@kde.org");
    emailList << QLatin1String("foo6@kde.org");
    completion.setEmailList(emailList);
    completion.setExcludeDomain(QStringList() << QLatin1String("kde.org"));
    QVERIFY(completion.cleanupEmailList().isEmpty());

    const QString newAddress = QLatin1String("foo6@linux.org");
    emailList << newAddress;
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList(), (QStringList() << newAddress) );

    completion.setExcludeDomain(QStringList() << QLatin1String("kde.org") << QLatin1String("linux.org") );
    QVERIFY(completion.cleanupEmailList().isEmpty());
}

void BalooCompletionEmailTest::shouldReturnEmailListWhenDomainListIsNotNull()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo@kde.org");
    emailList << QLatin1String("foo2@kde.org");
    emailList << QLatin1String("foo3@kde.org");
    emailList << QLatin1String("foo4@kde.org");
    emailList << QLatin1String("foo5@kde.org");
    emailList << QLatin1String("foo6@kde.org");
    emailList.sort();
    completion.setEmailList(emailList);
    completion.setExcludeDomain(QStringList() << QLatin1String(""));
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldDontDuplicateEmailWhenUseCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("foo");
    emailList << QLatin1String("foo2");
    emailList << QLatin1String("foo3");
    emailList << QLatin1String("foo4");
    emailList << QLatin1String("foo5");
    emailList << QLatin1String("foo6");

    QStringList caseEmailList;
    caseEmailList << QLatin1String("Foo");
    caseEmailList << QLatin1String("fOo2");
    caseEmailList << QLatin1String("FOo3");
    completion.setEmailList((QStringList() << emailList<<caseEmailList));
    QCOMPARE(completion.cleanupEmailList(), emailList);
}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDisplayName()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("John Doe <doe@example.com>");
    emailList << QLatin1String("\"John Doe\" <doe@example.com>");
    emailList << QLatin1String("\"\'John Doe\'\" <doe@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);
}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDisplayNameAddAddressWithDifferentCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("John Doe <doe@example.com>");
    emailList << QLatin1String("\"John Doe\" <doe@example.com>");
    emailList << QLatin1String("\"\'John Doe\'\" <doe@example.com>");
    emailList << QLatin1String("John Doe <Doe@example.com>");
    emailList << QLatin1String("John Doe <DOE@example.com>");
    emailList << QLatin1String("John Doe <dOE@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);

}

void BalooCompletionEmailTest::shouldExcludeDuplicateEntryWithDifferentDisplayNameAddAddressWithDifferentCase()
{
    KPIM::BalooCompletionEmail completion;
    QStringList emailList;
    emailList << QLatin1String("John Doe <doe@example.com>");
    emailList << QLatin1String("\"John Doe\" <doe@example.com>");
    emailList << QLatin1String("\"\'John Doe\'\" <doe@example.com>");
    emailList << QLatin1String("John Doe <doe@example.com>");
    emailList << QLatin1String("Doe John <Doe@example.com>");
    emailList << QLatin1String("John <DOE@example.com>");
    emailList << QLatin1String("Doe <dOE@example.com>");
    completion.setEmailList(emailList);
    QCOMPARE(completion.cleanupEmailList().count(), 1);
}

QTEST_KDEMAIN(BalooCompletionEmailTest, NoGUI)
