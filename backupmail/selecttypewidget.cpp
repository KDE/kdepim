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

#include "selecttypewidget.h"
#include "ui_selecttypewidget.h"

SelectTypeWidget::SelectTypeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::SelectTypeWidget)
{
  ui->setupUi(this);
}

SelectTypeWidget::~SelectTypeWidget()
{
  delete ui;
}

Util::BackupTypes SelectTypeWidget::backupTypesSelected() const
{
  Util::BackupTypes types = Util::None;
  if(ui->resources->isChecked())
  {
    types|= Util::Resources;
  }
  if(ui->mailtransport->isChecked())
  {
    types|= Util::MailTransport;
  }
  if(ui->config->isChecked())
  {
    types|= Util::Resources;
  }
  if(ui->identity->isChecked())
  {
    types|= Util::Identity;
  }
  if(ui->mails->isChecked())
  {
    types|= Util::Mails;
  }
  return types;
}


#include "selecttypewidget.moc"
