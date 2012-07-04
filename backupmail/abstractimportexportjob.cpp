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

#include "abstractimportexportjob.h"

#include <kpimidentities/identitymanager.h>
#include <KZip>
#include <KLocale>
#include <QWidget>
#include <QProgressDialog>

AbstractImportExportJob::AbstractImportExportJob( QWidget *parent, const QString &filename,BackupMailUtil::BackupTypes typeSelected)
  : QObject(parent),
    mTypeSelected(typeSelected),
    mArchive(new KZip(filename)),
    mIdentityManager(new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" )),
    mParent(parent),
    mProgressDialog(0)
{
}

AbstractImportExportJob::~AbstractImportExportJob()
{
  closeArchive();
  delete mArchive;
  delete mIdentityManager;
  delete mProgressDialog;
}

QProgressDialog *AbstractImportExportJob::progressDialog()
{
  return mProgressDialog;
}

void AbstractImportExportJob::createProgressDialog()
{
  if(!mProgressDialog) {
    mProgressDialog = new QProgressDialog(mParent);
    mProgressDialog->setWindowModality(Qt::WindowModal);
    mProgressDialog->setMinimum(0);
    mProgressDialog->setMaximum(7);
    connect(mProgressDialog, SIGNAL(canceled()), this, SLOT(slotCancel()));
  }
  mProgressDialog->setValue(0);
}

void AbstractImportExportJob::slotCancel()
{
  //TODO
}

void AbstractImportExportJob::increaseProgressDialog(int value)
{
  if(mProgressDialog) {
    mProgressDialog->setValue(value);
  }
}

void AbstractImportExportJob::closeArchive()
{
  if(mArchive && mArchive->isOpen()) {
    mArchive->close();
  }
}

bool AbstractImportExportJob::openArchive(bool write)
{
  bool result = mArchive->open(write ? QIODevice::WriteOnly : QIODevice::ReadOnly);
  if(!result) {
    if(write) {
      Q_EMIT error(i18n("Archive cannot be opened in write mode."));
    } else {
      Q_EMIT error(i18n("Archive cannot be opened in read mode."));
    }
  }
  return result;
}



#include "abstractimportexportjob.moc"
