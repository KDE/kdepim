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

#include "backupmailutil.h"
#include <KConfigGroup>
#include <QDir>

QString BackupMailUtil::transportsPath()
{
    return QLatin1String("transports/");
}

QString BackupMailUtil::resourcesPath()
{
    return QLatin1String("resources/");
}

QString BackupMailUtil::identitiesPath()
{
    return QLatin1String("identities/");
}

QString BackupMailUtil::mailsPath()
{
    return QLatin1String("mails/");
}

QString BackupMailUtil::configsPath()
{
    return QLatin1String("configs/");
}

QString BackupMailUtil::akonadiPath()
{
    return QLatin1String("akonadi/");
}

QString BackupMailUtil::dataPath()
{
    return QLatin1String("data/");
}

KUrl BackupMailUtil::resourcePath(KSharedConfigPtr resourceConfig)
{
    KConfigGroup group = resourceConfig->group(QLatin1String("General"));
    QString url = group.readEntry(QLatin1String("Path"),QString());
    url.replace("$HOME", QDir::homePath());
    return KUrl(url);
}
