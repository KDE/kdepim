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

#include "customtoolswidgetngtest.h"
#include "customtools/customtoolswidgetng.h"
#include <QStackedWidget>
#include <QTest>
#include <KXmlGui/KActionCollection>

CustomToolsWidgetNgTest::CustomToolsWidgetNgTest(QObject *parent)
    : QObject(parent)
{

}

CustomToolsWidgetNgTest::~CustomToolsWidgetNgTest()
{

}

void CustomToolsWidgetNgTest::shouldHaveDefaultValue()
{
    PimCommon::CustomToolsWidgetNg widget(new KActionCollection(this));
    widget.show();

    QStackedWidget *stackWidget = widget.findChild<QStackedWidget *>(QStringLiteral("stackedwidget"));
    QVERIFY(stackWidget);
}

QTEST_MAIN(CustomToolsWidgetNgTest)
