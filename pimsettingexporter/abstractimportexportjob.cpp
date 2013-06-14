/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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
#include "archivestorage.h"
#include "mailcommon/util/mailutil.h"

#include <kpimidentities/identitymanager.h>
#include <KZip>
#include <KLocale>
#include <KMessageBox>

#include <QWidget>
#include <QProgressDialog>

AbstractImportExportJob::AbstractImportExportJob(QWidget *parent, ArchiveStorage *archiveStorage, BackupMailUtil::BackupTypes typeSelected, int numberOfStep)
    : QObject(parent),
      mTypeSelected(typeSelected),
      mArchiveStorage(archiveStorage),
      mIdentityManager(new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" )),
      mParent(parent),
      mProgressDialog(0),
      mArchiveDirectory(0),
      mNumberOfStep(numberOfStep)
{
}

AbstractImportExportJob::~AbstractImportExportJob()
{
    delete mIdentityManager;
}

QProgressDialog *AbstractImportExportJob::progressDialog()
{
    return mProgressDialog;
}

void AbstractImportExportJob::createProgressDialog()
{
    if (!mProgressDialog) {
        mProgressDialog = new QProgressDialog(mParent);
        mProgressDialog->setWindowModality(Qt::WindowModal);
        mProgressDialog->setMinimum(0);
        mProgressDialog->setMaximum(mNumberOfStep);
    }
    mProgressDialog->show();
    mProgressDialog->setValue(0);
}


bool AbstractImportExportJob::wasCanceled() const
{
    if (mProgressDialog)
        return mProgressDialog->wasCanceled();
    return false;
}

void AbstractImportExportJob::increaseProgressDialog()
{
    if (mProgressDialog) {
        mProgressDialog->setValue(mProgressDialog->value()+1);
    }
}

void AbstractImportExportJob::showInfo(const QString&text)
{
    if (mProgressDialog) {
        mProgressDialog->setLabelText(text);
    }
    Q_EMIT info(text);
}

KZip *AbstractImportExportJob::archive()
{
    return mArchiveStorage->archive();
}

void AbstractImportExportJob::backupFile(const QString&filename, const QString& path, const QString&storedName)
{
    const bool fileAdded  = archive()->addLocalFile(filename, path + storedName);
    if (fileAdded)
        Q_EMIT info(i18n("\"%1\" backup done.",storedName));
    else
        Q_EMIT error(i18n("\"%1\" cannot be exported.",storedName));
}

int AbstractImportExportJob::mergeConfigMessageBox(const QString &configName) const
{
    return KMessageBox::warningYesNoCancel(mParent,i18n("\"%1\" already exists. Do you want to overwrite it or merge it?", configName),i18n("Restore"),KGuiItem(i18n("Overwrite")),KGuiItem(i18n("Merge")) );
}

bool AbstractImportExportJob::overwriteConfigMessageBox(const QString &configName) const
{
    return (KMessageBox::warningYesNo(mParent,i18n("\"%1\" already exists. Do you want to overwrite it?", configName),i18n("Restore")) == KMessageBox::Yes);
}

Akonadi::Collection::Id AbstractImportExportJob::convertPathToId(const QString& path)
{
    if (mHashConvertPathCollectionId.contains(path)) {
        return mHashConvertPathCollectionId.value(path);
    }
    const Akonadi::Collection::Id id = MailCommon::Util::convertFolderPathToCollectionId(path);
    if (id != -1) {
        mHashConvertPathCollectionId.insert(path,id);
    }
    return id;
}

#include "abstractimportexportjob.moc"
