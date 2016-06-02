/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterconfigurewidgettest.h"
#include "../widgets/pimsettingexporterconfigurewidget.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QTest>

PimSettingExporterConfigureWidgetTest::PimSettingExporterConfigureWidgetTest(QObject *parent)
    : QObject(parent)
{

}

PimSettingExporterConfigureWidgetTest::~PimSettingExporterConfigureWidgetTest()
{

}

void PimSettingExporterConfigureWidgetTest::shouldHaveDefaultValue()
{
    PimSettingExporterConfigureWidget w;

    QGroupBox *importGroupBox = w.findChild<QGroupBox *>(QStringLiteral("import_groupbox"));
    QVERIFY(importGroupBox);

    QCheckBox *alwaysOverrideFile = w.findChild<QCheckBox *>(QStringLiteral("alwaysoverridefile"));
    QVERIFY(alwaysOverrideFile);

    QCheckBox *alwaysOverrideDirectory = w.findChild<QCheckBox *>(QStringLiteral("alwaysoverridedirectory"));
    QVERIFY(alwaysOverrideDirectory);

    QCheckBox *alwaysMergeConfigFile = w.findChild<QCheckBox *>(QStringLiteral("alwaysmergeconfigfile"));
    QVERIFY(alwaysMergeConfigFile);
}

QTEST_MAIN(PimSettingExporterConfigureWidgetTest)
