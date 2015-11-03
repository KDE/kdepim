/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef COLLECTIONTYPEUTIL_H
#define COLLECTIONTYPEUTIL_H
#include "mailcommon_export.h"
#include <QByteArray>
#include <QString>

namespace MailCommon
{
class MAILCOMMON_EXPORT CollectionTypeUtil
{
public:
    CollectionTypeUtil();
    ~CollectionTypeUtil();

    static QByteArray kolabFolderType();
    static QByteArray kolabIncidencesFor();
    static QByteArray kolabSharedSeen();

    enum FolderContentsType {
        ContentsTypeMail = 0,
        ContentsTypeCalendar,
        ContentsTypeContact,
        ContentsTypeNote,
        ContentsTypeTask,
        ContentsTypeJournal,
        ContentsTypeConfiguration,
        ContentsTypeFreebusy,
        ContentsTypeFile,
        ContentsTypeLast = ContentsTypeFile
    };

    enum IncidencesFor {
        IncForNobody,
        IncForAdmins,
        IncForReaders
    };

    CollectionTypeUtil::IncidencesFor incidencesForFromString(const QString &string);
    CollectionTypeUtil::FolderContentsType typeFromKolabName(const QByteArray &name);
    QString folderContentDescription(CollectionTypeUtil::FolderContentsType type);
    QByteArray kolabNameFromType(CollectionTypeUtil::FolderContentsType type);
    QString incidencesForToString(CollectionTypeUtil::IncidencesFor type);
    CollectionTypeUtil::FolderContentsType contentsTypeFromString(const QString &type);
    QString typeNameFromKolabType(const QByteArray &type);
    QString iconNameFromContentsType(CollectionTypeUtil::FolderContentsType type);
};
}
#endif // COLLECTIONTYPEUTIL_H
