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

#include "shareserviceurlmanagertest.h"
#include "../shareserviceurlmanager.h"
#include <qtest.h>
#include <kactionmenu.h>
#include <QMenu>

ShareServiceUrlManagerTest::ShareServiceUrlManagerTest(QObject *parent)
    : QObject(parent)
{

}

ShareServiceUrlManagerTest::~ShareServiceUrlManagerTest()
{

}

void ShareServiceUrlManagerTest::shouldHaveDefaultValue()
{
    PimCommon::ShareServiceUrlManager manager;
    QVERIFY(manager.menu());
    QVERIFY(!manager.menu()->menu()->actions().isEmpty());
}

void ShareServiceUrlManagerTest::shouldGenerateServiceUrl_data()
{
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("link");
    QTest::addColumn<PimCommon::ShareServiceUrlManager::ServiceType>("serviceType");
    QTest::addColumn<QUrl>("output");
    //Url is valid ?
    QTest::addColumn<bool>("valid");
    QTest::newRow("no title no link") << QString() << QString() << PimCommon::ShareServiceUrlManager::Fbook << QUrl() << false;
}

void ShareServiceUrlManagerTest::shouldGenerateServiceUrl()
{
    QFETCH(QString, title);
    QFETCH(QString, link);
    QFETCH(PimCommon::ShareServiceUrlManager::ServiceType, serviceType);
    QFETCH(QUrl, output);
    QFETCH(bool, valid);

    PimCommon::ShareServiceUrlManager manager;
    const QUrl urlGenerated = manager.generateServiceUrl(link, title, serviceType);
    QCOMPARE(urlGenerated, output);
    QCOMPARE(urlGenerated.isValid(), valid);
}

QTEST_MAIN(ShareServiceUrlManagerTest)
