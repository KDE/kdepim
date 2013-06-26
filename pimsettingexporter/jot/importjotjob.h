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

#ifndef IMPORTJOTJOB_H
#define IMPORTJOTJOB_H

#include "abstractimportexportjob.h"

class ArchiveStorage;
class QWidget;

class ImportJotJob : public AbstractImportExportJob
{
    Q_OBJECT
public:
    explicit ImportJotJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ImportJotJob();

    void start();

private:
    void storeAlarmArchiveResource(const KArchiveDirectory *dir, const QString &prefix);
    void searchAllFiles(const KArchiveDirectory *dir,const QString &prefix);
    void importjotConfig(const KArchiveFile* kalarmFile, const QString &kalarmrc, const QString &filename,const QString &prefix);
    void restoreResources();
    void restoreConfig();
};

#endif // IMPORTJOTJOB_H
