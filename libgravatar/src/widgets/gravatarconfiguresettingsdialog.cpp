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

#include "gravatarconfiguresettingsdialog.h"
#include "misc/gravatarcache.h"
#include "gravatarsettings.h"
#include "PimCommon/ConfigureImmutableWidgetUtils"

#include <KLocalizedString>
#include <KPluralHandlingSpinBox>
#include <KSeparator>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>

using namespace Gravatar;
using namespace PimCommon::ConfigureImmutableWidgetUtils;

GravatarConfigureSettingsDialog::GravatarConfigureSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure Gravatar"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GravatarConfigureSettingsDialog::save);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GravatarConfigureSettingsDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &GravatarConfigureSettingsDialog::slotRestoreDefault);
    okButton->setDefault(true);

    mUseDefaultPixmap = new QCheckBox(i18n("Use Default Image"));
    mUseDefaultPixmap->setObjectName(QStringLiteral("usedefaultimage"));
    topLayout->addWidget(mUseDefaultPixmap);

    mUseHttps = new QCheckBox(i18n("Use HTTPS"));
    mUseHttps->setObjectName(QStringLiteral("usehttps"));
    topLayout->addWidget(mUseHttps);

    mUseLibravatar = new QCheckBox(i18n("Use Libravatar"));
    mUseLibravatar->setObjectName(QStringLiteral("uselibravatarcheckbox"));
    topLayout->addWidget(mUseLibravatar);

    mFallbackGravatar = new QCheckBox(i18n("Fallback to Gravatar"));
    mFallbackGravatar->setObjectName(QStringLiteral("fallbackgravatar"));
    topLayout->addWidget(mFallbackGravatar);
    connect(mUseLibravatar, &QCheckBox::toggled, mFallbackGravatar, &QCheckBox::setEnabled);
    mFallbackGravatar->setEnabled(false);

    QHBoxLayout *cacheSizeLayout = new QHBoxLayout;
    topLayout->addLayout(cacheSizeLayout);
    QLabel *lab = new QLabel(i18n("Gravatar Cache Size:"));
    lab->setObjectName(QStringLiteral("gravatarcachesizelabel"));
    cacheSizeLayout->addWidget(lab);

    mGravatarCacheSize = new KPluralHandlingSpinBox;
    mGravatarCacheSize->setMinimum(1);
    mGravatarCacheSize->setMaximum(9999);
    mGravatarCacheSize->setSuffix(ki18ncp("add space before image", " image", " images"));
    mGravatarCacheSize->setObjectName(QStringLiteral("gravatarcachesize"));
    cacheSizeLayout->addWidget(mGravatarCacheSize);
    cacheSizeLayout->addStretch();

    KSeparator *separator = new KSeparator(this);
    separator->setObjectName(QStringLiteral("separator"));
    topLayout->addWidget(separator);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    topLayout->addLayout(buttonLayout);
    mClearGravatarCache = new QPushButton(i18n("Clear Gravatar Cache"));
    mClearGravatarCache->setObjectName(QStringLiteral("cleargravatarcachebutton"));
    buttonLayout->addWidget(mClearGravatarCache);
    buttonLayout->addStretch();

    separator = new KSeparator(this);
    separator->setObjectName(QStringLiteral("separator2"));
    topLayout->addWidget(separator);

    connect(mClearGravatarCache, &QAbstractButton::clicked, this, &GravatarConfigureSettingsDialog::slotClearGravatarCache);
    topLayout->addWidget(buttonBox);
    load();
}

GravatarConfigureSettingsDialog::~GravatarConfigureSettingsDialog()
{

}

void GravatarConfigureSettingsDialog::slotRestoreDefault()
{
    const bool bUseDefaults = Gravatar::GravatarSettings::self()->useDefaults(true);
    load();
    Gravatar::GravatarSettings::self()->useDefaults(bUseDefaults);
}

void GravatarConfigureSettingsDialog::save()
{
    saveCheckBox(mUseDefaultPixmap, Gravatar::GravatarSettings::self()->gravatarUseDefaultImageItem());
    saveSpinBox(mGravatarCacheSize, Gravatar::GravatarSettings::self()->gravatarCacheSizeItem());
    saveCheckBox(mFallbackGravatar, Gravatar::GravatarSettings::self()->fallbackToGravatarItem());
    saveCheckBox(mUseLibravatar, Gravatar::GravatarSettings::self()->libravatarSupportEnabledItem());
    saveCheckBox(mUseHttps, Gravatar::GravatarSettings::self()->gravatarHttpsSupportItem());
    accept();
}

void GravatarConfigureSettingsDialog::load()
{
    loadWidget(mUseDefaultPixmap, Gravatar::GravatarSettings::self()->gravatarUseDefaultImageItem());
    loadWidget(mGravatarCacheSize, Gravatar::GravatarSettings::self()->gravatarCacheSizeItem());
    loadWidget(mFallbackGravatar, Gravatar::GravatarSettings::self()->fallbackToGravatarItem());
    loadWidget(mUseLibravatar, Gravatar::GravatarSettings::self()->libravatarSupportEnabledItem());
    loadWidget(mUseHttps, Gravatar::GravatarSettings::self()->gravatarHttpsSupportItem());
}

void GravatarConfigureSettingsDialog::slotClearGravatarCache()
{
    Gravatar::GravatarCache::self()->clearAllCache();
}
