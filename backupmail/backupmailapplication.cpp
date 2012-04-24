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

#include "backupmailapplication.h"
#include "backupmailwidget.h"
#include "backupdata.h"
#include "restoredata.h"

#include <KStandardAction>
#include <KAction>
#include <KActionCollection>

#include <KLocale>

BackupMailApplication::BackupMailApplication(QWidget *parent)
  : KXmlGuiWindow(parent),mBackupData(0),mRestoreData(0)
{
  setupActions();
  setupGUI(Default,"backupmailapplication.rc");
  mBackupMailWidget = new BackupMailWidget(this);

  setCentralWidget(mBackupMailWidget);
}

BackupMailApplication::~BackupMailApplication()
{
  if(mBackupData) {
    //TODO close backupData
    delete mBackupData;
  }
  if(mRestoreData) {
    //TODO close it
    delete mRestoreData;
  }
}

void BackupMailApplication::setupActions()
{
  KActionCollection* ac=actionCollection();

  KAction *backupAction = ac->addAction("backup",this,SLOT(slotBackupData()));
  backupAction->setText(i18n("Back Up Data..."));

  KAction *restoreAction = ac->addAction("restore",this,SLOT(slotRestoreData()));
  restoreAction->setText(i18n("Restore Data..."));
  KStandardAction::quit( this, SLOT(close()), ac );
}

void BackupMailApplication::slotBackupData()
{
  mBackupData = new BackupData();
  connect(mBackupData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
  connect(mBackupData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
}

void BackupMailApplication::slotAddInfo(const QString& info)
{
  mBackupMailWidget->addInfoLogEntry(info);
}

void BackupMailApplication::slotAddError(const QString& info)
{
  mBackupMailWidget->addErrorLogEntry(info);
}


void BackupMailApplication::slotRestoreData()
{
  mRestoreData = new RestoreData();
  connect(mRestoreData,SIGNAL(info(QString)),SLOT(slotAddInfo(QString)));
  connect(mRestoreData,SIGNAL(error(QString)),SLOT(slotAddError(QString)));
}


#include "backupmailapplication.moc"
