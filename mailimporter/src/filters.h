/***************************************************************************
                          filters.h  -  description
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

#ifndef FILTERS_HXX
#define FILTERS_HXX

#ifndef MAX_LINE
#define MAX_LINE 4096
#endif

#include "filterinfo.h"
#include "filters.h"
#include "mailimporter_export.h"

#include <Collection>
#include <KMime/KMimeMessage>
#include <Akonadi/KMime/MessageStatus>

#include <QDir>

namespace MailImporter
{
class MAILIMPORTER_EXPORT Filter
{
public:
    explicit Filter(const QString &name, const QString &author,
                    const QString &info = QString());
    virtual ~Filter();
    virtual void import() = 0;

    QString author() const;
    QString name() const;
    QString info() const;

    void setAuthor(const QString &);
    void setName(const QString &);
    void setInfo(const QString &);

    void clear();
    void setFilterInfo(MailImporter::FilterInfo *info);

    MailImporter::FilterInfo *filterInfo();

    void setCountDuplicates(int countDuplicate);
    int countDuplicates() const;

    void setMailDir(const QString &mailDir);
    QString mailDir() const;

protected:
    static int countDirectory(const QDir &dir, bool searchHiddenDirectory);
    /**
    * Adds a single subcollection to the given base collection and returns it.
    * Use parseFolderString() instead if you want to create hierachies of collections.
    */
    Akonadi::Collection addSubCollection(const Akonadi::Collection &baseCollection,
                                         const QString &newCollectionPathName);

    /**
    * Creates a hierachy of collections based on the given path string. The collection
    * hierachy will be placed under the root collection.
    * For example, if the folderParseString "foo/bar/test" is passsed to this method, it
    * will make sure the root collection has a subcollection named "foo", which in turn
    * has a subcollection named "bar", which again has a subcollection named "test".
    * The "test" collection will be returned.
    * An invalid collection will be returned in case of an error.
    */
    Akonadi::Collection parseFolderString(const QString &folderParseString);

    bool addAkonadiMessage(const Akonadi::Collection &collection,
                           const KMime::Message::Ptr &message, Akonadi::MessageStatus status = Akonadi::MessageStatus());

    bool addMessage(const QString &folder,
                    const QString &msgFile,
                    Akonadi::MessageStatus status = Akonadi::MessageStatus());

    /**
    * Checks for duplicate messages in the collection by message ID.
    * returns true if a duplicate was detected.
    * NOTE: Only call this method if a message ID exists, otherwise
    * you could get false positives.
    */
    bool checkForDuplicates(const QString &msgID,
                            const Akonadi::Collection &msgCollection,
                            const QString &messageFolder);
    bool addMessage_fastImport(const QString &folder,
                               const QString &msgFile,
                               Akonadi::MessageStatus status = Akonadi::MessageStatus());

private:
    bool doAddMessage(const QString &folderName,
                      const QString &msgPath,
                      bool duplicateCheck,
                      Akonadi::MessageStatus status = Akonadi::MessageStatus());
    class Private;
    Private *const d;
};

}

#endif

