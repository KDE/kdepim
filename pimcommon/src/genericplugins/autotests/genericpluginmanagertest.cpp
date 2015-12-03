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

#include "genericpluginmanagertest.h"
#include "../genericpluginmanager.h"
#include <QTest>

GenericPluginManagerTest::GenericPluginManagerTest(QObject *parent)
    : QObject(parent)
{

}

GenericPluginManagerTest::~GenericPluginManagerTest()
{

}

void GenericPluginManagerTest::shouldHaveDefaultValue()
{
    PimCommon::GenericPluginManager pluginManager;
    QVERIFY(!pluginManager.initializePlugins());
}

void GenericPluginManagerTest::shouldInitialized()
{
    PimCommon::GenericPluginManager pluginManager;
    QVERIFY(!pluginManager.initializePlugins());
    pluginManager.setServiceTypeName(QStringLiteral("foo"));
    QVERIFY(!pluginManager.initializePlugins());
    pluginManager.setPluginName(QStringLiteral("foo"));
    QVERIFY(pluginManager.initializePlugins());
}

QTEST_MAIN(GenericPluginManagerTest)
