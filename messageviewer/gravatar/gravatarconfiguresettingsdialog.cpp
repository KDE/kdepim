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

#include <KLocalizedString>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

using namespace MessageViewer;

GravatarConfigureSettingsDialog::GravatarConfigureSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
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

}

GravatarConfigureSettingsDialog::~GravatarConfigureSettingsDialog()
{

}

void GravatarConfigureSettingsDialog::save()
{

}

void GravatarConfigureSettingsDialog::load()
{

}

void GravatarConfigureSettingsDialog::slotClearGravatarCache()
{
    PimCommon::GravatarCache::self()->clearAllCache();
}
