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

#ifndef BACKUPMAILUTIL_H
#define BACKUPMAILUTIL_H
#include <QString>
#include <KUrl>
#include <KSharedConfig>

namespace Akonadi {
class AgentInstance;
}

namespace BackupMailUtil {
enum BackupType {
    None = 0,
    Identity = 1,
    Mails = 2,
    MailTransport = 4,
    Resources = 8,
    Config = 16,
    AkonadiDb = 32,
    Nepomuk = 64
    //TODO add more type to import/export
};
Q_DECLARE_FLAGS(BackupTypes, BackupType )

KUrl resourcePath(KSharedConfigPtr resourceConfig);
QString transportsPath();
QString resourcesPath();
QString identitiesPath();
QString mailsPath();
QString configsPath();
QString akonadiPath();
QString dataPath();
QString calendarPath();
QString addressbookPath();
QString alarmPath();
void convertCollectionListToRealPath(KConfigGroup &group, const QString &currentKey);
void convertCollectionToRealPath(KConfigGroup &group, const QString &currentKey);
KUrl resourcePath(const Akonadi::AgentInstance &agent);
}
#endif // UTIL_H
