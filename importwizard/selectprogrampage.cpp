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

#include "selectprogrampage.h"
#include "ui_selectprogrampage.h"

SelectProgramPage::SelectProgramPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectProgramPage)
{
    ui->setupUi(this);
    connect(ui->listProgramFound,SIGNAL(itemSelectionChanged()),this,SLOT(slotItemSelectionChanged()));
    connect( ui->listProgramFound, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)) );
}


SelectProgramPage::~SelectProgramPage()
{
    delete ui;
}

void SelectProgramPage::setFoundProgram(const QStringList& list)
{
    ui->listProgramFound->addItems(list);
}

void SelectProgramPage::slotItemSelectionChanged()
{
    if (ui->listProgramFound->currentItem()) {
        Q_EMIT programSelected(ui->listProgramFound->currentItem()->text());
    }
}

void SelectProgramPage::slotItemDoubleClicked( QListWidgetItem*item )
{
    if ( item ) {
        Q_EMIT doubleClicked();
    }
}

void SelectProgramPage::disableSelectProgram()
{
    ui->listProgramFound->setEnabled( false );
}

