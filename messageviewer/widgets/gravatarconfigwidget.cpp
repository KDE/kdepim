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

#include <QVBoxLayout>
#include <KLocalizedString>
#include <QCheckBox>
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

}

GravatarConfigWidget::~GravatarConfigWidget()
{

}

void GravatarConfigWidget::save()
{
    saveCheckBox(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
}

void GravatarConfigWidget::doLoadFromGlobalSettings()
{
    loadWidget(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());

}

void GravatarConfigWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults( true );
    loadWidget(mEnableGravatarSupport, MessageViewer::GlobalSettings::self()->gravatarSupportEnabledItem());
    GlobalSettings::self()->useDefaults( bUseDefaults );
}

