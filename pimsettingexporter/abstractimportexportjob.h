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

#include "utils.h"
#include <Akonadi/Collection>
#include <QStringList>

class QWidget;
class QProgressDialog;
class ArchiveStorage;
class KArchiveDirectory;
class KTempDir;
class KZip;
class KArchiveFile;
class KArchiveEntry;

namespace KPIMIdentities {
class Identity;
class IdentityManager;
}

namespace PimCommon {
class CreateResource;
}

class AbstractImportExportJob : public QObject
{
    Q_OBJECT
public:
    explicit AbstractImportExportJob(QWidget *parent, ArchiveStorage *archiveStorage, Utils::StoredTypes typeSelected, int numberOfStep);
    ~AbstractImportExportJob();

    virtual void start() = 0;

    bool wasCanceled() const;

    static int archiveVersion();
    static void setArchiveVersion(int version);

Q_SIGNALS:
    void info(const QString &);
    void error(const QString &);
    void title(const QString &);
    void endLine();

    void jobFinished();

protected:
    void infoAboutNewResource(const QString &resourceName);
    void copyToDirectory(const KArchiveEntry *entry, const QString &dest);
    void extractZipFile(const KArchiveFile *file, const QString &source, const QString &destination);

    void convertRealPathToCollection(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix=false);
    void convertRealPathToCollectionList(KConfigGroup &group, const QString &currentKey, bool addCollectionPrefix=true);
    void copyToFile(const KArchiveFile * archivefile, const QString &dest, const QString &filename, const QString &prefix);
    void initializeImportJob();
    void backupFile(const QString &filename, const QString &path, const QString &storedName);
    void backupResourceDirectory(const Akonadi::AgentInstance &agent, const QString &defaultPath);
    void backupConfigFile(const QString &configFileName);
    int mergeConfigMessageBox(const QString &configName) const;
    bool overwriteConfigMessageBox(const QString &configName) const;
    Akonadi::Collection::Id convertPathToId(const QString &path);
    void backupResourceFile(const Akonadi::AgentInstance &agent, const QString &defaultPath);
    void restoreResourceFile(const QString &resourceName, const QString &defaultPath, const QString &storePath, bool overwriteResources = false);
    bool backupFullDirectory(const KUrl &url, const QString &archivePath, const QString &archivename);
    virtual void addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings);
    void restoreConfigFile(const QString &configNameStr);
    bool overwriteDirectoryMessageBox(const QString &directory) const;
    void overwriteDirectory(const QString &path, const KArchiveEntry *entry);


    KZip *archive();

    QProgressDialog *progressDialog();
    void increaseProgressDialog();
    void createProgressDialog();

    void showInfo(const QString &text);

    QHash<QString, Akonadi::Collection::Id> mHashConvertPathCollectionId;
    QList<resourceFiles> mListResourceFile;

    QString mTempDirName;
    Utils::StoredTypes mTypeSelected;
    ArchiveStorage *mArchiveStorage;
    KPIMIdentities::IdentityManager *mIdentityManager;
    QWidget *mParent;
    KTempDir *mTempDir;
    QProgressDialog *mProgressDialog;
    const KArchiveDirectory* mArchiveDirectory;
    int mNumberOfStep;
    PimCommon::CreateResource *mCreateResource;
    QStringList mAgentPaths;
    static int sArchiveVersion;
};

#endif // ABSTRACTIMPORTEXPORTJOB_H
