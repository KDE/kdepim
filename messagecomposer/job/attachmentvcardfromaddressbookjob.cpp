/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "attachmentvcardfromaddressbookjob.h"
#include "messagecomposer/utils/util.h"
#include <KLocalizedString>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <KContacts/VCardConverter>
#include <akonadi/contact/contactgroupexpandjob.h>

using namespace MessageComposer;

AttachmentVcardFromAddressBookJob::AttachmentVcardFromAddressBookJob(const Akonadi::Item &item, QObject *parent)
    : MessageCore::AttachmentLoadJob(parent),
      mItem(item)
{

}

AttachmentVcardFromAddressBookJob::~AttachmentVcardFromAddressBookJob()
{

}

void AttachmentVcardFromAddressBookJob::addAttachment(const QByteArray &data, const QString &attachmentName)
{
    MessageCore::AttachmentPart::Ptr attachment = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart());
    if (!data.isEmpty()) {
        attachment->setName(attachmentName);
        attachment->setFileName(attachmentName);
        attachment->setData(data);
        attachment->setMimeType("text/x-vcard");
        // TODO what about the other fields?
    }
    setAttachmentPart(attachment);
    emitResult(); // Success.
}

void AttachmentVcardFromAddressBookJob::doStart()
{
    if (mItem.isValid()) {
        if ( mItem.hasPayload<KContacts::Addressee>() ) {
            const KContacts::Addressee contact = mItem.payload<KContacts::Addressee>();
            const QString contactRealName(contact.realName());
            const QString attachmentName = (contactRealName.isEmpty() ? QLatin1String("vcard") : contactRealName ) + QLatin1String( ".vcf" );

            QByteArray data = mItem.payloadData();
            //Workaround about broken kaddressbook fields.
            MessageComposer::Util::adaptVcard(data);
            addAttachment( data, attachmentName );
        } else if ( mItem.hasPayload<KContacts::ContactGroup>() ) {
            const KContacts::ContactGroup group = mItem.payload<KContacts::ContactGroup>();
            const QString groupName(group.name());
            const QString attachmentName = ( groupName.isEmpty() ? QLatin1String("vcard") : groupName ) + QLatin1String( ".vcf" );
            Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob( group, this );
            expandJob->setProperty("groupName", attachmentName);
            connect(expandJob, SIGNAL(result(KJob*)), this, SLOT(slotExpandGroupResult(KJob*)));
            expandJob->start();
        } else {
            setError(KJob::UserDefinedError);
            setErrorText(i18n("Unknown Contact Type"));
            emitResult();
        }
    } else {
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Invalid Contact"));
        emitResult();
    }
}

void AttachmentVcardFromAddressBookJob::slotExpandGroupResult(KJob *job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);
    Q_ASSERT(expandJob);

    const QString attachmentName = expandJob->property("groupName").toString();
    KContacts::VCardConverter converter;
    const QByteArray groupData = converter.exportVCards(expandJob->contacts(), KContacts::VCardConverter::v3_0);
    if (!groupData.isEmpty()) {
        addAttachment(groupData, attachmentName);
    } else {
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Impossible to generate vcard."));
        emitResult();
    }
}
