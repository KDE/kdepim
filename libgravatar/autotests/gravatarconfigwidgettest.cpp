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

#include "gravatarconfigwidgettest.h"
#include "../src/widgets/gravatarconfigwidget.h"
#include <QSpinBox>
#include <QCheckBox>
#include <qtest.h>
#include <QLabel>
#include <QPushButton>
#include <qtestmouse.h>
#include <QSignalSpy>
#include <QGroupBox>
#include <QStyleOption>

GravatarConfigWidgetTest::GravatarConfigWidgetTest(QObject *parent)
    : QObject(parent)
{

}

GravatarConfigWidgetTest::~GravatarConfigWidgetTest()
{

}

void GravatarConfigWidgetTest::shouldHaveDefaultValue()
{
    Gravatar::GravatarConfigWidget w;
    QCheckBox *enableGravatar = w.findChild<QCheckBox *>(QStringLiteral("gravatarcheckbox"));
    QVERIFY(enableGravatar);

    QPushButton *configure = w.findChild<QPushButton *>(QStringLiteral("configure"));
    QVERIFY(configure);
}

void GravatarConfigWidgetTest::shouldChangeState()
{
    Gravatar::GravatarConfigWidget w;
    w.show();
    QCheckBox *enableGravatar = w.findChild<QCheckBox *>(QStringLiteral("gravatarcheckbox"));

    QPushButton *configure = w.findChild<QPushButton *>(QStringLiteral("configure"));
    QVERIFY(!configure->isEnabled());
    enableGravatar->toggle();
    QVERIFY(configure->isEnabled());
}

void GravatarConfigWidgetTest::shoulEmitConfigChangedSignal()
{
    Gravatar::GravatarConfigWidget w;
    w.show();
    QCheckBox *enableGravatar = w.findChild<QCheckBox *>(QStringLiteral("gravatarcheckbox"));

    QSignalSpy spy(&w, SIGNAL(configChanged(bool)));
    QTest::mouseClick(enableGravatar, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(GravatarConfigWidgetTest)
