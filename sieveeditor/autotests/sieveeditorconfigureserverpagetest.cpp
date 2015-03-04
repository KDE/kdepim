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

#include "sieveeditorconfigureserverpagetest.h"
#include "../sieveeditorconfigureserverpage.h"
#include <QPushButton>
#include <qtest.h>
#include <QSignalSpy>
#include <qtestmouse.h>
SieveEditorConfigureServerPageTest::SieveEditorConfigureServerPageTest(QObject *parent)
    : QObject(parent)
{

}

SieveEditorConfigureServerPageTest::~SieveEditorConfigureServerPageTest()
{

}

void SieveEditorConfigureServerPageTest::shouldHaveDefaultValue()
{
    SieveEditorConfigureServerPage w;
    QPushButton *button = qFindChild<QPushButton*>(&w, QLatin1String("configure_button"));
    QVERIFY(button);
}

void SieveEditorConfigureServerPageTest::shouldEmitSignalWhenClickOnButton()
{
    SieveEditorConfigureServerPage w;
    QPushButton *button = qFindChild<QPushButton*>(&w, QLatin1String("configure_button"));
    QSignalSpy spy(&w, SIGNAL(configureClicked()));
    QTest::mouseClick(button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(SieveEditorConfigureServerPageTest)
