/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef UTILS_H
#define UTILS_H
#include "pimsettingexporter_export.h"
#include <QString>
#include <QUrl>
#include <KSharedConfig>
#include "pimsettingexportcore_debug.h"
class KZip;
namespace Akonadi
{
class AgentInstance;
}

struct resourceFiles {
    QString akonadiConfigFile;
    QString akonadiResources;
    QString akonadiAgentConfigFile;
    void debug() const
    {
        qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " akonadiconfigfile :" << akonadiConfigFile << " akonadiResources:" << akonadiResources << " akonadiAgentConfigFile:" << akonadiAgentConfigFile;
    }
};

namespace Utils
{
enum StoredType {
    None = 0,
    Identity = 1,
    Mails = 2,
    MailTransport = 4,
    Resources = 8,
    Config = 16,
    AkonadiDb = 32,
    Data = 64
           //TODO add more type to import/export
};
Q_DECLARE_FLAGS(StoredTypes, StoredType)

enum AppsType {
    Unknown = 0,
    KMail,
    KAddressBook,
    KAlarm,
    KOrganizer,
    KNotes,
    Akregator,
    Blogilo
};

struct importExportParameters {
    importExportParameters()
        : numberSteps(0),
          types(None)
    {

    }
    bool isEmpty() const
    {
        return (types == None);
    }
    int numberSteps;
    Utils::StoredTypes types;
};

QString resourcePath(KSharedConfigPtr resourceConfig, const QString &defaultPath = QString());
PIMSETTINGEXPORTER_EXPORT QString transportsPath();
PIMSETTINGEXPORTER_EXPORT QString resourcesPath();
PIMSETTINGEXPORTER_EXPORT QString identitiesPath();
PIMSETTINGEXPORTER_EXPORT QString mailsPath();
PIMSETTINGEXPORTER_EXPORT QString configsPath();
PIMSETTINGEXPORTER_EXPORT QString akonadiPath();
PIMSETTINGEXPORTER_EXPORT QString dataPath();
PIMSETTINGEXPORTER_EXPORT QString calendarPath();
PIMSETTINGEXPORTER_EXPORT QString addressbookPath();
PIMSETTINGEXPORTER_EXPORT QString alarmPath();
PIMSETTINGEXPORTER_EXPORT QString jotPath();
PIMSETTINGEXPORTER_EXPORT QString infoPath();
PIMSETTINGEXPORTER_EXPORT QString notePath();
PIMSETTINGEXPORTER_EXPORT QString prefixAkonadiConfigFile();
QString akonadiAgentName(const QString &configPath);

void convertCollectionListToRealPath(KConfigGroup &group, const QString &currentKey);
void convertCollectionToRealPath(KConfigGroup &group, const QString &currentKey);
void convertCollectionIdsToRealPath(KConfigGroup &group, const QString &currentKey);

QString resourcePath(const Akonadi::AgentInstance &agent, const QString &defaultPath = QString());
QString adaptResourcePath(KSharedConfigPtr resourceConfig, const QString &storedData);
QString storeResources(KZip *archive, const QString &identifier, const QString &path);
KZip *openZip(const QString &filename, QString &errorMsg);

void addVersion(KZip *archive);
int archiveVersion(KZip *archive);

int currentArchiveVersion();
PIMSETTINGEXPORTER_EXPORT QString appTypeToI18n(AppsType type);
PIMSETTINGEXPORTER_EXPORT QString storedTypeToI18n(StoredType type);
}
#endif // UTILS_H
