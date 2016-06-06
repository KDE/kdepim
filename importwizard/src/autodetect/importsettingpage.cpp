/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "importsettingpage.h"
#include "ui_importsettingpage.h"

ImportSettingPage::ImportSettingPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportSettingPage)
{
    ui->setupUi(this);
    connect(ui->importSettings, &QPushButton::clicked, this, &ImportSettingPage::importSettingsClicked);
}

ImportSettingPage::~ImportSettingPage()
{
    delete ui;
}

void ImportSettingPage::addImportInfo(const QString &log)
{
    ui->logSettings->addInfoLogEntry(log);
}

void ImportSettingPage::addImportError(const QString &log)
{
    ui->logSettings->addErrorLogEntry(log);
}

void ImportSettingPage::setImportButtonEnabled(bool enabled)
{
    ui->importSettings->setEnabled(enabled);
}

