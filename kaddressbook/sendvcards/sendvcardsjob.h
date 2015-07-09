/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef SENDVCARDSJOB_H
#define SENDVCARDSJOB_H
#include <QObject>
#include <AkonadiCore/Item>
#include <QStringList>
#include <KContacts/VCardConverter>

class QTemporaryDir;
namespace PimCommon
{
class AttachmentTemporaryFilesDirs;
}
namespace KABSendVCards
{
class SendVcardsJob : public QObject
{
    Q_OBJECT
public:
    explicit SendVcardsJob(const Akonadi::Item::List &listItem, QObject *parent = Q_NULLPTR);
    ~SendVcardsJob();

    bool start();

    KContacts::VCardConverter::Version version() const;
    void setVersion(const KContacts::VCardConverter::Version &version);

Q_SIGNALS:
    void sendVCardsError(const QString &error);

private Q_SLOTS:
    void slotExpandGroupResult(KJob *job);

private:
    void createTemporaryFile(const QByteArray &data, const QString &filename);
    void createTemporaryDir();
    void jobFinished();
    Akonadi::Item::List mListItem;
    PimCommon::AttachmentTemporaryFilesDirs *mAttachmentTemporary;
    QTemporaryDir *mTempDir;
    KContacts::VCardConverter::Version mVersion;
    int mExpandGroupJobCount;
};
}

#endif // SENDVCARDSJOB_H
