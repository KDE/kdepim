/*
   Copyright (C) 2014-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "serversievesettingstest.h"
#include "../src/serversievesettings.h"
#include <qtest.h>
#include <QSignalSpy>
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
    const QString password = QStringLiteral("password");
    const QString username = QStringLiteral("username");
    const QString servername = QStringLiteral("servername");
    ServerSieveSettings widget;
    widget.setServerName(servername);
    widget.setUserName(username);
    widget.setPassword(password);
    QCOMPARE(widget.serverName(), servername);
    QCOMPARE(widget.userName(), username);
    QCOMPARE(widget.password(), password);
}

void ServerSieveSettingsTest::shouldEmitEnableOkButtonSignal()
{
    ServerSieveSettings widget;
    widget.show();
    QTest::qWaitForWindowExposed(&widget);
    QSignalSpy spy(&widget, SIGNAL(enableOkButton(bool)));
    widget.setPassword(QStringLiteral("foo"));
    QCOMPARE(spy.count(), 0);

    int numberEmitSignal = 1;
    widget.setServerName(QStringLiteral("foo"));
    QCOMPARE(spy.count(), numberEmitSignal);
    ++numberEmitSignal;
    widget.setUserName(QStringLiteral("foo"));
    QCOMPARE(spy.count(), numberEmitSignal);
    ++numberEmitSignal;
    widget.setUserName(QStringLiteral(""));
    QCOMPARE(spy.count(), numberEmitSignal);
    ++numberEmitSignal;
    widget.setServerName(QStringLiteral(""));
    QCOMPARE(spy.count(), numberEmitSignal);
}

void ServerSieveSettingsTest::shouldEmitSignalWithValueTrue()
{
    ServerSieveSettings widget;
    widget.show();
    QTest::qWaitForWindowExposed(&widget);
    QSignalSpy spy(&widget, SIGNAL(enableOkButton(bool)));
    widget.setServerName(QStringLiteral("foo"));
    QCOMPARE(spy.count(), 1);
    //We need servername!=empty and username != empty
    QCOMPARE(spy.at(0).at(0).toBool(), false);

    widget.setUserName(QStringLiteral("foo"));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toBool(), true);

    //We don't want empty string
    widget.setUserName(QStringLiteral(" "));
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).at(0).toBool(), false);

    //We don't want empty string
    widget.setServerName(QStringLiteral(" "));
    QCOMPARE(spy.count(), 4);
    QCOMPARE(spy.at(3).at(0).toBool(), false);

}

QTEST_MAIN(ServerSieveSettingsTest)
