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

#ifndef ImportMailJob_H
#define ImportMailJob_H

#include "abstractimportexportjob.h"
#include <Akonadi/Collection>
#include <QStringList>
#include <QHash>

class KArchiveDirectory;
class KArchiveFile;
class KTempDir;
class ArchiveStorage;
namespace MailCommon {
  class CreateResource;
}

class ImportMailJob : public AbstractImportExportJob
{
public:
  explicit ImportMailJob(QWidget *widget, BackupMailUtil::BackupTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
  ~ImportMailJob();
  void start();
private:
  void startRestore();
  void restoreTransports();
  void restoreResources();
  void restoreMails();
  void restoreConfig();
  void restoreIdentity();
  void restoreAkonadiDb();
  void restoreNepomuk();
  void importTemplatesConfig(const KArchiveFile* templatesconfiguration, const QString& templatesconfigurationrc, const QString &filename, const QString &prefix);
  void importKmailConfig(const KArchiveFile* kmailsnippet, const QString& kmail2rc, const QString &filename, const QString &prefix);
  void importArchiveConfig(const KArchiveFile* archiveconfiguration, const QString& archiveconfigurationrc, const QString&filename,const QString& prefix);
  QString createResource( const QString& resources, const QString& name, const QMap<QString, QVariant>& settings );
  void searchAllFiles(const KArchiveDirectory*dir,const QString&prefix);
  void storeMailArchiveResource(const KArchiveDirectory*dir, const QString &prefix);

  void copyToFile(const KArchiveFile * file, const QString& dest, const QString&filename,const QString& prefix);
  void mergeLdapConfig(const KArchiveFile * archivefile, const QString&filename, const QString&prefix);
  void mergeKmailSnippetConfig(const KArchiveFile * archivefile, const QString&filename, const QString&prefix);
  void mergeArchiveMailAgentConfig(const KArchiveFile * archivefile, const QString&filename, const QString&prefix);
  void copyArchiveMailAgentConfigGroup(KSharedConfig::Ptr archiveConfigOrigin, KSharedConfig::Ptr archiveConfigDestination);

  Akonadi::Collection::Id convertPathToId(const QString& path);

  QHash<QString, QString> mHashMailArchive;
  QHash<uint, uint> mHashIdentity;
  QHash<int, int> mHashTransport;
  QHash<QString, QString> mHashResources;
  QHash<QString, Akonadi::Collection::Id> mHashConvertPathCollectionId;
  QStringList mFileList;
  QString mTempDirName;
  const KArchiveDirectory* mArchiveDirectory;
  KTempDir *mTempDir;
  MailCommon::CreateResource *mCreateResource;
};

#endif // ImportMailJob_H
