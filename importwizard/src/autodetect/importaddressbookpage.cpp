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

#include "importaddressbookpage.h"
#include "ui_importaddressbookpage.h"

ImportAddressbookPage::ImportAddressbookPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportAddressbookPage)
{
    ui->setupUi(this);
    connect(ui->importAddressBook, &QPushButton::clicked, this, &ImportAddressbookPage::importAddressbookClicked);
}

ImportAddressbookPage::~ImportAddressbookPage()
{
    delete ui;
}

void ImportAddressbookPage::addImportInfo(const QString &log)
{
    ui->logAddressbook->addInfoLogEntry(log);
}

void ImportAddressbookPage::addImportError(const QString &log)
{
    ui->logAddressbook->addErrorLogEntry(log);
}

void ImportAddressbookPage::setImportButtonEnabled(bool enabled)
{
    ui->importAddressBook->setEnabled(enabled);
}

