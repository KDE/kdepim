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
#include <KSharedConfig>
#include <QDateTime>
#include <time.h>
class ArchiveStorage;

class ExportMailJob : public AbstractImportExportJob
{
    Q_OBJECT
public:
    explicit ExportMailJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ExportMailJob();

    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void taskCanceled();

private Q_SLOTS:
    void slotCheckBackupIdentity();
    void slotCheckBackupMailTransport();
    void slotCheckBackupConfig();
    void slotCheckBackupMails();
    void slotCheckBackupResources();
    void slotMailsJobTerminated();
    void slotWriteNextArchiveResource();
    void slotTaskCanceled();
private:
    bool checkBackupType(Utils::StoredType type) const;
    void backupTransports();
    void backupResources();
    void backupConfig();
    void backupIdentity();
    //void backupAkonadiDb();
    //bool checkProgram();
    QDateTime mArchiveTime;
    int mIndexIdentifier;
};

#endif // ExportMailJob_H
