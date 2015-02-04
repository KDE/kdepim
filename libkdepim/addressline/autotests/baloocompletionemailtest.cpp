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

QTEST_KDEMAIN(BalooCompletionEmailTest, NoGUI)
