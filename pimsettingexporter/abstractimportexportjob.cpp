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

#include "pimcommon/util/createresource.h"

#include <kpimidentities/identitymanager.h>
#include <KZip>
#include <KTempDir>
#include <KLocale>
#include <KMessageBox>

#include <QWidget>
#include <QProgressDialog>
#include <QFile>
#include <QDir>

AbstractImportExportJob::AbstractImportExportJob(QWidget *parent, ArchiveStorage *archiveStorage, Utils::StoredTypes typeSelected, int numberOfStep)
    : QObject(parent),
      mTypeSelected(typeSelected),
      mArchiveStorage(archiveStorage),
      mIdentityManager(new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" )),
      mParent(parent),
      mTempDir(0),
      mProgressDialog(0),
      mArchiveDirectory(0),
      mNumberOfStep(numberOfStep),
      mCreateResource(0)
{
}

AbstractImportExportJob::~AbstractImportExportJob()
{
    delete mCreateResource;
    delete mIdentityManager;
    delete mTempDir;
    delete mProgressDialog;
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

void AbstractImportExportJob::convertRealPathToCollection(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix)
{
    if (group.hasKey(currentKey)) {
        const QString path = group.readEntry(currentKey);
        if (!path.isEmpty()) {
            const Akonadi::Collection::Id id = convertPathToId(path);
            if (id != -1) {
                if (addCollectionPrefix) {
                    group.writeEntry(currentKey, QString::fromLatin1("c%1").arg(id));
                } else {
                    group.writeEntry(currentKey, id);
                }
            } else {
                group.deleteEntry(currentKey);
            }
        }
    }
}

void AbstractImportExportJob::convertRealPathToCollectionList(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix)
{
    if (group.hasKey(currentKey)) {
        const QStringList listExpension = group.readEntry(currentKey, QStringList());
        QStringList result;
        if (!listExpension.isEmpty()) {
            Q_FOREACH (const QString &collection, listExpension) {
                const Akonadi::Collection::Id id = convertPathToId(collection);
                if (id != -1 ) {
                    if (addCollectionPrefix) {
                        result<< QString::fromLatin1("c%1").arg(id);
                    } else {
                        result<< QString::fromLatin1("%1").arg(id);
                    }
                }
            }
            if (result.isEmpty()) {
                group.deleteEntry(currentKey);
            } else {
                group.writeEntry(currentKey, result);
            }
        }
    }
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

void AbstractImportExportJob::initializeImportJob()
{
    mTempDir = new KTempDir();
    mTempDirName = mTempDir->name();
    mCreateResource = new PimCommon::CreateResource();
    connect(mCreateResource,SIGNAL(createResourceInfo(QString)),SIGNAL(info(QString)));
    connect(mCreateResource,SIGNAL(createResourceError(QString)),SIGNAL(error(QString)));
}

void AbstractImportExportJob::copyToFile(const KArchiveFile *archivefile, const QString &dest, const QString &filename, const QString &prefix)
{
    QDir dir(mTempDirName);
    dir.mkdir(prefix);

    const QString copyToDirName(mTempDirName + QLatin1Char('/') + prefix);
    archivefile->copyTo(copyToDirName);
    QFile file;
    file.setFileName(copyToDirName + QLatin1Char('/') + filename);

    //QFile doesn't overwrite => remove old file before
    QFile destination(dest);
    if (destination.exists()) {
        destination.remove();
    }
    if (!file.copy(dest)) {
        KMessageBox::error(mParent,i18n("File \"%1\" can not be copied to \"%2\".",filename,dest),i18n("Copy file"));
    }
}


#include "abstractimportexportjob.moc"
