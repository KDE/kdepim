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

#include "mboxmainwindow.h"
#include "mboximportwidget.h"
#include "mboximportkernel.h"
#include "mboximporterinfogui.h"

#include <mailimporter/filter_mbox.h>
#include <mailimporter/importmailswidget.h>

#include <mailcommon/kernel/mailkernel.h>

#include <KLocale>

MBoxMainWindow::MBoxMainWindow(const QString &filename, QWidget *parent)
    : KDialog(parent),
      mFileName(filename)
{
    setCaption( i18n( "Import mbox file" ) );
    setButtons( Close );

    setDefaultButton( Close );

    MBoxImporterKernel *kernel = new MBoxImporterKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    mImportWidget = new MBoxImportWidget;
    connect(mImportWidget, SIGNAL(importMailsClicked()), this, SLOT(slotImportMBox()));
    setMainWidget( mImportWidget );
    resize( 800, 600 );
}

MBoxMainWindow::~MBoxMainWindow()
{
}

void MBoxMainWindow::slotImportMBox()
{
    MailImporter::FilterInfo *info = new MailImporter::FilterInfo();
    MBoxImporterInfoGui *infoGui = new MBoxImporterInfoGui(mImportWidget);
    info->setFilterInfoGui(infoGui);
    info->setRootCollection( mImportWidget->selectedCollection() );
    info->clear(); // Clear info from last time

    info->setStatusMessage(i18n("Import in progress"));
    MailImporter::FilterMBox mbox;
    mbox.setFilterInfo( info );
    info->clear();
    mbox.importMails(QStringList()<<mFileName);
    info->setStatusMessage(i18n("Import finished"));
    delete info;
}

#include "mboxmainwindow.moc"
