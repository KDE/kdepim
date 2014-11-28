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

#include "serversievesettingstest.h"
#include "../serversievesettings.h"
#include <qtest_kde.h>
ServerSieveSettingsTest::ServerSieveSettingsTest(QObject *parent)
    : QObject(parent)
{

}

ServerSieveSettingsTest::~ServerSieveSettingsTest()
{

}

void ServerSieveSettingsTest::shouldHaveDefaultValue()
{
    ServerSieveSettings widget;
    QVERIFY(widget.serverName().isEmpty());
    QVERIFY(widget.userName().isEmpty());
    QVERIFY(widget.password().isEmpty());
}

void ServerSieveSettingsTest::shouldSetValue()
{
    const QString password = QLatin1String("password");
    const QString username = QLatin1String("username");
    const QString servername = QLatin1String("servername");
    ServerSieveSettings widget;
    widget.setServerName(servername);
    widget.setUserName(username);
    widget.setPassword(password);
    QCOMPARE(widget.serverName(), servername);
    QCOMPARE(widget.userName(), username);
    QCOMPARE(widget.password(), password);
}

QTEST_KDEMAIN(ServerSieveSettingsTest, GUI)
