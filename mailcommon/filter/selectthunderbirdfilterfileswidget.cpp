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

#include "selectthunderbirdfilterfileswidget.h"
#include "ui_selectthunderbirdfilterfileswidget.h"
#include <QAbstractButton>

SelectThunderbirdFilterFilesWidget::SelectThunderbirdFilterFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectThunderbirdFilterFilesWidget)
{
  ui->setupUi(this);
  connect(ui->buttonGroup,SIGNAL(buttonClicked (QAbstractButton*)),SLOT(slotButtonClicked(QAbstractButton*)));
  connect(ui->profiles,SIGNAL(currentIndexChanged(int)),SLOT(slotProfileChanged(int)));
}

SelectThunderbirdFilterFilesWidget::~SelectThunderbirdFilterFilesWidget()
{
  delete ui;
}

void SelectThunderbirdFilterFilesWidget::slotButtonClicked(QAbstractButton*button)
{
  if(button == ui->selectFile) {
    ui->fileUrl->setEnabled(true);
    ui->profiles->setEnabled(false);
    ui->listFile->setEnabled(false);
  } else {
    ui->fileUrl->setEnabled(false);
    ui->profiles->setEnabled(true);
    ui->listFile->setEnabled(true);
  }
}

void SelectThunderbirdFilterFilesWidget::slotProfileChanged(int)
{
    //TODO
}

#include "selectthunderbirdfilterfileswidget.moc"
