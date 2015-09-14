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

#include "migratefileinfo.h"
using namespace PimCommon;

MigrateFileInfo::MigrateFileInfo()
    : mVersion(-1),
      mFolder(false)
{

}

bool MigrateFileInfo::isValid() const
{
    return !mType.isEmpty() && !mPath.isEmpty();
}

int MigrateFileInfo::version() const
{
    return mVersion;
}

void MigrateFileInfo::setVersion(int version)
{
    mVersion = version;
}

QString MigrateFileInfo::type() const
{
    return mType;
}

void MigrateFileInfo::setType(const QString &type)
{
    mType = type;
}

QString MigrateFileInfo::path() const
{
    return mPath;
}

void MigrateFileInfo::setPath(const QString &path)
{
    mPath = path;
}

bool MigrateFileInfo::folder() const
{
    return mFolder;
}

void MigrateFileInfo::setFolder(bool folder)
{
    mFolder = folder;
}

QStringList MigrateFileInfo::filePatterns() const
{
    return mFilePattern;
}

void MigrateFileInfo::setFilePatterns(const QStringList &filePattern)
{
    mFilePattern = filePattern;
}
