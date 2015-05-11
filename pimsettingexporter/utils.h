/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef UTILS_H
#define UTILS_H
#include "pimsettingexporter_export.h"
#include <QString>
#include <KUrl>
#include <KSharedConfig>
#include <QDebug>
class KZip;
namespace Akonadi {
class AgentInstance;
}


struct resourceFiles
{
    QString akonadiConfigFile;
    QString akonadiResources;
    QString akonadiAgentConfigFile;
    void debug() const {
        qDebug() <<" akonadiconfigfile :"<<akonadiConfigFile<<" akonadiResources:"<<akonadiResources<<" akonadiAgentConfigFile:"<<akonadiAgentConfigFile;
    }
};

namespace Utils {
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
    KJots,
    KNotes,
    Akregator,
    Blogilo,
    KNode
};

struct importExportParameters
{
    importExportParameters()
        : numberSteps(0),
          types(None)
    {

    }
    bool isEmpty() const {
        return (types == None);
    }
    int numberSteps;
    Utils::StoredTypes types;
};


KUrl resourcePath(KSharedConfigPtr resourceConfig, const QString &defaultPath = QString());
PIMSETTINGEXPORT_EXPORT QString transportsPath();
PIMSETTINGEXPORT_EXPORT QString resourcesPath();
PIMSETTINGEXPORT_EXPORT QString identitiesPath();
PIMSETTINGEXPORT_EXPORT QString mailsPath();
PIMSETTINGEXPORT_EXPORT QString configsPath();
PIMSETTINGEXPORT_EXPORT QString akonadiPath();
PIMSETTINGEXPORT_EXPORT QString dataPath();
PIMSETTINGEXPORT_EXPORT QString calendarPath();
PIMSETTINGEXPORT_EXPORT QString addressbookPath();
PIMSETTINGEXPORT_EXPORT QString alarmPath();
PIMSETTINGEXPORT_EXPORT QString jotPath();
PIMSETTINGEXPORT_EXPORT QString infoPath();
PIMSETTINGEXPORT_EXPORT QString notePath();
PIMSETTINGEXPORT_EXPORT QString prefixAkonadiConfigFile();
PIMSETTINGEXPORT_EXPORT QString akonadiAgentName(const QString &configPath);

PIMSETTINGEXPORT_EXPORT void convertCollectionListToRealPath(KConfigGroup &group, const QString &currentKey);
PIMSETTINGEXPORT_EXPORT void convertCollectionToRealPath(KConfigGroup &group, const QString &currentKey);

PIMSETTINGEXPORT_EXPORT void convertCollectionIdsToRealPath(KConfigGroup &group, const QString &currentKey);

PIMSETTINGEXPORT_EXPORT KUrl resourcePath(const Akonadi::AgentInstance &agent, const QString &defaultPath = QString());
PIMSETTINGEXPORT_EXPORT KUrl adaptResourcePath(KSharedConfigPtr resourceConfig, const QString &storedData);
PIMSETTINGEXPORT_EXPORT QString storeResources(KZip *archive, const QString &identifier, const QString &path);
PIMSETTINGEXPORT_EXPORT KUrl akonadiAgentConfigPath(const QString &identifier);
PIMSETTINGEXPORT_EXPORT KZip *openZip(const QString &filename, QString &errorMsg);

PIMSETTINGEXPORT_EXPORT void addVersion(KZip *archive);
PIMSETTINGEXPORT_EXPORT int archiveVersion(KZip *archive);

PIMSETTINGEXPORT_EXPORT int currentArchiveVersion();
PIMSETTINGEXPORT_EXPORT QString appTypeToI18n(AppsType type);
PIMSETTINGEXPORT_EXPORT QString storedTypeToI18n(StoredType type);
}
#endif // UTILS_H
