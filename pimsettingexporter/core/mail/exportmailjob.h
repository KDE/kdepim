/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
#include "pimsettingexporter_export.h"
#include <KSharedConfig>
#include <QDateTime>
#include <time.h>
class QUrl;
class ArchiveStorage;

class PIMSETTINGEXPORTER_EXPORT ExportMailJob : public AbstractImportExportJob
{
public:
    explicit ExportMailJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ExportMailJob();

    void start() Q_DECL_OVERRIDE;

private:
    QUrl subdirPath(const QUrl &url) const;

    void backupTransports();
    void backupResources();
    void backupMails();
    void backupConfig();
    void backupIdentity();
    void backupAkonadiDb();
    void writeDirectory(const QString &path, const QString &relativePath, KZip *mailArchive);
    bool checkProgram();
    QDateTime mArchiveTime;
};

#endif // ExportMailJob_H
