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

#include "backupmailwindow.h"
#include "backupmailwidget.h"
#include "exportmailjob.h"
#include "importmailjob.h"
#include "backupmailkernel.h"
#include "selectiontypedialog.h"
#include "backupmailutil.h"
#include "archivestorage.h"

#include <mailcommon/mailkernel.h>

#include <Akonadi/Control>

#include <KStandardAction>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMessageBox>
#include <KStandardDirs>

#include <KLocale>

BackupMailWindow::BackupMailWindow(QWidget *parent)
  : KXmlGuiWindow(parent),mBackupData(0),mRestoreData(0)
{
  KGlobal::locale()->insertCatalog( "libmailcommon" );
  KGlobal::locale()->insertCatalog( "libpimcommon" );

  BackupMailKernel *kernel = new BackupMailKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  bool canZipFile = canZip();
  setupActions(canZipFile);
  setupGUI(Keys | StatusBar | Save | Create,"backupmail.rc");
  mBackupMailWidget = new BackupMailWidget(this);

  setCentralWidget(mBackupMailWidget);
  resize( 640, 480 );
  Akonadi::Control::widgetNeedsAkonadi(this);
  if(!canZipFile) {
    KMessageBox::error(this,i18n("Zip program not found. Install it before to launch this application."),i18n("Zip program not found."));
  }
}

BackupMailWindow::~BackupMailWindow()
{
  delete mRestoreData;
  delete mBackupData;
}

void BackupMailWindow::setupActions(bool canZipFile)
{
  KActionCollection* ac=actionCollection();

  KAction *backupAction = ac->addAction("backup",this,SLOT(slotBackupData()));
  backupAction->setText(i18n("Back Up Data..."));
  backupAction->setEnabled(canZipFile);

  KAction *restoreAction = ac->addAction("restore",this,SLOT(slotRestoreData()));
  restoreAction->setText(i18n("Restore Data..."));
  restoreAction->setEnabled(canZipFile);

  KStandardAction::quit( this, SLOT(close()), ac );
}

void BackupMailWindow::slotBackupData()
{
  if(KMessageBox::warningYesNo(this,i18n("Before to backup data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
    return;

  const QString filename = KFileDialog::getSaveFileName(KUrl("kfiledialog:///backupMail"),QLatin1String("*.zip"),this,i18n("Create backup"),KFileDialog::ConfirmOverwrite);
  if(filename.isEmpty())
    return;
  SelectionTypeDialog *dialog = new SelectionTypeDialog(this);
  if(dialog->exec()) {
    int numberOfStep = 0;
    BackupMailUtil::BackupTypes typeSelected = dialog->backupTypesSelected(numberOfStep);
    delete dialog;
    mBackupMailWidget->clear();
    delete mBackupData;

    ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
    if(!archiveStorage->openArchive(true)) {
        delete archiveStorage;
        return;
    }

    mBackupData = new ExportMailJob(this,typeSelected,archiveStorage,numberOfStep);
    connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
    connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
    mBackupData->start();
    delete mBackupData;
    mBackupData = 0;
    archiveStorage->closeArchive();
    delete archiveStorage;

  } else {
    delete dialog;
  }
}

void BackupMailWindow::slotAddInfo(const QString& info)
{
  mBackupMailWidget->addInfoLogEntry(info);
}

void BackupMailWindow::slotAddError(const QString& info)
{
  mBackupMailWidget->addErrorLogEntry(info);
}


void BackupMailWindow::slotRestoreData()
{
  if(KMessageBox::warningYesNo(this,i18n("Before to restore data, close all kdepim applications. Do you want to continue?"),i18n("Backup"))== KMessageBox::No)
    return;
  const QString filename = KFileDialog::getOpenFileName(KUrl("kfiledialog:///backupMail"),QLatin1String("*.zip"),this,i18n("Restore backup"));
  if(filename.isEmpty())
    return;

  SelectionTypeDialog *dialog = new SelectionTypeDialog(this);
  if(dialog->exec()) {
    int numberOfStep = 0;
    BackupMailUtil::BackupTypes typeSelected = dialog->backupTypesSelected(numberOfStep);
    delete dialog;
    mBackupMailWidget->clear();
    delete mRestoreData;

    ArchiveStorage *archiveStorage = new ArchiveStorage(filename,this);
    if(!archiveStorage->openArchive(false)) {
        delete archiveStorage;
        return;
    }

    mRestoreData = new ImportMailJob(this,typeSelected,archiveStorage, numberOfStep);
    connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
    connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
    mRestoreData->start();
    delete mRestoreData;
    mRestoreData = 0;
    archiveStorage->closeArchive();
    delete archiveStorage;
  } else {
    delete dialog;
  }
}

bool BackupMailWindow::canZip() const
{
  const QString zip = KStandardDirs::findExe( "zip" );
  if(zip.isEmpty()) {
    return false;
  }
  return true;
}

#include "backupmailwindow.moc"
