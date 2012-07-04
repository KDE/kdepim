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

#include "backupmailwidget.h"

#include "libkdepim/customlogwidget.h"

#include <QHBoxLayout>
#include <QListWidget>


BackupMailWidget::BackupMailWidget(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout *layout = new QHBoxLayout;
  mCustomLogWidget = new KPIM::CustomLogWidget;
  layout->addWidget(mCustomLogWidget);
  setLayout(layout);
}

BackupMailWidget::~BackupMailWidget()
{

}

void BackupMailWidget::clear()
{
  mCustomLogWidget->clear();
}

void BackupMailWidget::addInfoLogEntry( const QString& log )
{
  mCustomLogWidget->addInfoLogEntry(log);
}

void BackupMailWidget::addErrorLogEntry( const QString& log )
{
  mCustomLogWidget->addErrorLogEntry(log);
}

