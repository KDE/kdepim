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

#ifndef BACKUPDATA_H
#define BACKUPDATA_H

#include "abstractdata.h"
#include <KSharedConfig>
class KUrl;

namespace Akonadi {
  class AgentInstance;
}

class BackupData : public AbstractData
{
public:
  explicit BackupData(QWidget *parent,BackupMailUtil::BackupTypes typeSelected,const QString& filename);
  ~BackupData();
  void startBackup();

private:
  void backupTransports();
  void backupResources();
  void backupMails();
  void backupConfig();
  void backupIdentity();
  void backupAkonadiDb();
  void backupNepomuk();
  void storeResources(const QString&identifier, const QString& path);
  KUrl resourcePath(const Akonadi::AgentInstance& agent) const;
  void backupFile(const QString&filename, const QString& path, const QString&storedName);

};

#endif // BACKUPDATA_H
