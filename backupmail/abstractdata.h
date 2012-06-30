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

#ifndef ABSTRACTDATA_H
#define ABSTRACTDATA_H

#include "backupmailutil.h"

class KZip;
class QWidget;
namespace KPIMIdentities {
  class Identity;
  class IdentityManager;
}

class AbstractData : public QObject
{
  Q_OBJECT
public:
  explicit AbstractData(QWidget *parent, const QString& filename, BackupMailUtil::BackupTypes typeSelected);
  ~AbstractData();

  virtual void start() = 0;

Q_SIGNALS:
  void info(const QString&);
  void error(const QString&);
protected:
  void closeArchive();
  bool openArchive(bool write);

protected:
  BackupMailUtil::BackupTypes mTypeSelected;
  KZip *mArchive;
  KPIMIdentities::IdentityManager *mIdentityManager;
  QWidget *mParent;
};

#endif // ABSTRACTDATA_H
