/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "importcalendarpage.h"
#include "ui_importcalendarpage.h"

ImportCalendarPage::ImportCalendarPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportCalendarPage)
{
    ui->setupUi(this);
    connect(ui->importCalendar, &QPushButton::clicked, this, &ImportCalendarPage::importCalendarClicked);
}

ImportCalendarPage::~ImportCalendarPage()
{
    delete ui;
}

void ImportCalendarPage::addImportInfo(const QString &log)
{
    ui->logCalendar->addInfoLogEntry(log);
}

void ImportCalendarPage::addImportError(const QString &log)
{
    ui->logCalendar->addErrorLogEntry(log);
}

void ImportCalendarPage::setImportButtonEnabled(bool enabled)
{
    ui->importCalendar->setEnabled(enabled);
}

