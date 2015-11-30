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

#include "zoomactionmenutest.h"
#include "../src/widgets/zoomactionmenu.h"
#include <QTest>
#include <viewer/mailwebview.h>
#include <KActionCollection>

ZoomActionMenuTest::ZoomActionMenuTest(QObject *parent)
    : QObject(parent)
{

}

ZoomActionMenuTest::~ZoomActionMenuTest()
{

}

void ZoomActionMenuTest::shouldHaveDefaultValue()
{
    MessageViewer::ZoomActionMenu menu(new MessageViewer::MailWebView(), this);
    menu.setActionCollection(new KActionCollection(this));
    menu.createZoomActions();

    QVERIFY(menu.zoomTextOnlyAction());
    QVERIFY(menu.zoomInAction());
    QVERIFY(menu.zoomOutAction());
    QVERIFY(menu.zoomResetAction());
}

void ZoomActionMenuTest::shouldAssignZoomFactor()
{
    MessageViewer::ZoomActionMenu menu(new MessageViewer::MailWebView(), this);
    menu.setActionCollection(new KActionCollection(this));
    menu.createZoomActions();
    qreal initialValue = 50;
    menu.setZoomFactor(initialValue);
    QCOMPARE(menu.zoomFactor(), initialValue);
}

QTEST_MAIN(ZoomActionMenuTest)
