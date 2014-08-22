/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorconfigureserverwidget.h"
#include "ui_sieveeditorconfigureserverwidget.h"
#include "serversievesettingsdialog.h"

#include <KLocalizedString>
#include <KMessageBox>

SieveEditorConfigureServerWidget::SieveEditorConfigureServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SieveEditorConfigureServerWidget)
{
    ui->setupUi(this);
    connect(ui->modifyServer, &QPushButton::clicked, this, &SieveEditorConfigureServerWidget::slotModifyServer);
    connect(ui->addServer, &QPushButton::clicked, this, &SieveEditorConfigureServerWidget::slotAddServer);
    connect(ui->removeServer, &QPushButton::clicked, this, &SieveEditorConfigureServerWidget::slotDeleteServer);
    connect(ui->serverSieveListWidget, &ServerSieveListWidget::itemSelectionChanged, this, &SieveEditorConfigureServerWidget::slotItemSelectionChanged);
    slotItemSelectionChanged();
}

SieveEditorConfigureServerWidget::~SieveEditorConfigureServerWidget()
{
    delete ui;
}

void SieveEditorConfigureServerWidget::readConfig()
{
    ui->serverSieveListWidget->readConfig();
}

void SieveEditorConfigureServerWidget::writeConfig()
{
    ui->serverSieveListWidget->writeConfig();
}

void SieveEditorConfigureServerWidget::slotModifyServer()
{
    ui->serverSieveListWidget->modifyServerConfig();
}

void SieveEditorConfigureServerWidget::slotAddServer()
{
    ui->serverSieveListWidget->addServerConfig();
}

void SieveEditorConfigureServerWidget::slotDeleteServer()
{
    QListWidgetItem *item = ui->serverSieveListWidget->currentItem();
    if (!item) {
        return;
    }
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18n("Do you want to remove this server \'%1\'?", item->text()), i18n("Remove Server Sieve"))) {
        delete item;
        slotItemSelectionChanged();
    }
}

void SieveEditorConfigureServerWidget::slotItemSelectionChanged()
{
    const bool hasItemSelected = ui->serverSieveListWidget->currentItem();
    ui->modifyServer->setEnabled(hasItemSelected);
    ui->removeServer->setEnabled(hasItemSelected);
}
