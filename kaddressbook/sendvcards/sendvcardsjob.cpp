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
#include <KABC/Addressee>
#include <KABC/ContactGroup>
#include <KABC/VCardConverter>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <QDebug>
#include <util/vcardutil.h>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include "pimcommon/temporaryfile/attachmenttemporaryfilesdirs.h"
#include <KTempDir>
#include <KStandardDirs>
#include <KToolInvocation>
#include <QFile>

using namespace KABSendVCards;

SendVcardsJob::SendVcardsJob(const Akonadi::Item::List &listItem, QObject *parent)
    : QObject(parent),
      mListItem(listItem),
      mTempDir(0),
      mExpandGroupJobCount(0)
{
    //Don't delete it.
    mAttachmentTemporary = new PimCommon::AttachmentTemporaryFilesDirs();
}

SendVcardsJob::~SendVcardsJob()
{
    delete mTempDir;
    mTempDir = 0;
    //Don't delete it.
    mAttachmentTemporary = 0;
}

bool SendVcardsJob::start()
{
    if (mListItem.isEmpty()) {
        qDebug()<<" No Item found";
        mAttachmentTemporary->deleteLater();
        mAttachmentTemporary = 0;
        deleteLater();
        return false;
    }

    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            const KABC::Addressee contact = item.payload<KABC::Addressee>();
            QByteArray data = item.payloadData();
            //Workaround about broken kaddressbook fields.
            PimCommon::VCardUtil vcardUtil;
            vcardUtil.adaptVcard(data);
            createTemporaryDir();
            const QString contactRealName(contact.realName());
            const QString attachmentName = (contactRealName.isEmpty() ? QLatin1String("vcard") : contactRealName ) + QLatin1String( ".vcf" );
            createTemporaryFile(data, attachmentName);
        } else if (item.hasPayload<KABC::ContactGroup>()) {
            ++mExpandGroupJobCount;
            const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
            const QString groupName(group.name());
            const QString attachmentName = ( groupName.isEmpty() ? QLatin1String("vcard") : groupName ) + QLatin1String( ".vcf" );
            Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( group, this );
            expandJob->setProperty("groupName", attachmentName);
            connect( expandJob, SIGNAL(result(KJob*)), this, SLOT(slotExpandGroupResult(KJob*)) );
            expandJob->start();
        }
    }

    if (mExpandGroupJobCount == 0) {
        jobFinished();
    }
    return true;
}

void SendVcardsJob::createTemporaryDir()
{
    if (!mTempDir) {
        mTempDir = new KTempDir( KStandardDirs::locateLocal( "tmp", QLatin1String("sendvcards") ) );
        mTempDir->setAutoRemove(false);
        mAttachmentTemporary->addTempDir(mTempDir->name());
    }
}

void SendVcardsJob::jobFinished()
{
    const QStringList lstAttachment = mAttachmentTemporary->temporaryFiles();
    if (!lstAttachment.isEmpty()) {
        KToolInvocation::invokeMailer( QString(), QString(), QString(), QString(), QString(), QString(), lstAttachment );
    } else {
        //KF5 add i18n
        sendVCardsError(QLatin1String("No VCard created."));
    }
    mAttachmentTemporary->removeTempFiles();
    deleteLater();
}

void SendVcardsJob::slotExpandGroupResult(KJob* job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob*>( job );
    Q_ASSERT( expandJob );

    const QString attachmentName = expandJob->property("groupName").toString();
    KABC::VCardConverter converter;
    const QByteArray groupData = converter.exportVCards(expandJob->contacts(), KABC::VCardConverter::v3_0);
    createTemporaryDir();
    createTemporaryFile(groupData, attachmentName);

    --mExpandGroupJobCount;
    if (mExpandGroupJobCount == 0) {
        jobFinished();
    }
}

void SendVcardsJob::createTemporaryFile(const QByteArray &data, const QString &filename)
{
    QFile file(mTempDir->name() + QLatin1Char('/') + filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"Can not write vcard filename :"<<filename;
        //KF5 add i18n
        sendVCardsError(QString::fromLatin1("Temporary file \'%1\' can not created").arg(filename));
        return;
    }

    QTextStream out(&file);
    out << data;
    file.close();
    mAttachmentTemporary->addTempFile(file.fileName());
}
