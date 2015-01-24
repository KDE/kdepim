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

#include "sendvcardsjob.h"
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <AkonadiCore/Item>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <KContacts/VCardConverter>
#include <QDebug>
#include <util/vcardutil.h>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include "pimcommon/temporaryfile/attachmenttemporaryfilesdirs.h"

using namespace KABSendVCards;

SendVcardsJob::SendVcardsJob(const Akonadi::Item::List &listItem, QObject *parent)
    : QObject(parent),
      mListItem(listItem),
      mFetchJobCount(0)
{
    //Don't delete it.
    mAttachmentTemporary = new PimCommon::AttachmentTemporaryFilesDirs();
}

SendVcardsJob::~SendVcardsJob()
{
    mAttachmentTemporary = 0;
}

bool SendVcardsJob::start()
{
    if (mListItem.isEmpty()) {
        qDebug() << " No Item found";
        deleteLater();
        return false;
    }

    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KContacts::Addressee>()) {
            QByteArray data = item.payloadData();
            //Workaround about broken kaddressbook fields.
            PimCommon::VCardUtil vcardUtil;
            vcardUtil.adaptVcard(data);
        } else if (item.hasPayload<KContacts::ContactGroup>()) {
            ++mFetchJobCount;
            const KContacts::ContactGroup group = item.payload<KContacts::ContactGroup>();
            const QString groupName(group.name());
            const QString attachmentName = ( groupName.isEmpty() ? QLatin1String("vcard") : groupName ) + QLatin1String( ".vcf" );
            Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( group, this );
            expandJob->setProperty("groupName", attachmentName);
            connect( expandJob, SIGNAL(result(KJob*)), this, SLOT(slotExpandGroupResult(KJob*)) );
            expandJob->start();
        }
    }

    if (mFetchJobCount == 0) {
        jobFinished();
    }
    return true;
}

void SendVcardsJob::jobFinished()
{
    if (!mPath.isEmpty()) {
        //TODO
    }
    deleteLater();
}

void SendVcardsJob::slotExpandGroupResult(KJob* job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob*>( job );
    Q_ASSERT( expandJob );

    const QString attachmentName = expandJob->property("groupName").toString();
    KContacts::VCardConverter converter;
    const QByteArray groupData = converter.exportVCards(expandJob->contacts(), KContacts::VCardConverter::v3_0);
    //TODO
    --mFetchJobCount;
    if (mFetchJobCount == 0) {
        jobFinished();
    }
}

