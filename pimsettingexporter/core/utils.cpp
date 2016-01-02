/*
  Copyright (c) 2012-2016 Montel Laurent <montel@kde.org>

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

#include "utils.h"

#include "MailCommon/MailUtil"
#include "PimCommon/PimUtil"

#include <akonadi/private/xdgbasedirs_p.h>

#include <QSettings>
#include <KConfigGroup>

#include <KSharedConfig>
#include <QTemporaryFile>
#include <KLocalizedString>
#include <KZip>

#include <QDir>
#include <QStandardPaths>

int Utils::currentArchiveVersion()
{
    //Increase it when we add major feature!
    return 2;
}

QString Utils::transportsPath()
{
    return QStringLiteral("transports/");
}

QString Utils::resourcesPath()
{
    return QStringLiteral("resources/");
}

QString Utils::identitiesPath()
{
    return QStringLiteral("identities/");
}

QString Utils::mailsPath()
{
    return QStringLiteral("mails/");
}

QString Utils::configsPath()
{
    return QStringLiteral("configs/");
}

QString Utils::akonadiPath()
{
    return QStringLiteral("akonadi/");
}

QString Utils::dataPath()
{
    return QStringLiteral("data/");
}

QString Utils::calendarPath()
{
    return QStringLiteral("calendar/");
}

QString Utils::addressbookPath()
{
    return QStringLiteral("addressbook/");
}

QString Utils::alarmPath()
{
    return QStringLiteral("alarm/");
}

QString Utils::jotPath()
{
    return QStringLiteral("jot/");
}

QString Utils::notePath()
{
    return QStringLiteral("note/");
}

QString Utils::prefixAkonadiConfigFile()
{
    return QStringLiteral("agent_config_");
}

QString Utils::infoPath()
{
    return QStringLiteral("information/");
}

QString Utils::adaptResourcePath(KSharedConfigPtr resourceConfig, const QString &storedData)
{
    QString newUrl = Utils::resourcePath(resourceConfig);
    if (!newUrl.contains(QDir::homePath())) {
        QFileInfo fileInfo(newUrl);
        fileInfo.fileName();
        //qCDebug(PIMSETTINGEXPORTERCORE_LOG)<<" url "<<url.path();
        QString currentPath = QDir::homePath() + QLatin1Char('/') + storedData;
        newUrl = (currentPath + QLatin1Char('/') + fileInfo.fileName());
        if (!QDir(currentPath).exists()) {
            QDir().mkdir(currentPath);
        }
    }
    if (QFile(newUrl).exists()) {
        QString newFileName = newUrl;
        QFileInfo fileInfo(newFileName);
        for (int i = 0;; ++i) {
            const QString currentPath = fileInfo.path() + QLatin1Char('/') + QString::number(i) + QLatin1Char('/');
            newFileName = currentPath + fileInfo.fileName();
            if (!QFile(newFileName).exists()) {
                QDir().mkdir(currentPath);
                break;
            }
        }
        newUrl = newFileName;
    }
    return newUrl;
}

QString Utils::resourcePath(KSharedConfigPtr resourceConfig, const QString &defaultPath)
{
    KConfigGroup group = resourceConfig->group(QStringLiteral("General"));
    QString url = group.readEntry(QStringLiteral("Path"), defaultPath);
    if (!url.isEmpty()) {
        url.replace(QLatin1String("$HOME"), QDir::homePath());
    }
    return url;
}

void Utils::convertCollectionIdsToRealPath(KConfigGroup &group, const QString &currentKey)
{
    if (group.hasKey(currentKey)) {
        const QStringList value = group.readEntry(currentKey, QStringList());
        QStringList newValue;
        Q_FOREACH (const QString &str, value) {
            bool found = false;
            const int collectionId = str.toInt(&found);
            if (found) {
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionId));
                if (!realPath.isEmpty()) {
                    newValue << realPath;
                }
            }
        }
        group.writeEntry(currentKey, newValue);
    }
}

void Utils::convertCollectionListToRealPath(KConfigGroup &group, const QString &currentKey)
{
    if (group.hasKey(currentKey)) {
        const QStringList listExpension = group.readEntry(currentKey, QStringList());
        if (listExpension.isEmpty()) {
            group.deleteEntry(currentKey);
        } else {
            QStringList result;
            Q_FOREACH (QString collection, listExpension) {
                collection = collection.remove(QLatin1Char('c'));
                bool found = false;
                const int collectionValue = collection.toInt(&found);
                if (found && collectionValue != -1) {
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionValue));
                    if (!realPath.isEmpty()) {
                        result << realPath;
                    }
                }
            }
            if (result.isEmpty()) {
                group.deleteEntry(currentKey);
            } else {
                group.writeEntry(currentKey, result);
            }
        }
    }
}

void Utils::convertCollectionToRealPath(KConfigGroup &group, const QString &currentKey)
{
    if (group.hasKey(currentKey)) {
        QString collectionId = group.readEntry(currentKey);
        if (collectionId.isEmpty()) {
            group.deleteEntry(currentKey);
        } else {
            collectionId = collectionId.remove(QLatin1Char('c'));
            bool found = false;
            const int collectionValue = collectionId.toInt(&found);
            if (found && collectionValue != -1) {
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection(collectionValue));
                group.writeEntry(currentKey, realPath);
            } else {
                group.deleteEntry(currentKey);
            }
        }
    }
}

QString Utils::resourcePath(const Akonadi::AgentInstance &agent, const QString &defaultPath)
{
    const QString agentFileName = agent.identifier() + QLatin1String("rc");
    const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + agentFileName;

    KSharedConfigPtr resourceConfig = KSharedConfig::openConfig(configFileName);
    const QString url = Utils::resourcePath(resourceConfig, defaultPath);
    return url;
}

QString Utils::storeResources(KZip *archive, const QString &identifier, const QString &path)
{
    const QString agentFileName = identifier + QLatin1String("rc");
    const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + agentFileName;
    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "configFileName " << configFileName << "agentFileName " << configFileName;

    KSharedConfigPtr resourceConfig = KSharedConfig::openConfig(configFileName);
    QTemporaryFile tmp;
    tmp.open();
    KConfig *config = resourceConfig->copyTo(tmp.fileName());

    if (identifier.contains(POP3_RESOURCE_IDENTIFIER)) {
        const QString targetCollection = QStringLiteral("targetCollection");
        KConfigGroup group = config->group("General");
        if (group.hasKey(targetCollection)) {
            group.writeEntry(targetCollection, MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(targetCollection).toLongLong())));
        }
    } else if (PimCommon::Util::isImapResource(identifier)) {
        const QString trash = QStringLiteral("TrashCollection");
        KConfigGroup group = config->group("cache");
        if (group.hasKey(trash)) {
            group.writeEntry(trash, MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(trash).toLongLong())));
        }
    }
    //Customize resource if necessary here.
    config->sync();
    bool fileAdded  = archive->addLocalFile(tmp.fileName(), path + agentFileName);
    delete config;
    if (!fileAdded) {
        return i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName);
    }

    const QString agentConfigFileName = QLatin1String("agent_config_") + identifier;
    const QString agentConfigFileNamePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/akonadi/") + agentConfigFileName;
    QFile agentConfigFileNameFile(agentConfigFileNamePath);
    if (agentConfigFileNameFile.exists()) {
        fileAdded  = archive->addLocalFile(agentConfigFileNamePath, path + agentConfigFileName);
        if (!fileAdded) {
            return i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName);
        }
    }

    return QString();
}

QString Utils::akonadiAgentConfigPath(const QString &identifier)
{
    const QString relativeFileName = QStringLiteral("akonadi/%1%2").arg(Utils::prefixAkonadiConfigFile()).arg(identifier);
    const QString configFile = Akonadi::XdgBaseDirs::findResourceFile("config", relativeFileName);
    if (!configFile.isEmpty()) {
        return configFile;
    }
    return configFile;
}

QString Utils::akonadiAgentName(const QString &configPath)
{
    QSettings settings(configPath, QSettings::IniFormat);
    const QString name = settings.value(QStringLiteral("Agent/Name")).toString();
    return name;
}

KZip *Utils::openZip(const QString &filename, QString &errorMsg)
{
    KZip *zip = new KZip(filename);
    const bool result = zip->open(QIODevice::ReadOnly);
    if (!result) {
        errorMsg = i18n("Archive cannot be opened in read mode.");
        delete zip;
        return Q_NULLPTR;
    }
    return zip;
}

void Utils::addVersion(KZip *archive)
{
    QTemporaryFile tmp;
    tmp.open();
    const bool fileAdded  = archive->addLocalFile(tmp.fileName(), Utils::infoPath() + QStringLiteral("VERSION_%1").arg(currentArchiveVersion()));
    if (!fileAdded) {
        //TODO add i18n ?
        qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "version file can not add to archive";
    }
}

int Utils::archiveVersion(KZip *archive)
{
    const KArchiveEntry *informationFile = archive->directory()->entry(Utils::infoPath() + QLatin1String("VERSION_1"));
    if (informationFile && informationFile->isFile()) {
        return 1;
    }
    informationFile = archive->directory()->entry(Utils::infoPath() + QLatin1String("VERSION_2"));
    if (informationFile && informationFile->isFile()) {
        return 2;
    }
    //TODO add more version when new version
    return 0;
}

QString Utils::appTypeToI18n(AppsType type)
{
    switch (type) {
    case KMail:
        return i18n("KMail");
    case KAddressBook:
        return i18n("KAddressBook");
    case KAlarm:
        return i18n("KAlarm");
    case KOrganizer:
        return i18n("KOrganizer");
    case KNotes:
        return i18n("KNotes");
    case Akregator:
        return i18n("Akregator");
    case Blogilo:
        return i18n("Blogilo");
    case Unknown:
        break;
    }
    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " type unknown " << type;
    return QString();
}

QString Utils::storedTypeToI18n(StoredType type)
{
    switch (type) {
    case None:
        return QString();
    case Identity:
        return i18n("Identity");
    case Mails:
        return i18n("Mails");
    case MailTransport:
        return i18n("Mail Transport");
    case Resources:
        return i18n("Resources");
    case Config:
        return i18n("Config");
    case AkonadiDb:
        return i18n("Akonadi Database");
    case Data:
        return i18n("Data");
    }
    qCDebug(PIMSETTINGEXPORTERCORE_LOG) << " type unknown " << type;
    return QString();
}
