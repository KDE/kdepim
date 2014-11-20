/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "attachmentupdatejobtest.h"
#include <messagecore/attachment/attachmentupdatejob.h>
#include <messagecore/attachment/attachmentpart.h>
#include <qtest_kde.h>
#include "qtest_messagecore.h"

#define PATH_ATTACHMENTS QLatin1String( KDESRCDIR "/attachments/" )

AttachmentUpdateJobTest::AttachmentUpdateJobTest(QObject *parent)
    : QObject(parent)
{

}

AttachmentUpdateJobTest::~AttachmentUpdateJobTest()
{

}

void AttachmentUpdateJobTest::shouldHaveDefaultValue()
{
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);
    QCOMPARE(origPart, job->originalPart());
    QVERIFY(!job->updatedPart());
    delete job;
}

void AttachmentUpdateJobTest::shouldUpdateAttachment()
{
    const KUrl url = KUrl::fromPath(PATH_ATTACHMENTS + QString::fromLatin1("file.txt"));

    // Some data.
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/plain");
    origPart->setEncoding(KMime::Headers::CE7Bit);
    origPart->setData(data);
    origPart->setUrl(url);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);

    VERIFYEXEC(job);
    QVERIFY(origPart->size() != job->updatedPart()->size());
    QVERIFY(origPart->data() != job->updatedPart()->data());
}

void AttachmentUpdateJobTest::shouldHaveSameNameDescriptionAfterUpdate()
{
    const KUrl url = KUrl::fromPath(PATH_ATTACHMENTS + QString::fromLatin1("file.txt"));

    // Some data.
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/plain");
    origPart->setEncoding(KMime::Headers::CE7Bit);
    origPart->setData(data);
    origPart->setUrl(url);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);

    VERIFYEXEC(job);
    QCOMPARE(origPart->name(), job->updatedPart()->name());
    QCOMPARE(origPart->description(), job->updatedPart()->description());

}

void AttachmentUpdateJobTest::shouldHaveSameCryptoSignStatusAfterUpdate()
{
    const KUrl url = KUrl::fromPath(PATH_ATTACHMENTS + QString::fromLatin1("file.txt"));

    // Some data.
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/plain");
    origPart->setEncoding(KMime::Headers::CE7Bit);
    origPart->setData(data);
    origPart->setUrl(url);
    origPart->setSigned(true);
    origPart->setEncrypted(true);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);

    VERIFYEXEC(job);
    QCOMPARE(origPart->isSigned(), job->updatedPart()->isSigned());
    QCOMPARE(origPart->isEncrypted(), job->updatedPart()->isEncrypted());
}

void AttachmentUpdateJobTest::shouldHaveSameEncodingAfterUpdate()
{
    const KUrl url = KUrl::fromPath(PATH_ATTACHMENTS + QString::fromLatin1("file.txt"));

    // Some data.
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/pdf");
    origPart->setEncoding(KMime::Headers::CE8Bit);
    origPart->setData(data);
    origPart->setUrl(url);
    origPart->setSigned(true);
    origPart->setEncrypted(true);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);

    VERIFYEXEC(job);
    QCOMPARE(origPart->encoding(), job->updatedPart()->encoding());
}

void AttachmentUpdateJobTest::shouldHaveSameMimetypeAfterUpdate()
{
    const KUrl url = KUrl::fromPath(PATH_ATTACHMENTS + QString::fromLatin1("file.txt"));

    // Some data.
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/pdf");
    origPart->setEncoding(KMime::Headers::CE8Bit);
    origPart->setData(data);
    origPart->setUrl(url);
    origPart->setSigned(true);
    origPart->setEncrypted(true);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);

    VERIFYEXEC(job);
    QCOMPARE(origPart->mimeType(), job->updatedPart()->mimeType());

}

void AttachmentUpdateJobTest::shouldNotUpdateWhenUrlIsEmpty()
{
    QByteArray data("This is short enough that compressing it is not efficient.");
    const QString name = QString::fromLatin1("name.txt");
    const QString description = QString::fromLatin1("description");

    // Create the original part.
    MessageCore::AttachmentPart::Ptr origPart = MessageCore::AttachmentPart::Ptr(new MessageCore::AttachmentPart);
    origPart->setName(name);
    origPart->setDescription(description);
    origPart->setMimeType("text/plain");
    origPart->setEncoding(KMime::Headers::CE7Bit);
    origPart->setData(data);

    MessageCore::AttachmentUpdateJob *job = new MessageCore::AttachmentUpdateJob(origPart, this);
    job->exec();
    QVERIFY(!job->updatedPart());
}

QTEST_KDEMAIN(AttachmentUpdateJobTest, NoGUI)
