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

#include "gravatarupdatewidgettest.h"
#include <qtest.h>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <kaddressbook/gravatar/widgets/gravatarupdatewidget.h>
GravatarUpdateWidgetTest::GravatarUpdateWidgetTest(QObject *parent)
    : QObject(parent)
{

}

GravatarUpdateWidgetTest::~GravatarUpdateWidgetTest()
{

}

void GravatarUpdateWidgetTest::shouldHaveDefaultValue()
{
    KABGravatar::GravatarUpdateWidget widget;
    widget.show();
    QLabel *lab = widget.findChild<QLabel *>(QStringLiteral("emaillabel"));
    QVERIFY(lab);

    QLabel *emaillabel = widget.findChild<QLabel *>(QStringLiteral("email"));
    QVERIFY(emaillabel);

    QCheckBox *useHttps = widget.findChild<QCheckBox *>(QStringLiteral("usehttps"));
    QVERIFY(useHttps);
    QVERIFY(!useHttps->isChecked());
    QVERIFY(useHttps->isEnabled());

    QCheckBox *useLibravatar = widget.findChild<QCheckBox *>(QStringLiteral("uselibravatar"));
    QVERIFY(useLibravatar);
    QVERIFY(!useLibravatar->isChecked());
    QVERIFY(useLibravatar->isEnabled());

    QCheckBox *useFallbackGravatar = widget.findChild<QCheckBox *>(QStringLiteral("fallbackgravatar"));
    QVERIFY(useFallbackGravatar);
    QVERIFY(!useFallbackGravatar->isChecked());
    QVERIFY(!useFallbackGravatar->isEnabled());


    QPushButton *searchGravatar = widget.findChild<QPushButton *>(QStringLiteral("search"));
    QVERIFY(searchGravatar);
    QVERIFY(!searchGravatar->isEnabled());

    QLabel *resultGravatar = widget.findChild<QLabel *>(QStringLiteral("result"));
    QVERIFY(resultGravatar);

    QVERIFY(widget.pixmap().isNull());
    QVERIFY(widget.resolvedUrl().isEmpty());
}

void GravatarUpdateWidgetTest::shouldAffectEmail()
{
    KABGravatar::GravatarUpdateWidget widget;

    QLabel *emaillabel = widget.findChild<QLabel *>(QStringLiteral("email"));

    QPushButton *searchGravatar = widget.findChild<QPushButton *>(QStringLiteral("search"));

    const QString newEmail(QStringLiteral("foo@kde.org"));
    widget.setEmail(newEmail);

    QCOMPARE(emaillabel->text(), newEmail);
    QVERIFY(searchGravatar->isEnabled());

    QString cleanName;
    widget.setEmail(cleanName);
    QCOMPARE(emaillabel->text(), cleanName);
    QVERIFY(!searchGravatar->isEnabled());
}

QTEST_MAIN(GravatarUpdateWidgetTest)
