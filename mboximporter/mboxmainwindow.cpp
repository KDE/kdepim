/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "mboxmainwindow.h"
#include "mboximportwidget.h"
#include "mboximportkernel.h"
#include "mboximporterinfogui.h"

#include <mailimporter/filtermbox.h>
#include <mailimporter/importmailswidget.h>

#include <MailCommon/MailKernel>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

MBoxMainWindow::MBoxMainWindow(const QString &filename, QWidget *parent)
    : QDialog(parent),
      mFileName(filename)
{
    setWindowTitle(i18n("Import mbox file"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MBoxMainWindow::reject);

    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    MBoxImporterKernel *kernel = new MBoxImporterKernel(this);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget

    mImportWidget = new MBoxImportWidget;
    mainLayout->addWidget(mImportWidget);
    mainLayout->addWidget(buttonBox);

    connect(mImportWidget, &MBoxImportWidget::importMailsClicked, this, &MBoxMainWindow::slotImportMBox);
    resize(800, 600);
}

MBoxMainWindow::~MBoxMainWindow()
{
}

void MBoxMainWindow::slotImportMBox()
{
    MailImporter::FilterInfo *info = new MailImporter::FilterInfo();
    MBoxImporterInfoGui *infoGui = new MBoxImporterInfoGui(mImportWidget);
    info->setFilterInfoGui(infoGui);
    info->setRootCollection(mImportWidget->selectedCollection());
    info->clear(); // Clear info from last time

    info->setStatusMessage(i18n("Import in progress"));
    MailImporter::FilterMBox mbox;
    mbox.setFilterInfo(info);
    info->clear();
    mbox.importMails(QStringList() << mFileName);
    info->setStatusMessage(i18n("Import finished"));
    delete info;
}

