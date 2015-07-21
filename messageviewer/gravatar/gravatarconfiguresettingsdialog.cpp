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
#include "pimcommon/gravatar/gravatarcache.h"
#include "settings/globalsettings.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"

#include <KLocalizedString>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

using namespace MessageViewer;
using namespace PimCommon::ConfigureImmutableWidgetUtils;

GravatarConfigureSettingsDialog::GravatarConfigureSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GravatarConfigureSettingsDialog::save);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GravatarConfigureSettingsDialog::reject);
    okButton->setDefault(true);

    QVBoxLayout *groupboxLayout = new QVBoxLayout;
    setLayout(groupboxLayout);

    mUseLibravatar = new QCheckBox(i18n("Use Libravatar"));
    mUseLibravatar->setObjectName(QStringLiteral("uselibravatarcheckbox"));
    groupboxLayout->addWidget(mUseLibravatar);

    mFallbackGravatar = new QCheckBox(i18n("Fallback to Gravatar"));
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

    connect(mClearGravatarCache, &QAbstractButton::clicked, this, &GravatarConfigureSettingsDialog::slotClearGravatarCache);
    groupboxLayout->addWidget(buttonBox);
}

GravatarConfigureSettingsDialog::~GravatarConfigureSettingsDialog()
{

}

void GravatarConfigureSettingsDialog::save()
{
    saveCheckBox(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    saveSpinBox(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
    saveCheckBox(mFallbackGravatar, MessageViewer::GlobalSettings::self()->fallbackToGravatarItem());
    saveCheckBox(mUseLibravatar, MessageViewer::GlobalSettings::self()->libravatarSupportEnabledItem());
    accept();
}

void GravatarConfigureSettingsDialog::load()
{
    loadWidget(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    loadWidget(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
    loadWidget(mFallbackGravatar, MessageViewer::GlobalSettings::self()->fallbackToGravatarItem());
    loadWidget(mUseLibravatar, MessageViewer::GlobalSettings::self()->libravatarSupportEnabledItem());
}

void GravatarConfigureSettingsDialog::slotClearGravatarCache()
{
    PimCommon::GravatarCache::self()->clearAllCache();
}
