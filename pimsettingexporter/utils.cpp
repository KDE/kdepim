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

#include "utils.h"

#include "mailcommon/util/mailutil.h"

#include <KConfigGroup>
#include <KStandardDirs>
#include <KSharedConfig>
#include <KTemporaryFile>
#include <KLocale>
#include <KZip>

#include <QDir>


QString Utils::transportsPath()
{
    return QLatin1String("transports/");
}

QString Utils::resourcesPath()
{
    return QLatin1String("resources/");
}

QString Utils::identitiesPath()
{
    return QLatin1String("identities/");
}

QString Utils::mailsPath()
{
    return QLatin1String("mails/");
}

QString Utils::configsPath()
{
    return QLatin1String("configs/");
}

QString Utils::akonadiPath()
{
    return QLatin1String("akonadi/");
}

QString Utils::dataPath()
{
    return QLatin1String("data/");
}

QString Utils::calendarPath()
{
    return QLatin1String("calendar/");
}

QString Utils::addressbookPath()
{
    return QLatin1String("addressbook/");
}

QString Utils::alarmPath()
{
    return QLatin1String("alarm/");
}

QString Utils::jotPath()
{
    return QLatin1String("jot/");
}


KUrl Utils::adaptResourcePath(KSharedConfigPtr resourceConfig, const QString &storedData)
{
    const KUrl url = Utils::resourcePath(resourceConfig);
    KUrl newUrl = url;
    if (!url.path().contains(QDir::homePath())) {
        //qDebug()<<" url "<<url.path();
        newUrl.setPath(QDir::homePath() + QLatin1Char('/') + storedData + url.fileName());
    }
    if (QFile(newUrl.path()).exists()) {
        QString newFileName = newUrl.path();
        for (int i = 0;; ++i) {
            newFileName = newUrl.path() + QString::fromLatin1("_%1").arg(i);
            if (!QFile(newFileName).exists()) {
                break;
            }
        }
        newUrl=KUrl(newFileName);
    }
    return newUrl;
}

KUrl Utils::resourcePath(KSharedConfigPtr resourceConfig)
{
    KConfigGroup group = resourceConfig->group(QLatin1String("General"));
    QString url = group.readEntry(QLatin1String("Path"),QString());
    url.replace(QLatin1String("$HOME"), QDir::homePath());
    return KUrl(url);
}

void Utils::convertCollectionIdsToRealPath(KConfigGroup &group, const QString &currentKey)
{
    if (group.hasKey(currentKey)) {
        const QStringList value = group.readEntry(currentKey,QStringList());
        QStringList newValue;
        Q_FOREACH(const QString &str,value) {
            bool found = false;
            const int collectionId = str.toInt(&found);
            if (found) {
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionId ));
                if (!realPath.isEmpty()) {
                    newValue<<realPath;
                }
            }
        }
        group.writeEntry(currentKey,newValue);
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
                    const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionValue ));
                    if (!realPath.isEmpty())
                        result << realPath;
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
                const QString realPath = MailCommon::Util::fullCollectionPath(Akonadi::Collection( collectionValue ));
                group.writeEntry(currentKey,realPath);
            } else {
                group.deleteEntry(currentKey);
            }
        }
    }
}

KUrl Utils::resourcePath(const Akonadi::AgentInstance &agent)
{
    const QString agentFileName = agent.identifier() + QLatin1String("rc");
    const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

    KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
    KUrl url = Utils::resourcePath(resourceConfig);
    return url;
}

QString Utils::storeResources(KZip *archive, const QString &identifier, const QString &path)
{
    const QString agentFileName = identifier + QLatin1String("rc");
    const QString configFileName = KStandardDirs::locateLocal( "config", agentFileName );

    KSharedConfigPtr resourceConfig = KSharedConfig::openConfig( configFileName );
    KTemporaryFile tmp;
    tmp.open();
    KConfig * config = resourceConfig->copyTo( tmp.fileName() );

    if (identifier.contains(QLatin1String("akonadi_pop3_resource"))) {
        const QString targetCollection = QLatin1String("targetCollection");
        KConfigGroup group = config->group("General");
        if (group.hasKey(targetCollection)) {
            group.writeEntry(targetCollection,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(targetCollection).toLongLong())));
        }
    } else if (identifier.contains(QLatin1String("akonadi_imap_resource"))) {
        const QString trash = QLatin1String("TrashCollection");
        KConfigGroup group = config->group("cache");
        if (group.hasKey(trash)) {
            group.writeEntry(trash,MailCommon::Util::fullCollectionPath(Akonadi::Collection(group.readEntry(trash).toLongLong())));
        }
    }
    //Customize resource if necessary here.
    config->sync();
    const bool fileAdded  = archive->addLocalFile(tmp.fileName(), path + agentFileName);
    if (!fileAdded)
        return i18n("Resource file \"%1\" cannot be added to backup file.", agentFileName);
    return QString();
}

