/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicenavigationbuttontest.h"
#include "../storageservicenavigationbuttons.h"

#include <QAction>
#include <qtest_kde.h>

StorageServiceNavigationButtonTest::StorageServiceNavigationButtonTest()
{
}

void StorageServiceNavigationButtonTest::shouldHaveDefaultValuesOnCreation()
{
    StorageServiceNavigationButtons buttons;
    QVERIFY(buttons.goBack());
    QVERIFY(buttons.goForward());

    QCOMPARE(buttons.goBack()->isEnabled(), false);
    QCOMPARE(buttons.goForward()->isEnabled(), false);

    QCOMPARE(buttons.backUrls().isEmpty(), true);
    QCOMPARE(buttons.forwardUrls().isEmpty(), true);
}

void StorageServiceNavigationButtonTest::shouldEnabledBackButtonWhenWeListUrlIsNotEmpty()
{
    StorageServiceNavigationButtons buttons;
    QList<InformationUrl> lst;
    InformationUrl url;
    lst.append(url);
    buttons.setBackUrls(lst);
    QCOMPARE(buttons.backUrls().isEmpty(), false);
    QCOMPARE(buttons.goBack()->isEnabled(), true);

    buttons.setForwardUrls(lst);
    QCOMPARE(buttons.forwardUrls().isEmpty(), false);
    QCOMPARE(buttons.goForward()->isEnabled(), true);
}

void StorageServiceNavigationButtonTest::shouldDisableButtonWhenClearList()
{
    StorageServiceNavigationButtons buttons;
    QList<InformationUrl> lst;
    InformationUrl url;
    lst.append(url);
    buttons.setBackUrls(lst);
    QCOMPARE(buttons.goBack()->isEnabled(), true);

    buttons.setForwardUrls(lst);
    QCOMPARE(buttons.goForward()->isEnabled(), true);
    buttons.clear();
    QCOMPARE(buttons.goBack()->isEnabled(), false);
    QCOMPARE(buttons.goForward()->isEnabled(), false);

}



QTEST_KDEMAIN( StorageServiceNavigationButtonTest, GUI )
