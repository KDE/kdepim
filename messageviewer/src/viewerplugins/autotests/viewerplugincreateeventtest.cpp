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

#include "viewerplugincreateeventtest.h"
#include "../createeventplugin/viewerplugincreateevent.h"
#include <KActionCollection>
#include <QTest>

ViewerPluginCreateeventTest::ViewerPluginCreateeventTest(QObject *parent)
    : QObject(parent)
{

}

ViewerPluginCreateeventTest::~ViewerPluginCreateeventTest()
{

}

void ViewerPluginCreateeventTest::shouldHaveDefaultValue()
{
    MessageViewer::ViewerPluginCreateevent *event = new MessageViewer::ViewerPluginCreateevent(this);
    QVERIFY(!event->viewerPluginName().isEmpty());
    QVERIFY(event->createView(new QWidget(0), new KActionCollection(this)));
}

QTEST_MAIN(ViewerPluginCreateeventTest)
