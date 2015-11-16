/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#include "cryptocomposertest.h"

#include "qtest_messagecomposer.h"
#include "cryptofunctions.h"

#include "testhtmlwriter.h"
#include "testcsshelper.h"

#include <QDebug>
#include <qtest.h>

#include <Libkleo/Enum>

#include <kmime/kmime_headers.h>
using namespace KMime;

#include <MessageComposer/Composer>
#include <MessageComposer/ComposerViewBase>
#include <MessageComposer/RichTextComposerNg>
#include <MessageComposer/GlobalPart>
#include <MessageComposer/InfoPart>
#include <MessageComposer/TextPart>
#include <attachment/attachmentmodel.h>
#include <attachment/attachmentcontrollerbase.h>
using namespace MessageComposer;

#include <MessageCore/AttachmentPart>
#include <MessageCore/NodeHelper>
#include <setupenv.h>

#include <MessageViewer/ObjectTreeParser>

using MessageCore::AttachmentPart;

#include <gpgme++/key.h>

Q_DECLARE_METATYPE(MessageCore::AttachmentPart)

QTEST_MAIN(CryptoComposerTest)

void CryptoComposerTest::initTestCase()
{
    MessageComposer::Test::setupEnv();
}

Q_DECLARE_METATYPE(Headers::contentEncoding)

// openpgp
void CryptoComposerTest::testOpenPGPMime_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    QTest::newRow("SignOpenPGPMime") << data << true << false << Headers::CE7Bit;
    QTest::newRow("EncryptOpenPGPMime") << data << false << true << Headers::CE7Bit;
    QTest::newRow("SignEncryptOpenPGPMime") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testOpenPGPMime()
{
    QFETCH(QString, data);
    QFETCH(bool, sign);
    QFETCH(bool, encrypt);
    QFETCH(Headers::contentEncoding, cte);

    Composer *composer = new Composer;

    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    composer->setSignAndEncrypt(sign, encrypt);
    composer->setMessageCryptoFormat(Kleo::OpenPGPMIMEFormat);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = Q_NULLPTR;

    //qDebug()<< "message:" << message.data()->encodedContent();
    ComposerTestUtil::verify(sign, encrypt, message.data(), data.toUtf8(),
                             Kleo::OpenPGPMIMEFormat, cte);

    QCOMPARE(message->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(message->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));
}

// the following will do for s-mime as well, as the same sign/enc jobs are used
void CryptoComposerTest::testEncryptSameAttachments_data()
{
    QTest::addColumn<int>("format");

    QTest::newRow("OpenPGPMime") << (int) Kleo::OpenPGPMIMEFormat;
    //TODO: fix Inline PGP with encrypted attachments
    //QTest::newRow( "InlineOpenPGP" ) << (int) Kleo::InlineOpenPGPFormat;
}

void CryptoComposerTest::testEncryptSameAttachments()
{
    QFETCH(int, format);
    Composer *composer = new Composer;
    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    AttachmentPart::Ptr attachment = AttachmentPart::Ptr(new AttachmentPart);
    attachment->setData("abc");
    attachment->setMimeType("x-some/x-type");
    attachment->setFileName(QString::fromLocal8Bit("anattachment.txt"));
    attachment->setEncrypted(true);
    attachment->setSigned(false);
    composer->addAttachmentPart(attachment);

    composer->setSignAndEncrypt(false, true);
    composer->setMessageCryptoFormat((Kleo::CryptoMessageFormat) format);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = Q_NULLPTR;

    //qDebug()<< "message:" << message.data()->encodedContent();
    ComposerTestUtil::verifyEncryption(message.data(), data.toUtf8(),
                                       (Kleo::CryptoMessageFormat) format, true);

    QCOMPARE(message->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(message->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));

    TestHtmlWriter testWriter;
    TestCSSHelper testCSSHelper;
    MessageComposer::Test::TestObjectTreeSource testSource(&testWriter, &testCSSHelper);
    testSource.setAllowDecryption(true);
    MessageViewer::NodeHelper *nh = new MessageViewer::NodeHelper;
    MessageViewer::ObjectTreeParser otp(&testSource, nh);
    MessageViewer::ProcessResult pResult(nh);

    otp.parseObjectTree(message.data());
    KMime::Message::Ptr  unencrypted = nh->unencryptedMessage(message);

    KMime::Content *testAttachment = MessageViewer::ObjectTreeParser::findType(unencrypted.data(), "x-some", "x-type", true, true);

    QCOMPARE(testAttachment->body(), QString::fromLatin1("abc").toUtf8());
    QCOMPARE(testAttachment->contentDisposition()->filename(), QString::fromLatin1("anattachment.txt"));

}

void CryptoComposerTest::testEditEncryptAttachments_data()
{
    QTest::addColumn<int>("format");
    QTest::newRow("OpenPGPMime") << (int) Kleo::OpenPGPMIMEFormat;
    //TODO: SMIME should also be tested
}

void CryptoComposerTest::testEditEncryptAttachments()
{
    QFETCH(int, format);
    Composer *composer = new Composer;
    QString data(QStringLiteral("All happy families are alike; each unhappy family is unhappy in its own way."));
    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    AttachmentPart::Ptr attachment = AttachmentPart::Ptr(new AttachmentPart);
    const QString fileName = QStringLiteral("anattachment.txt");
    const QByteArray fileData = "abc";
    attachment->setData(fileData);
    attachment->setMimeType("x-some/x-type");
    attachment->setFileName(fileName);
    attachment->setEncrypted(true);
    attachment->setSigned(false);
    composer->addAttachmentPart(attachment);

    composer->setSignAndEncrypt(false, true);
    composer->setMessageCryptoFormat((Kleo::CryptoMessageFormat) format);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = 0;

    // setup a viewer
    ComposerViewBase view(this, 0);
    AttachmentModel model(this);
    AttachmentControllerBase controller(&model, 0, 0);
    MessageComposer::RichTextComposerNg editor;
    view.setAttachmentModel(&model);
    view.setAttachmentController(&controller);
    view.setEditor(&editor);

    // Let's load the email to the viewer
    view.setMessage(message, true);

    QModelIndex index =  model.index(0, 0);
    QCOMPARE(editor.toPlainText(), data);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(index, AttachmentModel::NameRole).toString(), fileName);
    AttachmentPart::Ptr part = model.attachments()[0];
    QCOMPARE(part->data(), fileData);
    QCOMPARE(part->fileName(), fileName);
}

void CryptoComposerTest::testEditEncryptAndLateAttachments_data()
{
    QTest::addColumn<int>("format");
    QTest::newRow("OpenPGPMime") << (int) Kleo::OpenPGPMIMEFormat;
    //TODO: SMIME should also be tested
}

void CryptoComposerTest::testEditEncryptAndLateAttachments()
{
    QFETCH(int, format);
    Composer *composer = new Composer;
    QString data(QStringLiteral("All happy families are alike; each unhappy family is unhappy in its own way."));
    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    AttachmentPart::Ptr attachment = AttachmentPart::Ptr(new AttachmentPart);
    const QString fileName = QStringLiteral("anattachment.txt");
    const QByteArray fileData = "abc";
    const QString fileName2 = QStringLiteral("nonencrypt.txt");
    const QByteArray fileData2 = "readable";
    attachment->setData(fileData);
    attachment->setMimeType("x-some/x-type");
    attachment->setFileName(fileName);
    attachment->setEncrypted(true);
    attachment->setSigned(false);
    composer->addAttachmentPart(attachment);

    attachment = AttachmentPart::Ptr(new AttachmentPart);
    attachment->setData(fileData2);
    attachment->setMimeType("x-some/x-type2");
    attachment->setFileName(fileName2);
    attachment->setEncrypted(false);
    attachment->setSigned(false);
    composer->addAttachmentPart(attachment);

    composer->setSignAndEncrypt(false, true);
    composer->setMessageCryptoFormat((Kleo::CryptoMessageFormat) format);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = 0;

    // setup a viewer
    ComposerViewBase view(this, 0);
    AttachmentModel model(this);
    AttachmentControllerBase controller(&model, 0, 0);
    MessageComposer::RichTextComposerNg editor;
    view.setAttachmentModel(&model);
    view.setAttachmentController(&controller);
    view.setEditor(&editor);

    // Let's load the email to the viewer
    view.setMessage(message, true);

    //QModelIndex index = model.index(0, 0);
    QCOMPARE(editor.toPlainText(), data);
    QCOMPARE(model.rowCount(), 2);
    AttachmentPart::Ptr part = model.attachments()[0];
    QCOMPARE(part->fileName(), fileName);
    QCOMPARE(part->data(), fileData);
    part = model.attachments()[1];
    QCOMPARE(part->fileName(), fileName2);
    QCOMPARE(part->data(), fileData2);
}

void CryptoComposerTest::testSignEncryptLateAttachments_data()
{
    QTest::addColumn<int>("format");

    QTest::newRow("OpenPGPMime") << (int) Kleo::OpenPGPMIMEFormat;
    QTest::newRow("InlineOpenPGP") << (int) Kleo::InlineOpenPGPFormat;
}

void CryptoComposerTest::testSignEncryptLateAttachments()
{
    QFETCH(int, format);
    Composer *composer = new Composer;
    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    AttachmentPart::Ptr attachment = AttachmentPart::Ptr(new AttachmentPart);
    attachment->setData("abc");
    attachment->setMimeType("x-some/x-type");
    attachment->setFileName(QString::fromLocal8Bit("anattachment.txt"));
    attachment->setEncrypted(false);
    attachment->setSigned(false);
    composer->addAttachmentPart(attachment);

    composer->setSignAndEncrypt(true, true);
    composer->setMessageCryptoFormat((Kleo::CryptoMessageFormat) format);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = Q_NULLPTR;

    // as we have an additional attachment, just ignore it when checking for sign/encrypt
    KMime::Content *b = MessageCore::NodeHelper::firstChild(message.data());
    ComposerTestUtil::verifySignatureAndEncryption(b, data.toUtf8(),
            (Kleo::CryptoMessageFormat) format, true);

    QCOMPARE(message->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(message->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));

    // now check the attachment separately
    QCOMPARE(QString::fromAscii(MessageCore::NodeHelper::nextSibling(b)->body()), QString::fromAscii("abc"));

}

void CryptoComposerTest::testBCCEncrypt_data()
{
    QTest::addColumn<int>("format");

    QTest::newRow("OpenPGPMime") << (int) Kleo::OpenPGPMIMEFormat;
    QTest::newRow("InlineOpenPGP") << (int) Kleo::InlineOpenPGPFormat;
}

// secondary recipients

void CryptoComposerTest::testBCCEncrypt()
{
    QFETCH(int, format);
    Composer *composer = new Composer;
    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    fillComposerData(composer, data);
    composer->infoPart()->setBcc(QStringList(QString::fromLatin1("bcc@bcc.org")));

    std::vector<GpgME::Key> keys = MessageComposer::Test::getKeys();

    QStringList primRecipients;
    primRecipients << QString::fromLocal8Bit("you@you.you");
    std::vector< GpgME::Key > pkeys;
    pkeys.push_back(keys[1]);

    QStringList secondRecipients;
    secondRecipients << QString::fromLocal8Bit("bcc@bcc.org");
    std::vector< GpgME::Key > skeys;
    skeys.push_back(keys[2]);

    QList<QPair<QStringList, std::vector<GpgME::Key> > > encKeys;
    encKeys.append(QPair<QStringList, std::vector<GpgME::Key> >(primRecipients, pkeys));
    encKeys.append(QPair<QStringList, std::vector<GpgME::Key> >(secondRecipients, skeys));

    composer->setSignAndEncrypt(true, true);
    composer->setMessageCryptoFormat((Kleo::CryptoMessageFormat) format);

    composer->setEncryptionKeys(encKeys);
    composer->setSigningKeys(keys);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 2);

    KMime::Message::Ptr primMessage = composer->resultMessages().first();
    KMime::Message::Ptr secMessage = composer->resultMessages()[1];
    delete composer;
    composer = Q_NULLPTR;

    ComposerTestUtil::verifySignatureAndEncryption(primMessage.data(), data.toUtf8(),
            (Kleo::CryptoMessageFormat) format);

    QCOMPARE(primMessage->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(primMessage->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));

    ComposerTestUtil::verifySignatureAndEncryption(secMessage.data(), data.toUtf8(),
            (Kleo::CryptoMessageFormat) format);

    QCOMPARE(secMessage->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(secMessage->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));

}

// inline pgp
void CryptoComposerTest::testOpenPGPInline_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    QTest::newRow("SignOpenPGPInline") << data << true << false << Headers::CE7Bit;
    QTest::newRow("EncryptOpenPGPInline") << data << false << true << Headers::CE7Bit;
    QTest::newRow("SignEncryptOpenPGPInline") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testOpenPGPInline()
{
    QFETCH(QString, data);
    QFETCH(bool, sign);
    QFETCH(bool, encrypt);
    QFETCH(Headers::contentEncoding, cte);

    Composer *composer = new Composer;

    fillComposerData(composer, data);
    fillComposerCryptoData(composer);

    composer->setSignAndEncrypt(sign, encrypt);
    composer->setMessageCryptoFormat(Kleo::InlineOpenPGPFormat);

    VERIFYEXEC(composer);
    QCOMPARE(composer->resultMessages().size(), 1);

    KMime::Message::Ptr message = composer->resultMessages().first();
    delete composer;
    composer = Q_NULLPTR;

    if (sign && !encrypt) {
        data += QString::fromLatin1("\n");
    }
    //qDebug() << "message:" << message->encodedContent();
    ComposerTestUtil::verify(sign, encrypt, message.data(), data.toUtf8(),
                             Kleo::InlineOpenPGPFormat, cte);

    QCOMPARE(message->from()->asUnicodeString(), QString::fromLocal8Bit("me@me.me"));
    QCOMPARE(message->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));
}

// s-mime

void CryptoComposerTest::testSMIME_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    QTest::newRow("SignSMIME") << data << true << false << Headers::CE7Bit;
    QTest::newRow("EncryptSMIME") << data << false << true << Headers::CE7Bit;
    QTest::newRow("SignEncryptSMIME") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testSMIME()
{
    QFETCH(bool, sign);
    QFETCH(bool, encrypt);

    runSMIMETest(sign, encrypt, false);
}

void CryptoComposerTest::testSMIMEOpaque_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    QTest::newRow("SignSMIMEOpaque") << data << true << false << Headers::CE7Bit;
    QTest::newRow("EncryptSMIMEOpaque") << data << false << true << Headers::CE7Bit;
    QTest::newRow("SignEncryptSMIMEOpaque") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testSMIMEOpaque()
{
    QFETCH(bool, sign);
    QFETCH(bool, encrypt);

    runSMIMETest(sign, encrypt, true);
}

// contentTransferEncoding

void CryptoComposerTest::testCTEquPr_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way. [ä]"));
    QTest::newRow("CTEquPr:Sign") << data << true << false << Headers::CEquPr;
    QTest::newRow("CTEquPr:Encrypt") << data << false << true << Headers::CE7Bit;
    QTest::newRow("CTEquPr:SignEncrypt") << data << true << true << Headers::CE7Bit;

    data = QStringLiteral("All happy families are alike;\n\n\n\neach unhappy family is unhappy in its own way.\n--\n hallloasdfasdfsadfsdf asdf sadfasdf sdf sdf sdf sadfasdf sdaf daf sdf asdf sadf asdf asdf [ä]");
    QTest::newRow("CTEquPr:Sign:Newline") << data << true << false << Headers::CEquPr;
    QTest::newRow("CTEquPr:Encrypt:Newline") << data << false << true << Headers::CE7Bit;
    QTest::newRow("CTEquPr:SignEncrypt:Newline") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testCTEquPr()
{
    testSMIME();
    testSMIMEOpaque();
    testOpenPGPMime();
    testOpenPGPInline();
}

void CryptoComposerTest::testCTEbase64_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("sign");
    QTest::addColumn<bool>("encrypt");
    QTest::addColumn<Headers::contentEncoding>("cte");

    QString data(QStringLiteral("[ääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääääää]"));
    QTest::newRow("CTEbase64:Sign") << data << true << false << Headers::CEbase64;
    QTest::newRow("CTEbase64:Encrypt") << data << false << true << Headers::CE7Bit;
    QTest::newRow("CTEbase64:SignEncrypt") << data << true << true << Headers::CE7Bit;
}

void CryptoComposerTest::testCTEbase64()
{
    testSMIME();
    testSMIMEOpaque();
    testOpenPGPMime();
    testOpenPGPInline();
}

// Helper methods
void CryptoComposerTest::fillComposerData(Composer *composer, QString data)
{
    composer->globalPart()->setFallbackCharsetEnabled(true);
    composer->infoPart()->setFrom(QString::fromLatin1("me@me.me"));
    composer->infoPart()->setTo(QStringList(QString::fromLatin1("you@you.you")));
    composer->textPart()->setWrappedPlainText(data);
}

void CryptoComposerTest::fillComposerCryptoData(Composer *composer)
{
    std::vector<GpgME::Key> keys = MessageComposer::Test::getKeys();

    qDebug() << "got num of keys:" << keys.size();

    QStringList recipients;
    recipients << QString::fromLocal8Bit("you@you.you");

    QList<QPair<QStringList, std::vector<GpgME::Key> > > encKeys;
    encKeys.append(QPair<QStringList, std::vector<GpgME::Key> >(recipients, keys));

    composer->setEncryptionKeys(encKeys);
    composer->setSigningKeys(keys);
}

void CryptoComposerTest::runSMIMETest(bool sign, bool enc, bool opaque)
{
    QFETCH(QString, data);
    QFETCH(Headers::contentEncoding, cte);

    Composer *composer = new Composer;

    fillComposerData(composer, data);
    composer->infoPart()->setFrom(QString::fromLatin1("test@example.com"));

    std::vector<GpgME::Key> keys = MessageComposer::Test::getKeys(true);
    QStringList recipients;
    recipients << QString::fromLocal8Bit("you@you.you");
    QList<QPair<QStringList, std::vector<GpgME::Key> > > encKeys;
    encKeys.append(QPair<QStringList, std::vector<GpgME::Key> >(recipients, keys));
    composer->setEncryptionKeys(encKeys);
    composer->setSigningKeys(keys);
    composer->setSignAndEncrypt(sign, enc);
    Kleo::CryptoMessageFormat f;
    if (opaque) {
        f = Kleo::SMIMEOpaqueFormat;
    } else {
        f = Kleo::SMIMEFormat;
    }
    composer->setMessageCryptoFormat(f);

    const bool result = composer->exec();
    //QEXPECT_FAIL("", "GPG setup problems", Continue);
    QVERIFY(result);
    if (result) {
        QCOMPARE(composer->resultMessages().size(), 1);
        KMime::Message::Ptr message = composer->resultMessages().first();
        delete composer;
        composer = Q_NULLPTR;

        //qDebug() << "message:" << message->encodedContent();

        ComposerTestUtil::verify(sign, enc, message.data(), data.toUtf8(),  f, cte);

        QCOMPARE(message->from()->asUnicodeString(), QString::fromLocal8Bit("test@example.com"));
        QCOMPARE(message->to()->asUnicodeString(), QString::fromLocal8Bit("you@you.you"));
    }
}

