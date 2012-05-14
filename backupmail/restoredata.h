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

#ifndef RESTOREDATA_H
#define RESTOREDATA_H

#include "abstractdata.h"
#include <QStringList>
#include <QHash>

class KArchiveDirectory;

class RestoreData : public AbstractData
{
public:
  explicit RestoreData(BackupMailUtil::BackupTypes typeSelected, const QString &filename);
  ~RestoreData();
  void startRestore();
private:
  void restoreTransports();
  void restoreResources();
  void restoreMails();
  void restoreConfig();
  void restoreIdentity();
  void restoreAkonadiDb();
  void restoreNepomuk();
  QHash<QString, QString> mHashIdentity;
  QHash<QString, QString> mHashTransport;
  QHash<QString, QString> mHashResources;
  QStringList mFileList;
  const KArchiveDirectory* mArchiveDirectory;
};

#endif // RESTOREDATA_H
