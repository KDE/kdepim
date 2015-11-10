/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef IMPORTADDRESSBOOKJOB_H
#define IMPORTADDRESSBOOKJOB_H

#include "abstractimportexportjob.h"

class ArchiveStorage;
class KArchiveFile;

class ImportAddressbookJob : public AbstractImportExportJob
{
    Q_OBJECT
public:
    explicit ImportAddressbookJob(QObject *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ImportAddressbookJob();

    void start() Q_DECL_OVERRIDE;

protected Q_SLOTS:
    void slotNextStep() Q_DECL_OVERRIDE;

private:
    void importSubdirectory(const QString &subdirectoryRelativePath);
    void searchAllFiles(const KArchiveDirectory *dir, const QString &prefix);
    void storeAddressBookArchiveResource(const KArchiveDirectory *dir, const QString &prefix);
    void importkaddressBookConfig(const KArchiveFile *file, const QString &config, const QString &filename, const QString &prefix);
    void restoreResources();
    void restoreConfig();
    void addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings) Q_DECL_OVERRIDE;
};

#endif // IMPORTADDRESSBOOKJOB_H
