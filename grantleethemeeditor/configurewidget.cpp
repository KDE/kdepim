/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "configurewidget.h"
#include "ui_configurewidget.h"

#include <KConfigGroup>
#include <KSharedConfig>

using namespace GrantleeThemeEditor;
ConfigureWidget::ConfigureWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ConfigureWidget)
{
    ui->setupUi(this);
}

ConfigureWidget::~ConfigureWidget()
{
    delete ui;
}

void ConfigureWidget::save()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QLatin1String("Global"));
    group.writeEntry("path", ui->defaultPath->url());
    group.writeEntry("authorEmail", ui->authorEmail->text());
}

void ConfigureWidget::load()
{
    KSharedConfig::Ptr config = KGlobal::config();
    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        ui->defaultPath->setUrl(group.readEntry("path", KUrl()));
        ui->authorEmail->setText(group.readEntry("authorEmail"));
    }
}

void ConfigureWidget::setDefault()
{
    ui->defaultPath->setUrl(KUrl());
    ui->authorEmail->clear();
}


#include "configurewidget.moc"
