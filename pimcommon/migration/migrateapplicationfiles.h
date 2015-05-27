/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef KMMIGRATEKMAIL4CONFIG_H
#define KMMIGRATEKMAIL4CONFIG_H

#include <QObject>
#include <QVector>
#include "migratefileinfo.h"
#include "pimcommon_export.h"
namespace PimCommon
{
class PIMCOMMON_EXPORT MigrateApplicationFiles : public QObject
{
    Q_OBJECT
public:
    explicit MigrateApplicationFiles(QObject *parent = Q_NULLPTR);
    ~MigrateApplicationFiles();

    bool start();
    bool checkIfNecessary();
    void insertMigrateInfo(const MigrateFileInfo &info);

    int version() const;
    void setVersion(int version);

    QString configFileName() const;
    void setConfigFileName(const QString &configFileName);

    int currentConfigVersion() const;
    void setCurrentConfigVersion(int currentConfigVersion);

Q_SIGNALS:
    void migrateDone();

private:
    void writeConfig();
    void migrateFolder(const MigrateFileInfo &info);
    void migrateFile(const MigrateFileInfo &info);
    bool migrateConfig();
    QVector<MigrateFileInfo> mMigrateInfoList;
    QString mConfigFileName;
    int mMigrateApplicationVersion;
    int mCurrentConfigVersion;
};
}
#endif // KMMIGRATEKMAIL4CONFIG_H
