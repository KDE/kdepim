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
  connect(ui->resources,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
  connect(ui->mailtransport,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
  connect(ui->config,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
  connect(ui->identity,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
  connect(ui->mails,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
  connect(ui->akonadi,SIGNAL(clicked(bool)),SLOT(slotTypeClicked()));
}

SelectTypeWidget::~SelectTypeWidget()
{
  delete ui;
}

void SelectTypeWidget::slotTypeClicked()
{
  const bool selected = (ui->resources->isChecked() ||
                         ui->mailtransport->isChecked() ||
                         ui->config->isChecked() ||
                         ui->identity->isChecked() ||
                         ui->mails->isChecked() ||
                         ui->akonadi->isChecked());
  Q_EMIT itemSelected(selected);
}

BackupMailUtil::BackupTypes SelectTypeWidget::backupTypesSelected() const
{
  BackupMailUtil::BackupTypes types = BackupMailUtil::None;
  if(ui->resources->isChecked())
  {
    types|= BackupMailUtil::Resources;
  }
  if(ui->mailtransport->isChecked())
  {
    types|= BackupMailUtil::MailTransport;
  }
  if(ui->config->isChecked())
  {
    types|= BackupMailUtil::Config;
  }
  if(ui->identity->isChecked())
  {
    types|= BackupMailUtil::Identity;
  }
  if(ui->mails->isChecked())
  {
    types|= BackupMailUtil::Mails;
  }
  if(ui->akonadi->isChecked())
  {
    types|= BackupMailUtil::AkonadiDb;
  }
  return types;
}


#include "selecttypewidget.moc"
