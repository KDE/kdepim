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

#include <qtest_kde.h>
#include <qtestmouse.h>
#include <qtestkeyboard.h>

Q_DECLARE_METATYPE(InformationUrl)
StorageServiceNavigationButtonTest::StorageServiceNavigationButtonTest()
{
    qRegisterMetaType<InformationUrl>();
}

void StorageServiceNavigationButtonTest::shouldHaveDefaultValuesOnCreation()
{
    StorageServiceNavigationButtons buttons;
    QVERIFY(buttons.goBack());
    QVERIFY(buttons.goForward());
    QVERIFY(buttons.home());

    QCOMPARE(buttons.goBack()->isEnabled(), false);
    QCOMPARE(buttons.goForward()->isEnabled(), false);
    QCOMPARE(buttons.home()->isEnabled(), true);

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

void StorageServiceNavigationButtonTest::shouldEmitSignalWhenClickOnButtonAndListNotEmpty()
{
    StorageServiceNavigationButtons buttons;
    QList<InformationUrl> lst;
    InformationUrl url;
    lst.append(url);
    buttons.setBackUrls(lst);
    QSignalSpy spy(&buttons, SIGNAL(changeUrl(InformationUrl)));
    buttons.goBack()->trigger();
    QCOMPARE(spy.count(), 1);

    //clear goBack
    buttons.clear();
    QSignalSpy spy2(&buttons, SIGNAL(changeUrl(InformationUrl)));
    buttons.goForward()->trigger();
    QCOMPARE(spy2.count(), 0);
    QSignalSpy spy3(&buttons, SIGNAL(changeUrl(InformationUrl)));
    buttons.goBack()->trigger();
    QCOMPARE(spy3.count(), 0);

    buttons.setForwardUrls(lst);
    QSignalSpy spy4(&buttons, SIGNAL(changeUrl(InformationUrl)));
    buttons.goForward()->trigger();
    QCOMPARE(spy4.count(), 1);
}

void StorageServiceNavigationButtonTest::shouldEmitSignalWhenClickOnHome()
{
    StorageServiceNavigationButtons buttons;
    QSignalSpy spy(&buttons, SIGNAL(goHome()));
    buttons.home()->trigger();
    QCOMPARE(spy.count(), 1);
}

void StorageServiceNavigationButtonTest::shouldEnabledBackButtonWhenAndInfoAndItValids()
{
    StorageServiceNavigationButtons buttons;
    QCOMPARE(buttons.backUrls().isEmpty(), true);
    QCOMPARE(buttons.forwardUrls().isEmpty(), true);

    InformationUrl urlInvalid;
    buttons.addBackUrl(urlInvalid);
    buttons.addForwadUrl(urlInvalid);
    QCOMPARE(buttons.backUrls().isEmpty(), true);
    QCOMPARE(buttons.forwardUrls().isEmpty(), true);

    InformationUrl urlValid;
    urlValid.currentUrl = QLatin1String("Foo");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addBackUrl(urlValid);
    buttons.addForwadUrl(urlValid);
    QCOMPARE(buttons.backUrls().isEmpty(), false);
    QCOMPARE(buttons.forwardUrls().isEmpty(), false);
}

void StorageServiceNavigationButtonTest::shouldEnabledBackButtonWhenAddNewInfo()
{
    StorageServiceNavigationButtons buttons;
    QCOMPARE(buttons.backUrls().isEmpty(), true);
    InformationUrl urlValid;
    urlValid.currentUrl = QLatin1String("Foo");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);
    QCOMPARE(buttons.backUrls().isEmpty(), false);
}

void StorageServiceNavigationButtonTest::shouldIncreaseNumberOfElement()
{
    StorageServiceNavigationButtons buttons;
    InformationUrl urlValid;
    urlValid.currentUrl = QLatin1String("Foo");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);
    buttons.addNewUrl(urlValid);
    buttons.addNewUrl(urlValid);
    buttons.addNewUrl(urlValid);
    buttons.addNewUrl(urlValid);
    buttons.addNewUrl(urlValid);
    QCOMPARE(buttons.backUrls().count(), 6);
}

void StorageServiceNavigationButtonTest::shouldMoveInfoInGoForwardWhenClickOnGoBack()
{
    StorageServiceNavigationButtons buttons;
    InformationUrl urlValid;
    urlValid.currentUrl = QLatin1String("Foo");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);
    QCOMPARE(buttons.backUrls().count(), 1);
    buttons.goBack()->trigger();
    QCOMPARE(buttons.backUrls().count(), 0);
    QCOMPARE(buttons.forwardUrls().count(), 1);

    //Compare that button goback is disable, goforward is enable after that
    QCOMPARE(buttons.goBack()->isEnabled(), false);
    QCOMPARE(buttons.goForward()->isEnabled(), true);
}

void StorageServiceNavigationButtonTest::shouldMoveInfoInGoBackWhenClickOnGoForward()
{
    StorageServiceNavigationButtons buttons;
    InformationUrl urlValid;
    urlValid.currentUrl = QLatin1String("Foo");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);
    buttons.goBack()->trigger();

    buttons.goForward()->trigger();

    QCOMPARE(buttons.backUrls().count(), 1);
    QCOMPARE(buttons.forwardUrls().count(), 0);

    //Compare that button goback is disable, goforward is enable after that
    QCOMPARE(buttons.goBack()->isEnabled(), true);
    QCOMPARE(buttons.goForward()->isEnabled(), false);
}

void StorageServiceNavigationButtonTest::shouldMoveInfoToTopWhenClickOnBack()
{
    StorageServiceNavigationButtons buttons;
    InformationUrl urlValid;

    urlValid.currentUrl = QLatin1String("Foo1");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);

    urlValid.currentUrl = QLatin1String("Foo2");
    urlValid.parentUrl = QLatin1String("Foo2");
    buttons.addNewUrl(urlValid);

    urlValid.currentUrl = QLatin1String("Foo3");
    urlValid.parentUrl = QLatin1String("Foo3");
    buttons.addNewUrl(urlValid);

    buttons.goBack()->trigger();
    InformationUrl forwardUrl = buttons.forwardUrls().first();
    QCOMPARE(forwardUrl.currentUrl, QLatin1String("Foo3"));

    buttons.goBack()->trigger();
    forwardUrl = buttons.forwardUrls().first();
    QCOMPARE(forwardUrl.currentUrl, QLatin1String("Foo2"));

    buttons.goBack()->trigger();
    forwardUrl = buttons.forwardUrls().first();
    QCOMPARE(forwardUrl.currentUrl, QLatin1String("Foo1"));

}

void StorageServiceNavigationButtonTest::shouldMoveInfoToTopWhenClickOnBackAndAfterForward()
{
    StorageServiceNavigationButtons buttons;
    InformationUrl urlValid;

    urlValid.currentUrl = QLatin1String("Foo1");
    urlValid.parentUrl = QLatin1String("Foo1");
    buttons.addNewUrl(urlValid);

    urlValid.currentUrl = QLatin1String("Foo2");
    urlValid.parentUrl = QLatin1String("Foo2");
    buttons.addNewUrl(urlValid);

    urlValid.currentUrl = QLatin1String("Foo3");
    urlValid.parentUrl = QLatin1String("Foo3");
    buttons.addNewUrl(urlValid);

    buttons.goBack()->trigger();
    InformationUrl forwardUrl = buttons.forwardUrls().first();
    QCOMPARE(forwardUrl.currentUrl, QLatin1String("Foo3"));

    buttons.goForward()->trigger();
    forwardUrl = buttons.backUrls().first();
    QCOMPARE(forwardUrl.currentUrl, QLatin1String("Foo3"));
}



QTEST_KDEMAIN( StorageServiceNavigationButtonTest, GUI )
