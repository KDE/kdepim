/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "checkprogrampage.h"
#include "ui_checkprogrampage.h"

CheckProgramPage::CheckProgramPage(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::CheckProgramPage)
{
  ui->setupUi(this);
  connect(ui->listProgramFound,SIGNAL(itemSelectionChanged()),this,SLOT(slotItemSelectionChanged()));
}

CheckProgramPage::~CheckProgramPage()
{
  delete ui;
}

void CheckProgramPage::setFoundProgram(const QStringList& list)
{
    ui->listProgramFound->addItems(list);
}

void CheckProgramPage::slotItemSelectionChanged()
{
    if(ui->listProgramFound->currentItem())
        emit programSelected(ui->listProgramFound->currentItem()->text());
}

void CheckProgramPage::disableSelectProgram()
{
  ui->listProgramFound->setEnabled( false );
}

#include "checkprogrampage.moc"
