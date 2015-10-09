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

#include "gravatarconfiguresettingsdialogtest.h"
#include "../src/widgets/gravatarconfiguresettingsdialog.h"
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <KPluralHandlingSpinBox>
#include <QTest>
#include <KSeparator>

GravatarConfigureSettingsDialogTest::GravatarConfigureSettingsDialogTest(QObject *parent)
    : QObject(parent)
{

}

GravatarConfigureSettingsDialogTest::~GravatarConfigureSettingsDialogTest()
{

}

void GravatarConfigureSettingsDialogTest::shouldHaveDefaultValue()
{
    Gravatar::GravatarConfigureSettingsDialog dlg;
    QCheckBox *useDefaultImage = dlg.findChild<QCheckBox *>(QStringLiteral("usedefaultimage"));
    QVERIFY(useDefaultImage);

    QPushButton *clearGravatarCache = dlg.findChild<QPushButton *>(QStringLiteral("cleargravatarcachebutton"));
    QVERIFY(clearGravatarCache);

    QLabel *lab = dlg.findChild<QLabel *>(QStringLiteral("gravatarcachesizelabel"));
    QVERIFY(lab);

    KPluralHandlingSpinBox *gravatarCacheSize = dlg.findChild<KPluralHandlingSpinBox *>(QStringLiteral("gravatarcachesize"));
    QVERIFY(gravatarCacheSize);

    QCheckBox *useLibravatar = dlg.findChild<QCheckBox *>(QStringLiteral("uselibravatarcheckbox"));
    QVERIFY(useLibravatar);
    QVERIFY(useLibravatar->isEnabled());

    QCheckBox *fallbackGravatar = dlg.findChild<QCheckBox *>(QStringLiteral("fallbackgravatar"));
    QVERIFY(fallbackGravatar);
    QVERIFY(!fallbackGravatar->isEnabled());

    QCheckBox *useHttps = dlg.findChild<QCheckBox *>(QStringLiteral("usehttps"));
    QVERIFY(useHttps);

    KSeparator *separator = dlg.findChild<KSeparator *>(QStringLiteral("separator"));
    QVERIFY(separator);

    KSeparator *separator2 = dlg.findChild<KSeparator *>(QStringLiteral("separator2"));
    QVERIFY(separator2);
}

QTEST_MAIN(GravatarConfigureSettingsDialogTest)
