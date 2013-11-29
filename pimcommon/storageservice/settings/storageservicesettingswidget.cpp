/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "storageservicesettingswidget.h"
#include "addservicestoragedialog.h"
#include <KLocale>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QPointer>

using namespace PimCommon;

StorageServiceSettingsWidget::StorageServiceSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;

    QVBoxLayout *vlay = new QVBoxLayout;

    mListService = new QListWidget;
    vlay->addWidget(mListService);

    QHBoxLayout *hlay = new QHBoxLayout;

    mAddService = new QPushButton(i18n("Add..."));
    connect(mAddService, SIGNAL(clicked()), this, SLOT(slotAddService()));
    hlay->addWidget(mAddService);

    mRemoveService = new QPushButton(i18n("Remove"));
    connect(mRemoveService, SIGNAL(clicked()), this, SLOT(slotRemoveService()));
    hlay->addWidget(mRemoveService);

    vlay->addLayout(hlay);


    mainLayout->addLayout(vlay);

    mDescription = new QLabel;
    mainLayout->addWidget(mDescription);
    setLayout(mainLayout);
    connect(mListService, SIGNAL(itemSelectionChanged()), this, SLOT(slotServiceSelected()));
}

StorageServiceSettingsWidget::~StorageServiceSettingsWidget()
{

}

void StorageServiceSettingsWidget::slotRemoveService()
{

}

void StorageServiceSettingsWidget::slotAddService()
{
    QPointer<AddServiceStorageDialog> dlg = new AddServiceStorageDialog(this);
    if (dlg->exec()) {
        //TODO
    }
    delete dlg;
}

void StorageServiceSettingsWidget::slotServiceSelected()
{
    //TODO
}

void StorageServiceSettingsWidget::loadConfig()
{

}

void StorageServiceSettingsWidget::writeConfig()
{

}

