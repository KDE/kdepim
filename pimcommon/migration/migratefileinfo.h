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

#ifndef MIGRATEFILEINFO_H
#define MIGRATEFILEINFO_H

#include <QString>
#include <QStringList>

#include "pimcommon_export.h"
namespace PimCommon
{
class PIMCOMMON_EXPORT MigrateFileInfo
{
public:
    MigrateFileInfo();

    QString type() const;
    void setType(const QString &type);

    QString path() const;
    void setPath(const QString &path);

    bool folder() const;
    void setFolder(bool folder);

    bool isValid() const;

    int version() const;
    void setVersion(int version);

    QStringList filePatterns() const;
    void setFilePatterns(const QStringList &filePatterns);

private:
    QStringList mFilePattern;
    QString mType;
    QString mPath;
    int mVersion;
    bool mFolder;
};
}

#endif // MIGRATEFILEINFO_H
