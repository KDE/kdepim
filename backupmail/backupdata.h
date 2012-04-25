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

#include <QObject>
#include "util.h"
class KZip;
namespace KPIMIdentities {
  class Identity;
  class IdentityManager;
}

class BackupData : public QObject
{
  Q_OBJECT
public:
  explicit BackupData(Util::BackupTypes typeSelected);
  ~BackupData();
Q_SIGNALS:
  void info(const QString&);
  void error(const QString&);
private:
  void backupTransports();
  void backupResources();
  void backupMails();
  void backupConfig();
  void backupIdentity();

  qint64 writeFile(const char* data, qint64 len);
  KZip *mArchive;
  KPIMIdentities::IdentityManager *mIdentityManager;

};

#endif // BACKUPDATA_H
