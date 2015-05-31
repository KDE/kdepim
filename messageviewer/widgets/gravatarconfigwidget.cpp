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
#include <QDebug>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QSpinBox>
#include <QGroupBox>
#include "settings/globalsettings.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"

using namespace MessageViewer;
using namespace PimCommon::ConfigureImmutableWidgetUtils;

GravatarConfigWidget::GravatarConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->setMargin(0);

    mEnableGravatarSupport = new QGroupBox(i18n("Enable Gravatar Support"));
    mEnableGravatarSupport->setObjectName(QStringLiteral("gravatarcheckbox"));
    mEnableGravatarSupport->setCheckable(true);
    mEnableGravatarSupport->setChecked(false);

    QVBoxLayout *groupboxLayout = new QVBoxLayout;
    mEnableGravatarSupport->setLayout(groupboxLayout);
    mainLayout->addWidget(mEnableGravatarSupport);

    mUseLibravatar = new QCheckBox(i18n("Use Libravatar"));
    mUseLibravatar->setObjectName(QStringLiteral("uselibravatarcheckbox"));
    groupboxLayout->addWidget(mUseLibravatar);

    mFallbackGravatar = new QCheckBox(i18n("Fallback to gravatar"));
    mFallbackGravatar->setObjectName(QStringLiteral("fallbackgravatar"));
    groupboxLayout->addWidget(mFallbackGravatar);

    mUseDefaultPixmap = new QCheckBox(i18n("Use Default Image"));
    mUseDefaultPixmap->setObjectName(QStringLiteral("usedefaultimage"));
    groupboxLayout->addWidget(mUseDefaultPixmap);

    QHBoxLayout *cacheSizeLayout = new QHBoxLayout;
    groupboxLayout->addLayout(cacheSizeLayout);
    QLabel *lab = new QLabel(i18n("Gravatar Cache Size:"));
    lab->setObjectName(QStringLiteral("gravatarcachesizelabel"));
    cacheSizeLayout->addWidget(lab);

    mGravatarCacheSize = new QSpinBox;
    mGravatarCacheSize->setMinimum(1);
    mGravatarCacheSize->setMaximum(9999);
    mGravatarCacheSize->setObjectName(QStringLiteral("gravatarcachesize"));
    connect(mGravatarCacheSize, SIGNAL(valueChanged(int)), this, SLOT(slotGravatarCacheSizeChanged()));
    cacheSizeLayout->addWidget(mGravatarCacheSize);
    cacheSizeLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    groupboxLayout->addLayout(buttonLayout);
    mClearGravatarCache = new QPushButton(i18n("Clear Gravatar Cache"));
    mClearGravatarCache->setObjectName(QStringLiteral("cleargravatarcachebutton"));
    buttonLayout->addWidget(mClearGravatarCache);
    buttonLayout->addStretch();

    connect(mEnableGravatarSupport, &QGroupBox::clicked, this, &GravatarConfigWidget::slotGravatarEnableChanged);

    connect(mUseDefaultPixmap, &QAbstractButton::clicked, this, &GravatarConfigWidget::configChanged);
    connect(mClearGravatarCache, &QAbstractButton::clicked, this, &GravatarConfigWidget::slotClearGravatarCache);

    connect(mUseLibravatar, &QAbstractButton::clicked, this, &GravatarConfigWidget::configChanged);
    connect(mFallbackGravatar, &QAbstractButton::clicked, this, &GravatarConfigWidget::configChanged);

    updateWidgetState(false);
}

GravatarConfigWidget::~GravatarConfigWidget()
{

}

void GravatarConfigWidget::slotGravatarEnableChanged(bool state)
{
    updateWidgetState(state);
    Q_EMIT configChanged(state);
}

void GravatarConfigWidget::updateWidgetState(bool state)
{
    mUseDefaultPixmap->setEnabled(state);
    mClearGravatarCache->setEnabled(state);
    mGravatarCacheSize->setEnabled(state);
    mFallbackGravatar->setEnabled(state);
    mUseLibravatar->setEnabled(state);
}

void GravatarConfigWidget::save()
{
    saveGroupBox(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    saveCheckBox(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    saveSpinBox(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
    saveCheckBox(mFallbackGravatar, MessageViewer::GlobalSettings::self()->fallbackToGravatarItem());
    saveCheckBox(mUseLibravatar, MessageViewer::GlobalSettings::self()->libravatarSupportEnabledItem());
    if (!mEnableGravatarSupport->isChecked()) {
        PimCommon::GravatarCache::self()->clearAllCache();
    }
}

void GravatarConfigWidget::doLoadFromGlobalSettings()
{
    loadWidget(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    loadWidget(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    loadWidget(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
    loadWidget(mFallbackGravatar, MessageViewer::GlobalSettings::self()->fallbackToGravatarItem());
    loadWidget(mUseLibravatar, MessageViewer::GlobalSettings::self()->libravatarSupportEnabledItem());
    updateWidgetState(mEnableGravatarSupport->isChecked());
}

void GravatarConfigWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults(true);
    doLoadFromGlobalSettings();
    GlobalSettings::self()->useDefaults(bUseDefaults);
}

void GravatarConfigWidget::slotClearGravatarCache()
{
    PimCommon::GravatarCache::self()->clearAllCache();
}

void GravatarConfigWidget::slotGravatarCacheSizeChanged()
{
    Q_EMIT configChanged(true);
}

