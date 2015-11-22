/***************************************************************************
                          filters.cxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>                      */

// Local Includes
#include "filters.h"
#include "filterinfo.h"

// Akonadi Includes
#include <CollectionFetchJob>
#include <Item>
#include <ItemCreateJob>
#include <ItemFetchJob>
#include <ItemFetchScope>
#include <CollectionCreateJob>
#include <Akonadi/KMime/MessageParts>
#include <Akonadi/KMime/MessageFlags>

// KDE Includes
#include <QUrl>
#include <KLocalizedString>
#include "mailimporter_debug.h"

#include <QScopedPointer>

//////////////////////////////////////////////////////////////////////////////////
//
// The generic filter class
//
//////////////////////////////////////////////////////////////////////////////////

using namespace MailImporter;

class Q_DECL_HIDDEN Filter::Private
{
public:
    Private(const QString &_name, const QString &_author, const QString &_info)
        : name(_name),
          author(_author),
          info(_info),
          count_duplicates(0),
          filterInfo(0)
    {
    }
    ~Private()
    {
    }
    QString name;
    QString author;
    QString info;
    QString mailDir;
    QMultiMap<QString, QString> messageFolderMessageIDMap;
    QMap<QString, Akonadi::Collection> messageFolderCollectionMap;
    int count_duplicates; //to count all duplicate messages

    MailImporter::FilterInfo *filterInfo;
};

Filter::Filter(const QString &name, const QString &author,
               const QString &info)
    : d(new Private(name, author, info))
{
}

Filter::~Filter()
{
    delete d;
}

void Filter::clear()
{
    d->messageFolderMessageIDMap.clear();
    d->messageFolderCollectionMap.clear();
    d->mailDir.clear();
    d->count_duplicates = 0;
}

void Filter::setMailDir(const QString &mailDir)
{
    d->mailDir = mailDir;
}

QString Filter::mailDir() const
{
    return d->mailDir;
}

void Filter::setFilterInfo(FilterInfo *info)
{
    d->filterInfo = info;
    clear();
}

MailImporter::FilterInfo *Filter::filterInfo()
{
    if (!d->filterInfo) {
        qCDebug(MAILIMPORTER_LOG) << " filterInfo must never be null. You forgot to create a filterinfo";
    }
    return d->filterInfo;
}

void Filter::setCountDuplicates(int countDuplicate)
{
    d->count_duplicates = countDuplicate;
}

int Filter::countDuplicates() const
{
    return d->count_duplicates;
}

bool Filter::addAkonadiMessage(const Akonadi::Collection &collection,
                               const KMime::Message::Ptr &message, Akonadi::MessageStatus status)
{
    Akonadi::Item item;

    item.setMimeType(QStringLiteral("message/rfc822"));

    if (status.isOfUnknownStatus()) {
        KMime::Headers::Base *statusHeaders = message->headerByType("X-Status");
        if (statusHeaders) {
            if (!statusHeaders->isEmpty()) {
                status.setStatusFromStr(statusHeaders->asUnicodeString());
                item.setFlags(status.statusFlags());
            }
        }
    } else {
        item.setFlags(status.statusFlags());
    }

    Akonadi::MessageFlags::copyMessageFlags(*message, item);
    item.setPayload<KMime::Message::Ptr>(message);
    QScopedPointer<Akonadi::ItemCreateJob> job(new Akonadi::ItemCreateJob(item, collection));
    job->setAutoDelete(false);
    if (!job->exec()) {
        d->filterInfo->alert(i18n("<b>Error:</b> Could not add message to folder %1. Reason: %2",
                                  collection.name(), job->errorString()));
        return false;
    }
    return true;
}

QString Filter::author() const
{
    return d->author;
}

QString Filter::name() const
{
    return d->name;
}

QString Filter::info() const
{
    return d->info;
}

void Filter::setAuthor(const QString &_author)
{
    d->author = _author;
}

void Filter::setName(const QString &_name)
{
    d->name = _name;
}
void Filter::setInfo(const QString &_info)
{
    d->info = _info;
}

Akonadi::Collection Filter::parseFolderString(const QString &folderParseString)
{
    // Return an already created collection:
    QMap<QString, Akonadi::Collection>::const_iterator end(d->messageFolderCollectionMap.constEnd());
    for (QMap<QString, Akonadi::Collection>::const_iterator it = d->messageFolderCollectionMap.constBegin(); it != end; ++it) {
        if (it.key() ==  folderParseString) {
            return it.value();
        }
    }

    // The folder hasn't yet been created, create it now.
    const QStringList folderList = folderParseString.split(QLatin1Char('/'), QString::SkipEmptyParts);
    bool isFirst = true;
    QString folderBuilder;
    Akonadi::Collection lastCollection;

    // Create each folder on the folder list and add it the map.
    foreach (const QString &folder, folderList) {
        if (isFirst) {
            d->messageFolderCollectionMap[folder] = addSubCollection(d->filterInfo->rootCollection(), folder);
            folderBuilder = folder;
            lastCollection = d->messageFolderCollectionMap[folder];
            isFirst = false;
        } else {
            folderBuilder += QLatin1Char('/') + folder;
            d->messageFolderCollectionMap[folderBuilder] = addSubCollection(lastCollection, folder);
            lastCollection = d->messageFolderCollectionMap[folderBuilder];
        }
    }

    return lastCollection;
}

Akonadi::Collection Filter::addSubCollection(const Akonadi::Collection &baseCollection,
        const QString &newCollectionPathName)
{
    // Ensure that the collection doesn't already exsit, if it does just return it.
    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(baseCollection,
            Akonadi::CollectionFetchJob::FirstLevel);
    if (!fetchJob->exec()) {
        d->filterInfo->alert(i18n("<b>Warning:</b> Could not check that the folder already exists. Reason: %1",
                                  fetchJob->errorString()));
        return Akonadi::Collection();
    }

    foreach (const Akonadi::Collection &subCollection, fetchJob->collections()) {
        if (subCollection.name() == newCollectionPathName) {
            return subCollection;
        }
    }

    // The subCollection doesn't exsit, create a new one
    Akonadi::Collection newSubCollection;
    newSubCollection.setParentCollection(baseCollection);
    newSubCollection.setName(newCollectionPathName);

    QScopedPointer<Akonadi::CollectionCreateJob> job(new Akonadi::CollectionCreateJob(newSubCollection));
    job->setAutoDelete(false);
    if (!job->exec()) {
        d->filterInfo->alert(i18n("<b>Error:</b> Could not create folder. Reason: %1",
                                  job->errorString()));
        return Akonadi::Collection();
    }
    // Return the newly created collection
    Akonadi::Collection collection = job->collection();
    return collection;
}

bool Filter::checkForDuplicates(const QString &msgID,
                                const Akonadi::Collection &msgCollection,
                                const QString &messageFolder)
{
    bool folderFound = false;

    // Check if the contents of this collection have already been found.
    QMultiMap<QString, QString>::const_iterator end(d->messageFolderMessageIDMap.constEnd());
    for (QMultiMap<QString, QString>::const_iterator it = d->messageFolderMessageIDMap.constBegin(); it != end; ++it) {
        if (it.key() == messageFolder) {
            folderFound = true;
            break;
        }
    }

    if (!folderFound) {
        // Populate the map with message IDs that are in that collection.
        if (msgCollection.isValid()) {
            Akonadi::ItemFetchJob job(msgCollection);
            job.fetchScope().fetchPayloadPart(Akonadi::MessagePart::Header);
            if (!job.exec()) {
                d->filterInfo->addInfoLogEntry(i18n("<b>Warning:</b> Could not fetch mail in folder %1. Reason: %2"
                                                    " You may have duplicate messages.", messageFolder, job.errorString()));
            } else {
                foreach (const Akonadi::Item &messageItem, job.items()) {
                    if (!messageItem.isValid()) {
                        d->filterInfo->addInfoLogEntry(i18n("<b>Warning:</b> Got an invalid message in folder %1.", messageFolder));
                    } else {
                        if (!messageItem.hasPayload<KMime::Message::Ptr>()) {
                            continue;
                        }
                        const KMime::Message::Ptr message = messageItem.payload<KMime::Message::Ptr>();
                        const KMime::Headers::Base *messageID = message->messageID(false);
                        if (messageID) {
                            if (!messageID->isEmpty()) {
                                d->messageFolderMessageIDMap.insert(messageFolder, messageID->asUnicodeString());
                            }
                        }
                    }
                }
            }
        }
    }

    // Check if this message has a duplicate
    QMultiMap<QString, QString>::const_iterator endMsgID(d->messageFolderMessageIDMap.constEnd());
    for (QMultiMap<QString, QString>::const_iterator it = d->messageFolderMessageIDMap.constBegin(); it != endMsgID; ++it) {
        if (it.key() == messageFolder &&
                it.value() == msgID) {
            return true;
        }
    }

    // The message isn't a duplicate, but add it to the map for checking in the future.
    d->messageFolderMessageIDMap.insert(messageFolder, msgID);
    return false;
}

bool Filter::addMessage(const QString &folderName,
                        const QString &msgPath,
                        Akonadi::MessageStatus status)
{
    // Add the message.
    return doAddMessage(folderName, msgPath, true, status);
}

bool Filter::addMessage_fastImport(const QString &folderName,
                                   const QString &msgPath,
                                   Akonadi::MessageStatus status
                                  )
{
    // Add the message.
    return doAddMessage(folderName, msgPath, false, status);
}

bool Filter::doAddMessage(const QString &folderName,
                          const QString &msgPath,
                          bool duplicateCheck,
                          Akonadi::MessageStatus status)
{
    QString messageID;
    // Create the mail folder (if not already created).
    Akonadi::Collection mailFolder = parseFolderString(folderName);
    QUrl msgUrl = QUrl::fromLocalFile(msgPath);
    if (!msgUrl.isEmpty() && msgUrl.isLocalFile()) {

        QFile f(msgUrl.toLocalFile());
        QByteArray msgText;
        if (!f.open(QIODevice::ReadOnly)) {
            qCWarning(MAILIMPORTER_LOG) << "Failed to read temporary file: " << f.errorString();
        } else {
            msgText = f.readAll();
            f.close();
        }
        if (msgText.isEmpty()) {
            d->filterInfo->addErrorLogEntry(i18n("Error: failed to read temporary file at %1", msgPath));
            return false;
        }

        // Construct a message.
        KMime::Message::Ptr newMessage(new KMime::Message());
        newMessage->setContent(msgText);
        newMessage->parse();

        if (duplicateCheck) {
            // Get the messageID.
            const KMime::Headers::Base *messageIDHeader = newMessage->messageID(false);
            if (messageIDHeader) {
                messageID = messageIDHeader->asUnicodeString();
            }

            if (!messageID.isEmpty()) {
                // Check for duplicate.
                if (checkForDuplicates(messageID, mailFolder, folderName)) {
                    d->count_duplicates++;
                    return false;
                }
            }
        }

        // Add it to the collection.
        if (mailFolder.isValid()) {
            addAkonadiMessage(mailFolder, newMessage, status);
        } else {
            d->filterInfo->alert(i18n("<b>Warning:</b> Got a bad message folder, adding to root folder."));
            addAkonadiMessage(d->filterInfo->rootCollection(), newMessage, status);
        }
    } else {
        qCWarning(MAILIMPORTER_LOG) << "Url is not temporary file: " << msgUrl;
    }
    return true;
}

int Filter::countDirectory(const QDir &dir, bool searchHiddenDirectory)
{
    int countDir = 0;
    QStringList subDirs;
    if (searchHiddenDirectory) {
        subDirs = dir.entryList(QStringList(QStringLiteral("*")), QDir::Dirs | QDir::Hidden, QDir::Name);
    } else {
        subDirs = dir.entryList(QStringList(QStringLiteral("[^\\.]*")), QDir::Dirs, QDir::Name);    // Removal of . and ..
    }

    QStringList::ConstIterator end = subDirs.constEnd();
    for (QStringList::ConstIterator filename = subDirs.constBegin(); filename != end; ++filename) {
        if (!(*filename == QLatin1String(".") || *filename == QLatin1String(".."))) {
            countDir += countDirectory(QDir(dir.filePath(*filename)), searchHiddenDirectory) + 1;
        }
    }
    return countDir;
}

