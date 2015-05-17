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
#include <KStandardDirs>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QCheckBox>
#include <QPushButton>
#include <KGlobal>
#include <QLabel>
#include <QDir>
#include <KIntNumInput>
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

    //KF5 add i18n
    mEnableGravatarSupport = new QCheckBox(QLatin1String("Enable Gravatar Support"));
    mEnableGravatarSupport->setObjectName(QLatin1String("gravatarcheckbox"));
    mainLayout->addWidget(mEnableGravatarSupport);

    //KF5 add i18n
    mUseDefaultPixmap = new QCheckBox(QLatin1String("Use Default Image"));
    mUseDefaultPixmap->setObjectName(QLatin1String("usedefaultimage"));
    mainLayout->addWidget(mUseDefaultPixmap);

    //KF5 add i18n
    QHBoxLayout *cacheSizeLayout = new QHBoxLayout;
    mainLayout->addLayout(cacheSizeLayout);
    QLabel *lab = new QLabel(QLatin1String("Gravatar Cache Size:"));
    lab->setObjectName(QLatin1String("gravatarcachesizelabel"));
    cacheSizeLayout->addWidget(lab);

    mGravatarCacheSize = new KIntSpinBox;
    mGravatarCacheSize->setMinimum(1);
    mGravatarCacheSize->setMaximum(9999);
    mGravatarCacheSize->setObjectName(QLatin1String("gravatarcachesize"));
    connect(mGravatarCacheSize, SIGNAL(valueChanged(int)), this, SLOT(slotGravatarCacheSizeChanged()));
    cacheSizeLayout->addWidget(mGravatarCacheSize);
    cacheSizeLayout->addStretch();

    //KF5 add i18n
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);
    mClearGravatarCache = new QPushButton(QLatin1String("Clear Gravatar Cache"));
    mClearGravatarCache->setObjectName(QLatin1String("cleargravatarcachebutton"));
    buttonLayout->addWidget(mClearGravatarCache);
    buttonLayout->addStretch();


    connect(mClearGravatarCache, SIGNAL(clicked(bool)), this, SLOT(slotClearGravatarCache()));
    connect(mUseDefaultPixmap, SIGNAL(clicked(bool)), SIGNAL(configChanged(bool)));
    connect(mEnableGravatarSupport, SIGNAL(clicked(bool)), SIGNAL(configChanged(bool)));
}

GravatarConfigWidget::~GravatarConfigWidget()
{

}

void GravatarConfigWidget::save()
{
    saveCheckBox(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    saveCheckBox(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    saveSpinBox(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
}

void GravatarConfigWidget::doLoadFromGlobalSettings()
{
    loadWidget(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    loadWidget(mUseDefaultPixmap, MessageViewer::GlobalSettings::self()->gravatarUseDefaultImageItem());
    loadWidget(mGravatarCacheSize, MessageViewer::GlobalSettings::self()->gravatarCacheSizeItem());
}

void GravatarConfigWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults( true );
    doLoadFromGlobalSettings();
    GlobalSettings::self()->useDefaults( bUseDefaults );
}

void GravatarConfigWidget::slotClearGravatarCache()
{
    const QString path = KGlobal::dirs()->locateLocal("data", QLatin1String("gravatar/"));
    if (!path.isEmpty()) {
        QDir dir(path);
        if (dir.exists()) {
            QFileInfoList list = dir.entryInfoList();  // get list of matching files and delete all
            QFileInfo it;
            Q_FOREACH( it, list ) {
                dir.remove(it.fileName());
            }
        }
    }
    PimCommon::GravatarCache::self()->clear();
}

void GravatarConfigWidget::slotGravatarCacheSizeChanged()
{
    Q_EMIT configChanged(true);
}

