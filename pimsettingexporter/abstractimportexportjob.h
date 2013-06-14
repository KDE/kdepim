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

#ifndef ABSTRACTIMPORTEXPORTJOB_H
#define ABSTRACTIMPORTEXPORTJOB_H

#include "backupmailutil.h"
#include <Akonadi/Collection>

#include <KZip>

class QWidget;
class QProgressDialog;
class ArchiveStorage;
class KArchiveDirectory;
namespace KPIMIdentities {
class Identity;
class IdentityManager;
}

class AbstractImportExportJob : public QObject
{
    Q_OBJECT
public:
    explicit AbstractImportExportJob(QWidget *parent, ArchiveStorage *archiveStorage, BackupMailUtil::BackupTypes typeSelected, int numberOfStep);
    ~AbstractImportExportJob();

    virtual void start() = 0;

    bool wasCanceled() const;

    virtual QString componentName() const = 0;

Q_SIGNALS:
    void info(const QString &);
    void error(const QString &);

protected:
    void backupFile(const QString &filename, const QString &path, const QString &storedName);
    int mergeConfigMessageBox(const QString &configName) const;
    bool overwriteConfigMessageBox(const QString &configName) const;
    Akonadi::Collection::Id convertPathToId(const QString &path);

    KZip *archive();

    QProgressDialog *progressDialog();
    void increaseProgressDialog();
    void createProgressDialog();

    void showInfo(const QString &text);

    QHash<QString, Akonadi::Collection::Id> mHashConvertPathCollectionId;

    BackupMailUtil::BackupTypes mTypeSelected;
    ArchiveStorage *mArchiveStorage;
    KPIMIdentities::IdentityManager *mIdentityManager;
    QWidget *mParent;
    QProgressDialog *mProgressDialog;
    const KArchiveDirectory* mArchiveDirectory;
    int mNumberOfStep;
};

#endif // ABSTRACTIMPORTEXPORTJOB_H
