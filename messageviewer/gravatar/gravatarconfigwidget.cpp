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

#include "gravatarconfigwidget.h"
#include "pimcommon/gravatar/gravatarcache.h"
#include "gravatarconfiguresettingsdialog.h"
#include <QDebug>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QSpinBox>
#include <QPointer>
#include <QGroupBox>
#include "settings/globalsettings.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"

using namespace MessageViewer;
using namespace PimCommon::ConfigureImmutableWidgetUtils;

GravatarConfigWidget::GravatarConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    mainLayout->setMargin(0);

    mEnableGravatarSupport = new QCheckBox(i18n("Enable Gravatar Support"));
    mEnableGravatarSupport->setObjectName(QStringLiteral("gravatarcheckbox"));
    mEnableGravatarSupport->setChecked(false);
    mainLayout->addWidget(mEnableGravatarSupport);

    mConfigureGravatarSetting = new QPushButton(i18n("Configure"));
    mConfigureGravatarSetting->setObjectName(QStringLiteral("configure"));
    mainLayout->addWidget(mConfigureGravatarSetting);
    connect(mConfigureGravatarSetting, &QPushButton::clicked, this, &GravatarConfigWidget::slotConfigureSettings);
    mainLayout->addStretch();

    connect(mEnableGravatarSupport, &QCheckBox::toggled, this, &GravatarConfigWidget::slotGravatarEnableChanged);
    updateWidgetState(false);
}

GravatarConfigWidget::~GravatarConfigWidget()
{

}

void GravatarConfigWidget::slotConfigureSettings()
{
    QPointer<MessageViewer::GravatarConfigureSettingsDialog> dlg = new MessageViewer::GravatarConfigureSettingsDialog(this);
    dlg->exec();
    delete dlg;
}

void GravatarConfigWidget::slotGravatarEnableChanged(bool state)
{
    updateWidgetState(state);
    Q_EMIT configChanged(state);
}

void GravatarConfigWidget::updateWidgetState(bool state)
{
    mConfigureGravatarSetting->setEnabled(state);
}

void GravatarConfigWidget::save()
{
    saveCheckBox(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    if (!mEnableGravatarSupport->isChecked()) {
        PimCommon::GravatarCache::self()->clearAllCache();
    }
}

void GravatarConfigWidget::doLoadFromGlobalSettings()
{
    loadWidget(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    updateWidgetState(mEnableGravatarSupport->isChecked());
}

void GravatarConfigWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults(true);
    doLoadFromGlobalSettings();
    GlobalSettings::self()->useDefaults(bUseDefaults);
}

