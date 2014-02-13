/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "viewertest.h"
#include <qtest_kde.h>
#include "messageviewer/viewer/viewer.h"
#include <qtestmouse.h>
#include <KActionCollection>
#include <KAction>

ViewerTest::ViewerTest()
{
}

void ViewerTest::shouldHaveDefaultValuesOnCreation()
{
    MessageViewer::Viewer viewer(0, 0, new KActionCollection(this));
    viewer.show();
    QTest::qWaitForWindowShown(&viewer);

    QVERIFY(!viewer.message());
    QWidget *createtodowidget = qFindChild<QWidget *>(&viewer, QLatin1String("createtodowidget"));
    QVERIFY(createtodowidget);
    QCOMPARE(createtodowidget->isVisible(), false);
}

void ViewerTest::shouldShowCreateTodoWidgetWhenActivateItAndWeHaveAMessage()
{
    MessageViewer::Viewer viewer(0, 0, new KActionCollection(this));
    viewer.show();
    QTest::qWaitForWindowShown(&viewer);
    QWidget *createtodowidget = qFindChild<QWidget *>(&viewer, QLatin1String("createtodowidget"));
    QVERIFY(viewer.createTodoAction());

    viewer.createTodoAction()->trigger();
    //No message => we can show it.
    QCOMPARE(createtodowidget->isVisible(), false);

    KMime::Message::Ptr msg(new KMime::Message);
    viewer.setMessage(msg);

    viewer.createTodoAction()->trigger();
    QCOMPARE(createtodowidget->isVisible(), true);
}

QTEST_KDEMAIN( ViewerTest, GUI )
