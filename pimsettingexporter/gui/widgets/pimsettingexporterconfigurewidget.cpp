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

#include "pimsettingexporterconfigurewidget.h"
#include "pimsettingexporterglobalconfig.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <KLocalizedString>

PimSettingExporterConfigureWidget::PimSettingExporterConfigureWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    mAlwaysOverrideFile = new QCheckBox(i18n("Always Override File"));
    mAlwaysOverrideFile->setObjectName(QStringLiteral("alwaysoverridefile"));
    layout->addWidget(mAlwaysOverrideFile);

    initialize();
}

PimSettingExporterConfigureWidget::~PimSettingExporterConfigureWidget()
{

}

void PimSettingExporterConfigureWidget::initialize()
{
    mAlwaysOverrideFile->setChecked(PimSettingExportGlobalConfig::self()->alwaysOverrideFile());
}

void PimSettingExporterConfigureWidget::save()
{
    PimSettingExportGlobalConfig::self()->setAlwaysOverrideFile(mAlwaysOverrideFile->isChecked());
}

void PimSettingExporterConfigureWidget::resetToDefault()
{
    const bool bUseDefaults = PimSettingExportGlobalConfig::self()->useDefaults(true);
    initialize();

    PimSettingExportGlobalConfig::self()->useDefaults(bUseDefaults);
}
