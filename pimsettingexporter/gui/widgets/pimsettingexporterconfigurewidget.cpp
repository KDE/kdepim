/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterconfigurewidget.h"
#include "pimsettingexporterglobalconfig.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <KLocalizedString>
#include <QGroupBox>

PimSettingExporterConfigureWidget::PimSettingExporterConfigureWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    QGroupBox *groupBox = new QGroupBox(i18n("Import"));
    groupBox->setObjectName(QStringLiteral("import_groupbox"));
    layout->addWidget(groupBox);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout;
    groupBox->setLayout(groupBoxLayout);

    mAlwaysOverrideFile = new QCheckBox(i18n("Always Override File"));
    mAlwaysOverrideFile->setObjectName(QStringLiteral("alwaysoverridefile"));
    groupBoxLayout->addWidget(mAlwaysOverrideFile);

    mAlwaysOverrideDirectory = new QCheckBox(i18n("Always Override File"));
    mAlwaysOverrideDirectory->setObjectName(QStringLiteral("alwaysoverridedirectory"));
    groupBoxLayout->addWidget(mAlwaysOverrideDirectory);

    mAlwaysMergeConfigFile = new QCheckBox(i18n("Always Merge Config File"));
    mAlwaysMergeConfigFile->setObjectName(QStringLiteral("alwaysmergeconfigfile"));
    groupBoxLayout->addWidget(mAlwaysMergeConfigFile);

    initialize();
}

PimSettingExporterConfigureWidget::~PimSettingExporterConfigureWidget()
{

}

void PimSettingExporterConfigureWidget::initialize()
{
    mAlwaysOverrideFile->setChecked(PimSettingExportGlobalConfig::self()->alwaysOverrideFile());
    mAlwaysMergeConfigFile->setChecked(PimSettingExportGlobalConfig::self()->alwaysMergeConfigFile());
    mAlwaysOverrideDirectory->setChecked(PimSettingExportGlobalConfig::self()->alwaysOverrideDirectory());
}

void PimSettingExporterConfigureWidget::save()
{
    PimSettingExportGlobalConfig::self()->setAlwaysOverrideFile(mAlwaysOverrideFile->isChecked());
    PimSettingExportGlobalConfig::self()->setAlwaysMergeConfigFile(mAlwaysMergeConfigFile->isChecked());
    PimSettingExportGlobalConfig::self()->setAlwaysOverrideDirectory(mAlwaysOverrideDirectory->isChecked());
}

void PimSettingExporterConfigureWidget::resetToDefault()
{
    const bool bUseDefaults = PimSettingExportGlobalConfig::self()->useDefaults(true);
    initialize();

    PimSettingExportGlobalConfig::self()->useDefaults(bUseDefaults);
}
