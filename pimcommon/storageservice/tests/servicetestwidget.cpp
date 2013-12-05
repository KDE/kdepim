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


#include "servicetestwidget.h"
#include "pimcommon/storageservice/storageserviceabstract.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>
#include <QTextEdit>



ServiceTestWidget::ServiceTestWidget(PimCommon::StorageServiceAbstract *service,QWidget *parent)
    : QWidget(parent),
      mStorageService(service)
{
    mEdit = new QTextEdit;
    QVBoxLayout *lay = new QVBoxLayout;
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    bar->addAction(QLatin1String("List Folder..."), this, SLOT(slotListFolder()));
    bar->addAction(QLatin1String("Create Folder..."), this, SLOT(slotCreateFolder()));
    bar->addAction(QLatin1String("Account info..."), this, SLOT(slotAccountInfo()));
    lay->addWidget(mEdit);
    setLayout(lay);
}

ServiceTestWidget::~ServiceTestWidget()
{

}

void ServiceTestWidget::slotAccountInfo()
{
    mStorageService->accountInfo();
}

void ServiceTestWidget::slotCreateFolder()
{
    const QString folder = QInputDialog::getText(this,i18n("Folder Name"), i18n("Folder:"));
    if (!folder.isEmpty()) {
        mStorageService->createFolder(folder);
    }
}

void ServiceTestWidget::slotListFolder()
{
    mStorageService->listFolder();
}
