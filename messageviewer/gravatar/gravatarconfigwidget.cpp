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
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QCheckBox>
#include <QPushButton>
#include <QPointer>
#include "settings/globalsettings.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"

using namespace MessageViewer;
using namespace PimCommon::ConfigureImmutableWidgetUtils;

class MessageViewer::GravatarConfigWidgetPrivate
{
public:
    GravatarConfigWidgetPrivate()
        : mEnableGravatarSupport(Q_NULLPTR),
          mConfigureGravatarSetting(Q_NULLPTR)
    {

    }

    QCheckBox *mEnableGravatarSupport;
    QPushButton *mConfigureGravatarSetting;
};

GravatarConfigWidget::GravatarConfigWidget(QWidget *parent)
    : QWidget(parent),
      d(new MessageViewer::GravatarConfigWidgetPrivate)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    mainLayout->setMargin(0);

    d->mEnableGravatarSupport = new QCheckBox(i18n("Enable Gravatar Support"));
    d->mEnableGravatarSupport->setObjectName(QStringLiteral("gravatarcheckbox"));
    d->mEnableGravatarSupport->setChecked(false);
    mainLayout->addWidget(d->mEnableGravatarSupport);

    d->mConfigureGravatarSetting = new QPushButton(i18n("Configure..."));
    d->mConfigureGravatarSetting->setObjectName(QStringLiteral("configure"));
    mainLayout->addWidget(d->mConfigureGravatarSetting);
    connect(d->mConfigureGravatarSetting, &QPushButton::clicked, this, &GravatarConfigWidget::slotConfigureSettings);
    mainLayout->addStretch();

    connect(d->mEnableGravatarSupport, &QCheckBox::toggled, this, &GravatarConfigWidget::slotGravatarEnableChanged);
    updateWidgetState(false);
}

GravatarConfigWidget::~GravatarConfigWidget()
{
    delete d;
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
    d->mConfigureGravatarSetting->setEnabled(state);
}

void GravatarConfigWidget::save()
{
    saveCheckBox(d->mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    if (!d->mEnableGravatarSupport->isChecked()) {
        PimCommon::GravatarCache::self()->clearAllCache();
    }
}

void GravatarConfigWidget::doLoadFromGlobalSettings()
{
    loadWidget(d->mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    updateWidgetState(d->mEnableGravatarSupport->isChecked());
}

void GravatarConfigWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults(true);
    doLoadFromGlobalSettings();
    GlobalSettings::self()->useDefaults(bUseDefaults);
}

