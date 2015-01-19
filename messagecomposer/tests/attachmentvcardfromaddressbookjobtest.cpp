/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "attachmentvcardfromaddressbookjobtest.h"
#include "messagecomposer/job/attachmentvcardfromaddressbookjob.h"
#include <qtest_kde.h>
#include <KABC/Addressee>

AttachmentVcardFromAddressBookJobTest::AttachmentVcardFromAddressBookJobTest(QObject *parent)
    : QObject(parent)
{

}

AttachmentVcardFromAddressBookJobTest::~AttachmentVcardFromAddressBookJobTest()
{

}

void AttachmentVcardFromAddressBookJobTest::testAttachmentVCardWithInvalidItem()
{
    Akonadi::Item item;
    MessageComposer::AttachmentVcardFromAddressBookJob *job = new MessageComposer::AttachmentVcardFromAddressBookJob(item);
    QVERIFY(!job->exec());
}

void AttachmentVcardFromAddressBookJobTest::testAttachmentVCardWithValidItem()
{
    Akonadi::Item item(42);
    item.setMimeType( KABC::Addressee::mimeType() );
    KABC::Addressee address;
    const QString name = QLatin1String("foo1");
    address.setName(name);
    item.setPayload<KABC::Addressee>( address );
    MessageComposer::AttachmentVcardFromAddressBookJob *job = new MessageComposer::AttachmentVcardFromAddressBookJob(item);
    QVERIFY(job->exec());

    MessageCore::AttachmentPart::Ptr part = job->attachmentPart();
    delete job;
    job = 0;

    QVERIFY( !part->data().isEmpty() );
    QCOMPARE( part->mimeType(), QByteArray("text/x-vcard") );
    const QString newName = name + QLatin1String(".vcf");
    QCOMPARE( part->name(), newName );
    QVERIFY( part->description().isEmpty() );
    QVERIFY( !part->isInline() );
    QVERIFY( !part->fileName().isEmpty() );
}

void AttachmentVcardFromAddressBookJobTest::testAttachmentVCardWithInvalidVCard()
{
    Akonadi::Item item(42);
    MessageComposer::AttachmentVcardFromAddressBookJob *job = new MessageComposer::AttachmentVcardFromAddressBookJob(item);
    QVERIFY(!job->exec());
}

void AttachmentVcardFromAddressBookJobTest::testAttachmentVCardWithEmptyVCard()
{
    Akonadi::Item item(42);
    item.setMimeType( KABC::Addressee::mimeType() );
    KABC::Addressee address;
    item.setPayload<KABC::Addressee>( address );
    MessageComposer::AttachmentVcardFromAddressBookJob *job = new MessageComposer::AttachmentVcardFromAddressBookJob(item);
    QVERIFY(job->exec());
    //TODO
}

QTEST_KDEMAIN(AttachmentVcardFromAddressBookJobTest, GUI)
