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

#ifndef ExportMailJob_H
#define ExportMailJob_H

#include "abstractimportexportjob.h"
#include <KSharedConfig>
class KUrl;
class ArchiveStorage;

namespace Akonadi {
class AgentInstance;
}

class ExportMailJob : public AbstractImportExportJob
{
public:
    explicit ExportMailJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ExportMailJob();

    void start();
    QString componentName() const;

private:
    KUrl subdirPath(const KUrl &url ) const;

    void backupTransports();
    void backupResources();
    void backupMails();
    void backupConfig();
    void backupIdentity();
    void backupAkonadiDb();
    void backupNepomuk();
    void writeDirectory(const QString &path, const QString &relativePath, KZip *mailArchive);
    bool backupMailData(const KUrl &url, const QString &archivePath);
    bool checkProgram();
};

#endif // ExportMailJob_H
