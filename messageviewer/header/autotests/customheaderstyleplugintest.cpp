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


#include "customheaderstyleplugintest.h"
#include "header/customheaderstyleplugin/customheaderstyleplugin.h"
#include "header/customheaderstyleplugin/customheaderstyleinterface.h"
#include <QTest>
#include <KActionCollection>
CustomHeaderStylePluginTest::CustomHeaderStylePluginTest(QObject *parent)
    : QObject(parent)
{

}

CustomHeaderStylePluginTest::~CustomHeaderStylePluginTest()
{

}

void CustomHeaderStylePluginTest::shouldHaveDefaultValue()
{
    MessageViewer::CustomHeaderStylePlugin plugin;
    QVERIFY(plugin.headerStyle());
    QVERIFY(plugin.headerStrategy());
}

void CustomHeaderStylePluginTest::shouldCreateInterface()
{
    MessageViewer::CustomHeaderStylePlugin plugin;
    MessageViewer::HeaderStyleInterface *interface = plugin.createView(new KActionCollection(this));
    QVERIFY(interface);
    QVERIFY(!interface->action().isEmpty());
}

QTEST_MAIN(CustomHeaderStylePluginTest)
