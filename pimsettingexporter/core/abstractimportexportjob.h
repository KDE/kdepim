/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef ABSTRACTIMPORTEXPORTJOB_H
#define ABSTRACTIMPORTEXPORTJOB_H

#include <QObject>
#include "utils.h"
#include <AkonadiCore/Collection>
#include "pimsettingexporter_export.h"
#include <QStringList>
#include <QUrl>

class ArchiveStorage;
class KArchiveDirectory;
class QTemporaryDir;
class KZip;
class KArchiveFile;
class KArchiveEntry;

namespace KIdentityManagement
{
class Identity;
class IdentityManager;
}

namespace PimCommon
{
class CreateResource;
}
class ImportExportProgressIndicatorBase;
class PIMSETTINGEXPORTER_EXPORT AbstractImportExportJob : public QObject
{
    Q_OBJECT
public:
    explicit AbstractImportExportJob(QObject *parent, ArchiveStorage *archiveStorage, Utils::StoredTypes typeSelected, int numberOfStep);
    ~AbstractImportExportJob();

    virtual void start() = 0;

    bool wasCanceled() const;

    static int archiveVersion();
    static void setArchiveVersion(int version);

    void setImportExportProgressIndicator(ImportExportProgressIndicatorBase *importExportProgressIndicator);

    ImportExportProgressIndicatorBase *importExportProgressIndicator() const;

Q_SIGNALS:
    void info(const QString &);
    void error(const QString &);
    void title(const QString &);
    void endLine();
    void needSynchronizeResource(const QString &name, const QString &identifier);
    void jobFinished();

private Q_SLOTS:
    void slotAllResourceSynchronized();
    void slotSynchronizeInstanceDone(const QString &, const QString &identifier);
    void slotSynchronizeInstanceFailed(const QString &instance);
    void slotTaskCanceled();

protected:
    virtual void slotNextStep();

protected:
    void initializeListStep();
    void startSynchronizeResources(const QStringList &listResourceToSync);
    void infoAboutNewResource(const QString &resourceName);
    void copyToDirectory(const KArchiveEntry *entry, const QString &dest);
    void extractZipFile(const KArchiveFile *file, const QString &source, const QString &destination, bool isStoredAsZippedArchive = true);

    void convertRealPathToCollection(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix = false);
    void convertRealPathToCollectionList(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix = true);
    void copyToFile(const KArchiveFile *archivefile, const QString &dest, const QString &filename, const QString &prefix);
    void initializeImportJob();
    void backupFile(const QString &filename, const QString &path, const QString &storedName);
    void backupConfigFile(const QString &configFileName);
    void backupUiRcFile(const QString &configFileName, const QString &applicationName);
    void restoreUiRcFile(const QString &configNameStr, const QString &applicationName);

    int mergeConfigMessageBox(const QString &configName) const;
    bool overwriteConfigMessageBox(const QString &configName) const;
    Akonadi::Collection::Id convertPathToId(const QString &path);
    void backupResourceFile(const Akonadi::AgentInstance &agent, const QString &defaultPath);
    QStringList restoreResourceFile(const QString &resourceName, const QString &defaultPath, const QString &storePath, bool overwriteResources = false);
    virtual void addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings);
    void restoreConfigFile(const QString &configNameStr);
    bool overwriteDirectoryMessageBox(const QString &directory) const;
    void overwriteDirectory(const QString &path, const KArchiveEntry *entry);
    virtual bool isAConfigFile(const QString &name) const;
    void searchAllFiles(const KArchiveDirectory *dir, const QString &prefix, const QString &searchEntryName);
    void storeArchiveInfoResources(const KArchiveDirectory *dir, const QString &prefix);

    KZip *archive() const;

    void increaseProgressDialog();
    void createProgressDialog(const QString &title = QString());

    void setProgressDialogLabel(const QString &text);
    void storeDirectory(const QString &subDirectory);
    void importDataSubdirectory(const QString &subdirectoryRelativePath);

    QHash<QString, Akonadi::Collection::Id> mHashConvertPathCollectionId;
    QVector<resourceFiles> mListResourceFile;

    QString mTempDirName;
    Utils::StoredTypes mTypeSelected;
    ArchiveStorage *mArchiveStorage;
    KIdentityManagement::IdentityManager *mIdentityManager;
    QTemporaryDir *mTempDir;
    const KArchiveDirectory *mArchiveDirectory;
    int mNumberOfStep;
    PimCommon::CreateResource *mCreateResource;
    QStringList mAgentPaths;
    QList<Utils::StoredType> mListStep;
    int mIndex;
    static int sArchiveVersion;

private:
    ImportExportProgressIndicatorBase *mImportExportProgressIndicator;
};

#endif // ABSTRACTIMPORTEXPORTJOB_H
