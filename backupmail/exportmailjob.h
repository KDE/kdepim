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

#ifndef ExportMailJob_H
#define ExportMailJob_H

#include "abstractimportexportjob.h"
#include <KSharedConfig>
class KUrl;

namespace Akonadi {
  class AgentInstance;
}

class ExportMailJob : public AbstractImportExportJob
{
public:
  explicit ExportMailJob(QWidget *parent, BackupMailUtil::BackupTypes typeSelected, const QString& filename, int numberOfStep);
  ~ExportMailJob();
  void start();

private:
  void startBackup();

  KUrl subdirPath(const KUrl &url ) const;

  void backupTransports();
  void backupResources();
  void backupMails();
  void backupConfig();
  void backupIdentity();
  void backupAkonadiDb();
  void backupNepomuk();
  void writeDirectory(QString path, const QString &relativePath, KZip *mailArchive);
  void storeResources(const QString&identifier, const QString& path);
  KUrl resourcePath(const Akonadi::AgentInstance& agent) const;
  void backupFile(const QString&filename, const QString& path, const QString&storedName);
  bool backupMailData(const KUrl& url, const QString& archivePath);

};

#endif // ExportMailJob_H
